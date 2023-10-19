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

#ifndef TP_PARAUTIL_H
#define TP_PARAUTIL_H

 #include "just.h"

/* INTRODUCTION
This package contains a few utilites for paramodulation and demodulation.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from parautil.c */

void init_paramod(void);

void mark_renamable_flip(Term atom);

BOOL renamable_flip_eq(Term atom);

BOOL renamable_flip_eq_test(Term atom);

void mark_oriented_eq(Term atom);

BOOL oriented_eq(Term atom);

BOOL same_term_structure(Term t1, Term t2);

void flip_eq(Term atom, int n);

void orient_equalities(Topform c, BOOL allow_flips);

BOOL eq_tautology(Topform c);

Term top_flip(Term a);

void zap_top_flip(Term a);

Literals literal_flip(Literals a);

void zap_literal_flip(Literals a);

Topform new_constant(Topform c, int new_sn);

Topform fold_denial(Topform c, int alpha_max);

BOOL equational_def_2(Term alpha, Term beta);

int equational_def(Topform c);

Ordertype unfold_order(Term alpha, Term beta);

Topform build_reflex_eq(void);

#endif  /* conditional compilation of whole file */
