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

#ifndef TP_INDEX_LITS_H
#define TP_INDEX_LITS_H

#include "../ladr/clock.h"
#include "../ladr/subsume.h"
#include "../ladr/di_tree.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from index_lits.c */

void init_literals_index(void);

void destroy_literals_index(void);

void index_literals(Topform c, Indexop op, Clock clock, BOOL no_fapl);

void index_denial(Topform c, Indexop op, Clock clock);

void unit_conflict(Topform c, void (*empty_proc) (Topform));

void unit_deletion(Topform c);

Plist back_unit_deletable(Topform c);

Topform forward_subsumption(Topform d);

Plist back_subsumption(Topform c);

void lits_idx_report(void);

#endif  /* conditional compilation of whole file */
