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

#ifndef TP_INTERP_H
#define TP_INTERP_H

#include "parse.h"
#include "topform.h"

/* INTRODUCTION
*/

/* Public definitions */

typedef struct interp *Interp;

enum { SEMANTICS_NOT_EVALUATED,
       SEMANTICS_NOT_EVALUABLE,
       SEMANTICS_TRUE,
       SEMANTICS_FALSE
 };

/* End of public definitions */

/* Public function prototypes from interp.c */

void fprint_interp_mem(FILE *fp, BOOL heading);

void p_interp_mem();

int int_power(int n, int exp);

Interp compile_interp(Term t, BOOL allow_incomplete);

void transpose_binary(Term t);

void zap_interp(Interp p);

void fprint_interp_tex(FILE *fp, Interp p);

void fprint_interp_xml(FILE *fp, Interp p);

void fprint_interp_standard(FILE *fp, Interp p);

void fprint_interp_standard2(FILE *fp, Interp p);

void fprint_interp_portable(FILE *fp, Interp p);

void p_interp(Interp p);

void fprint_interp_cooked(FILE *fp, Interp p);

void fprint_interp_tabular(FILE *fp, Interp p);

void fprint_interp_raw(FILE *fp, Interp p);

int eval_term_ground(Term t, Interp p, int *vals);

BOOL eval_literals(Literals lits, Interp p);

int eval_literals_true_instances(Literals lits, Interp p);

int eval_literals_false_instances(Literals lits, Interp p);

BOOL eval_formula(Formula f, Interp p);

Term interp_remove_constants_recurse(Term ops);

void interp_remove_constants(Term t);

Term interp_remove_others_recurse(Term ops, Plist keepers);

void interp_remove_others(Term t, Plist keepers);

Interp copy_interp(Interp p);

Interp permute_interp(Interp source, int *p);

BOOL ident_interp_perm(Interp a, Interp b, int *p);

Interp normal_interp(Interp a);

BOOL isomorphic_interps(Interp a, Interp b, BOOL normal);

int interp_size(Interp a);

Term interp_comments(Interp a);

int *interp_table(Interp p, char *sym, int arity);

long unsigned iso_checks(void);

long unsigned iso_perms(void);

BOOL evaluable_term(Term t, Interp p);

BOOL evaluable_atom(Term a, Interp p);

BOOL evaluable_literals(Literals lits, Interp p);

BOOL evaluable_formula(Formula f, Interp p);

BOOL evaluable_topform(Topform tf, Interp p);

void update_interp_with_constant(Interp p, Term constant, int val);

BOOL eval_topform(Topform tf, Interp p);

Ordertype compare_interp(Interp a, Interp b);

BOOL ident_interp(Interp a, Interp b);

Interp canon_interp(Interp a);

void assign_discriminator_counts(Interp a, Plist discriminators);

BOOL same_discriminator_counts(Interp a, Interp b);

void update_profile(Topform c, Interp a, int *next);
     /* vecs[domain_element][profile_component] */
      

void create_profile(Interp a, Plist discriminators);

void p_interp_profile(Interp a, Plist discriminators);

Interp normal3_interp(Interp a, Plist discriminators);

BOOL same_profiles(Interp a, Interp b);

long unsigned perms_required(Interp a);

long unsigned factorial(int n);

#endif  /* conditional compilation of whole file */
