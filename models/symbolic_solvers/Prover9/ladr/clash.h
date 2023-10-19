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

#ifndef TP_CLASH_H
#define TP_CLASH_H

#include "mindex.h"
#include "parautil.h"

/* INTRODUCTION
This package deals with clash structures, which are used for
binary resolution, hyperresolution, and UR-resolution.
The inference rule sets up a clash list (corresponding
to the nucleus), and then calls clash() to compute all of the
resolvents.
*/

/* Public definitions */

typedef struct clash  * Clash;

struct clash {
  BOOL       clashable;
  BOOL       clashed;
  BOOL       flipped;  /* Is nuc_lit or sat_lit a flipped equality? */
  Literals   nuc_lit;
  Context    nuc_subst;
  Literals   sat_lit;
  Context    sat_subst;
  Mindex     mate_index;
  Mindex_pos mate_pos;
  Clash      next;
};

/* End of public definitions */

/* Public function prototypes from clash.c */

void fprint_clash_mem(FILE *fp, BOOL heading);

void p_clash_mem();

Clash append_clash(Clash p);

void  zap_clash(Clash p);

Literals atom_to_literal(Term atom);

Literals apply_lit(Literals lit, Context c);

void clash(Clash c,
	   BOOL (*sat_test) (Literals),
	   Just_type rule,
	   void (*proc_proc) (Topform));

#endif  /* conditional compilation of whole file */
