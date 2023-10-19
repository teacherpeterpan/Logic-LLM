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

#ifndef TP_WEIGHT_H
#define TP_WEIGHT_H

#include "literals.h"
#include "unify.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from weight.c */

void init_weight(Plist rules,
		 double variable_weight,
		 double constant_weight,
		 double not_weight,
		 double or_weight,
		 double sk_constant_weight,
		 double prop_atom_weight,
		 double nest_penalty,
		 double depth_penalty,
		 double var_penalty,
		 double complexity);

double weight(Term t, Context subst);

double clause_weight(Literals lits);

#endif  /* conditional compilation of whole file */
