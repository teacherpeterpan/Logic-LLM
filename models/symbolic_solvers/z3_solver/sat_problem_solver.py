from collections import OrderedDict
from .code_translator import *
import subprocess
from subprocess import check_output
from os.path import join
import os

class LSAT_Z3_Program:
    def __init__(self, logic_program:str, dataset_name:str) -> None:
        self.logic_program = logic_program
        try:
            self.parse_logic_program()
            self.standard_code = self.to_standard_code()
        except Exception as e:
            self.standard_code = None
            self.flag = False
            return
        
        self.flag = True
        self.dataset_name = dataset_name

        # create the folder to save the Pyke program
        cache_dir = os.path.join(os.path.dirname(__file__), '.cache_program')
        if not os.path.exists(cache_dir):
            os.makedirs(cache_dir)
        self.cache_dir = cache_dir

    def parse_logic_program(self):
        # split the logic program into different parts
        lines = [x for x in self.logic_program.splitlines() if not x.strip() == ""]

        decleration_start_index = lines.index("# Declarations")
        constraint_start_index = lines.index("# Constraints")
        option_start_index = lines.index("# Options")
 
        declaration_statements = lines[decleration_start_index + 1:constraint_start_index]
        constraint_statements = lines[constraint_start_index + 1:option_start_index]
        option_statements = lines[option_start_index + 1:]

        try:
            (self.declared_enum_sorts, self.declared_int_sorts, self.declared_lists, self.declared_functions, self.variable_constrants) = self.parse_declaration_statements(declaration_statements)

            self.constraints = [x.split(':::')[0].strip() for x in constraint_statements]
            self.options = [x.split(':::')[0].strip() for x in option_statements if not x.startswith("Question :::")]
        except Exception as e:
            return False
        
        return True

    def __repr__(self):
        return f"LSATSatProblem:\n\tDeclared Enum Sorts: {self.declared_enum_sorts}\n\tDeclared Lists: {self.declared_lists}\n\tDeclared Functions: {self.declared_functions}\n\tConstraints: {self.constraints}\n\tOptions: {self.options}"

    def parse_declaration_statements(self, declaration_statements):
        enum_sort_declarations = OrderedDict()
        int_sort_declarations = OrderedDict()
        function_declarations = OrderedDict()
        pure_declaration_statements = [x for x in declaration_statements if "Sort" in x or "Function" in x]
        variable_constrant_statements = [x for x in declaration_statements if not "Sort" in x and not "Function" in x]
        for s in pure_declaration_statements:
            if "EnumSort" in s:
                sort_name = s.split("=")[0].strip()
                sort_member_str = s.split("=")[1].strip()[len("EnumSort("):-1]
                sort_members = [x.strip() for x in sort_member_str[1:-1].split(",")]
                enum_sort_declarations[sort_name] = sort_members
            elif "IntSort" in s:
                sort_name = s.split("=")[0].strip()
                sort_member_str = s.split("=")[1].strip()[len("IntSort("):-1]
                sort_members = [x.strip() for x in sort_member_str[1:-1].split(",")]
                int_sort_declarations[sort_name] = sort_members
            elif "Function" in s:
                function_name = s.split("=")[0].strip()
                if "->" in s and "[" not in s:
                    function_args_str = s.split("=")[1].strip()[len("Function("):]
                    function_args_str = function_args_str.replace("->", ",").replace("(", "").replace(")", "")
                    function_args = [x.strip() for x in function_args_str.split(",")]
                    function_declarations[function_name] = function_args
                elif "->" in s and "[" in s:
                    function_args_str = s.split("=")[1].strip()[len("Function("):-1]
                    function_args_str = function_args_str.replace("->", ",").replace("[", "").replace("]", "")
                    function_args = [x.strip() for x in function_args_str.split(",")]
                    function_declarations[function_name] = function_args
                else:
                    # legacy way
                    function_args_str = s.split("=")[1].strip()[len("Function("):-1]
                    function_args = [x.strip() for x in function_args_str.split(",")]
                    function_declarations[function_name] = function_args
            else:
                raise RuntimeError("Unknown declaration statement: {}".format(s))

        declared_enum_sorts = OrderedDict()
        declared_lists = OrderedDict()
        self.declared_int_lists = OrderedDict()

        declared_functions = function_declarations
        already_declared = set()
        for name, members in enum_sort_declarations.items():
            # all contained by other enum sorts
            if all([x not in already_declared for x in members]):
                declared_enum_sorts[name] = members
                already_declared.update(members)
            declared_lists[name] = members

        for name, members in int_sort_declarations.items():
            self.declared_int_lists[name] = members
            # declared_lists[name] = members

        return declared_enum_sorts, int_sort_declarations, declared_lists, declared_functions, variable_constrant_statements
    
    def to_standard_code(self):
        declaration_lines = []
        # translate enum sorts
        for name, members in self.declared_enum_sorts.items():
            declaration_lines += CodeTranslator.translate_enum_sort_declaration(name, members)

        # translate int sorts
        for name, members in self.declared_int_sorts.items():
            declaration_lines += CodeTranslator.translate_int_sort_declaration(name, members)

        # translate lists
        for name, members in self.declared_lists.items():
            declaration_lines += CodeTranslator.translate_list_declaration(name, members)

        scoped_list_to_type = {}
        for name, members in self.declared_lists.items():
            if all(x.isdigit() for x in members):
                scoped_list_to_type[name] = CodeTranslator.ListValType.INT
            else:
                scoped_list_to_type[name] = CodeTranslator.ListValType.ENUM

        for name, members in self.declared_int_lists.items():
            scoped_list_to_type[name] = CodeTranslator.ListValType.INT
        
        # translate functions
        for name, args in self.declared_functions.items():
            declaration_lines += CodeTranslator.translate_function_declaration(name, args)

        pre_condidtion_lines = []

        for constraint in self.constraints:
            pre_condidtion_lines += CodeTranslator.translate_constraint(constraint, scoped_list_to_type)

        # additional function scope control
        for name, args in self.declared_functions.items():
            if args[-1] in scoped_list_to_type and scoped_list_to_type[args[-1]] == CodeTranslator.ListValType.INT:
                # FIX
                if args[-1] in self.declared_int_lists:
                    continue
                
                list_range = [int(x) for x in self.declared_lists[args[-1]]]
                assert list_range[-1] - list_range[0] == len(list_range) - 1
                scoped_vars = [x[0] + str(i) for i, x in enumerate(args[:-1])]
                func_call = f"{name}({', '.join(scoped_vars)})"

                additional_cons = "ForAll([{}], And({} <= {}, {} <= {}))".format(
                    ", ".join([f"{a}:{b}" for a, b in zip(scoped_vars, args[:-1])]),
                    list_range[0], func_call, func_call, list_range[-1]
                )
                pre_condidtion_lines += CodeTranslator.translate_constraint(additional_cons, scoped_list_to_type)


        for constraint in self.constraints:
            pre_condidtion_lines += CodeTranslator.translate_constraint(constraint, scoped_list_to_type)

        # each block should express one option
        option_blocks = [CodeTranslator.translate_constraint(option, scoped_list_to_type) for option in self.options]

        return CodeTranslator.assemble_standard_code(declaration_lines, pre_condidtion_lines, option_blocks)
    
    def execute_program(self):
        filename = join(self.cache_dir, f'tmp.py')
        with open(filename, "w") as f:
            f.write(self.standard_code)
        try:
            output = check_output(["python", filename], stderr=subprocess.STDOUT, timeout=1.0)
        except subprocess.CalledProcessError as e:
            outputs = e.output.decode("utf-8").strip().splitlines()[-1]
            return None, outputs
        except subprocess.TimeoutExpired:
            return None, 'TimeoutError'
        output = output.decode("utf-8").strip()
        result = output.splitlines()
        if len(result) == 0:
            return None, 'No Output'
        
        return result, ""
    
    def answer_mapping(self, answer):
        mapping = {'(A)': 'A', '(B)': 'B', '(C)': 'C', '(D)': 'D', '(E)': 'E',
                   'A': 'A', 'B': 'B', 'C': 'C', 'D': 'D', 'E': 'E'}
        return mapping[answer[0].strip()]

if __name__=="__main__":
    logic_program = '''# Declarations
people = EnumSort([Vladimir, Wendy])
meals = EnumSort([breakfast, lunch, dinner, snack])
foods = EnumSort([fish, hot_cakes, macaroni, omelet, poached_eggs])
eats = Function([people, meals] -> [foods])

# Constraints
ForAll([m:meals], eats(Vladimir, m) != eats(Wendy, m)) ::: At no meal does Vladimir eat the same kind of food as Wendy
ForAll([p:people, f:foods], Count([m:meals], eats(p, m) == f) <= 1) ::: Neither of them eats the same kind of food more than once during the day
ForAll([p:people], Or(eats(p, breakfast) == hot_cakes, eats(p, breakfast) == poached_eggs, eats(p, breakfast) == omelet)) ::: For breakfast, each eats exactly one of the following: hot cakes, poached eggs, or omelet
ForAll([p:people], Or(eats(p, lunch) == fish, eats(p, lunch) == hot_cakes, eats(p, lunch) == macaroni, eats(p, lunch) == omelet)) ::: For lunch, each eats exactly one of the following: fish, hot cakes, macaroni, or omelet
ForAll([p:people], Or(eats(p, dinner) == fish, eats(p, dinner) == hot_cakes, eats(p, dinner) == macaroni, eats(p, dinner) == omelet)) ::: For dinner, each eats exactly one of the following: fish, hot cakes, macaroni, or omelet
ForAll([p:people], Or(eats(p, snack) == fish, eats(p, snack) == omelet)) ::: For a snack, each eats exactly one of the following: fish or omelet
eats(Wendy, lunch) == omelet ::: Wendy eats an omelet for lunch

# Options
Question ::: Vladimir must eat which one of the following foods?
is_valid(Exists([m:meals], eats(Vladimir, m) == fish)) ::: (A)
is_valid(Exists([m:meals], eats(Vladimir, m) == hot_cakes)) ::: (B)
is_valid(Exists([m:meals], eats(Vladimir, m) == macaroni)) ::: (C)
is_valid(Exists([m:meals], eats(Vladimir, m) == omelet)) ::: (D)
is_valid(Exists([m:meals], eats(Vladimir, m) == poached_eggs)) ::: (E)'''

    z3_program = LSAT_Z3_Program(logic_program, 'AR-LSAT')
    print(z3_program.standard_code)

    output, error_message = z3_program.execute_program()
    print(output)
    print(z3_program.answer_mapping(output))