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

#ifndef TP_BASIC_H
#define TP_BASIC_H

#include "unify.h"
#include "termflag.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from basic.c */

void init_basic_paramod(void);

void set_basic_paramod(BOOL flag);

BOOL basic_paramod(void);

void mark_term_nonbasic(Term t);

void mark_all_nonbasic(Term t);

BOOL nonbasic_term(Term t);

BOOL basic_term(Term t);

int nonbasic_flag(void);

Term apply_basic(Term t, Context c);

Term apply_basic_substitute(Term t, Term beta, Context c_from,
			    Term into_term, Context c_into);

void clear_all_nonbasic_marks(Term t);

void p_term_basic(Term t);

#endif  /* conditional compilation of whole file */
