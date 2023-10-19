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

#ifndef TP_BTM_H
#define TP_BTM_H

#include "unify.h"
#include "accanon.h"

/* INTRODUCTION
This package handles "backtrack matching", that is, matching
that allows more than one unifier for a pair of terms, and computes
the unifiers incrementally by backtracking.  As I write this,
we support associative commutative (AC) operations and 
commutative/symmetric (C) operations.  Symbols are declared to be
AC with set_assoc_comm() and C with set_commutative().
The use of Terms and Contexts is similar to ordinary matching,
except that the means for undoing substitutions is different.
*/

/* Public definitions */

typedef struct btm_state * Btm_state;

/* End of public definitions */

/* Public function prototypes from btm.c */

void fprint_btm_mem(FILE *fp, BOOL heading);

void p_btm_mem();

void p_btm_state(Btm_state bt);

Btm_state match_bt_first(Term t1, Context c1, Term t2, int partial);

Btm_state match_bt_next(Btm_state bt1);

void match_bt_cancel(Btm_state bt);

#endif  /* conditional compilation of whole file */
