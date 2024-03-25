"""Performs iterative self-refinement of logic programs.

Loads logic programs from a previous round. For programs that resulted in errors, generates a prompt to correct the program using a language model. Executes the revised programs to check if errors are resolved. Saves the revised programs after each round.
"""
# input: logic program file
# output: logic program file after one round of self-refinement

import json
import os
from tqdm import tqdm
from symbolic_solvers.fol_solver.prover9_solver import FOL_Prover9_Program
from symbolic_solvers.pyke_solver.pyke_solver import Pyke_Program
from symbolic_solvers.csp_solver.csp_solver import CSP_Program
from symbolic_solvers.z3_solver.sat_problem_solver import LSAT_Z3_Program
import argparse
import random
from backup_answer_generation import Backup_Answer_Generator
from utils import OpenAIModel, HuggingFaceModel, LLMClass

class SelfRefinementEngine:
    def __init__(self, args, current_round, llm_model = None):
        self.args = args
        self.split = args.split
        self.model_name = args.model_name
        self.dataset_name = args.dataset_name
        self.backup_strategy = args.backup_strategy
        self.framework_to_use = args.framework_to_use
        if llm_model is None:
            if self.framework_to_use == "OpenAI":
                self.llm_model = OpenAIModel(args.api_key, 'gpt-4', args.stop_words, args.max_new_tokens)
            elif self.framework_to_use == "HuggingFace":
                self.llm_model = HuggingFaceModel(model_id=self.model_name, stop_words = args.stop_words, max_new_tokens=args.max_new_tokens, is_AWQ=args.is_AWQ)
            else:
                self.llm_model = LLMClass()
        else:
            self.llm_model = llm_model

        self.model_name=self.model_name.replace("/","-")

        self.current_round = current_round

        self.logic_programs = self.load_logic_programs()
        # self.reasoning_results = self.load_inference_results()

        program_executor_map = {'FOLIO': FOL_Prover9_Program, 
                                'ProntoQA': Pyke_Program, 
                                'ProofWriter': Pyke_Program,
                                'LogicalDeduction': CSP_Program,
                                'AR-LSAT': LSAT_Z3_Program}
        self.program_executor = program_executor_map[self.dataset_name]
        self.backup_generator = Backup_Answer_Generator(args.mode, self.dataset_name, args.split, args.model_name, self.backup_strategy, self.args.backup_LLM_result_path)

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
        error_message = str(error_message).strip()
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
    
    def single_round_self_refinement(self, batch_size = 10):
        outputs = []
        error_logic_programs = []
        # separate into correct and incorrect logic programs
        for example in tqdm(self.logic_programs):
            logic_program = example['raw_logic_programs'][0].strip()
            answer, status, error_message = self.safe_execute_program(example['id'], logic_program)

            if status == 'execution error':
                if not error_message == 'No Output': # this is not execution error, but parsing error
                    # perform self-correction based on the error message
                    error_logic_programs.append(example)
                else:
                    outputs.append(example)
            elif status == 'parsing error':
                # perform self-correction based on the error message
                error_logic_programs.append(example)
            else:
                outputs.append(example)

        error_chunks = [error_logic_programs[i:i + batch_size] for i in range(0, len(error_logic_programs), batch_size)]
        for chunk in tqdm(error_chunks):
            full_prompts = []
            error_logic_programs = []
            for example in chunk:
                logic_program = example['raw_logic_programs'][0].strip()
                answer, status, error_message = self.safe_execute_program(example['id'], logic_program)
                # perform self-correction based on the error message
                if status == 'execution error':
                    full_prompts.append(self.load_prompt(logic_program, error_message))
                elif status == 'parsing error':
                    full_prompts.append(self.load_prompt(logic_program, 'Parsing Error'))

            try:
                batch_outputs = self.llm_model.batch_generate(full_prompts)
                # create output
                for sample, output in zip(chunk, batch_outputs):
                    programs = [output]
                    output = {'id': sample['id'], 
                            'context': sample['context'],
                            'question': sample['question'], 
                            'answer': sample['answer'],
                            'options': sample['options'],
                            'raw_logic_programs': programs}
                    outputs.append(output)
            except:
                # generate one by one if batch generation fails
                for sample, full_prompt in zip(chunk, full_prompts):
                    try:
                        output = self.llm_model.generate(full_prompt)
                        programs = [output]
                        output = {'id': sample['id'], 
                                'context': sample['context'],
                                'question': sample['question'], 
                                'answer': sample['answer'],
                                'options': sample['options'],
                                'raw_logic_programs': programs}
                        outputs.append(output)
                    except Exception as e:
                        print(e)
                        print('Error in generating logic programs for example: ', sample['id'])

        # remove examples with duplicate ids from the result
        outputs = list({output['id']: output for output in outputs}.values())
        print(f"Generated {len(outputs)} examples.")

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
    parser.add_argument('--backup_LLM_result_path', type=str, default='./outputs/results')
    parser.add_argument('--model_name', type=str, default='text-davinci-003')
    parser.add_argument('--framework_to_use', type=str, default='HuggingFace')
    parser.add_argument('--timeout', type=int, default=60)
    parser.add_argument('--api_key', type=str, default='KEY')
    parser.add_argument('--stop_words', type=str, default='------')
    parser.add_argument('--max_new_tokens', type=int, default=1024)
    parser.add_argument('--is_AWQ', type=str, default="auto")
    parser.add_argument('--mode', type=str, default='CoT')
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    args = parse_args()
    engine = SelfRefinementEngine(args, 1)
    for round in range(1, args.maximum_rounds+1):
        print(f"Round {round} self-refinement")
        engine.single_round_self_refinement()
        engine.current_round += 1