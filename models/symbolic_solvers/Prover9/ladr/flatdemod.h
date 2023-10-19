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

#ifndef TP_FLATDEMOD_H
#define TP_FLATDEMOD_H

#include "parautil.h"
#include "mindex.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from flatdemod.c */

Flatterm fapply_demod(Term t, Context c);

Plist discrim_flat_retrieve_leaf(Flatterm fin, Discrim root,
				 Context subst, Flatterm *ppos);

void *discrim_flat_retrieve_first(Flatterm f, Discrim root,
				  Context subst, Discrim_pos *ppos);

void *discrim_flat_retrieve_next(Discrim_pos pos);

void discrim_flat_cancel(Discrim_pos pos);

Term fdemodulate(Term t, Discrim root,
		 int *step_limit, int *increase_limit, int *sequence,
		 I3list *just_head, BOOL lex_order_vars);

int fdemod_attempts();

int fdemod_rewrites();

void fdemod_clause(Topform c, Mindex idx,
		   int *step_limit, int *increase_limit, BOOL lex_order_vars);

#endif  /* conditional compilation of whole file */
