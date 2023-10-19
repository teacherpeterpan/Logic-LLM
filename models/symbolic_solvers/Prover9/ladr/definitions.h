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

#ifndef TP_DEFINITIONS_H
#define TP_DEFINITIONS_H

#include "formula.h"
#include "topform.h"
#include "clauseid.h"
#include "just.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from definitions.c */

BOOL is_definition(Formula f);

Formula expand_with_definition(Formula f, Formula def);

void process_definitions(Plist formulas,
			 Plist *results,
			 Plist *defs,
			 Plist *rewritten);

void expand_with_definitions(Plist formulas,
			     Plist defs,
			     Plist *results,
			     Plist *rewritten);

void separate_definitions(Plist formulas,
			  Plist *defs,
			  Plist *nondefs);

#endif  /* conditional compilation of whole file */
