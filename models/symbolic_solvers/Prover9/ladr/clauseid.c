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

#include "clauseid.h"

/* Private definitions and types */

#define CLAUSE_ID_TAB_SIZE  50000

static Plist     Topform_id_tab[CLAUSE_ID_TAB_SIZE];
static unsigned  Topform_id_count = 0;

/*************
 *
 *   next_clause_id()
 *
 *************/

static
int next_clause_id(void)
{
  return ++Topform_id_count;
}  /* next_clause_id */

/*************
 *
 *   clause_ids_assigned()
 *
 *************/

/* DOCUMENTATION
What is the most recently assigned clause ID?
*/

/* PUBLIC */
int clause_ids_assigned(void)
{
  return Topform_id_count;
}  /* clause_ids_assigned */

/*************
 *
 *   assign_clause_id(c)
 *
 *************/

/* DOCUMENTATION
This routine assigns a unique identifier to the id field of a clause.
It also inserts the clause into a hash table so that given an id
number, the corresponding clause can be retrieved quickly (see
find_clause_by_id()).
*/

/* PUBLIC */
void assign_clause_id(Topform c)
{
  int i;

  if (c->id > 0) {
    p_clause(c);
    fatal_error("assign_clause_id, clause already has ID.");
  }
  c->id = next_clause_id();
  i = c->id % CLAUSE_ID_TAB_SIZE;
  Topform_id_tab[i] = insert_clause_into_plist(Topform_id_tab[i], c, TRUE);
  c->official_id = 1;
}  /* assign_clause_id */

/*************
 *
 *     unassign_clause_id(c)
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from the ID hash table and resets
the ID of the clause to 0.  A fatal error occurs if the clause
has not been assigned an ID.
*/

/* PUBLIC */
void unassign_clause_id(Topform c)
{
  if (c->official_id) {
    int i;
    Plist p1, p2;

    i = c->id % CLAUSE_ID_TAB_SIZE;
    for (p2 = NULL, p1 = Topform_id_tab[i];
	 p1 && ((Topform) p1->v)->id < c->id;
	 p2 = p1, p1 = p1->next);
    if (p1 == NULL || ((Topform) p1->v)->id != c->id) {
      p_clause(c);
      fatal_error("unassign_clause_id, cannot find clause.");
    }
    if (p2)
      p2->next = p1->next;
    else
      Topform_id_tab[i] = p1->next;
    c->id = 0;
  free_plist(p1);
  c->official_id = 0;
  }
}  /* unassign_clause_id */

/*************
 *
 *     find_clause_by_id(id)
 *
 *     Given a clause ID, retrieve the clause (or NULL).
 *
 *************/

/* DOCUMENTATION
This routine retrieves the clause with the given ID number
(or NULL, if there is no such clause).
*/

/* PUBLIC */
Topform find_clause_by_id(int id)
{
  int i;
  Plist p1;

  i = id % CLAUSE_ID_TAB_SIZE;
  for (p1 = Topform_id_tab[i];
       p1 && ((Topform) p1->v)->id < id;
       p1 = p1->next);
  if (p1 != NULL && ((Topform) p1->v)->id == id)
    return(p1->v);
  else
    return(NULL);
}  /* find_clause_by_id */

/*************
 *
 *     fprint_clause_id_tab(fp)
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) all the clauses in the ID hash table.
*/

/* PUBLIC */
void fprint_clause_id_tab(FILE *fp)
{
  int i;
  Plist p;

  fprintf(fp, "\nID clause table:\n");
  for (i=0; i<CLAUSE_ID_TAB_SIZE; i++)
    for (p = Topform_id_tab[i]; p; p = p->next)
      fprint_clause(fp, p->v);
  fflush(fp);
}  /* fprint_clause_id_tab */

/*************
 *
 *     p_clause_id_tab(tab)
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) all the clauses in the ID hash table.
*/

/* PUBLIC */
void p_clause_id_tab()
{
  fprint_clause_id_tab(stdout);
}  /* p_clause_id_tab */

/*************
 *
 *   insert_clause_into_plist()
 *
 *************/

/* DOCUMENTATION
This routine inserts a clause into a sorted (by ID) Plist of clauses.
Boolean paramemeter "increasing" tells whether the list is increasing
or decreasing.
The updated Plist is returned.
If the clause is already there, nothing happens.
*/

/* PUBLIC */
Plist insert_clause_into_plist(Plist p, Topform c, BOOL increasing)
{
  Plist prev, curr, new;
  prev = NULL;
  curr = p;
  while (curr != NULL && (increasing ? ((Topform) curr->v)->id < c->id
	                             : ((Topform) curr->v)->id > c->id)) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL || ((Topform) curr->v)->id != c->id) {
    new = get_plist();
    new->v = c;
    new->next = curr;
    if (prev != NULL)
      prev->next = new;
    else
      p = new;
  }
  return p;
}  /* insert_clause_into_plist */

/*************
 *
 *   clause_plist_member()
 *
 *************/

/* DOCUMENTATION
This routine checks if a clause occurs in a sorted (by ID) Plist of clauses.
Boolean paramemeter "increasing" tells whether the list is increasing
or decreasing.
*/

/* PUBLIC */
BOOL clause_plist_member(Plist p, Topform c, BOOL increasing)
{
  Plist curr = p;
  while (curr != NULL && (increasing ? ((Topform) curr->v)->id < c->id
	                             : ((Topform) curr->v)->id > c->id)) {
    curr = curr->next;
  }
  return (curr != NULL && ((Topform) curr->v)->id == c->id);
}  /* clause_plist_member */

