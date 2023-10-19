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

#ifndef TP_UTILITIES_H
#define TP_UTILITIES_H

#include "search-structures.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from utilities.c */

void print_memory_stats(FILE *fp);

void fsym_report(Ilist fsyms, Plist clauses);

BOOL inverse_order(Clist clauses);

void p_sym_list(Ilist syms);

void symbol_order(Clist usable, Clist sos, Clist demods, BOOL echo);

Ilist unary_symbols(Ilist a);

void auto_kbo_weights(Clist usable, Clist sos);

int neg_pos_depth_difference(Plist sos);

void structure_of_clauses(Clist clauses);

int plist_size_of_diff(Plist a, Plist b);

void structure_of_variables(Clist clauses);

Ordertype clause_compare_m4(Topform a, Topform b);

int bogo_ticks(void);

Topform first_negative_clause(Plist proof);

Plist neg_clauses_and_descendants(Plist proof,
				  Clist a_list, Clist b_list, Clist c_list);

Plist neg_descendants(Topform top_neg,
		      Clist a_list, Clist b_list, Clist c_list);
		      

void check_constant_sharing(Plist clauses);

#endif  /* conditional compilation of whole file */
