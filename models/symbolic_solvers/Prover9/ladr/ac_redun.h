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

#ifndef TP_AC_REDUN_H
#define TP_AC_REDUN_H

#include "parautil.h"
#include "accanon.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from ac_redun.c */

BOOL same_top(Term t1, Term t2);

int commutativity(Term atom);

int associativity(Term atom);

int c_associativity(Term atom);

int associativity3(Term atom);

int associativity4(Term atom);

BOOL cac_tautology(Literals lits);

BOOL cac_redundancy(Topform c, BOOL print);

#endif  /* conditional compilation of whole file */
