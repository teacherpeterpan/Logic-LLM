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

#ifndef TP_CLAUSES_H
#define TP_CLAUSES_H

#include "topform.h"

/* INTRODUCTION
This package contains various functions on Plists of clauses.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from clauses.c */

Topform clause_member_plist(Plist p, Topform c);

Plist intersect_clauses(Plist a, Plist b);

double max_clause_weight(Plist p);

int max_clause_symbol_count(Plist p);

Plist nonneg_clauses(Plist clauses);

BOOL all_clauses_horn(Plist l);

BOOL all_clauses_unit(Plist l);

BOOL all_clauses_positive(Plist l);

int neg_nonunit_clauses(Plist l);

int negative_clauses(Plist l);

int most_literals(Plist clauses);

BOOL pos_equality_in_clauses(Plist clauses);

BOOL equality_in_clauses(Plist clauses);

#endif  /* conditional compilation of whole file */
