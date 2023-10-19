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

#ifndef TP_ACCANON_H
#define TP_ACCANON_H

#include "termflag.h"
#include "termorder.h"

/* INTRODUCTION
*/

/* Public definitions */

/* If you get an error message "flatten, too many arguments", increase
 * the following symbol (2500 is enough for the dem_alu problem.)
 */

#define MAX_ACM_ARGS 2500

/* End of public definitions */

/* Public function prototypes from accanon.c */

void flatten(Term t, Term *a, int *ip);

void ac_canonical2(Term t, int bit,
		   Ordertype (*term_compare_proc) (Term, Term));

void ac_canonical(Term t, int bit);

int check_ac_canonical(Term t);

#endif  /* conditional compilation of whole file */
