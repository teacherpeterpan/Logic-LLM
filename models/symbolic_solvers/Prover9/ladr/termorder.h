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

#ifndef TP_TERMORDER_H
#define TP_TERMORDER_H

#include "flatterm.h"

/* INTRODUCTION
*/

/* Public definitions */

/* Term ordering method */

typedef enum { LRPO_METHOD,
	       LPO_METHOD,
	       RPO_METHOD,
	       KBO_METHOD
             } Order_method;

/* End of public definitions */

/* Public function prototypes from termorder.c */

void assign_order_method(Order_method method);

Ordertype term_compare_basic(Term t1, Term t2);

Ordertype term_compare_ncv(Term t1, Term t2);

Ordertype term_compare_vcp(Term t1, Term t2);

Ordertype term_compare_vr(Term t1, Term t2);

Ordertype flatterm_compare_vr(Flatterm a, Flatterm b);

BOOL lrpo_multiset(Term t1, Term t2, BOOL lex_order_vars);

BOOL lrpo(Term s, Term t, BOOL lex_order_vars);

void init_kbo_weights(Plist weights);

int kbo_weight(Term t);

BOOL kbo(Term alpha, Term beta, BOOL lex_order_vars);

BOOL term_greater(Term alpha, Term beta, BOOL lex_order_vars);

Ordertype term_order(Term alpha, Term beta);

int flat_kbo_weight(Flatterm f);

BOOL flat_lrpo(Flatterm s, Flatterm t, BOOL lex_order_vars);

BOOL flat_greater(Flatterm alpha, Flatterm beta, BOOL lex_order_vars);

BOOL greater_multiset_current_ordering(Term t1, Term t2);

#endif  /* conditional compilation of whole file */
