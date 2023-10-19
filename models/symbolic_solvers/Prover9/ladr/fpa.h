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

#ifndef TP_FPA_H
#define TP_FPA_H

#include "unify.h"
#include "index.h"
#include "fpalist.h"

/* INTRODUCTION
This package implements FPA/Path indexing for first-order terms.
It is used to retrieive terms that are likely to unify (in various
ways) with a query term.  The answers are not guaranteed to unify,
and the caller must call some kind of unification routine to
check and to construct a substitution.
<P>
The unification types are
FPA_UNIFY, FPA_INSTANCE, FPA_GENERALIZATION, FPA_VARIANT, and FPA_IDENTICAL.
Performance is very good for
FPA_INSTANCE, FPA_VARIANT, and FPA_IDENTICAL,
fair for FPA_UNIFY, and
poor for FPA_GENERALIZATION.
(Use Discrimination indexing for FPA_GENERALIZATION, e.g.,
forward demodulation and forward subsumption.)
<P>
Associative-commutative (AC) function symbols are handled
by simply ignoring all subterms of AC symbols (which can give
bad performance).
Commutative/symmetric (C) operations are handled.
*/

/* Public definitions */

/* #define FPA_DEBUG */

typedef struct fpa_index * Fpa_index;
typedef struct fpa_state * Fpa_state;

/* End of public definitions */

/* Public function prototypes from fpa.c */

void fprint_fpa_mem(FILE *fp, BOOL heading);

void p_fpa_mem();

void fprint_path(FILE *fp, Ilist p);

void p_path(Ilist p);

void fprint_fpa_index(FILE *fp, Fpa_index idx);

void p_fpa_index(Fpa_index idx);

Fpa_index fpa_init_index(int depth);

void fpa_update(Term t, Fpa_index idx, Indexop op);

void fprint_fpa_state(FILE *fp, Fpa_state q, int depth);

void p_fpa_state(Fpa_state q);

void p_fpa_query(Term t, Querytype query_type, Fpa_index idx);

Term fpa_next_answer(Fpa_state q);

Term fpa_first_answer(Term t, Context c, Querytype query_type,
		      Fpa_index idx, Fpa_state *ppos);

void fpa_cancel(Fpa_state q);

void zap_fpa_index(Fpa_index idx);

BOOL fpa_empty(Fpa_index idx);

void p_fpa_density(Fpa_index idx);

unsigned mega_next_calls(void);

#endif  /* conditional compilation of whole file */
