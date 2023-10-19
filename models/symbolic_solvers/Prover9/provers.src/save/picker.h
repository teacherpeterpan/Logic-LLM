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

#ifndef TP_PICKER_H
#define TP_PICKER_H

#include "search-structures.h"
#include "utilities.h"

/* INTRODUCTION
A *picker* is a method for selecting the given clause.  Examples
are an "age picker", which always picks the oldest clause, and
a "weight picker", which always picks the lightest clause.
See picker.c for details.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from picker.c */

void update_picker_ratios(Prover_options opt);

void init_pickers(Prover_options opt);

void update_pickers(Topform c, BOOL insert);

void insert_into_sos1(Topform c, Clist sos);

void remove_from_sos1(Topform c, Clist sos);

Topform get_given_clause1(Clist sos, int num_given,
			 Prover_options opt, char **type);

BOOL sos_keep1(Topform c, Clist sos, Prover_options opt);

void sos_displace1(Topform c,
		  void (*disable_proc) (Topform));

void zap_picker_indexes(void);

void picker_report(void);

#endif  /* conditional compilation of whole file */
