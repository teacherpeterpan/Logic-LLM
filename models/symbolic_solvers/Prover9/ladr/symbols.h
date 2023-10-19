/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License,
    version 2.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef TP_SYMBOLS_H
#define TP_SYMBOLS_H

#include "strbuf.h"
#include "glist.h"

/* INTRODUCTION
This collection of routines manages the global symbol table.
Each symbol is a pair (string,arity) and has a unique ID number.
For example, ("f",2), sometimes written f/2, is a different symbol
from f/3.
<P>
These symbols are used mostly as constant, function, and predicate
symbols, but they can be used for variables and other things as
well.
<P>
The symbol table routines call malloc() and free() directly.
The LADR memory package (tp_alloc()) is not used.
*/

/* Public definitions */

/* maximum number of chars in string part of symbol (includes '\0') */

#define MAX_NAME      51

/* parse/print properties of symbols */

typedef enum {NOTHING_SPECIAL,
	      INFIX,         /* xfx */
	      INFIX_LEFT ,   /* yfx */
	      INFIX_RIGHT,   /* xfy */
	      PREFIX_PAREN,  /* fx  */
	      PREFIX,        /* fy  */
	      POSTFIX_PAREN, /* xf  */
	      POSTFIX        /* yf  */
             } Parsetype;

#define MIN_PRECEDENCE      1
#define MAX_PRECEDENCE    999

/* Function/relation properties of symbols */

typedef enum { UNSPECIFIED_SYMBOL,
	       FUNCTION_SYMBOL,
	       PREDICATE_SYMBOL
             } Symbol_type;

/* Unification properties of symbols */

typedef enum { EMPTY_THEORY,
	       COMMUTE,
	       ASSOC_COMMUTE
             } Unif_theory;

/* LRPO status */

typedef enum { LRPO_LR_STATUS,
	       LRPO_MULTISET_STATUS
             } Lrpo_status;

/* Variable style */

typedef enum { STANDARD_STYLE,      /* x,y,z,... */
	       PROLOG_STYLE,        /* A,B,C,... */
	       INTEGER_STYLE        /* 0,1,2,... */
             } Variable_style;

typedef struct symbol * Symbol;

/* End of public definitions */

/* Public function prototypes from symbols.c */

char *true_sym(void);

char *false_sym();

char *and_sym();

char *or_sym();

char *not_sym();

char *iff_sym();

char *imp_sym();

char *impby_sym();

char *all_sym();

char *exists_sym();

char *quant_sym();

char *attrib_sym();

char *eq_sym();

char *neq_sym();

void set_operation_symbol(char *operation, char *symbol);

char *get_operation_symbol(char *operation);

BOOL symbol_in_use(char *str);

int str_to_sn(char *str, int arity);

void fprint_syms(FILE *fp);

void p_syms(void);

void fprint_sym(FILE *fp, int symnum);

void sprint_sym(String_buf sb, int symnum);

void p_sym(int symnum);

BOOL str_exists(char *str);

int greatest_symnum(void);

char *sn_to_str(int symnum);

BOOL is_symbol(int symnum, char *str, int arity);

int sn_to_arity(int symnum);

int sn_to_occurrences(int symnum);

void set_unfold_symbol(int symnum);

BOOL is_unfold_symbol(int symnum);

void declare_aux_symbols(Ilist syms);

char *parse_type_to_str(Parsetype type);

void clear_parse_type_for_all_symbols(void);

void clear_parse_type(char *str);

void set_parse_type(char *str, int precedence, Parsetype type);

BOOL binary_parse_type(char *str, int *precedence, Parsetype *type);

BOOL unary_parse_type(char *str, int *precedence, Parsetype *type);

int special_parse_type(char *str);

void set_assoc_comm(char *str, BOOL set);

void set_commutative(char *str, BOOL set);

BOOL assoc_comm_symbols(void);

BOOL comm_symbols(void);

BOOL is_assoc_comm(int sn);

BOOL is_commutative(int sn);

BOOL is_eq_symbol(int symnum);

int not_symnum(void);

int or_symnum(void);

void declare_base_symbols(void);

void set_variable_style(Variable_style style);

Variable_style variable_style(void);

BOOL variable_name(char *s);

void symbol_for_variable(char *str, int varnum);

Ilist variable_symbols(Ilist syms);

Ilist remove_variable_symbols(Ilist syms);

void set_symbol_type(int symnum, Symbol_type type);

Symbol_type get_symbol_type(int symnum);

BOOL function_symbol(int symnum);

BOOL relation_symbol(int symnum);

BOOL function_or_relation_symbol(int symnum);

void declare_functions_and_relations(Ilist fsyms, Ilist rsyms);

int function_or_relation_sn(char *str);

Ilist all_function_symbols(void);

Ilist all_relation_symbols(void);

void set_lrpo_status(int symnum, Lrpo_status status);

void all_symbols_lrpo_status(Lrpo_status status);

Lrpo_status sn_to_lrpo_status(int sn);

void set_kb_weight(int symnum, int weight);

BOOL zero_wt_kb(void);

int sn_to_kb_wt(int symnum);

void print_kbo_weights(FILE *fp);

void set_skolem(int symnum);

void skolem_check(BOOL flag);

int next_skolem_symbol(int arity);

Ilist skolem_symbols(void);

BOOL is_skolem(int symnum);

void skolem_reset(void);

void decommission_skolem_symbols(void);

void set_skolem_symbols(Ilist symnums);

void set_lex_val(int symnum, int lex_val);

int sn_to_lex_val(int sn);

Ordertype sym_precedence(int symnum_1, int symnum_2);

Ilist syms_with_lex_val(void);

BOOL exists_preliminary_precedence(Symbol_type type);

Ordertype preliminary_lex_compare(Symbol a, Symbol b);

Ordertype lex_compare_arity_0123(Symbol s1, Symbol s2);

Ordertype lex_compare_arity_0213(Symbol s1, Symbol s2);

void lex_order(Ilist fsyms, Ilist rsyms,
	       I2list fsyms_multiset, I2list rsyms_multiset,
	       Ordertype (*comp_proc) (Symbol, Symbol));

Ilist sort_by_lex_val(Ilist p);

Ilist current_fsym_precedence();

Ilist current_rsym_precedence();

Ilist not_in_preliminary_precedence(Ilist syms, Symbol_type type);

void print_fsym_precedence(FILE *fp);

void print_rsym_precedence(FILE *fp);

int min_lex_val(void);

void assign_greatest_precedence(int symnum);

BOOL has_greatest_precedence(int symnum);

void lex_insert_after_initial_constants(Ilist syms);

void add_skolems_to_preliminary_precedence(void);

int fresh_symbol(char *prefix, int arity);

int gen_new_symbol(char *prefix, int arity, Ilist syms);

void mark_for_new_symbols(void);

I2list new_symbols_since_mark(void);

void add_new_symbols(I2list syms);

void new_constant_properties(int sn);

Ilist arity_check(Ilist fsyms, Ilist rsyms);

int symbol_with_string(Ilist syms, char *str);

void process_skolem_list(Plist skolem_strings, Ilist fsyms);

void process_lex_list(Plist lex_strings, Ilist syms, Symbol_type type);

Ilist symnums_of_arity(Ilist p, int i);

#endif  /* conditional compilation of whole file */
