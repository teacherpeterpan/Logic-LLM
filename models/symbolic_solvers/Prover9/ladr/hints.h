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

#ifndef TP_HINTS_H
#define TP_HINTS_H

#include "subsume.h"
#include "clist.h"
#include "backdemod.h"
#include "resolve.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from hints.c */

void init_hints(Uniftype utype,
		int bsub_wt_attr,
		BOOL collect_labels,
		BOOL back_demod_hints,
		void (*demod_proc) (Topform, int, int, BOOL, BOOL));

void done_with_hints(void);

int redundant_hints(void);

void index_hint(Topform c);

void unindex_hint(Topform c);

void adjust_weight_with_hints(Topform c,
			      BOOL degrade,
			      BOOL breadth_first_hints);

void keep_hint_matcher(Topform c);

void back_demod_hints(Topform demod, int type, BOOL lex_order_vars);

#endif  /* conditional compilation of whole file */
