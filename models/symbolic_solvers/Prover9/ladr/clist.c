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

#include "clist.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_CLIST_POS PTRS(sizeof(struct clist_pos))
static unsigned Clist_pos_gets, Clist_pos_frees;

#define PTRS_CLIST PTRS(sizeof(struct clist))
static unsigned Clist_gets, Clist_frees;

/*************
 *
 *   Clist_pos get_clist_pos()
 *
 *************/

static
Clist_pos get_clist_pos(void)
{
  Clist_pos p = get_cmem(PTRS_CLIST_POS);
  Clist_pos_gets++;
  return(p);
}  /* get_clist_pos */

/*************
 *
 *    free_clist_pos()
 *
 *************/

static
void free_clist_pos(Clist_pos p)
{
  free_mem(p, PTRS_CLIST_POS);
  Clist_pos_frees++;
}  /* free_clist_pos */

/*************
 *
 *   Clist get_clist()
 *
 *************/

static
Clist get_clist(void)
{
  Clist p = get_cmem(PTRS_CLIST);
  Clist_gets++;
  return(p);
}  /* get_clist */

/*************
 *
 *    free_clist()
 *
 *************/

static
void free_clist(Clist p)
{
  free_mem(p, PTRS_CLIST);
  Clist_frees++;
}  /* free_clist */

/*************
 *
 *   fprint_clist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clist package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_clist_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct clist_pos);
  fprintf(fp, "clist_pos (%4d)    %11u%11u%11u%9.1f K\n",
          n, Clist_pos_gets, Clist_pos_frees,
          Clist_pos_gets - Clist_pos_frees,
          ((Clist_pos_gets - Clist_pos_frees) * n) / 1024.);

  n = sizeof(struct clist);
  fprintf(fp, "clist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Clist_gets, Clist_frees,
          Clist_gets - Clist_frees,
          ((Clist_gets - Clist_frees) * n) / 1024.);

}  /* fprint_clist_mem */

/*************
 *
 *   p_clist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clist package.
*/

/* PUBLIC */
void p_clist_mem()
{
  fprint_clist_mem(stdout, TRUE);
}  /* p_clist_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   clist_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty Clist, which
is a doubly-linked list of pointers to clauses.
You give it a string (any length, which is copied), representing
the name of the list.  If don't wish to name the list, send NULL.
(You can name the list later with name_clist().)
*/

/* PUBLIC */
Clist clist_init(char *name)
{
  Clist p = get_clist();

  if (name == NULL)
    p->name = NULL;
  else
    p->name = new_str_copy(name);
  return p;
}  /* clist_init */

/*************
 *
 *   name_clist()
 *
 *************/

/* DOCUMENTATION
This routine names or renames a Clist.
The string you supply can be any length and is copied.
*/

/* PUBLIC */
void name_clist(Clist p, char *name)
{
  if (p->name != NULL)
    free(p->name);

  if (name == NULL)
    p->name = NULL;
  else
    p->name = new_str_copy(name);
}  /* name_clist */

/*************
 *
 *   clist_free()
 *
 *************/

/* DOCUMENTATION
This routine frees an empty Clist.  If the Clist is not empty,
nothing happens.
*/

/* PUBLIC */
void clist_free(Clist p)
{
  if (p->first == NULL) {
    if (p->name != NULL) {
      free(p->name);
      p->name = NULL;
    }
    free_clist(p);
  }
}  /* clist_free */

/*************
 *
 *     clist_append()
 *
 *************/

/* DOCUMENTATION
This routine appends a Topform to a Clist.
*/

/* PUBLIC */
void clist_append(Topform c, Clist l)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = NULL;
  p->prev = l->last;
  l->last = p;
  if (p->prev)
    p->prev->next = p;
  else
    l->first = p;
  l->length++;

}  /* clist_append */

/*************
 *
 *     clist_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Topform as the first member of a Clist.
*/

/* PUBLIC */
void clist_prepend(Topform c, Clist l)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->prev = NULL;
  p->next = l->first;
  l->first = p;
  if (p->next)
    p->next->prev = p;
  else
    l->last = p;
  l->length++;
    
}  /* clist_prepend */

/*************
 *
 *     clist_insert_before()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Topform before a given position in a Clist.
*/

/* PUBLIC */
void clist_insert_before(Topform c, Clist_pos pos)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = pos;
  p->prev = pos->prev;
  pos->prev = p;
  if (p->prev)
    p->prev->next = p;
  else
    pos->list->first = p;
  pos->list->length++;
    
}  /* clist_insert_before */

/*************
 *
 *     clist_insert_after()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Topform after a given position in a Clist.
*/

/* PUBLIC */
void clist_insert_after(Topform c, Clist_pos pos)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->prev = pos;
  p->next = pos->next;
  pos->next = p;
  if (p->next)
    p->next->prev = p;
  else
    pos->list->last = p;
  pos->list->length++;
    
}  /* clist_insert_after */

/*************
 *
 *     clist_remove()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from a Clist.  If the Topform occurs
more than once in the Clist, the most recently inserted occurrence
is removed.  A fatal error occurs if the Topform is not in the Clist.
*/

/* PUBLIC */
void clist_remove(Topform c, Clist l)
{
  Clist_pos p, prev;

  /* Find position from containment list of clause. */
  for (p = c->containers, prev = NULL;
       p && p->list != l;
       prev = p, p = p->nocc);
    
  if (!p)
    fatal_error("clist_remove: clause not in list");

  /* First update normal links. */
  if (p->prev)
    p->prev->next = p->next;
  else
    p->list->first = p->next;
  if (p->next)
    p->next->prev = p->prev;
  else
    p->list->last = p->prev;

    /* Now update containment links. */
  if (prev)
    prev->nocc = p->nocc;
  else
    c->containers = p->nocc;

  free_clist_pos(p);
  l->length--;
}  /* clist_remove */

/*************
 *
 *   clist_remove_all_clauses()
 *
 *************/

/* DOCUMENTATION
This routine removes all clauses from a clist.
The clauses are NOT deleted, even if they occur nowhere else.
*/

/* PUBLIC */
void clist_remove_all_clauses(Clist l)
{
  while (l->first)
    clist_remove(l->first->c, l);
}  /* clist_remove_all_clauses */

/*************
 *
 *     clist_remove_all()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from all lists in which it occurs.
The number of lists from which it was removed is returned.
*/

/* PUBLIC */
int clist_remove_all(Topform c)
{
  int i = 0;
  while (c->containers) {
    clist_remove(c, c->containers->list);
    i++;
  }
  return i;
}  /* clist_remove_all */

/*************
 *
 *     clist_member()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if a Topform is a member of a Clist.
*/

/* PUBLIC */
int clist_member(Topform c, Clist l)
{
  Clist_pos p;

  for (p = c->containers; p; p = p->nocc) {
    if (p->list == l)
      return TRUE;
  }
  return FALSE;
}  /* clist_member */

/*************
 *
 *    fprint_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) each clause in a Clist.
If the Clist has a non-empty name, say "usable", the
string "list(usable).\n" is printed first.
The string "end_of_list.\n" is always printed at the end.
*/

/* PUBLIC */
void fprint_clist(FILE *fp, Clist l)
{
  Clist_pos p;

  if (l->name != NULL)
    fprintf(fp, "list(%s).\n", l->name);

  for(p = l->first; p; p = p->next)
    fprint_clause(fp, p->c);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fprint_clist */

/*************
 *
 *    p_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) each clause in a Clist.
See fprint_clist().
*/

/* PUBLIC */
void p_clist(Clist l)
{
  fprint_clist(stdout, l);
}  /* p_clist */

/*************
 *
 *    clist_zap()
 *
 *************/

/* DOCUMENTATION
For each Topform (occurrence) in a Clist, remove it, and if it occurs
in no other Clist, call zap_topform() to delete the clause.
Then, free the Clist.
*/

/* PUBLIC */
void clist_zap(Clist l)
{
  Clist_pos p;
  Topform c;

  p = l->first;
  while (p) {
    c = p->c;
    p = p->next;
    clist_remove(c, l);
    if (c->containers == NULL)
      zap_topform(c);
  }
  clist_free(l);
}  /* clist_zap */

/*************
 *
 *     clist_check()
 *
 *************/

/* DOCUMENTATION
This routine checks the integrity of a Clist.  If any errors are
found, a message is sent to stdout.  This is used for debugging.
*/

/* PUBLIC */
void clist_check(Clist l)
{
  Clist_pos p;
  int n = 0;

  for (p = l->first; p; p = p->next) {
    n++;
    if (p->list != l)
      printf("clist_check error0\n");
    if (p->next) {
      if (p->next->prev != p)
	printf("clist_check error1\n");
    }
    else if (p != l->last)
      printf("clist_check error2\n");
    if (p->prev) {
      if (p->prev->next != p)
	printf("clist_check error3\n");
    }
    else if (p != l->first)
      printf("clist_check error4\n");
  }
  if (l->length != n)
    printf("clist_check error5\n");
}  /* clist_check */

/*************
 *
 *   clist_append_all()
 *
 *************/

/* DOCUMENTATION
Append each member of l2 to l1, then zap l2.  Do not refer to l2
after the call.
*/

/* PUBLIC */
void clist_append_all(Clist l1, Clist l2)
{
  Clist_pos p;
  for (p = l2->first; p != NULL; p = p->next)
    clist_append(p->c, l1);
  clist_zap(l2); /* This doesn't zap clauses, because they now occur in l1. */
}  /* clist_append_all */

/*************
 *
 *   clist_empty()
 *
 *************/

/* DOCUMENTATION
This function checks if a (non-NULL) Clist is empty.
*/

/* PUBLIC */
BOOL clist_empty(Clist lst)
{
  return lst->first == NULL;
}  /* clist_empty */

/*************
 *
 *   clist_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clist_length(Clist l)
{
  return l->length;
}  /* clist_length */

/*************
 *
 *   max_wt_in_clist()
 *
 *************/

/* DOCUMENTATION
Scan a Clist, and return the maximum clause weight seen.
*/

/* PUBLIC */
int max_wt_in_clist(Clist l)
{
  int max = INT_MIN;
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (p->c->weight > max)
      max = p->c->weight;
  return max;
}  /* max_wt_in_clist */

/*************
 *
 *   horn_clist()
 *
 *************/

/* DOCUMENTATION
Is every clause in the list a Horn clause?
*/

/* PUBLIC */
BOOL horn_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (!horn_clause(p->c->literals))
      return FALSE;
  return TRUE;
}  /* horn_clist */

/*************
 *
 *   unit_clist()
 *
 *************/

/* DOCUMENTATION
Is every clause in the list a unit clause?
*/

/* PUBLIC */
BOOL unit_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (!unit_clause(p->c->literals))
      return FALSE;
  return TRUE;
}  /* unit_clist */

/*************
 *
 *   equality_in_clist()
 *
 *************/

/* DOCUMENTATION
Does the list contain a clause with a positive equality literal?
*/

/* PUBLIC */
BOOL equality_in_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (contains_pos_eq(p->c->literals))
      return TRUE;
  return FALSE;
}  /* equality_in_clist */

/*************
 *
 *   neg_nonunit_in_clist()
 *
 *************/

/* DOCUMENTATION
Does the list contain a negative nonunit clause ?
*/

/* PUBLIC */
BOOL neg_nonunit_in_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (negative_clause(p->c->literals) &&
	number_of_literals(p->c->literals) > 1)
      return TRUE;
  return FALSE;
}  /* neg_nonunit_in_clist */

/*************
 *
 *   clauses_in_clist()
 *
 *************/

/* DOCUMENTATION
Return the intersection of Plist p of clauses and Clist c of clauses.
*/

/* PUBLIC */
Plist clauses_in_clist(Plist p, Clist l)
{
  Plist q;
  Plist result = NULL;
  for (q = p; q; q = q->next) {
    Topform c = q->v;
    if (clist_member(c, l))
      result = plist_append(result, c);
  }
  return result;
}  /* clauses_in_clist */

/*************
 *
 *   clist_swap()
 *
 *************/

/* DOCUMENTATION
Every clause occurs in a set (maybe empty) of Clists.  Given clauses A and B,
this routine puts A into the lists in which B occurs and vice versa.
*/

/* PUBLIC */
void clist_swap(Topform a, Topform b)
{
  Clist_pos p;
  /* Replace refs to a with refs to b. */
  for (p = a->containers; p; p = p->nocc)
    p->c = b;
  /* Replace refs to b with refs to a. */
  for (p = b->containers; p; p = p->nocc)
    p->c = a;
  /* Swap the containment lists. */
  p = a->containers;
  a->containers = b->containers;
  b->containers = p;
}  /* clist_swap */

/*************
 *
 *   clist_move_clauses()
 *
 *************/

/* DOCUMENTATION
Move all clauses from a to the end of b.
*/

/* PUBLIC */
void clist_move_clauses(Clist a, Clist b)
{
  while (a->first) {
    Topform c = a->first->c;
    clist_remove(c, a);
    clist_append(c, b);
  }
}  /* clist_move_clauses */

/*************
 *
 *   move_clist_to_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist move_clist_to_plist(Clist a)
{
  Plist p = NULL;
  while (a->first) {
    Topform c = a->first->c;
    clist_remove(c, a);
    p = plist_prepend(p, c);
  }
  clist_free(a);
  p = reverse_plist(p);
  return p;
}  /* move_clist_to_plist */

/*************
 *
 *   copy_clist_to_plist_shallow()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist copy_clist_to_plist_shallow(Clist a)
{
  Clist_pos pos;
  Plist p = NULL;
  for (pos = a->first; pos; pos = pos->next)
    p = plist_prepend(p, pos->c);
  return reverse_plist(p);
}  /* copy_clist_to_plist_shallow */

/*************
 *
 *   plist_to_clist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clist plist_to_clist(Plist p, char *name)
{
  Clist a = clist_init(name);
  Plist q;
  for (q = p; q; q = q->next)
    clist_append(q->v, a);
  zap_plist(p);
  return a;
}  /* plist_to_clist */

/*************
 *
 *   clist_reverse()
 *
 *************/

/* DOCUMENTATION
Reverse a Clist.
*/

/* PUBLIC */
void clist_reverse(Clist l)
{
  Clist_pos p = l->first;
  while (p) {
    Clist_pos next = p->next;
    p->next = p->prev;
    p->prev = next;
    p = next;
  }
  p = l->first;
  l->first = l->last;
  l->last = p;
}  /* clist_reverse */

/*************
 *
 *   pos_in_clist()
 *
 *************/

/* DOCUMENTATION
Return the Clist_pos of a Topform in a Clist.
 */

/* PUBLIC */
Clist_pos pos_in_clist(Clist lst, Topform c)
{
  Clist_pos p = c->containers;
  while (p && p->list != lst)
    p = p->nocc;
  return p;
}  /* pos_in_clist */
/*************
 *
 *   clist_append_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void clist_append_plist(Clist lst, Plist clauses)
{
  Plist p;
  for (p = clauses; p; p = p->next) {
    clist_append(p->v, lst);
  }
}  /* clist_append_plist */

/*************
 *
 *   prepend_clist_to_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist prepend_clist_to_plist(Plist p, Clist c)
{
  if (c == NULL)
    return p;
  else {
    Clist_pos a;
    for (a = c->last; a; a = a->prev)
      p = plist_prepend(p, a->c);
    return p;
  }
}  /* prepend_clist_to_plist */

/*************
 *
 *   clist_number_of_weight()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clist_number_of_weight(Clist lst, int weight)
{
  if (lst == NULL)
    return 0;
  else {
    int n = 0;
    Clist_pos a;
    for (a = lst->first; a; a = a->next) {
      if (a->c->weight == weight)
	n++;
    }
    return n;
  }
}  /* clist_number_of_weight */

/*************
 *
 *   compare_clause_ids()
 *
 *************/

static
Ordertype compare_clause_ids(Topform c, Topform d)
{
  if (c->id < d->id)
    return LESS_THAN;
  else if (c->id > d->id)
    return GREATER_THAN;
  else
    return SAME_AS;
}  /* compare_clause_ids */

/*************
 *
 *   sort_clist_by_id()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sort_clist_by_id(Clist lst)
{
  int n = clist_length(lst);
  Topform *a = malloc(n * sizeof(Topform *));
  int i;
  Clist_pos p;

  for (p = lst->first, i = 0; p; p = p->next, i++)
    a[i] = p->c;

  clist_remove_all_clauses(lst);

  merge_sort((void **) a, n, (Ordertype (*)(void*, void*)) compare_clause_ids);

  for (i = 0; i < n; i++)
    clist_append(a[i], lst);
  
  free(a);
}  /* sort_clist_by_id */

/*************
 *
 *   neg_clauses_in_clist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist neg_clauses_in_clist(Clist a)
{
  Clist_pos p;
  Plist negs = NULL;
  for (p = a->first; p; p = p->next) {
    if (negative_clause(p->c->literals))
      negs = plist_prepend(negs, p->c);
  }
  negs = reverse_plist(negs);
  return negs;
}  /* neg_clauses_in_clist */

/*************
 *
 *   fprint_clause_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a Clist of clauses in standard prefix form.
*/

/* PUBLIC */
void fprint_clause_clist(FILE *fp, Clist lst)
{
  Clist_pos p;
    
  if (lst->name == NULL || str_ident(lst->name, ""))
    fprintf(fp, "\nclauses(anonymous).\n");
  else
    fprintf(fp, "\nclauses(%s).\n", lst->name);
	  
  for (p = lst->first; p != NULL; p = p->next)
    fprint_clause(fp, p->c);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fprint_clause_clist */

