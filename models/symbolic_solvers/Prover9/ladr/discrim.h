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

#ifndef TP_DISCRIM_H
#define TP_DISCRIM_H

#include "unify.h"
#include "index.h"

/* INTRODUCTION
This package implements two kinds of discrimination indexing
for first-order terms.  Both kinds support GENERALIZATION retrieval
only (e.g., for forward demodulation and forward subsumption).
<P>
The "wild" kind is an imperfect filter, and it does not bind variables.
The caller must also call a routine, say match(), to check if
the answers are really more general than the query term and to
construct the substitution.
Wild indexing supports associative-commutative (AC) and
commutative (C) symbols.
Indexing terms with AC symbols works by considering the number
of arguments and the number of nonvariable arguments of 
AC terms that do not occur in other AC terms.
(The term "wild" is used because all variables in the discrimination
tree are treated as the the wildcard symbol *).
<P>
With the "bind" kind, every answer is more general than the query term,
and the matching substitution is constructed during the retrieval.
Wild indexing supports commutative (C) symbols,
but it <I>does not support</I> associative-commutative (AC) symbols.
Retrieval with C symbols can produce duplicate answers.
<P>
There is probably a higher-level package (mindex ?) which
provides a uniform interface to these and other indexing methods.
*/
/* Public definitions */

typedef struct discrim * Discrim;

struct discrim {       /* node in a discrimination tree */
  Discrim   next;      /* sibling */
  union {
    Discrim kids;      /* for internal nodes */
    Plist data;        /* for leaves */
  } u;
  short symbol;        /* variable number or symbol number */
  char type;           /* term type and for ac indexing type */
};

typedef struct discrim_pos * Discrim_pos;

struct discrim_pos {  /* to save position in set of answers */
  void    *query;
  Context subst;        /* substitution */
  Plist   data;         /* identical terms from leaf of discrim tree */
  void    *backtrack;   /* data for backtracking */
};

/* type of discrimination tree node */

enum { DVARIABLE, DRIGID, AC_ARG_TYPE, AC_NV_ARG_TYPE };

#define DVAR(d)  ((d)->type == DVARIABLE)

/* End of public definitions */

/* Public function prototypes from discrim.c */

Discrim get_discrim(void);

void free_discrim(Discrim p);

Discrim_pos get_discrim_pos(void);

void free_discrim_pos(Discrim_pos p);

void fprint_discrim_mem(FILE *fp, BOOL heading);

void p_discrim_mem(void);

Discrim discrim_init(void);

void discrim_dealloc(Discrim d);

void destroy_discrim_tree(Discrim d);

BOOL discrim_empty(Discrim d);

#endif  /* conditional compilation of whole file */
