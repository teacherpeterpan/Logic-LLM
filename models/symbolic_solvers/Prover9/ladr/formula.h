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

#ifndef TP_FORMULA_H
#define TP_FORMULA_H

#include "attrib.h"
#include "tlist.h"
#include "termorder.h"
#include "hash.h"

/* INTRODUCTION
*/

/* Public definitions */

/* formula types */

typedef enum {
  ATOM_FORM=0, AND_FORM, OR_FORM, NOT_FORM, IFF_FORM,
  IMP_FORM, IMPBY_FORM, ALL_FORM, EXISTS_FORM} Ftype;

typedef struct formula * Formula;
struct formula {
  Ftype       type;
  int         arity;
  char        *qvar;         /* quantified variable */
  Formula     *kids;         /* for non-atoms */
  Term        atom;          /* for atoms */
  Attribute   attributes;    /* */
  int excess_refs;           /* count of extra references */
};

/* formula preference */

typedef enum { CONJUNCTION,
	       DISJUNCTION
             } Fpref;

/* macros */

#define TRUE_FORMULA(f)  ((f)->type == AND_FORM && (f)->arity == 0)
#define FALSE_FORMULA(f) ((f)->type == OR_FORM  && (f)->arity == 0)

/* End of public definitions */

/* Public function prototypes from formula.c */

void free_formula(Formula p);

void fprint_formula_mem(FILE *fp, BOOL heading);

void p_formula_mem();

unsigned formula_megs(void);

Formula formula_get(int arity, Ftype type);

void zap_formula(Formula f);

BOOL logic_term(Term t);

void gather_symbols_in_formula_term(Term t, I2list *rsyms, I2list *fsyms);

void gather_symbols_in_formula(Formula f, I2list *rsyms, I2list *fsyms);

void gather_symbols_in_formulas(Plist lst, I2list *rsyms, I2list *fsyms);

Ilist function_symbols_in_formula(Formula f);

Ilist relation_symbols_in_formula(Formula f);

BOOL relation_symbol_in_formula(int sn, Formula f);

Formula term_to_formula(Term t);

Term formula_to_term(Formula f);

void fprint_formula(FILE *fp, Formula f);

void p_formula(Formula c);

unsigned hash_formula(Formula f);

BOOL formula_ident(Formula f, Formula g);

Formula formula_copy(Formula f);

BOOL dual_type(int op);

Formula dual(Formula f);

Formula and(Formula a, Formula b);

Formula or(Formula a, Formula b);

Formula imp(Formula a, Formula b);

Formula impby(Formula a, Formula b);

Formula negate(Formula a);

BOOL quant_form(Formula f);

Formula flatten_top(Formula f);

Formula formula_flatten(Formula f);

Formula nnf2(Formula f, Fpref pref);

Formula nnf(Formula f);

Formula make_conjunction(Formula f);

Formula make_disjunction(Formula f);

void formula_canon_eq(Formula f);

int formula_size(Formula f);

int greatest_qvar(Formula f);

int greatest_symnum_in_formula(Formula f);

void subst_free_var(Formula f, Term target, Term replacement);

Formula eliminate_rebinding(Formula f);

BOOL closed_formula(Formula f);

Formula get_quant_form(Ftype type, char *qvar, Formula subformula);

Formula universal_closure(Formula f);

BOOL free_variable(char *svar, Formula f);

Formula formulas_to_conjunction(Plist formulas);

Formula formulas_to_disjunction(Plist formulas);

Plist copy_plist_of_formulas(Plist formulas);

BOOL literal_formula(Formula f);

BOOL clausal_formula(Formula f);

void formula_set_variables(Formula f, int max_vars);

BOOL positive_formula(Formula f);

BOOL formula_contains_attributes(Formula f);

BOOL subformula_contains_attributes(Formula f);

Ilist constants_in_formula(Formula f);

BOOL relation_in_formula(Formula f, int symnum);

void rename_all_bound_vars(Formula f);

void rename_these_bound_vars(Formula f, Ilist vars);

#endif  /* conditional compilation of whole file */
