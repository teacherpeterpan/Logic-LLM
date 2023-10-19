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

#ifndef TP_DIOPH_H
#define TP_DIOPH_H

#include <stdio.h>

/* INTRODUCTION
This package solves linear homogeneous Diophantine equations, which
can be used to compute the set of associative-commutative unifiers
for a pair of first-order terms.  The way this usually works is
that you first call dio() to get a basis of solutions, then
call one of the the next_combo routines to (incrementally)
get the relevant solutions.
<P>
The file dioph.c has a main() program, and it can be compiled and linked
by itself for testing.  The command for doing so is "gcc -DSOLO dioph.c".
*/

/* Public definitions */

#define MAX_COEF   250  /* total # of coef. from both sides */
#define MAX_BASIS  100  /* must be <= MAX_VARS, because rows are indexed */
#define MAX_COMBOS 200  /* for superset-restricted AC unif. */

/* End of public definitions */

/* Public function prototypes from dioph.c */

int dio(int ab[MAX_COEF],
	int m, int n,
	int constraints[MAX_COEF],
	int basis[MAX_BASIS][MAX_COEF],
	int *num_basis);

int next_combo_a(int length, int basis[MAX_BASIS][MAX_COEF],
		 int num_basis, int constraints[MAX_COEF],
		 int combo[MAX_BASIS], int sum[MAX_COEF],
		 int start_flag);

int next_combo_b(int length, int basis[MAX_BASIS][MAX_COEF],
		 int num_basis, int constraints[MAX_COEF],
		 int combo[MAX_BASIS], int sum[MAX_COEF],
		 int start_flag);

int next_combo_c(int length, int basis[MAX_BASIS][MAX_COEF],
		 int num_basis, int constraints[MAX_COEF],
		 int combo[MAX_BASIS], int sum[MAX_COEF],
		 int start_flag);

int next_combo_ss(int length, int basis[MAX_BASIS][MAX_COEF],
		  int num_basis, int constraints[MAX_COEF],
		  int combo[MAX_BASIS], int sum[MAX_COEF],
		  int start_flag,
		  int combos[MAX_COMBOS][MAX_BASIS],
		  int *np,
		  int ss_parm);

void p_ac_basis(int basis[MAX_BASIS][MAX_COEF],
		int num_basis, int m, int n);

#endif  /* conditional compilation of whole file */
