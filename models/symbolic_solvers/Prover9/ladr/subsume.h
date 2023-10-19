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

#ifndef TP_SUBSUME_H
#define TP_SUBSUME_H

#include "parautil.h"
#include "lindex.h"
#include "features.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from subsume.c */

int nonunit_subsumption_tests(void);

BOOL subsumes(Topform c, Topform d);

BOOL subsumes_bt(Topform c, Topform d);

Topform forward_subsume(Topform d, Lindex idx);

Plist back_subsume(Topform c, Lindex idx);

Topform back_subsume_one(Topform c, Lindex idx);

void unit_conflict_by_index(Topform c, Lindex idx, void (*empty_proc) (Topform));

Topform try_unit_conflict(Topform a, Topform b);

void unit_delete(Topform c, Lindex idx);

Plist back_unit_del_by_index(Topform unit, Lindex idx);

void simplify_literals(Topform c);

BOOL eq_removable_literal(Topform c, Literals lit);

void simplify_literals2(Topform c);

#endif  /* conditional compilation of whole file */
