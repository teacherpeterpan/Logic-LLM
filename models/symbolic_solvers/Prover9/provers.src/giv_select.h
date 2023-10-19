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

#ifndef TP_GIV_SELECT_H
#define TP_GIV_SELECT_H

#include "search-structures.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from giv_select.c */

void init_giv_select(Plist rules);

void insert_into_sos2(Topform c, Clist sos);

void remove_from_sos2(Topform c, Clist sos);

BOOL givens_available(void);

Topform get_given_clause2(Clist sos, int num_given,
			 Prover_options opt, char **type);

BOOL sos_keep2(Topform c, Clist sos, Prover_options opt);

void sos_displace2(void (*disable_proc) (Topform));

void zap_given_selectors(void);

void selector_report(void);

Term selector_rule_term(char *name, char *priority,
			char *order, char *rule, int part);

Plist selector_rules_from_options(Prover_options opt);

#endif  /* conditional compilation of whole file */
