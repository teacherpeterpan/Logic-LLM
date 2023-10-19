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

#ifndef TP_MAXIMAL_H
#define TP_MAXIMAL_H

#include "literals.h"
#include "termorder.h"

/* INTRODUCTION
*/

/* Public definitions */

enum {  /* how to check for maximal literals */
  FLAG_CHECK,
  FULL_CHECK
};

/* End of public definitions */

/* Public function prototypes from maximal.c */

void init_maximal(void);

BOOL max_lit_test(Literals lits, Literals lit);

BOOL max_signed_lit_test(Literals lits, Literals lit);

void mark_maximal_literals(Literals lits);

BOOL maximal_literal(Literals lits, Literals lit, int check);

BOOL maximal_signed_literal(Literals lits, Literals lit, int check);

int number_of_maximal_literals(Literals lits, int check);

void mark_selected_literal(Literals lit);

void mark_selected_literals(Literals lits, char *selection);

BOOL selected_literal(Literals lit);

BOOL exists_selected_literal(Literals lits);

void copy_selected_literal_marks(Literals a, Literals b);

#endif  /* conditional compilation of whole file */
