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

#ifndef TP_DISCRIMB_H
#define TP_DISCRIMB_H

#include "discrim.h"

/* INTRODUCTION
Discrimination tree indexing in which variables are
distinguished in the index and are bound
as soon as possible during retrieval.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from discrimb.c */

void fprint_discrimb_mem(FILE *fp, BOOL heading);

void p_discrimb_mem(void);

void check_discrim_bind_tree(Discrim d, int n);

void fprint_discrim_bind_index(FILE *fp, Discrim d);

void p_discrim_bind_index(Discrim d);

void discrim_bind_update(Term t, Discrim root, void *object, Indexop op);

void *discrim_bind_retrieve_first(Term t, Discrim root,
				  Context subst, Discrim_pos *ppos);

void *discrim_bind_retrieve_next(Discrim_pos pos);

void discrim_bind_cancel(Discrim_pos pos);

#endif  /* conditional compilation of whole file */
