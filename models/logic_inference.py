import json
import os
from tqdm import tqdm
from program_parser import PykeInterpreter, FOLogicInterpreter, CSPInterpreter, LogicProgram
import argparse
from pyke import knowledge_engine
import random
import re
from self_debug import SelfDebugger
from result_interpreter import CSP_ResultInterpreter_RuleBased

class LogicInferenceEngine:
    def __init__(self, args):
        self.args = args
        self.dataset_name = args.dataset_name
        self.split = args.split
        self.interpreter = args.interpreter
        self.model_name = args.model_name
        self.save_path = args.save_path
        self.self_debug = args.self_debug
        self.backup_strategy = args.backup_strategy
        self.dataset = self.load_logic_programs()
        keywords = []
        if self.interpreter == 'pyke':
            keywords = ['Query:', 'Rules:', 'Facts:', 'Predicates:']
        elif self.interpreter == 'fol':
            keywords = ['Query:', 'Facts:']
        elif self.interpreter == 'csp':
            keywords = ['Query:', 'Constraints:', 'Variables:', 'Domain:']

        self.generate_backup_answer = {'random': self.random_answer, 'LLM': self.LLM_answer}[self.backup_strategy]
        if self.backup_strategy == 'LLM':
            with open(os.path.join(args.backup_LLM_result_path, f'Direct_{self.dataset_name}_{self.split}_{self.model_name}.json'), 'r') as f:
                LLM_result = json.load(f)
            self.backup_results = {sample['id'] : sample['predicted_answer'] for sample in LLM_result}

        # parse logic programs
        self.logic_programs = []
        self.logic_program_lookup = {}
        for example in self.dataset:
            program = LogicProgram(example['id'], example['raw_logic_programs'][0], keywords)
            self.logic_programs.append(program)
            self.logic_program_lookup[example['id']] = program

    def load_logic_programs(self):
        with open(os.path.join('./logic_programs', self.interpreter ,f'{self.dataset_name}_{self.split}_{self.model_name}.json')) as f:
            dataset = json.load(f)
        return dataset

    # generate random answer for unexecutable programs
    def random_answer(self, ID):
        if self.dataset_name == 'ProntoQA':
            return random.choice(['A', 'B'])
        elif self.dataset_name == 'ProofWriter' or self.dataset_name == 'FOLIO':
            return random.choice(['A', 'B', 'C'])
        else:
            raise ValueError(f'Invalid dataset name: {self.dataset_name}')

    # use LLM to generate answer for unexecutable programs
    def LLM_answer(self, ID):
        return self.backup_results[ID]

    def inference_on_dataset(self):
        pass

    def save_results(self, outputs):
        self_debug_flag = 'self-debug_' if self.self_debug else ''
        if not os.path.exists(os.path.join(self.save_path, self.interpreter)):
            os.makedirs(os.path.join(self.save_path, self.interpreter))
        
        with open(os.path.join(self.save_path, self.interpreter, f'{self_debug_flag}{self.dataset_name}_{self.split}_{self.model_name}_backup-{self.backup_strategy}.json'), 'w') as f:
            json.dump(outputs, f, indent=2, ensure_ascii=False)

class PykeLogicInferenceEngine(LogicInferenceEngine):
    def __init__(self, args):
        super().__init__(args)
        self.pyke_data_path = args.pyke_data_path
        self.pyke_interpreter = PykeInterpreter(os.path.join(self.pyke_data_path, args.dataset_name))
        self.prepare_pyke_inference_files()
        self.answer_map = {'ProntoQA': self.answer_map_prontoqa, 
                           'ProofWriter': self.answer_map_proofwriter}

    def answer_map_prontoqa(self, result, value_to_check):
        if result == value_to_check:
            return 'A'
        else:
            return 'B'

    def answer_map_proofwriter(self, result, value_to_check):
        if result is None:
            return 'C'
        elif result == value_to_check:
            return 'A'
        else:
            return 'B'

    def prepare_pyke_inference_files(self):
        unparseable_programs_ids = []
        for program in tqdm(self.logic_programs):
            if not program.Query is None:
                flag = self.pyke_interpreter.interpret_program(program)
                if not flag:
                    unparseable_programs_ids.append(program.sample_id)
                    print(f'Unable to parse into pyke program for {program.sample_id}.')
            else:
                unparseable_programs_ids.append(program.sample_id)
                print(f'Incorrect logic program for {program.sample_id}.')
        
        print(f'Unparseable programs: {len(unparseable_programs_ids)}')
        self.unparseable_programs_ids = unparseable_programs_ids

    '''
    for example: Is Marvin from Mars?
    Query: FromMars(Marvin, $label)
    '''
    def check_specific_predicate(self, subject_name, predicate_name, engine):
        results = []
        with engine.prove_goal(f'facts.{predicate_name}({subject_name}, $label)') as gen:
            for vars, plan in gen:
                results.append(vars['label'])

        with engine.prove_goal(f'rules.{predicate_name}({subject_name}, $label)') as gen:
            for vars, plan in gen:
                results.append(vars['label'])

        if len(results) == 1:
            return results[0]
        elif len(results) == 2:
            return results[0] and results[1]
        elif len(results) == 0:
            return None

    '''
    Input Example: Metallic(Wren, False)
    '''
    def parse_query(self, query):
        pattern = r'(\w+)\(([^,]+),\s*([^)]+)\)'
        match = re.match(pattern, query)
        if match:
            function_name = match.group(1)
            arg1 = match.group(2)
            arg2 = match.group(3)
            arg2 = True if arg2 == 'True' else False
            return function_name, arg1, arg2
        else:
            raise ValueError(f'Invalid query: {query}')

    '''
    Inference on a single logic program
    '''
    def inference(self, logic_program, ID):
        # delete the compiled_krb dir
        if os.path.exists('./compiled_krb'):
            print('removing compiled_krb')
            os.system(f'rm -rf ./compiled_krb/*')

        engine = knowledge_engine.engine(os.path.join(self.pyke_data_path, self.dataset_name, ID))
        engine.activate('rules')
        engine.get_kb('facts')

        # parse the logic query into pyke query
        predicate, subject, value_to_check = self.parse_query(logic_program.Query[0])
        result = self.check_specific_predicate(subject, predicate, engine)
        answer = self.answer_map[self.dataset_name](result, value_to_check)
        return answer
    
    def inference_on_dataset(self):
        outputs = []
        error_count = 0
        for example in self.dataset:
            if example['id'] in self.unparseable_programs_ids:
                output = {
                    'id': example['id'],
                    'question': example['question'],
                    'answer': example['answer'],
                    'predicted_answer': self.generate_backup_answer(example['id'])
                }
                outputs.append(output)
            else:
                logic_program = self.logic_program_lookup[example['id']]
                try:
                    result = self.inference(logic_program, example['id'])

                    # create output
                    output = {
                        'id': example['id'],
                        'question': example['question'],
                        'answer': example['answer'],
                        'predicted_answer': result
                    }
                    outputs.append(output)
                except Exception as e:
                    print(f'Execution error in inference for {example["id"]}')
                    output = {
                        'id': example['id'],
                        'question': example['question'],
                        'answer': example['answer'],
                        'predicted_answer': self.generate_backup_answer(example['id'])
                    }
                    error_count += 1
                    outputs.append(output)
        
        print(f'Execution error count: {error_count}')
        self.save_results(outputs)

class FOLLogicInferenceEngine(LogicInferenceEngine):
    def __init__(self, args):
        super().__init__(args)
        self.fol_interpreter = FOLogicInterpreter(args.timeout)
        self.answer_map = {'ProntoQA': self.answer_map_prontoqa, 
                           'ProofWriter': self.answer_map_proofwriter,
                           'FOLIO': self.answer_map_proofwriter}
        if self.self_debug == True:
            self.debugger = SelfDebugger(self.model_name, 'fol', args.debug_api_key, 1024, ['------'])

    def answer_map_prontoqa(self, result):
        answer_map = {'ENTAILMENT': 'A', 'CONTRADICTION': 'B', 'CONTINGENT': 'B'}
        return answer_map[result]

    def answer_map_proofwriter(self, result):
        answer_map = {'ENTAILMENT': 'A', 'CONTRADICTION': 'B', 'CONTINGENT': 'C'}
        return answer_map[result]
    
    ## multi-round debugging
    def inference(self, logic_program, ID):
        ans, err_msg = self.fol_interpreter.execute_program(logic_program)
        # success at first attempt
        if not ans is None:
            return self.answer_map[self.dataset_name](ans), 'debug-0', ""
        
        # fail at first attempt, but it is due to timeout or the self debug is closed
        if self.self_debug == False or err_msg == "timeout":
            return self.generate_backup_answer(ID), 'llm', ""
        
        # run self debug for MAX_ATTEMPTS
        MAX_ATTEMPTS = 3
        current_program_str = logic_program.raw_program_str
        for idx in range(MAX_ATTEMPTS):
            # we need to debug the program and try it again
            current_program_str = self.debugger.debug(current_program_str, err_msg)
            keywords = ['Query:', 'Facts:']
            current_program = LogicProgram(ID, current_program_str, keywords)
            ans, err_msg = self.fol_interpreter.execute_program(current_program)
            if not ans is None:
                return self.answer_map[self.dataset_name](ans), f'debug-{idx+1}', current_program_str
            if err_msg == "timeout":
                break

        # if we still cannot get an answer after MAX_ATTEMPTS
        return self.generate_backup_answer(ID), 'llm', current_program_str

    def inference_on_dataset(self):
        outputs = []
        error_samples = []
        for example in tqdm(self.dataset):
            logic_program = self.logic_program_lookup[example['id']]
            result, flag, corrected_logic_program = self.inference(logic_program, example['id'])

            if flag == 'llm':
                error_samples.append(example['id'])
                # print(f'Execution error in inference for {example["id"]}')

            # create output
            output = {
                'id': example['id'],
                'question': example['question'],
                'answer': example['answer'],
                'predicted_answer': result,
                'corrected_logic_program': corrected_logic_program,
                'flag': flag
            }
            outputs.append(output)
        
        print(f'Execution error count: {len(error_samples)}')
        print(f'Error samples: {error_samples}')
        self.save_results(outputs)

class CSPLogicInferenceEngine(LogicInferenceEngine):
    def __init__(self, args):
        super().__init__(args)
        self.csp_interpreter = CSPInterpreter(args.timeout)
        self.result_interpreter = CSP_ResultInterpreter_RuleBased()

    def inference(self, example, logic_program):
        ID = example['id']
        csp_ans, err_msg = self.csp_interpreter.execute_program(logic_program)
        current_program_str = logic_program.raw_program_str
        # success at first attempt
        if not csp_ans is None:
            if len(csp_ans) == 0:
                return self.generate_backup_answer(ID), [], 'debug-0', current_program_str

            # ans = self.result_interpreter.interpret_result(example, logic_program.Domain, csp_ans)
            ans = self.result_interpreter.interpret_result(logic_program.Query, csp_ans)
            
            # backoff to the first answer if we cannot get an answer
            if ans is None and len(csp_ans) > 1:
                ans = self.result_interpreter.interpret_result(logic_program.Query, [csp_ans[0]])
            
            # backoff to the backup answer if we still cannot get an answer
            if ans is None:
                return self.generate_backup_answer(ID), None, 'llm', current_program_str
            
            return ans, csp_ans, 'debug-0', ""

        # if we still cannot get an answer after MAX_ATTEMPTS
        return self.generate_backup_answer(ID), None, 'llm', current_program_str
    
    def inference_on_dataset(self):
        outputs = []
        error_samples = []
        for example in tqdm(self.dataset):
            logic_program = self.logic_program_lookup[example['id']]
            result, csp_result, flag, corrected_logic_program = self.inference(example, logic_program)

            if flag == 'llm':
                error_samples.append(example['id'])

            # create output
            output = {
                'id': example['id'],
                'question': example['question'],
                'answer': example['answer'],
                'predicted_answer': result,
                'csp_result': csp_result,
                'corrected_logic_program': corrected_logic_program,
                'flag': flag
            }
            outputs.append(output)
        
        print(f'Execution error count: {len(error_samples)}')
        print(f'Error samples: {error_samples}')
        self.save_results(outputs)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--pyke_data_path', type=str, default='./pyke_programs')
    parser.add_argument('--dataset_name', type=str)
    parser.add_argument('--split', type=str, default='dev')
    parser.add_argument('--interpreter', type=str, choices=['pyke', 'fol', 'csp'])
    parser.add_argument('--save_path', type=str, default='./results')
    parser.add_argument('--backup_strategy', type=str, default='random', choices=['random', 'LLM'])
    parser.add_argument('--backup_LLM_result_path', type=str, default='../baselines/results')
    parser.add_argument('--model_name', type=str, default='text-davinci-003')
    parser.add_argument('--self_debug', action='store_true')
    parser.add_argument('--debug_api_key', type=str)
    parser.add_argument('--timeout', type=int, default=60)
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = parse_args()
    if args.interpreter == 'pyke':
        inference_engine = PykeLogicInferenceEngine(args)
        inference_engine.inference_on_dataset()
        os.system(f'rm -rf ./compiled_krb')
    elif args.interpreter == 'fol':
        inference_engine = FOLLogicInferenceEngine(args)
        inference_engine.inference_on_dataset()
    elif args.interpreter == 'csp':
        inference_engine = CSPLogicInferenceEngine(args)
        inference_engine.inference_on_dataset()
    else:
        raise ValueError(f'Invalid interpreter: {args.interpreter}')