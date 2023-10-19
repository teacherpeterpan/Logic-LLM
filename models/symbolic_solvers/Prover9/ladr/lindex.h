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

#ifndef TP_LINDEX_H
#define TP_LINDEX_H

#include "mindex.h"
#include "maximal.h"
#include "topform.h"

/* INTRODUCTION
This is a simple package that can be used when you need a pair
of indexes (Mindexes), one for positive literals, and one for negative
literals.  The name Lindex means "literal index".
<P>
When you allocate the Lindex (lindex_init()), you
specify the parameters as you would for two separate Mindexes.
When you insert into or delete from the Lindex, you give a clause,
and each literal of the clause is indexed: positive literals
go into the positive component of the Lindex, and negative literals
go into the negative component.
<P>
There are no retrieval operations in this package.  Instead, use
the retrieval operations from the Mindex package on the appropriate
member (positive or negative) of the pair.
<P>
See the package "mindex" for information on the initialization
parameters and retrieval operations.
*/

/* Public definitions */

typedef struct lindex * Lindex;

struct lindex {
  Mindex pos;   /* index for positive literals */
  Mindex neg;   /* index for negative literals */
  Lindex next;  /* for avail list */
};

/* End of public definitions */

/* Public function prototypes from lindex.c */

void fprint_lindex_mem(FILE *fp, BOOL heading);

void p_lindex_mem();

Lindex lindex_init(Mindextype pos_mtype, Uniftype pos_utype, int pos_fpa_depth,
		   Mindextype neg_mtype, Uniftype neg_utype, int neg_fpa_depth);

void lindex_destroy(Lindex ldx);

void lindex_update(Lindex ldx, Topform c, Indexop op);

void lindex_update_first(Lindex ldx, Topform c, Indexop op);

BOOL lindex_empty(Lindex idx);

BOOL lindex_backtrack(Lindex idx);

#endif  /* conditional compilation of whole file */
