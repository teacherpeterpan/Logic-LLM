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

#ifndef TP_CLAUSE_EVAL_H
#define TP_CLAUSE_EVAL_H

#include "topform.h"

/* INTRODUCTION
*/

/* Public definitions */

typedef struct clause_eval *Clause_eval;

/* End of public definitions */

/* Public function prototypes from clause_eval.c */

void zap_clause_eval_rule(Clause_eval p);

Clause_eval compile_clause_eval_rule(Term t);

BOOL eval_clause_in_rule(Topform c, Clause_eval p);

BOOL rule_contains_semantics(Clause_eval p);

#endif  /* conditional compilation of whole file */
