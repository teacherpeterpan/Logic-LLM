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

#ifndef TP_SEMANTICS_H
#define TP_SEMANTICS_H

#include "../ladr/interp.h"
#include "../ladr/ioutil.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from semantics.c */

void init_semantics(Plist interp_terms, Clock eval_clock,
		    char *type, int eval_limit, int eval_var_limit);

void eval_in_interps(Topform c);

void set_semantics(Topform c);

void update_semantics_new_constant(Topform c);

#endif  /* conditional compilation of whole file */
