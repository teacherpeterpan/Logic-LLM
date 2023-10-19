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

#ifndef TP_CLIST_H
#define TP_CLIST_H

#include "topform.h"

/* INTRODUCTION
This package handles Clists, which are doubly-linked lists of (pointers
to) clauses.  This is the "official" way of building lists of clauses.
(If you need a temporary, singly linked list, you can use Plist instead.)
<P>
An important property of Clists is that each clause knows what Clists
it is on.  In particular, each clause has a (singly linked) list of
containing Clists, constructed from the same nodes as the ordinary Clist
(see the definition of struct clist_pos).
*/

/* Public definitions */

typedef struct clist_pos * Clist_pos;
typedef struct clist * Clist;

struct clist {
  char       *name;
  Clist_pos  first, last;
  int        length;
};

struct clist_pos {
  Clist_pos  prev, next;  /* previous and next member of Clist */
  Clist_pos  nocc;        /* next member of containment list */
  Clist      list;        /* the head of the list */
  Topform     c;          /* pointer to the clause */
};

/* End of public definitions */

/* Public function prototypes from clist.c */

void fprint_clist_mem(FILE *fp, BOOL heading);

void p_clist_mem();

Clist clist_init(char *name);

void name_clist(Clist p, char *name);

void clist_free(Clist p);

void clist_append(Topform c, Clist l);

void clist_prepend(Topform c, Clist l);

void clist_insert_before(Topform c, Clist_pos pos);

void clist_insert_after(Topform c, Clist_pos pos);

void clist_remove(Topform c, Clist l);

void clist_remove_all_clauses(Clist l);

int clist_remove_all(Topform c);

int clist_member(Topform c, Clist l);

void fprint_clist(FILE *fp, Clist l);

void p_clist(Clist l);

void clist_zap(Clist l);

void clist_check(Clist l);

void clist_append_all(Clist l1, Clist l2);

BOOL clist_empty(Clist lst);

int clist_length(Clist l);

int max_wt_in_clist(Clist l);

BOOL horn_clist(Clist l);

BOOL unit_clist(Clist l);

BOOL equality_in_clist(Clist l);

BOOL neg_nonunit_in_clist(Clist l);

Plist clauses_in_clist(Plist p, Clist l);

void clist_swap(Topform a, Topform b);

void clist_move_clauses(Clist a, Clist b);

Plist move_clist_to_plist(Clist a);

Plist copy_clist_to_plist_shallow(Clist a);

Clist plist_to_clist(Plist p, char *name);

void clist_reverse(Clist l);

Clist_pos pos_in_clist(Clist lst, Topform c);

void clist_append_plist(Clist lst, Plist clauses);

Plist prepend_clist_to_plist(Plist p, Clist c);

int clist_number_of_weight(Clist lst, int weight);

void sort_clist_by_id(Clist lst);

Plist neg_clauses_in_clist(Clist a);

void fprint_clause_clist(FILE *fp, Clist lst);

#endif  /* conditional compilation of whole file */
