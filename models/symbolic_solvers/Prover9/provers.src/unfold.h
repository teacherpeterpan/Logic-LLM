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

#ifndef TP_UNFOLD_H
#define TP_UNFOLD_H

#include "../ladr/parautil.h"
#include "../ladr/clist.h"
#include "../ladr/ioutil.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from unfold.c */

void unfold_eq_defs(Clist clauses, int arity_limit, int constant_limit, BOOL print);

BOOL fold_eq_defs(Clist clauses, BOOL kbo);

BOOL one_unary_def(Clist a, Clist b);

#endif  /* conditional compilation of whole file */
