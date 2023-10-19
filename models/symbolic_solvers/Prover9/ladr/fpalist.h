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

#ifndef TP_FPALIST_H
#define TP_FPALIST_H

#include "term.h"

/* INTRODUCTION
*/

/* Public definitions */

/* We use the Term ID to order FPA lists. */

#define FPA_ID_TYPE unsigned
#define FPA_ID_MAX UINT_MAX
#define FPA_ID(t) (((Term) t)->u.id)

/*
  I experimented with using the address of the term as the
  term ID for ordering FPA lists.  Although not technically
  legal in C (because addresses in different arrays (malloced blocks)
  are compared), I believe it works correctly on all modern systems.
  (It didn't work in DOS.)
  However, there is a practical problem.
  On some systems, malloc() returns addresses in increasing order,
  and on others, they are decreasing, giving answers to queries in
  the reverse order, causing different searches.

  define FPA_ID_TYPE Term
  define FPA_ID_MAX ((Term) ULONG_MAX) //
  define FPA_ID(t) ((Term) t)
*/

#define FLT(x,y) (FPA_ID(x) <  FPA_ID(y))
#define FGT(x,y) (FPA_ID(x) >  FPA_ID(y))
#define FLE(x,y) (FPA_ID(x) <= FPA_ID(y))
#define FGE(x,y) (FPA_ID(x) >= FPA_ID(y))

#define FTERM(p) ((p).f == NULL ? NULL : (p).f->d[(p).i])

/* FPA lists */

typedef struct fpa_chunk *Fpa_chunk;
typedef struct fpa_list *Fpa_list;

struct fpa_chunk {
  int size;         /* size of array */
  Term *d;          /* array for chunk */
  int n;            /* current number of items in chunk (right justified) */
  Fpa_list head;    /* beginning of list to which this chunk belongs */
  Fpa_chunk next;   /* list of chunks is singly-linked */
};

struct fpa_list {
  Fpa_chunk chunks;
  int num_chunks;
  int chunksize;
  int num_terms;
};

/* to maintain a position in an FPA list while traversing for set operations */

struct fposition {
  Fpa_chunk f;
  int i;
};

/* End of public definitions */

/* Public function prototypes from fpalist.c */

Fpa_list get_fpa_list();

void fprint_fpalist_mem(FILE *fp, BOOL heading);

void p_fpalist_mem();

void fpalist_insert(Fpa_list p, Term t);

void fpalist_delete(Fpa_list p, Term t);

struct fposition first_fpos(Fpa_list f);

struct fposition next_fpos(struct fposition p);

void zap_fpa_chunks(Fpa_chunk p);

void zap_fpalist(Fpa_list p);

BOOL fpalist_empty(Fpa_list p);

void p_fpa_list(Fpa_chunk c);

#endif  /* conditional compilation of whole file */
