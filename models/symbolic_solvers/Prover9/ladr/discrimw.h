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

#ifndef TP_DISCRIMW_H
#define TP_DISCRIMW_H

#include "discrim.h"

/* INTRODUCTION
Discrimination tree indexing in which all variables
variables are represented as wildcards in the index.
Matching occurs after retrieval of candidates.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from discrimw.c */

void fprint_discrimw_mem(FILE *fp, BOOL heading);

void p_discrimw_mem(void);

void fprint_discrim_wild_index(FILE *fp, Discrim d);

void p_discrim_wild_index(Discrim d);

void discrim_wild_update(Term t, Discrim root, void *object, Indexop op);

void *discrim_wild_retrieve_first(Term t, Discrim root,
				  Discrim_pos *ppos);

void *discrim_wild_retrieve_next(Discrim_pos pos);

void discrim_wild_cancel(Discrim_pos pos);

#endif  /* conditional compilation of whole file */
