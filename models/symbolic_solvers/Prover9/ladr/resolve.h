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

#ifndef TP_RESOLVE_H
#define TP_RESOLVE_H

#include "clash.h"
#include "lindex.h"

/* INTRODUCTION
*/

/* Public definitions */

enum {  /* literal selection */
  LIT_SELECTION_NONE,
  LIT_SELECTION_MAXIMAL,
  LIT_SELECTION_ALL
};

enum {  /* types of resolution (binary, hyper, UR) */
  POS_RES,  /* positive */
  NEG_RES,  /* negative */
  ANY_RES   /* unrestricted by sign */
};

/* End of public definitions */

/* Public function prototypes from resolve.c */


void resolution_options(BOOL ordered,
			BOOL check_instances,
			BOOL initial_nuclei,
			int ur_nucleus_limit,
			BOOL production_mode);

int res_instance_prunes();

void hyper_resolution(Topform c, int pos_or_neg, Lindex idx,
		      void (*proc_proc) (Topform));

void ur_resolution(Topform c, int target_constraint, Lindex idx,
		   void (*proc_proc) (Topform));

Topform instantiate_clause(Topform c, Context subst);

void binary_resolution(Topform c,
		       int res_type,  /* POS_RES, NEG_RES, ANY_RES */
		       Lindex idx,
		       void (*proc_proc) (Topform));

void binary_factors(Topform c, void (*proc_proc) (Topform));

void merge_literals(Topform c);

Topform copy_inference(Topform c);

Topform resolve2(Topform c1, int n1, Topform c2, int n2, BOOL renumber_vars);

Topform resolve3(Topform c1, Literals l1, Topform c2, Literals l2, BOOL renumber_vars);

Topform xx_resolve2(Topform c, int n, BOOL renumber_vars);

#endif  /* conditional compilation of whole file */
