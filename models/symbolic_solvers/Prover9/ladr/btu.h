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

#ifndef TP_BTU_H
#define TP_BTU_H

#include "dioph.h"
#include "unify.h"

/* INTRODUCTION
This package handles "backtrack unification", that is, unification
that allows more than one unifier for a pair of terms, and computes
the unifiers incrementally by backtracking.  As I write this,
we support associative commutative (AC) operations and 
commutative/symmetric (C) operations.  Symbols are declared to be
AC with set_assoc_comm() and C with set_commutative().
The use of Terms and Contexts is similar to ordinary unification,
except that the means for undoing substitutions is different.
*/

/* Public definitions */

typedef struct btu_state * Btu_state;

/* End of public definitions */

/* Public function prototypes from btu.c */

void fprint_btu_mem(FILE *fp, BOOL heading);

void p_btu_mem();

Btu_state unify_bt_first(Term t1, Context c1,
			Term t2, Context c2);

Btu_state unify_bt_next(Btu_state bt1);

void unify_bt_cancel(Btu_state bt);

void p_bt_tree(Btu_state bt, int n);

#endif  /* conditional compilation of whole file */
