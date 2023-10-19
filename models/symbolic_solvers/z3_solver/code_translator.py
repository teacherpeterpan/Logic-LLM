from collections import OrderedDict, namedtuple
from enum import Enum
import re

TAB_STR = "    "
CHOICE_INDEXES = ["(A)", "(B)", "(C)", "(D)", "(E)"]

class CodeTranslator:
    class LineType(Enum):
        DECL = 1
        CONS = 2

    class ListValType(Enum):
        INT = 1
        ENUM = 2

    StdCodeLine = namedtuple("StdCodeLine", "line line_type")

    @staticmethod
    def translate_enum_sort_declaration(enum_sort_name, enum_sort_values):
        if all([x.isdigit() for x in enum_sort_values]):
            return [CodeTranslator.StdCodeLine(f"{enum_sort_name}_sort = IntSort()", CodeTranslator.LineType.DECL)]

        line = "{}, ({}) = EnumSort({}, [{}])".format(
            f"{enum_sort_name}_sort",
            ", ".join(enum_sort_values),
            f"'{enum_sort_name}'",
            ", ".join([f"'{x}'" for x in enum_sort_values])
        )
        return [CodeTranslator.StdCodeLine(line, CodeTranslator.LineType.DECL)]

    @staticmethod
    def translate_int_sort_declaration(int_sort_name, int_sort_values):
        line1 = CodeTranslator.StdCodeLine(f"{int_sort_name}_sort = IntSort()", CodeTranslator.LineType.DECL)
        line2 = "{} = Ints('{}')".format(
            ", ".join([str(x) for x in int_sort_values]),
            " ".join([str(x) for x in int_sort_values])
        )
        line2 = CodeTranslator.StdCodeLine(line2, CodeTranslator.LineType.DECL)

        line3 = "{} = [{}]".format(
            int_sort_name,
            ", ".join([str(x) for x in int_sort_values])
        )
        line3 = CodeTranslator.StdCodeLine(line3, CodeTranslator.LineType.DECL)
        return [line1, line2, line3]
    


    @staticmethod
    def translate_list_declaration(list_name, list_mebmers):
        line = "{} = [{}]".format(
            list_name,
            ", ".join(list_mebmers),
        )
        return [CodeTranslator.StdCodeLine(line, CodeTranslator.LineType.DECL)]

    @staticmethod
    def type_str_to_type_sort(arg):
        if arg == "bool":
            return "BoolSort()"
        elif arg == "int":
            return "IntSort()"
        else:
            return f"{arg}_sort"

    @staticmethod
    def translate_function_declaration(function_name, function_args):
        args = []
        for arg in function_args:
            args.append(CodeTranslator.type_str_to_type_sort(arg))

        line = "{} = Function('{}', {})".format(
            function_name,
            function_name,
            ", ".join(args),
        )
        return [CodeTranslator.StdCodeLine(line, CodeTranslator.LineType.DECL)]

    @staticmethod
    def extract_paired_token_index(statement, start_index, left_token, right_token):
        if statement[start_index] != left_token:
            raise RuntimeError("Invalid argument")

        level = 1
        for i in range(start_index + 1, len(statement)):
            if statement[i] == left_token:
                level += 1
            elif statement[i] == right_token:
                level -= 1
                if level == 0:
                    return i

    @staticmethod
    def extract_temperal_variable_name_and_scope(scope_contents):
        scope_fragments = [x.strip() for x in scope_contents.split(",")]
        return [x.split(":") for x in scope_fragments]

    @staticmethod
    def handle_count_function(statement):
        index = statement.find("Count(")
        content_start_index = index + len("Count")
        content_end_index = CodeTranslator.extract_paired_token_index(statement, content_start_index, "(", ")")
        count_arg_contents = statement[content_start_index + 1:content_end_index]
        scope_end_index = CodeTranslator.extract_paired_token_index(count_arg_contents, 0, "[", "]")
        scope_contents = count_arg_contents[1:scope_end_index]
        vars_and_scopes = CodeTranslator.extract_temperal_variable_name_and_scope(scope_contents)
        count_expr = count_arg_contents[scope_end_index + 1:].lstrip(", ")

        transformed_count_statement = "Sum([{} {}])".format(
            count_expr,
            " ".join([f"for {var} in {scope}" for var, scope in vars_and_scopes])
        )

        statement = statement[:index] + transformed_count_statement + statement[content_end_index + 1:]

        return statement

    @staticmethod
    def handle_distinct_function(statement):
        scoped_distinct_regex = r"Distinct\(\[[a-zA-Z0-9_]+:[a-zA-Z0-9_]"
        match = re.search(scoped_distinct_regex, statement)
        index = match.start()
        content_start_index = index + len("Distinct")
        content_end_index = CodeTranslator.extract_paired_token_index(statement, content_start_index, "(", ")")
        distinct_arg_contents = statement[content_start_index + 1:content_end_index]
        scope_end_index = CodeTranslator.extract_paired_token_index(distinct_arg_contents, 0, "[", "]")
        scope_contents = distinct_arg_contents[1:scope_end_index]
        vars_and_scopes = CodeTranslator.extract_temperal_variable_name_and_scope(scope_contents)
        distinct_expr = distinct_arg_contents[scope_end_index + 1:].lstrip(", ")
        assert len(vars_and_scopes) == 1
        transformed_distinct_statement = "Distinct([{} for {} in {}])".format(
            distinct_expr,
            vars_and_scopes[0][0],
            vars_and_scopes[0][1],
        )

        statement = statement[:index] + transformed_distinct_statement + statement[content_end_index + 1:]

        return statement
 
    @staticmethod
    def handle_quantifier_function(statement, scoped_list_to_type):
        scoped_quantifier_regex = r"(Exists|ForAll)\(\[([a-zA-Z0-9_]+):([a-zA-Z0-9_]+)"
        match = re.search(scoped_quantifier_regex, statement)
        quant_name = match.group(1)

        index = match.start()
        content_start_index = index + len(quant_name)
        content_end_index = CodeTranslator.extract_paired_token_index(statement, content_start_index, "(", ")")
        quant_arg_contents = statement[content_start_index + 1:content_end_index]
        scope_end_index = CodeTranslator.extract_paired_token_index(quant_arg_contents, 0, "[", "]")
        scope_contents = quant_arg_contents[1:scope_end_index]
        vars_and_scopes = CodeTranslator.extract_temperal_variable_name_and_scope(scope_contents)
        quant_expr = quant_arg_contents[scope_end_index + 1:].lstrip(", ")

        var_need_declaration = []
        var_need_compresion = []
        for (var_name, var_scope) in vars_and_scopes:
            if var_scope in scoped_list_to_type:
                if scoped_list_to_type[var_scope] == CodeTranslator.ListValType.ENUM:
                    var_need_declaration.append((var_name, var_scope))
                else:
                    var_need_compresion.append((var_name, var_scope))
            else:
                assert var_scope in ["int", "bool"]
                var_need_declaration.append((var_name, var_scope))

        decl_lines = []
        std_scope = []
        if var_need_declaration:
            for (var_name, var_scope) in var_need_declaration:
                decl_lines.append(f"{var_name} = Const('{var_name}', {CodeTranslator.type_str_to_type_sort(var_scope)})")
                std_scope.append(var_name)
            std_constraint = "{}([{}], {})".format(quant_name, ", ".join(std_scope), quant_expr)
        else:
            std_constraint = quant_expr
    
        if var_need_compresion:
            logic_f = "And" if quant_name == "ForAll" else "Or"
            std_constraint = "{}([{} {}])".format(logic_f, std_constraint, " ".join([f"for {var_name} in {var_scope}" for (var_name, var_scope) in var_need_compresion]))

        std_constraint = statement[:index] + std_constraint + statement[content_end_index + 1:]

        return decl_lines, std_constraint

    @staticmethod
    def translate_constraint(constraint, scoped_list_to_type):
        # handle special operators into standard python operators
        while "Count(" in constraint:
            constraint = CodeTranslator.handle_count_function(constraint)

        scoped_distinct_regex = r"Distinct\(\[[a-zA-Z0-9_]+:[a-zA-Z0-9_]"
        # check if we can find scoped_distinct_regex in constraint
        while re.search(scoped_distinct_regex, constraint):
            constraint = CodeTranslator.handle_distinct_function(constraint)

        scoped_quantifier_regex = r"(Exists|ForAll)\(\[([a-zA-Z0-9_]+):([a-zA-Z0-9_]+)"
        all_decl_lines = []
        while re.search(scoped_quantifier_regex, constraint):
            decl_lines, constraint = CodeTranslator.handle_quantifier_function(constraint, scoped_list_to_type)
            all_decl_lines += decl_lines

        lines = [CodeTranslator.StdCodeLine(l, CodeTranslator.LineType.DECL) for l in all_decl_lines] + [CodeTranslator.StdCodeLine(constraint, CodeTranslator.LineType.CONS)]
        return lines

    @staticmethod
    def translate_option_verification(option_block, choice_name):
        lines = []
        lines.append("solver = Solver()")
        lines.append("solver.add(pre_conditions)")
        for l in option_block:
            lines.append("solver.add(Not({}))".format(l))
        lines.append("if solver.check() == unsat:")
        lines.append("\tprint('{}')".format(choice_name))
        return lines

    @staticmethod
    def assemble_standard_code(declaration_lines, pre_condidtion_lines, option_blocks):
        lines = []

        header_lines = [
            "from z3 import *", ""
        ]

        lines += header_lines
        lines += [x.line for x in declaration_lines]
        lines += [""]

        lines += ["pre_conditions = []"]
        for line in pre_condidtion_lines:
            if line.line_type == CodeTranslator.LineType.DECL:
                lines += [line.line]
            else:
                lines += ["pre_conditions.append({})".format(line.line)]
        lines += [""]

        function_lines = [
            "def is_valid(option_constraints):",
            TAB_STR + "solver = Solver()",
            TAB_STR + "solver.add(pre_conditions)",
            TAB_STR + "solver.add(Not(option_constraints))",
            TAB_STR + "return solver.check() == unsat",
            "",
            "def is_unsat(option_constraints):",
            TAB_STR + "solver = Solver()",
            TAB_STR + "solver.add(pre_conditions)",
            TAB_STR + "solver.add(option_constraints)",
            TAB_STR + "return solver.check() == unsat",
            "",
            "def is_sat(option_constraints):",
            TAB_STR + "solver = Solver()",
            TAB_STR + "solver.add(pre_conditions)",
            TAB_STR + "solver.add(option_constraints)",
            TAB_STR + "return solver.check() == sat",
            "",
            "def is_accurate_list(option_constraints):",
            TAB_STR + "return is_valid(Or(option_constraints)) and all([is_sat(c) for c in option_constraints])",
            "",
            "def is_exception(x):",
            TAB_STR + "return not x",
            ""
        ]


        lines += function_lines
        lines += [""]

        # handle option blocks
        for option_block, choice_name in zip(option_blocks, CHOICE_INDEXES):
            assert len([l for l in option_block if l.line_type == CodeTranslator.LineType.CONS]) == 1
            for line in option_block:
                if line.line_type == CodeTranslator.LineType.DECL:
                    lines += [line.line]
                else:
                    lines += [f"if {line.line}: print('{choice_name}')"]
        return "\n".join(lines)