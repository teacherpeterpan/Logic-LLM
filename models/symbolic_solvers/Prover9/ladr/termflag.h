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

#ifndef TP_TERMFLAG_H
#define TP_TERMFLAG_H

#include "term.h"

/* INTRODUCTION
*/

/* Public definitions */

#define TP_BIT(bits, flag)     (bits & flag)

/* End of public definitions */

/* Public function prototypes from termflag.c */

int claim_term_flag();

void release_term_flag(int bit);

void term_flag_set(Term t, int flag);

void term_flag_clear(Term t, int flag);

BOOL term_flag(Term t, int flag);

int term_flags();

Term copy_term_with_flags(Term t);

Term copy_term_with_flag(Term t, int flag);

void term_flag_clear_recursively(Term t, int flag);

#endif  /* conditional compilation of whole file */
