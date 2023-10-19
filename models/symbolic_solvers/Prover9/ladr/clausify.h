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

#ifndef TP_CLAUSIFY_H
#define TP_CLAUSIFY_H

#include "topform.h"
#include "cnf.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from clausify.c */

Literals formula_to_literal(Formula f);

Literals formula_to_literals(Formula f);

Topform formula_to_clause(Formula f);

Plist formula_to_clauses(Formula f);

Plist clausify_formula(Formula f);

Formula clause_to_formula(Topform c);

#endif  /* conditional compilation of whole file */
