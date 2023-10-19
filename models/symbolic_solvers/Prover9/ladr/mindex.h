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

#ifndef TP_MINDEX_H
#define TP_MINDEX_H

#include "fpa.h"
#include "discrimb.h"
#include "discrimw.h"
#include "btu.h"
#include "btm.h"

/* INTRODUCTION
This is an indexing/unification package to store terms and to
retrieve unifiable terms.
(It is really just an interface to the various indexing and
unification packages.)
Several types of indexing and unification are supported.
When you allocate an index (with mindex_init()), you specify the
type of index and the type of unification.
<P>
Types of Retrieval
<P>
<TT>UNIFY, INSTANCE, GENERALIZATION, VARIANT, IDENTICAL</TT>.
<P>
Types of Unification
<P>
Associative-commutative (AC) and commutative/symmetric (C) symbols
are supported in most cases.  If you have any AC or C symbols,
you must use backtrack unification, which handles more than one
unifier for a pair of terms; otherwise, you can use ordinary unification,
which assumes at most one unifier for a pair of terms.
(For the empty theory, ordinary unification is a bit
faster than backtrack unification.)
<P>
Types of Indexing
<UL>
<LI><TT>LINEAR       :</TT> This is indexing without an index.
The terms you insert are put on a list, and each is tested in turn.
This supports AC and C symbols.
<LI><TT>FPA          :</TT> This is FPA/Path indexing.
Supports AC symbols (in a primitive way, by treating them
as constants) and C symbols.
<LI><TT>DISCRIM_WILD :</TT> Discrimination indexing,
where matching is separate from unidexing.
This supports AC symbols and C symbols.
With C symbols, you can get duplicate answers.
Supports <TT>GENERALIZATION</TT> retrieval only.
<LI><TT>DISCRIM_BIND :</TT> Discrimination indexing, where matching
occurs during indexing. This supports C symbols, <I>but not AC symbols</I>.
Supports <TT>GENERALIZATION</TT> retrieval only.
</UL>
*/

/* Public definitions */

/* types of index */

typedef enum { LINEAR,
	       FPA,
	       DISCRIM_WILD,
	       DISCRIM_BIND
             } Mindextype;

/* types of unification */

typedef enum { ORDINARY_UNIF,
	       BACKTRACK_UNIF
             } Uniftype;

typedef struct mindex * Mindex;
typedef struct mindex_pos * Mindex_pos;

struct mindex {
  Mindextype index_type;
  Uniftype   unif_type;

  /* FPA */
  Fpa_index  fpa;

  /* LINEAR */
  Plist   linear_first;
  Plist   linear_last;

  /* DISCRIM_WILD and DISCRIM_BIND */
  Discrim   discrim_tree;

  Mindex     next;  /* for avail list */
};

/* End of public definitions */

/* Public function prototypes from mindex.c */

void fprint_mindex_mem(FILE *fp, BOOL heading);

void p_mindex_mem();

Mindex mindex_init(Mindextype mtype, Uniftype utype, int fpa_depth);

BOOL mindex_empty(Mindex mdx);

void mindex_free(Mindex mdx);

void mindex_destroy(Mindex mdx);

void mindex_update(Mindex mdx, Term t, Indexop op);

Term mindex_retrieve_first(Term t, Mindex mdx, Querytype qtype,
			   Context query_subst, Context found_subst,
			   BOOL partial_match,
			   Mindex_pos *ppos);

Term mindex_retrieve_next(Mindex_pos pos);

void mindex_retrieve_cancel(Mindex_pos pos);

void fprint_mindex(FILE *fp, Mindex mdx);

#endif  /* conditional compilation of whole file */
