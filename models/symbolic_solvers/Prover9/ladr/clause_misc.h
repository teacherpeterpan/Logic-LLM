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

#ifndef TP_CLAUSE_MISC_H
#define TP_CLAUSE_MISC_H

#include "clist.h"
#include "mindex.h"
#include "just.h"
#include "basic.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from clause_misc.c */

Clist clist_copy(Clist a, BOOL assign_ids);

Clist copy_clauses_to_clist(Plist clauses, char *name, BOOL assign_ids);

Clist move_clauses_to_clist(Plist clauses, char *name, BOOL assign_ids);

Plist input_clauses(Plist a);

void delete_clause(Topform c);

void delete_clist(Clist l);

Topform copy_clause_ija(Topform c);

Plist copy_clauses_ija(Plist p);

void delete_clauses(Plist p);

void make_clause_basic(Topform c);

#endif  /* conditional compilation of whole file */
