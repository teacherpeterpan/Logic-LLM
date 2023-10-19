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

#ifndef TP_ARITHMETIC_H
#define TP_ARITHMETIC_H

/* #include "" */

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from arithmetic.c */

void init_arithmetic(void);

BOOL domain_term(Term t, int domain_size);

BOOL arith_op_sn(int i);

BOOL arith_rel_sn(int i);

BOOL arith_op_term(Term t);

BOOL arith_rel_term(Term t);

BOOL arith_term(Term t);

BOOL arith_quasi_evaluable(Term t);

int arith_evaluate(Term t, BOOL *evaluated);

int arith_eval(Term t, BOOL *evaluated);

BOOL ok_for_arithmetic(Plist clauses, int domain_size);

BOOL check_with_arithmetic(Plist ground_clauses);

#endif  /* conditional compilation of whole file */
