# input: logic program file
# output: logic program file after one round of self-refinement

import json
import os
from tqdm import tqdm
from symbolic_solvers.z3_solver.sat_problem_solver import LSAT_Z3_Program
from symbolic_solvers.fol_solver.prover9_solver import FOL_Prover9_Program
import argparse
import random
from backup_answer_generation import Backup_Answer_Generator
from utils import OpenAIModel

class SelfRefinementEngine:
    def __init__(self, args, current_round):
        self.args = args
        self.split = args.split
        self.model_name = args.model_name
        self.dataset_name = args.dataset_name
        self.backup_strategy = args.backup_strategy
        self.openai_api = OpenAIModel(args.api_key, 'gpt-4', args.stop_words, args.max_new_tokens)
        self.current_round = current_round

        self.logic_programs = self.load_logic_programs()
        # self.reasoning_results = self.load_inference_results()

        program_executor_map = {'AR-LSAT': LSAT_Z3_Program,
                                'FOLIO': FOL_Prover9_Program}
        self.program_executor = program_executor_map[self.dataset_name]
        self.backup_generator = Backup_Answer_Generator(self.dataset_name, self.backup_strategy, self.args.backup_LLM_result_path)

    def load_logic_programs(self):
        prefix = ""
        if self.current_round > 1:
            prefix = f'self-refine-{self.current_round-1}_'
        with open(os.path.join('./outputs/logic_programs', f'{prefix}{self.dataset_name}_{self.split}_{self.model_name}.json')) as f:
            dataset = json.load(f)
        print(f"Loaded {len(dataset)} examples from {self.split} split.")
        return dataset
    
    def load_prompt(self, program, error_message):
        program = program.strip()
        error_message = error_message.strip()
        with open(f'./models/prompts/self-correct-{self.dataset_name}.txt', 'r') as f:
            prompt_template = f.read()
        full_prompt = prompt_template.replace('[[PROGRAM]]', program).replace('[[ERROR MESSAGE]]', error_message)
        return full_prompt

    def safe_execute_program(self, id, logic_program, debug = False):
        program = self.program_executor(logic_program, self.dataset_name)
        # cannot parse the program
        if program.flag == False:
            answer = self.backup_generator.get_backup_answer(id)
            return answer, 'parsing error', ''
        # execuate the program
        answer, error_message = program.execute_program()
        # not executable
        if answer is None:
            answer = self.backup_generator.get_backup_answer(id)

            ## output debug info
            if debug == True:
                if not os.path.exists('./debug'):
                    os.makedirs('./debug')
                with open(f'./debug/{id}.py', 'w') as f:
                    f.write(program.standard_code)
                with open(f'./debug/{id}.program.txt', 'w') as f:
                    f.write(logic_program)
                    f.write('\n')
                    f.write(error_message)

            return answer, 'execution error', error_message
        # successfully executed
        answer = program.answer_mapping(answer)
        return answer, 'success', error_message
    
    def single_round_self_refinement(self):
        outputs = []
        for example in tqdm(self.logic_programs):
            logic_program = example['raw_logic_programs'][0].strip()
            answer, status, error_message = self.safe_execute_program(example['id'], logic_program)

            if status == 'execution error':
                if not error_message == 'No Output': # this is not execution error, but parsing error
                    # perform self-correction based on the error message
                    full_prompt = self.load_prompt(logic_program, error_message)
                    revised_program = self.openai_api.generate(full_prompt).strip()
                    programs = [revised_program]
                    output = {'id': example['id'], 
                            'context': example['context'],
                            'question': example['question'], 
                            'answer': example['answer'],
                            'options': example['options'],
                            'raw_logic_programs': programs}
                    outputs.append(output)
                else:
                    outputs.append(example)
            elif status == 'parsing error':
                # perform self-correction based on the error message
                full_prompt = self.load_prompt(logic_program, 'Parsing Error')
                revised_program = self.openai_api.generate(full_prompt).strip()
                programs = [revised_program]
                output = {'id': example['id'], 
                        'context': example['context'],
                        'question': example['question'], 
                        'answer': example['answer'],
                        'options': example['options'],
                        'raw_logic_programs': programs}
                outputs.append(output)
            else:
                outputs.append(example)

        # save results
        if not os.path.exists('./outputs/logic_programs'):
            os.makedirs('./outputs/logic_programs')

        # save outputs
        save_path = f'./outputs/logic_programs/self-refine-{self.current_round}_{self.dataset_name}_{self.split}_{self.model_name}.json'
        with open(save_path, 'w') as f:
            json.dump(outputs, f, indent=2, ensure_ascii=False)
    
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--maximum_rounds', type=int, default=3)
    parser.add_argument('--dataset_name', type=str)
    parser.add_argument('--split', type=str, default='dev')
    parser.add_argument('--backup_strategy', type=str, default='random', choices=['random', 'LLM'])
    parser.add_argument('--backup_LLM_result_path', type=str, default='../baselines/results')
    parser.add_argument('--model_name', type=str, default='text-davinci-003')
    parser.add_argument('--timeout', type=int, default=60)
    parser.add_argument('--api_key', type=str)
    parser.add_argument('--stop_words', type=str, default='------')
    parser.add_argument('--max_new_tokens', type=int, default=1024)
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    args = parse_args()
    for round in range(1, args.maximum_rounds+1):
        print(f"Round {round} self-refinement")
        engine = SelfRefinementEngine(args, round)
        engine.single_round_self_refinement()