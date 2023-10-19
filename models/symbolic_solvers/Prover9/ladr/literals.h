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

#ifndef TP_LITERALS_H
#define TP_LITERALS_H

#include "termflag.h"
#include "tlist.h"

/* INTRODUCTION
*/

/* Public definitions */

typedef struct literals * Literals;

struct literals {
  BOOL      sign;
  Term      atom;
  Literals  next;
};

/* End of public definitions */

/* Public function prototypes from literals.c */

Literals get_literals(void);

void free_literals(Literals p);

void fprint_literals_mem(FILE *fp, int heading);

void p_literals_mem();

void zap_literal(Literals l);

void zap_literals(Literals l);

Literals new_literal(int sign, Term atom);

Literals copy_literal(Literals lit);

Literals append_literal(Literals lits, Literals lit);

Literals term_to_literals(Term t, Literals lits);

Term literal_to_term(Literals l);

Term literals_to_term(Literals l);

Term lits_to_term(Literals l);

void free_lits_to_term(Term t);

int positive_literals(Literals lits);

int negative_literals(Literals lits);

BOOL positive_clause(Literals lits);

BOOL any_clause(Literals lits);

BOOL negative_clause(Literals lits);

BOOL mixed_clause(Literals lits);

int number_of_literals(Literals lits);

BOOL unit_clause(Literals lits);

BOOL horn_clause(Literals lits);

BOOL definite_clause(Literals lits);

int greatest_variable_in_clause(Literals lits);

Plist vars_in_clause(Literals lits);

Ilist varnums_in_clause(Literals lits);

int number_of_variables(Literals lits);

BOOL ground_clause(Literals lits);

Literals copy_literals(Literals lits);

Literals copy_literals_with_flags(Literals lits);

Literals copy_literals_with_flag(Literals lits, int flag);

int literal_number(Literals lits, Literals lit);

int atom_number(Literals lits, Term atom);

Literals ith_literal(Literals lits, int i);

BOOL true_clause(Literals lits);

BOOL tautology(Literals lits);

int symbol_occurrences_in_clause(Literals lits, int symnum);

Literals remove_null_literals(Literals l);

Literals first_literal_of_sign(Literals lits, BOOL sign);

Ilist constants_in_clause(Literals lits);

BOOL clause_ident(Literals lits1, Literals lits2);

int clause_symbol_count(Literals lits);

int clause_depth(Literals lits);

BOOL pos_eq(Literals lit);

BOOL neg_eq(Literals lit);

BOOL pos_eq_unit(Literals lits);

BOOL neg_eq_unit(Literals lits);

BOOL contains_pos_eq(Literals lits);

BOOL contains_eq(Literals lits);

BOOL only_eq(Literals lits);

int literals_depth(Literals lits);

Term term_at_position(Literals lits, Ilist pos);

Ilist pos_predicates(Ilist p, Literals lits);

#endif  /* conditional compilation of whole file */
