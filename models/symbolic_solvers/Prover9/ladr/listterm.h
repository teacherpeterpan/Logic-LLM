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

#ifndef TP_LISTTERM_H
#define TP_LISTTERM_H

#include "term.h"

/* INTRODUCTION
This package has routines for managing binary-tree lists built
from Terms, which allows you to do some LISPy and Prology things.
However, you must be careful to keep track of any terms that are
shared and to recycle your own garbage.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from listterm.c */

Term get_nil_term();

Term listterm_cons(Term t1, Term t2);

BOOL cons_term(Term t);

BOOL nil_term(Term t);

BOOL proper_listterm(Term t);

Term listterm_append(Term list, Term element);

int listterm_length(Term t);

Term listterm_i(Term lst, int i);

BOOL listterm_member(Term t, Term lst);

Plist listterm_to_tlist(Term t);

void listterm_zap(Term t);

Term listterm_reverse(Term t);

#endif  /* conditional compilation of whole file */
