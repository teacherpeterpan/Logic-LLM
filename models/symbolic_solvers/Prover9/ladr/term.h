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

#ifndef TP_TERM_H
#define TP_TERM_H

#include "symbols.h"

/* INTRODUCTION
The Term data structure is designed mainly to represent
<GL>first-order untyped term</GL>s.
It is generally used for <GL>atoms</GL> as well,
because the indexing and unification methods don't care whether
an object is a term or an atom.
<P>
No <GL>term structure sharing</GL> is supported at this level of
abstraction.  (Higher-level packagers can build terms with shared
structure if they wish.)  Because we envision applications with tens
of millions of terms, small size for the individual nodes is
important.  So we have some overloaded fields, and macros are
provided to get some of the information from from term nodes.
<P>
There are three types of term, and the Boolean macros
<TT>VARIABLE(t)</TT>, <TT>CONSTANT(t)</TT>, and <TT>COMPLEX(t)</TT>
should be used to find out what type a term is.  If you have a
<TT>CONSTANT</TT> or <TT>COMPLEX</TT> term t, you can get its symbol
id with <TT>SYMNUM(t)</TT> (from which you can get other information
about the symbol such as the print string and any special properties).
If you have a variable t, you can get its index with
<TT>VARNUM(t)</TT>, which is a signed integral type in the range
[0..<TT>MAX_VAR</TT>].  <I>Warning</I>: <TT>MAX_VAR</TT> is a big
number---a higher-level unification package will typically have a much
smaller <TT>MAX_VARS</TT> defined for array sizes, because it does
array indexing with <TT>VARNUM(t)</TT>.
<P>
The macro <TT>ARITY(t)</TT> gets the arity of a term (constants and
variables have arity 0), and <TT>ARG(t,i)</TT> gets the i-th argument
<I>(counting from 0)</I> of a term.  When using <TT>ARG(t,i)</TT>,
make sure that i is in range [0..<TT>ARITY(t)</TT>-1], because
<TT>ARG</TT> does not check.
<P>
Here is an example of recursing through a term.
<PRE>
int symbol_count(Term t)
{
  int count = 0;
  int i;
  for (i = 0; i < ARITY(t); i++)
    count += symbol_count(ARG(t,i));
  return count+1;
}
</PRE>
*/

/* Public definitions */

#define MAX_VARS  100   /* max number of (distinct) variables per term */
#define MAX_VNUM  5000  /* maximum variable ID, for array of vars */

#define MAX_VAR   INT_MAX     /* max var ID that fits in sym field of term */
#define MAX_SYM   INT_MAX     /* max ID of any rigid symbol */
#define MAX_ARITY UCHAR_MAX   /* max arity of any term (don't make this big) */

#define FLAGS_TYPE unsigned char  /* for private_flags field of Term */

typedef struct term * Term;     /* Term is a pointer to a term struct */

struct term {
  int            private_symbol; /* const/func/pred/var symbol ID */
  unsigned char  arity;          /* number of auguments */
  FLAGS_TYPE     private_flags;  /* for marking terms in various ways */
  Term           *args;          /* array (size arity) of pointers to args */
  void           *container;     /* containing object */
  union {
    unsigned     id;             /* unique ID, probably for FPA indexing */
    void         *vp;            /* auxiliary pointer */
  } u;
};

/* to check type of term */
#define VARIABLE(t) ((t)->private_symbol >= 0)
#define CONSTANT(t) ((t)->private_symbol < 0 && (t)->arity == 0)
#define COMPLEX(t)  ((t)->private_symbol < 0 && (t)->arity > 0)

/* to get symbol ID from a CONSTANT or COMPLEX term */
#define SYMNUM(t)   (-((t)->private_symbol))

/* to get the variable number of a VARIABLE term */
#define VARNUM(t)   ((t)->private_symbol)

/* to get the arity of a term (VARIABLE terms have arity 0) */
#define ARITY(t)    ((t)->arity)

/* to get the i-th argument of a term (make sure i is in [0..arity-1]) */
#define ARG(t,i)    ((t)->args[i])

/* to get the array of arguments */
#define ARGS(t)    ((t)->args)

/* End of public definitions */

/* Public function prototypes from term.c */

void free_term(Term p);

void fprint_term_mem(FILE *fp, BOOL heading);

void p_term_mem(void);

Term get_variable_term(int var_num);

Term get_rigid_term_like(Term t);

Term get_rigid_term(char *sym, int arity);

Term get_rigid_term_dangerously(int symnum, int arity);

void zap_term(Term t);

BOOL term_ident(Term t1, Term t2);

Term copy_term(Term t);

BOOL ground_term(Term t);

int biggest_variable(Term t);

int term_depth(Term t);

int symbol_count(Term t);

BOOL occurs_in(Term t1, Term t2);

void fprint_term(FILE *fp, Term t);

void sprint_term(String_buf sb, Term t);

char *term_to_string(Term t);

void p_term(Term t);

BOOL all_args_vars(Term t);

Term build_binary_term(int sn, Term a1, Term a2);

Term build_binary_term_safe(char *str, Term a1, Term a2);

Term build_unary_term(int sn, Term a);

Term build_unary_term_safe(char *str, Term a);

Term subst_term(Term t, Term target, Term replacement);

Term subst_var_term(Term t, int symnum, int varnum);

int greatest_variable(Term t);

int greatest_symnum_in_term(Term t);

void upward_term_links(Term t, void *p);

BOOL check_upward_term_links(Term t, void *p);

int occurrences(Term t, Term target);

void term_set_variables(Term t, int max_vars);

Term nat_to_term(int n);

Term int_to_term(int i);

Term bool_to_term(BOOL val);

Term double_to_term(double d);

int natural_constant_term(Term t);

int arg_position(Term parent, Term child);

BOOL is_term(Term t, char *str, int arity);

BOOL is_constant(Term t, char *str);

char *term_symbol(Term t);

BOOL term_to_int(Term t, int *result);

BOOL term_to_double(Term t, double *result);

BOOL term_to_number(Term t, double *result);

BOOL true_term(Term t);

BOOL false_term(Term t);

BOOL term_to_bool(Term t, BOOL *result);

I2list symbols_in_term(Term t, I2list g);

Ilist fsym_set_in_term(Term t);

Term renum_vars_recurse(Term t, int vmap[], int max_vars);

Term set_vars_recurse(Term t, char *vnames[], int max_vars);

I2list multiset_of_vars(Term t, I2list vars);

I2list multiset_vars(Term t);

Plist set_of_vars(Term t, Plist vars);

Plist set_of_variables(Term t);

int number_of_vars_in_term(Term t);

Ilist set_of_ivars(Term t, Ilist ivars);

Ilist set_of_ivariables(Term t);

BOOL variables_subset(Term t1, Term t2);

BOOL variables_multisubset(Term a, Term b);

Term term_at_pos(Term t, Ilist pos);

Ilist position_of_subterm(Term t, Term subterm);

int symbol_occurrences(Term t, int symnum);

BOOL args_distinct_vars(Term t);

unsigned hash_term(Term t);

BOOL skolem_term(Term t);

BOOL contains_skolem_term(Term t);

BOOL contains_skolem_function(Term t);

Term term0(char *sym);

Term term1(char *sym, Term arg);

Term term2(char *sym, Term arg1, Term arg2);

BOOL symbol_in_term(int symnum, Term t);

BOOL same_structure(Term a, Term b);

Plist copy_plist_of_terms(Plist terms);

void zap_plist_of_terms(Plist lst);

BOOL eq_term(Term a);

Plist plist_of_subterms(Term t);

BOOL tlist_member(Term t, Plist lst);

int position_of_term_in_tlist(Term t, Plist lst);

BOOL tlist_subset(Plist a, Plist b);

BOOL tlist_set(Plist a);

Plist free_vars_term(Term t, Plist vars);

#endif  /* conditional compilation of whole file */
