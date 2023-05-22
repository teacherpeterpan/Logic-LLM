# parse the generated logic program
import os
import func_timeout
import re

class LogicProgram:
    def __init__(self, id, program_str, keywords = ['Query:', 'Rules:', 'Facts:', 'Predicates:']) -> None:
        self.raw_program_str = program_str
        self.sample_id = id
        self.keywords = keywords
        try:
            # parse the program string, return None if the program is invalid
            for keyword in keywords:
                program_str, segment_list = self.parse_program(program_str, keyword)
                setattr(self, keyword[:-1], segment_list)
        except:
            # print('Error parsing program.')
            for keyword in keywords:
                setattr(self, keyword[:-1], None)

    def parse_program(self, program_str, key_phrase):
        remain_program_str, segment = program_str.split(key_phrase)
        segment_list = segment.strip().split('\n')
        for i in range(len(segment_list)):
            segment_list[i] = segment_list[i].split(':::')[0].strip()
        return remain_program_str, segment_list

    def __str__(self) -> str:
        message = [f'{keyword[:-1]}: {getattr(self, keyword[:-1])} \n' for keyword in self.keywords]
        return ''.join(message)

# take a logic program and convert it to a Pyke program
class PykeInterpreter:
    def __init__(self, save_path) -> None:
        self.save_path = save_path

    def interpret_facts(self, facts):
        with open(os.path.join(self.sample_dir, 'facts.kfb'), 'w') as f:
            for fact in facts:
                # check for invalid facts
                if not fact.find('$x') >= 0:
                    f.write(fact + '\n')

    # example rule: Vumpuses($x, True) >>> Tumpuses($x, True)
    # def parse_forward_rule(self, f_index, rule):
    #     premise, conclusion = rule.split('>>>')
    #     premise = premise.strip()
    #     conclusion = conclusion.strip()
    #     pyke_rule = f'''fact{f_index}\n\tforeach\n\t\tfacts.{premise}\n\tassert\n\t\tfacts.{conclusion}'''
    #     return pyke_rule
    
    # example rule: Furry($x, True) && Quite($x, True) >>> White($x, True)
    def parse_forward_rule(self, f_index, rule):
        premise, conclusion = rule.split('>>>')
        premise = premise.strip()
        # split the premise into multiple facts if needed
        premise = premise.split('&&')
        premise_list = [p.strip() for p in premise]

        conclusion = conclusion.strip()
        # split the conclusion into multiple facts if needed
        conclusion = conclusion.split('&&')
        conclusion_list = [c.strip() for c in conclusion]

        # create the Pyke rule
        pyke_rule = f'''fact{f_index}\n\tforeach'''
        for p in premise_list:
            pyke_rule += f'''\n\t\tfacts.{p}'''
        pyke_rule += f'''\n\tassert'''
        for c in conclusion_list:
            pyke_rule += f'''\n\t\tfacts.{c}'''
        return pyke_rule

    def interpret_rules(self, rules):
        pyke_rules = []
        for idx, rule in enumerate(rules):
            pyke_rules.append(self.parse_forward_rule(idx + 1, rule))

        with open(os.path.join(self.sample_dir, 'rules.krb'), 'w') as f:
            f.write('\n\n'.join(pyke_rules))

    def interpret_program(self, logic_program : LogicProgram):
        # create the folder to save the Pyke program
        sample_dir = os.path.join(self.save_path, logic_program.sample_id)
        if not os.path.exists(sample_dir):
            os.makedirs(sample_dir)
        self.sample_dir = sample_dir
        
        try:
            self.interpret_facts(logic_program.Facts)
            self.interpret_rules(logic_program.Rules)
        except:
            return False
        return True

class FOLogicInterpreter:
    def __init__(self, timeout = 20) -> None:
        self.timeout = timeout

    def safe_execute(self, code_string: str, keys = None, debug_mode = False):
        def execute(x):
            try:
                exec(x)
                locals_ = locals()
                if keys is None:
                    return locals_.get('ans', None), "no error"
                else:
                    return [locals_.get(k, None) for k in keys], "no error"
            except Exception as e:
                if debug_mode:
                    print(e)
                return None, e
        try:
            ans, error_msg = func_timeout.func_timeout(self.timeout, execute, args=(code_string,))
        except func_timeout.FunctionTimedOut:
            ans = None
            error_msg = "timeout"

        return ans, error_msg

    # def check_and_fix_bug(self, expression):
    #     # Can't modify database with a query with free variables
    #     if expression.find('$x1') >= 0 and expression.find('Forall') < 0:
    #         expression = f'''Forall('$x1', {expression})'''
    #     return expression

    def execute_program(self, logic_program : LogicProgram, debug_mode = False):
        # parse the logic program into FOL python program
        python_program_list = ['from fol_engine.logic import *', 'kb = createResolutionKB()']
        # add facts
        for fact in logic_program.Facts:
            python_program_list.append(f'kb.tell({fact.strip()})')
        # ask query
        query = logic_program.Query[0]
        python_program_list.append(f'ans = kb.ask({query.strip()}).status')
        # execute the python program
        py_program_str = '\n'.join(python_program_list)
        if debug_mode:
            print(py_program_str)
        
        ans, err_msg = self.safe_execute(py_program_str, debug_mode=debug_mode)
        if ans is not None:
            ans = ans.strip()
        return ans, err_msg

class CSPInterpreter:
    def __init__(self, timeout = 20) -> None:
        self.timeout = timeout

    def safe_execute(self, code_string: str, keys = None, debug_mode = False):
        def execute(x):
            try:
                exec(x)
                locals_ = locals()
                if keys is None:
                    return locals_.get('ans', None), "no error"
                else:
                    return [locals_.get(k, None) for k in keys], "no error"
            except Exception as e:
                if debug_mode:
                    print(e)
                return None, e
        try:
            ans, error_msg = func_timeout.func_timeout(self.timeout, execute, args=(code_string,))
        except func_timeout.FunctionTimedOut:
            ans = None
            error_msg = "timeout"

        return ans, error_msg

    # comparison (>, <), fixed value (==, !=), etc
    def parse_numeric_constraint(self, constraint):
        # get all the variables in the rule from left to right
        pattern = r'\b[a-zA-Z_]+\b'  # Matches word characters (letters and underscores)
        variables_in_rule = re.findall(pattern, constraint)
        unique_list = []
        for item in variables_in_rule:
            if item not in unique_list:
                unique_list.append(item)
        str_variables_in_rule = ', '.join(unique_list)
        str_variables_in_rule_with_quotes = ', '.join([f'"{v}"' for v in unique_list]) + ','
        parsed_constraint = f"lambda {str_variables_in_rule}: {constraint}, ({str_variables_in_rule_with_quotes})"
        return parsed_constraint
    
    # all different constraint
    def parse_all_different_constraint(self, constraint):
        pattern = r'AllDifferentConstraint\(\[(.*?)\]\)'
        # Extract the content inside the parentheses
        result = re.search(pattern, constraint)
        if result:
            values_str = result.group(1)
            values = [value.strip() for value in values_str.split(',')]
        else:
            return None
        parsed_constraint = f"AllDifferentConstraint(), {str(values)}"
        return parsed_constraint

    def execute_program(self, logic_program : LogicProgram, debug_mode = False):
        # parse the logic program into CSP python program
        python_program_list = ['from constraint import *', 'problem = Problem()']
        # add variables
        for variable in logic_program.Variables:
            variable_name, variable_domain = variable.split('[IN]')
            variable_name, variable_domain = variable_name.strip(), variable_domain.strip()
            # variable_domain = ast.literal_eval(variable_domain)
            python_program_list.append(f'problem.addVariable("{variable_name}", {variable_domain})')
        
        # add constraints
        for rule in logic_program.Constraints:
            rule = rule.strip()
            parsed_constraint = None
            if rule.startswith('AllDifferentConstraint'):
                parsed_constraint = self.parse_all_different_constraint(rule)
            else:
                parsed_constraint = self.parse_numeric_constraint(rule)
            # create the constraint
            python_program_list.append(f'problem.addConstraint({parsed_constraint})')
        
        # solve the problem
        python_program_list.append(f'ans = problem.getSolutions()')
        # execute the python program
        py_program_str = '\n'.join(python_program_list)
        if debug_mode:
            print(py_program_str)
        
        ans, err_msg = self.safe_execute(py_program_str, debug_mode=debug_mode)
        return ans, err_msg