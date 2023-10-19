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

#ifndef TP_PINDEX_H
#define TP_PINDEX_H

#include "clist.h"

/* INTRODUCTION
This package has code for indexing clauses that are to be retrieved in
pairs.  When a clause is inserted, its weight is given.  Retrieval
is by sum of the weights of the pair -- lowest first.  Say we have
clauses with weights 0--4.  Then pairs will be returned in this order:
<PRE>
(0,0)
(0,1)
(1,1)  (0,2)
(1,2)  (0,3)
(2,2)  (1,3)  (0,4)
(2,3)  (1,4)
(3,3)  (2,4)
(3,4)
(4,4)
</PRE>
Clauses can be inserted or deleted after retrieval has begun; the smallest
available pair will always be returned.  When the index is
initialized, the caller supplies a parameter N, and the actual
weight range for indexing will be 0...N-1.  If an inserted clause has
weight outside of this range, the weight will be changed to 0 or N-1.
<P>
This is intended to be used for binary inference rules such as
paramodulation and resolution.
It is similar to the method in ``A Theorem-Proving Language
for Experimentation'' by Henschen, Overbeek, Wos, CACM 17(6), 1974.
*/

/* Public definitions */

typedef struct pair_index * Pair_index;

/* End of public definitions */

/* Public function prototypes from pindex.c */

void fprint_pindex_mem(FILE *fp, BOOL heading);

void p_pindex_mem();

Pair_index init_pair_index(int n);

void zap_pair_index(Pair_index p);

int pairs_exhausted(Pair_index p);

void insert_pair_index(Topform c, int wt, Pair_index p);

void delete_pair_index(Topform c, int wt, Pair_index p);

void retrieve_pair(Pair_index p, Topform *cp1, Topform *cp2);

int pair_already_used(Topform c1, int weight1,
		      Topform c2, int weight2,
		      Pair_index p);

#endif  /* conditional compilation of whole file */
