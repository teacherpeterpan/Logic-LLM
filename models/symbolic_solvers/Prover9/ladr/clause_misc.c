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

#include "clause_misc.h"

/* Private definitions and types */

/*
 * memory management
 */

/*************
 *
 *   clist_copy()
 *
 *************/

/* DOCUMENTATION
Copy a clist of clauses.  Justificatons and attributes are
copied, and the clauses get new IDs.
*/

/* PUBLIC */
Clist clist_copy(Clist a, BOOL assign_ids)
{
  Clist b = clist_init(a->name);
  Clist_pos p;
  for(p = a->first; p; p = p->next) {
    Topform c = copy_clause(p->c);
    c->justification = copy_justification(p->c->justification);
    c->attributes = copy_attributes(p->c->attributes);
    if (assign_ids)
      assign_clause_id(c);
    clist_append(c, b);
  }
  return b;
}  /* clist_copy */

/*************
 *
 *   copy_clauses_to_clist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clist copy_clauses_to_clist(Plist clauses, char *name, BOOL assign_ids)
{
  Clist b = clist_init(name);
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform x = p->v;
    Topform c = copy_clause(x);
    c->justification = copy_justification(x->justification);
    c->attributes = copy_attributes(x->attributes);
    if (assign_ids)
      assign_clause_id(c);
    clist_append(c, b);
  }
  return b;
}  /* copy_clauses_to_clist */

/*************
 *
 *   move_clauses_to_clist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clist move_clauses_to_clist(Plist clauses, char *name, BOOL assign_ids)
{
  Clist b = clist_init(name);
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    if (assign_ids && c->id == 0)
      assign_clause_id(c);
    clist_append(c, b);
  }
  return b;
}  /* move_clauses_to_clist */

/*************
 *
 *   input_clauses()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses, return the Plist of input clauses (in the
same order).
*/

/* PUBLIC */
Plist input_clauses(Plist a)
{
  if (a == NULL)
    return NULL;
  else {
    Plist c = input_clauses(a->next);
    if (has_input_just(a->v))
      return plist_prepend(c, a->v);
    else
      return c;
  }
}  /* input_clauses */

/*************
 *
 *   delete_clause()
 *
 *************/

/* DOCUMENTATION
This routine frees a clause and all of its subterms.
If the clause has an ID, it is unassigned.
If the clause has a justification list, it is freed.
<P>
This routine is not in the clause package, because
(at this time) the clause package doesn't know about
just.h or clauseid.h.
*/

/* PUBLIC */
void delete_clause(Topform c)
{
  zap_just(c->justification);  /* ok if NULL */
  unassign_clause_id(c);       /* ok if ID is not "official" */
  zap_topform(c);               /* zaps attributes */
}  /* delete_clause */

/*************
 *
 *   delete_clist()
 *
 *************/

/* DOCUMENTATION
For each Topform in the Clist, remove it from the Clist;
if it occurs in no other Clists, call delete_clause().
Finally, free the Clist.
<P>
This routine is not in the clist package, because
(at this time) the clist package doesn't know about
just.h or clauseid.h.
*/

/* PUBLIC */
void delete_clist(Clist l)
{
  Clist_pos p;
  Topform c;

  p = l->first;
  while (p) {
    c = p->c;
    p = p->next;
    clist_remove(c, l);
    if (c->containers == NULL)
      delete_clause(c);
  }
  clist_free(l);
}  /* delete_clist */

/*************
 *
 *   copy_clause_ija()
 *
 *************/

/* DOCUMENTATION
Copy a clause, including ID, justification, attributes, and
termflags.
Clauses constructed with this routine should be deallocated
with delete_clause().
*/

/* PUBLIC */
Topform copy_clause_ija(Topform c)
{
  Topform d = copy_clause_with_flags(c);
  d->id = c->id;
  d->justification = copy_justification(c->justification);
  d->attributes = copy_attributes(c->attributes);
  return d;
}  /* copy_clause_ija */

/*************
 *
 *   copy_clauses_ija()
 *
 *************/

/* DOCUMENTATION
Copy a Plist of clauses.  Clauses are coped with copy_clause_ija(),
which copies ID, justification, attributes, and termflags.
*/

/* PUBLIC */
Plist copy_clauses_ija(Plist p)
{
  Plist a;
  Plist b = NULL;   /* the new list */

  for (a = p; a; a = a->next) {
    Topform old = a->v;
    Topform new = copy_clause_ija(old);
    b = plist_prepend(b, new);  /* build it backward */
  }
  return reverse_plist(b);
}  /* copy_clauses_ija */

/*************
 *
 *   delete_clauses()
 *
 *************/

/* DOCUMENTATION
Delete (deep) a Pist of clauses.
*/

/* PUBLIC */
void delete_clauses(Plist p)
{
  if (p != NULL) {
    delete_clauses(p->next);
    delete_clause(p->v);
    free_plist(p);
  }
}  /* delete_clauses */

/*************
 *
 *   make_clause_basic()
 *
 *************/

/* DOCUMENTATION
This routine clears all of the "nonbasic" marks in a clause.
*/

/* PUBLIC */
void make_clause_basic(Topform c)
{
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next)
    clear_all_nonbasic_marks(lit->atom);
}  /* make_clause_basic */

