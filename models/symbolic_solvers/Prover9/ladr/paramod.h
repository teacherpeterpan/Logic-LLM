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

#ifndef TP_PARAMOD_H
#define TP_PARAMOD_H

#include "resolve.h"
#include "basic.h"

/* INTRODUCTION
This package has a paramodulation inference rule.
*/

/* Public definitions */

/* where to paramodulate into */

typedef enum { PARA_ALL,
	       PARA_ALL_EXCEPT_TOP,
	       PARA_TOP_ONLY } Para_loc;

/* End of public definitions */

/* Public function prototypes from paramod.c */


void paramodulation_options(BOOL ordered_inference,
			    BOOL check_instances,
			    BOOL positive_inference,
			    BOOL basic_paramodulation,
			    BOOL para_from_vars,
			    BOOL para_into_vars,
			    BOOL para_from_small);

int para_instance_prunes();

int basic_paramodulation_prunes(void);

Topform paramodulate(Literals from_lit, int from_side, Context from_subst,
		     Topform into_clause, Ilist into_pos, Context into_subst);

void para_from_into(Topform from, Context cf,
		    Topform into, Context ci,
		    BOOL check_top,
		    void (*proc_proc) (Topform));

Topform para_pos(Topform from_clause, Ilist from_pos,
		 Topform into_clause, Ilist into_pos);

Topform para_pos2(Topform from, Ilist from_pos, Topform into, Ilist into_pos);

#endif  /* conditional compilation of whole file */
