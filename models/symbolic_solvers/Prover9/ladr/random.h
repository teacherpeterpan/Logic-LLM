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

#ifndef TP_RANDOM_H
#define TP_RANDOM_H

#include "topform.h"

/* INTRODUCTION
These are some routines I used for testing and debugging some
of the low-level code.  The main reason I wrote these is so that
I could write and test the early code without having to input terms.
Maybe I'll see how far I can go before I have to write a term parser.
<P>
And, who knows, maybe the next big breakthrough in automated theorem
proving will depend on randonly generated terms!
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from random.c */

Term random_term(int v, int a0, int a1, int a2, int a3,
		 int max_depth);

Term random_nonvariable_term(int v, int a0, int a1, int a2, int a3,
			     int max_depth);

Term random_complex_term(int v, int a0, int a1, int a2, int a3,
			     int max_depth);

Ilist random_path(int length_max, int value_max);

void random_permutation(int *a, int size);

Topform random_clause(int v, int a0, int a1, int a2, int a3,
		     int max_depth, int max_lits);

Term random_op_term(int depth);

#endif  /* conditional compilation of whole file */
