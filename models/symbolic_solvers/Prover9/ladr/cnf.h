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

#ifndef TP_CNF_H
#define TP_CNF_H

#include "formula.h"
#include "clock.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from cnf.c */

BOOL formula_ident_share(Formula f, Formula g);

Formula formula_copy_share(Formula f);

Formula cnf(Formula f);

Formula dnf(Formula f);

Formula skolemize(Formula f);

Formula unique_quantified_vars(Formula f);

Formula remove_universal_quantifiers(Formula f);

Formula clausify_prepare(Formula f);

Formula miniscope(Formula f);

Formula miniscope_formula(Formula f, unsigned mega_fid_call_limit);

int cnf_max_clauses(Formula f);

#endif  /* conditional compilation of whole file */
