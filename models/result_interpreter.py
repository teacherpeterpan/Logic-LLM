from utils import *
import re
from collections import defaultdict

# Rule-based CSP result interpreter
class CSP_ResultInterpreter_RuleBased:
    def __init__(self):
        self.option_pattern = r'^\w+\)'
        self.expression_pattern = r'\w+ == \d+'       

    def interpret_result(self, query, execution_result):
        variable_ans_map = defaultdict(set)
        for result in execution_result:
            for variable, value in result.items():
                variable_ans_map[variable].add(value)

        for option_str in query:
            # Extract the option using regex
            option_match = re.match(self.option_pattern, option_str)
            option = option_match.group().replace(')', '')
            # Extract the expression using regex
            expression_match = re.search(self.expression_pattern, option_str)
            expression_str = expression_match.group()
            # Extract the variable and its value
            variable, value = expression_str.split('==')
            variable, value = variable.strip(), int(value.strip())
            # Check if the variable is in the execution result
            if len(variable_ans_map[variable]) == 1 and value in variable_ans_map[variable]:
                return option

        return None