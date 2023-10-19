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

#ifndef TP_CLAUSEID_H
#define TP_CLAUSEID_H

#include "topform.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from clauseid.c */

int clause_ids_assigned(void);

void assign_clause_id(Topform c);

void unassign_clause_id(Topform c);

Topform find_clause_by_id(int id);

void fprint_clause_id_tab(FILE *fp);

void p_clause_id_tab();

Plist insert_clause_into_plist(Plist p, Topform c, BOOL increasing);

BOOL clause_plist_member(Plist p, Topform c, BOOL increasing);

#endif  /* conditional compilation of whole file */
