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

#include "glist.h"

/*
 * memory management
 */

#define PTRS_ILIST PTRS(sizeof(struct ilist))
static unsigned Ilist_gets, Ilist_frees;

#define PTRS_PLIST PTRS(sizeof(struct plist))
static unsigned Plist_gets, Plist_frees;

#define PTRS_I2LIST PTRS(sizeof(struct i2list))
static unsigned I2list_gets, I2list_frees;

#define PTRS_I3LIST PTRS(sizeof(struct i3list))
static unsigned I3list_gets, I3list_frees;

/*************
 *
 *   Ilist get_ilist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist get_ilist(void)
{
  Ilist p = get_mem(PTRS_ILIST);
  p->next = NULL;
  Ilist_gets++;
  return(p);
}  /* get_ilist */

/*************
 *
 *    free_ilist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_ilist(Ilist p)
{
  free_mem(p, PTRS_ILIST);
  Ilist_frees++;
}  /* free_ilist */

/*************
 *
 *   Plist get_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist get_plist(void)
{
  Plist p = get_mem(PTRS_PLIST);
  p->next = NULL;
  Plist_gets++;
  return(p);
}  /* get_plist */

/*************
 *
 *    free_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_plist(Plist p)
{
  free_mem(p, PTRS_PLIST);
  Plist_frees++;
}  /* free_plist */

/*************
 *
 *   I2list get_i2list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list get_i2list(void)
{
  I2list p = get_mem(PTRS_I2LIST);
  p->next = NULL;
  I2list_gets++;
  return(p);
}  /* get_i2list */

/*************
 *
 *    free_i2list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_i2list(I2list p)
{
  free_mem(p, PTRS_I2LIST);
  I2list_frees++;
}  /* free_i2list */

/*************
 *
 *   I3list get_i3list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I3list get_i3list(void)
{
  I3list p = get_mem(PTRS_I3LIST);
  p->next = NULL;
  I3list_gets++;
  return(p);
}  /* get_i3list */

/*************
 *
 *    free_i3list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_i3list(I3list p)
{
  free_mem(p, PTRS_I3LIST);
  I3list_frees++;
}  /* free_i3list */

/*************
 *
 *   fprint_glist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the glist package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_glist_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct ilist);
  fprintf(fp, "ilist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Ilist_gets, Ilist_frees,
          Ilist_gets - Ilist_frees,
          ((Ilist_gets - Ilist_frees) * n) / 1024.);

  n = sizeof(struct plist);
  fprintf(fp, "plist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Plist_gets, Plist_frees,
          Plist_gets - Plist_frees,
          ((Plist_gets - Plist_frees) * n) / 1024.);

  n = sizeof(struct i2list);
  fprintf(fp, "i2list (%4d)       %11u%11u%11u%9.1f K\n",
          n, I2list_gets, I2list_frees,
          I2list_gets - I2list_frees,
          ((I2list_gets - I2list_frees) * n) / 1024.);

}  /* fprint_glist_mem */

/*************
 *
 *   p_glist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the glist package.
*/

/* PUBLIC */
void p_glist_mem()
{
  fprint_glist_mem(stdout, TRUE);
}  /* p_glist_mem */

/*
 *  end of memory management
 */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*************
 *
 *   plist_cat()
 *
 *************/

/* DOCUMENTATION
Concatenate two Plists and return the result.  The result is constructed
from the arguments, so do not refer to either of the arguments after the call.
<P>
That is, both args are destroyed.
*/

/* PUBLIC */
Plist plist_cat(Plist p1, Plist p2)
{
  if (p1 == NULL)
    return p2;
  else if (p2 == NULL)
    return p1;
  else {
    Plist p = p1;
    while (p->next != NULL)
      p = p->next;
    p->next = p2;
    return p1;
  }
}  /* plist_cat */

/*************
 *
 *   plist_cat2()
 *
 *************/

/* DOCUMENTATION
Concatenate two Plists and return the result.
In this version, the second plist is copied and
placed at the end of p1.
<P>
That is, the first arg is destroyed, and the second
is preserved.
*/

/* PUBLIC */
Plist plist_cat2(Plist p1, Plist p2)
{
  return plist_cat(p1, copy_plist(p2));
}  /* plist_cat2 */

/*************
 *
 *   plist_pop()
 *
 *************/

/* DOCUMENTATION
This routine takes a nonempty Plist, removes and frees the first node
(ignoring the contents), and returns the remainder of the list.
*/

/* PUBLIC */
Plist plist_pop(Plist p)
{
  Plist q = p;
  p = p->next;
  free_plist(q);
  return p;
}  /* plist_pop */

/*************
 *
 *   plist_count()
 *
 *************/

/* DOCUMENTATION
This routine returns the length of a Plist.
*/

/* PUBLIC */
int plist_count(Plist p)
{
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}  /* plist_count */

/*************
 *
 *   rev_app_plist(p1, p2)
 *
 *************/

static
Plist rev_app_plist(Plist p1, Plist p2)
{
  Plist p3;

  if (p1 == NULL)
    return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_plist(p3, p1));
  }
}  /* rev_app_plist */

/*************
 *
 *     reverse_plist(p1)
 *
 *************/

/* DOCUMENTATION
This routine reverses a Plist.  The list is reversed in-place,
so you should not refer to the argument after calling this routine.
A good way to use it is like this:
<PRE>
p = reverse_plist(p);
</PRE>
*/

/* PUBLIC */
Plist reverse_plist(Plist p)
{
  return rev_app_plist(p, NULL);
}  /* reverse_plist */

/*************
 *
 *   copy_plist(p)
 *
 *************/

/* DOCUMENTATION
This routine copies a Plist (the whole list) and returns the copy.
*/

/* PUBLIC */
Plist copy_plist(Plist p)
{
  Plist start, p1, p2;

  start = NULL;
  p2 = NULL;
  for ( ; p; p = p->next) {
    p1 = get_plist();
    p1->v = p->v;
    p1->next = NULL;
    if (p2)
      p2->next = p1;
    else
      start = p1;
    p2 = p1;
  }
  return(start);
}  /* copy_plist */

/*************
 *
 *   zap_plist(p)
 *
 *************/

/* DOCUMENTATION
This routine frees a Plist (the whole list).  The things to which
the members point are not freed.
*/

/* PUBLIC */
void zap_plist(Plist p)
{
  Plist curr, prev;

  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_plist(prev);
  }
}  /* zap_plist */

/*************
 *
 *   plist_append()
 *
 *************/

/* DOCUMENTATION
This routine appends a pointer to a Plist.  The updated Plist
is returned.
*/

/* PUBLIC */
Plist plist_append(Plist lst, void *v)
{
  if (lst == NULL) {
    Plist g = get_plist();
    g->v = v;
    g->next = NULL;
    return g;
  }
  else {
    lst->next = plist_append(lst->next, v);
    return lst;
  }
}  /* plist_append */

/*************
 *
 *   plist_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts a pointer as the first member of a Plist.
The updated Plist is returned.
*/

/* PUBLIC */
Plist plist_prepend(Plist lst, void *v)
{
  Plist g = get_plist();
  g->v = v;
  g->next = lst;
  return g;
}  /* plist_prepend */

/*************
 *
 *   plist_member()
 *
 *************/

/* DOCUMENTATION
This function checks if a pointer is a member of a Plist.
*/

/* PUBLIC */
BOOL plist_member(Plist lst, void *v)
{
  if (lst == NULL)
    return FALSE;
  else if (lst->v == v)
    return TRUE;
  else
    return plist_member(lst->next, v);
}  /* plist_member */

/*************
 *
 *   plist_subtract()
 *
 *************/

/* DOCUMENTATION
Return the members of p1 that are not in p2.
<P>
The arguments are not changed.
*/

/* PUBLIC */
Plist plist_subtract(Plist p1, Plist p2)
{
  if (p1 == NULL)
    return NULL;
  else {
    Plist r = plist_subtract(p1->next, p2);
    if (plist_member(p2, p1->v))
      return r;
    else
      return plist_prepend(r, p1->v);
  }
}  /* plist_subtract */

/*************
 *
 *   plist_subset()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL plist_subset(Plist a, Plist b)
{
  if (a == NULL)
    return TRUE;
  else if (!plist_member(b, a->v))
    return FALSE;
  else
    return plist_subset(a->next, b);
}  /* plist_subset */

/*************
 *
 *   plist_remove()
 *
 *************/

/* DOCUMENTATION
Remove the first occurrence of a pointer from a Plist.
*/

/* PUBLIC */
Plist plist_remove(Plist p, void *v)
{
  if (p == NULL)
    fatal_error("plist_remove: pointer not found");

  if (p->v == v) {
    Plist x = p;
    p = p->next;
    free_plist(x);
    return p;
  }
  else {
    p->next = plist_remove(p->next, v);
    return p;
  }
}  /* plist_remove */

/*************
 *
 *   plist_remove_string()
 *
 *************/

/* DOCUMENTATION
Remove the first occurrence of a pointer from a Plist.
*/

/* PUBLIC */
Plist plist_remove_string(Plist p, char *s)
{
  if (p == NULL)
    fatal_error("plist_remove_string: pointer not found");

  if (str_ident(p->v, s)) {
    Plist x = p;
    p = p->next;
    free_plist(x);
    return p;
  }
  else {
    p->next = plist_remove_string(p->next, s);
    return p;
  }
}  /* plist_remove_string */

/*************
 *
 *   sort_plist()
 *
 *************/

/* DOCUMENTATION
 */

/* PUBLIC */
Plist sort_plist(Plist objects,	Ordertype (*comp_proc) (void *, void *))
{
  int n = plist_count(objects);
  void **a = malloc(n * sizeof(void *));
  int i;
  Plist p;
  for (p = objects, i = 0; p; p = p->next, i++)
    a[i] = p->v;
  merge_sort(a, n, comp_proc);
  for (p = objects, i = 0; p; p = p->next, i++)
    p->v = a[i];
  free(a);
  return objects;
}  /* sort_plist */

/*************
 *
 *   plist_remove_last()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist plist_remove_last(Plist p)
{
  if (p == NULL)
    return NULL;
  else if (p->next == NULL) {
    free_plist(p);
    return NULL;
  }
  else {
    p->next = plist_remove_last(p->next);
    return p;
  }
}  /* plist_remove_last */

/*************
 *
 *   position_of_string_in_plist()
 *
 *************/

/* DOCUMENTATION
Count from 1; return -1 if the string is not in the Plist.
*/

/* PUBLIC */
int position_of_string_in_plist(char *s, Plist p)
{
  if (p == NULL)
    return -1;
  else if (str_ident(s, p->v))
    return 1;
  else {
    int pos = position_of_string_in_plist(s, p->next);
    if (pos == -1)
      return -1;
    else
      return pos+1;
  }
}  /* position_of_string_in_plist */

/*************
 *
 *   string_member_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL string_member_plist(char *s, Plist p)
{
  return position_of_string_in_plist(s, p) >= 0;
}  /* string_member_plist */

/*************
 *
 *   longest_string_in_plist()
 *
 *************/

/* DOCUMENTATION
Return -1 if the Plist is empty.
*/

/* PUBLIC */
int longest_string_in_plist(Plist p)
{
  if (p == NULL)
    return -1;
  else {
    int n1 = strlen(p->v);
    int n2 = longest_string_in_plist(p->next);
    return IMAX(n1,n2);
  }
}  /* longest_string_in_plist */

/*************
 *
 *   ith_in_plist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void *ith_in_plist(Plist p, int i)
{
  if (p == NULL || i <= 0)
    return NULL;
  else if (i == 1)
    return p->v;
  else
    return ith_in_plist(p->next, i-1);
}  /* ith_in_plist */

/*************
 *
 *   plist_last()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void *plist_last(Plist p)
{
  if (p == NULL)
    return NULL;
  else if (p->next == NULL)
    return p->v;
  else
    return plist_last(p->next);
}  /* plist_last */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*************
 *
 *   ilist_cat()
 *
 *************/

/* DOCUMENTATION
Concatenate two Ilists and return the result.  The result is constructed
from the arguments, so do not refer to either of the arguments after the call.
<P>
That is, both arguments are "used up".
*/

/* PUBLIC */
Ilist ilist_cat(Ilist p1, Ilist p2)
{
  if (p1 == NULL)
    return p2;
  else if (p2 == NULL)
    return p1;
  else {
    Ilist p = p1;
    while (p->next != NULL)
      p = p->next;
    p->next = p2;
    return p1;
  }
}  /* ilist_cat */

/*************
 *
 *   ilist_cat2()
 *
 *************/

/* DOCUMENTATION
Concatenate two Ilists and return the result.
In this version, the second ilist is copied and
placed at the end of p1.  That is, p1 is "used up",
but p2 is not.
*/

/* PUBLIC */
Ilist ilist_cat2(Ilist p1, Ilist p2)
{
  if (p1 == NULL)
    return copy_ilist(p2);
  else {
    p1->next = ilist_cat2(p1->next, p2);
    return p1;
  }
}  /* ilist_cat2 */

/*************
 *
 *   ilist_pop()
 *
 *************/

/* DOCUMENTATION
This routine takes a nonempty Ilist, removes and frees the first node
(ignoring the contents), and returns the remainder of the list.
*/

/* PUBLIC */
Ilist ilist_pop(Ilist p)
{
  Ilist q = p;
  p = p->next;
  free_ilist(q);
  return p;
}  /* ilist_pop */

/*************
 *
 *   ilist_count()
 *
 *************/

/* DOCUMENTATION
This routine returns the length of a Ilist.
*/

/* PUBLIC */
int ilist_count(Ilist p)
{
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}  /* ilist_count */

/*************
 *
 *   rev_app_ilist(p1, p2)
 *
 *************/

static
Ilist rev_app_ilist(Ilist p1, Ilist p2)
{
  Ilist p3;

  if (p1 == NULL)
    return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_ilist(p3, p1));
  }
}  /* rev_app_ilist */

/*************
 *
 *     reverse_ilist(p1)
 *
 *************/

/* DOCUMENTATION
This routine reverses a Ilist.  The list is reversed in-place,
so you should not refer to the argument after calling this routine.
A good way to use it is like this:
<PRE>
p = reverse_ilist(p);
</PRE>
*/

/* PUBLIC */
Ilist reverse_ilist(Ilist p)
{
  return rev_app_ilist(p, NULL);
}  /* reverse_ilist */

/*************
 *
 *   copy_ilist(p)
 *
 *************/

/* DOCUMENTATION
This routine copies a Ilist (the whole list) and returns the copy.
*/

/* PUBLIC */
Ilist copy_ilist(Ilist p)
{
  Ilist start, p1, p2;

  start = NULL;
  p2 = NULL;
  for ( ; p; p = p->next) {
    p1 = get_ilist();
    p1->i = p->i;
    p1->next = NULL;
    if (p2)
      p2->next = p1;
    else
      start = p1;
    p2 = p1;
  }
  return(start);
}  /* copy_ilist */

/*************
 *
 *   zap_ilist(p)
 *
 *************/

/* DOCUMENTATION
This routine frees a Ilist (the whole list).
*/

/* PUBLIC */
void zap_ilist(Ilist p)
{
  Ilist curr, prev;

  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_ilist(prev);
  }
}  /* zap_ilist */

/*************
 *
 *   ilist_append()
 *
 *************/

/* DOCUMENTATION
This routine appends an integer to a Ilist.  The updated Ilist
is returned.
*/

/* PUBLIC */
Ilist ilist_append(Ilist lst, int i)
{
  if (lst == NULL) {
    Ilist g = get_ilist();
    g->i = i;
    g->next = NULL;
    return g;
  }
  else {
    lst->next = ilist_append(lst->next, i);
    return lst;
  }
}  /* ilist_append */

/*************
 *
 *   ilist_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts an integer as the first member of a Ilist.
The updated Ilist is returned.
*/

/* PUBLIC */
Ilist ilist_prepend(Ilist lst, int i)
{
  Ilist g = get_ilist();
  g->i = i;
  g->next = lst;
  return g;
}  /* ilist_prepend */

/*************
 *
 *   ilist_last()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist ilist_last(Ilist lst)
{
  if (lst == NULL)
    return NULL;
  else if (lst->next == NULL)
    return lst;
  else
    return ilist_last(lst->next);
}  /* ilist_last */

/*************
 *
 *   ilist_member()
 *
 *************/

/* DOCUMENTATION
This function checks if an integer is a member of a Ilist.
(If a node in the Ilist contains a pointer instead of an integer,
the result is undefined.)
*/

/* PUBLIC */
BOOL ilist_member(Ilist lst, int i)
{
  if (lst == NULL)
    return FALSE;
  else if (lst->i == i)
    return TRUE;
  else
    return ilist_member(lst->next, i);
}  /* ilist_member */

/*************
 *
 *   ilist_subtract()
 *
 *************/

/* DOCUMENTATION
Return the members of p1 that are not in p2.
<P>
The arguments are not changed.
*/

/* PUBLIC */
Ilist ilist_subtract(Ilist p1, Ilist p2)
{
  if (p1 == NULL)
    return NULL;
  else {
    Ilist r = ilist_subtract(p1->next, p2);
    if (ilist_member(p2, p1->i))
      return r;
    else
      return ilist_prepend(r, p1->i);
  }
}  /* ilist_subtract */

/*************
 *
 *   ilist_removeall()
 *
 *************/

/* DOCUMENTATION
Remove all occurrences of i.
<P>
The argument is "used up".
*/

/* PUBLIC */
Ilist ilist_removeall(Ilist p, int i)
{
  if (p == NULL)
    return NULL;
  else if (p->i == i) {
    Ilist r = p->next;
    free_ilist(p);
    return ilist_removeall(r, i);
  }
  else {
    p->next = ilist_removeall(p->next, i);
    return p;
  }
}  /* ilist_removeall */

/*************
 *
 *   ilist_intersect()
 *
 *************/

/* DOCUMENTATION
Construct the intersection (as a new Ilist).
<P>
The arguments are not changed.
*/

/* PUBLIC */
Ilist ilist_intersect(Ilist a, Ilist b)
{
  if (a == NULL)
    return NULL;
  else {
    Ilist r = ilist_intersect(a->next, b);
    if (ilist_member(b, a->i))
      return ilist_prepend(r, a->i);
    else
      return r;
  }
}  /* ilist_intersect */

/*************
 *
 *   ilist_union()
 *
 *************/

/* DOCUMENTATION
Construct the union (as a new Ilist).
<p>
The arguments need not be sets, the result is a set.
<p>
The arguments are not changed.
*/

/* PUBLIC */
Ilist ilist_union(Ilist a, Ilist b)
{
  if (a == NULL)
    return ilist_set(b);  /* copies members of b (does not change b) */
  else if (ilist_member(b, a->i))
    return ilist_union(a->next, b);
  else
    return ilist_prepend(ilist_union(a->next, b), a->i);
}  /* ilist_union */

/*************
 *
 *   ilist_set()
 *
 *************/

/* DOCUMENTATION
Take a list of integers and remove duplicates.
This creates a new list and leave the old list as it was.
Don't make any assumptions about the order of the result.
*/

/* PUBLIC */
Ilist ilist_set(Ilist m)
{
  if (m == NULL)
    return NULL;
  else {
    Ilist s = ilist_set(m->next);
    if (ilist_member(s, m->i))
      return s;
    else
      return ilist_prepend(s, m->i);
  }
}  /* ilist_set */

/*************
 *
 *   ilist_rem_dups()
 *
 *************/

/* DOCUMENTATION
Take a list of integers and remove duplicates.
<P>
This version "uses up" the argument.
*/

/* PUBLIC */
Ilist ilist_rem_dups(Ilist m)
{
  if (m == NULL)
    return NULL;
  else {
    Ilist s = ilist_rem_dups(m->next);
    if (ilist_member(s, m->i)) {
      free_ilist(m);
      return s;
    }
    else {
      m->next = s;
      return m;
    }
  }
}  /* ilist_rem_dups */

/*************
 *
 *   ilist_is_set()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL ilist_is_set(Ilist a)
{
  if (a == NULL)
    return TRUE;
  else if (ilist_member(a->next, a->i))
    return FALSE;
  else
    return ilist_is_set(a->next);
}  /* ilist_is_set */

/*************
 *
 *   ilist_subset()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL ilist_subset(Ilist a, Ilist b)
{
  if (a == NULL)
    return TRUE;
  else if (!ilist_member(b, a->i))
    return FALSE;
  else
    return ilist_subset(a->next, b);
}  /* ilist_subset */

/*************
 *
 *   fprint_ilist()
 *
 *************/

/* DOCUMENTATION
The list of integers is printed to FILE *fp like this: (4 5 1 3),
without a newline.
*/

/* PUBLIC */
void fprint_ilist(FILE *fp, Ilist p)
{
  fprintf(fp, "(");
  for (; p != NULL; p = p->next) {
    fprintf(fp, "%d", p->i);
    fprintf(fp, "%s", p->next ? " " : "");
  }
  fprintf(fp, ")");
  fflush(fp);
}  /* fprint_ilist */

/*************
 *
 *   p_ilist()
 *
 *************/

/* DOCUMENTATION
The list of integers is printed to stdout like this: (4 5 1 3),
with a '\n' at the end.
*/

/* PUBLIC */
void p_ilist(Ilist p)
{
  fprint_ilist(stdout, p);
  printf("\n");
  fflush(stdout);
}  /* p_ilist */

/*************
 *
 *   ilist_copy()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist ilist_copy(Ilist p)
{
  if (p == NULL)
    return NULL;
  else {
    Ilist new = get_ilist();
    new->i = p->i;
    new->next = ilist_copy(p->next);
    return new;
  }
}  /* ilist_copy */

/*************
 *
 *   ilist_remove_last()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist ilist_remove_last(Ilist p)
{
  if (p == NULL)
    return NULL;
  else if (p->next == NULL) {
    free_ilist(p);
    return NULL;
  }
  else {
    p->next = ilist_remove_last(p->next);
    return p;
  }
}  /* ilist_remove_last */

/*************
 *
 *   ilist_occurrences()
 *
 *************/

/* DOCUMENTATION
How many times does an integer occur in an ilist?
*/

/* PUBLIC */
int ilist_occurrences(Ilist p, int i)
{
  if (p == NULL)
    return 0;
  else
    return ilist_occurrences(p->next, i) + (p->i == i ? 1 : 0);
}  /* ilist_occurrences */

/*************
 *
 *   ilist_insert_up()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist ilist_insert_up(Ilist p, int i)
{
  if (p == NULL || p->i >= i) {
    Ilist g = get_ilist();
    g->i = i;
    g->next = p;
    return g;
  }
  else {
    p->next = ilist_insert_up(p->next, i);
    return p;
  }
}  /* ilist_insert_up */

/*************
 *
 *   position_in_ilist()
 *
 *************/

/* DOCUMENTATION
Count from 1; return -1 if the int is not in the Ilist.
*/

/* PUBLIC */
int position_in_ilist(int i, Ilist p)
{
  if (p == NULL)
    return -1;
  else if (p->i == i)
    return 1;
  else {
    int pos = position_in_ilist(i, p->next);
    if (pos == -1)
      return -1;
    else
      return pos+1;
  }
}  /* position_in_ilist */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*************
 *
 *   zap_i2list(p)
 *
 *************/

/* DOCUMENTATION
This routine frees an I2list (the whole list).
*/

/* PUBLIC */
void zap_i2list(I2list p)
{
  I2list curr, prev;

  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_i2list(prev);
  }
}  /* zap_i2list */

/*************
 *
 *   i2list_append()
 *
 *************/

/* DOCUMENTATION
This routine appends an integer to a I2list.  The updated I2list
is returned.
*/

/* PUBLIC */
I2list i2list_append(I2list lst, int i, int j)
{
  if (lst == NULL) {
    I2list g = get_i2list();
    g->i = i;
    g->j = j;
    g->next = NULL;
    return g;
  }
  else {
    lst->next = i2list_append(lst->next, i, j);
    return lst;
  }
}  /* i2list_append */

/*************
 *
 *   i2list_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts an integer triple as the first member of a I2list.
The updated I2list is returned.
*/

/* PUBLIC */
I2list i2list_prepend(I2list lst, int i, int j)
{
  I2list g = get_i2list();
  g->i = i;
  g->j = j;
  g->next = lst;
  return g;
}  /* i2list_prepend */

/*************
 *
 *   i2list_removeall()
 *
 *************/

/* DOCUMENTATION
Remove all occurrences of i.
<P>
The argument is "used up".
*/

/* PUBLIC */
I2list i2list_removeall(I2list p, int i)
{
  if (p == NULL)
    return NULL;
  else if (p->i == i) {
    I2list r = p->next;
    free_i2list(p);
    return i2list_removeall(r, i);
  }
  else {
    p->next = i2list_removeall(p->next, i);
    return p;
  }
}  /* i2list_removeall */

/*************
 *
 *   i2list_member()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list i2list_member(I2list lst, int i)
{
  if (lst == NULL)
    return NULL;
  else if (lst->i == i)
    return lst;
  else
    return i2list_member(lst->next, i);
}  /* i2list_member */

/*************
 *
 *   p_i2list()
 *
 *************/

/* DOCUMENTATION
The list of integers is printed to stdout like this: (4 5 1 3),
with a '\n' at the end.
*/

/* PUBLIC */
void p_i2list(I2list p)
{
  printf("(");
  for (; p != NULL; p = p->next) {
    printf("%d/%d", p->i, p->j);
    printf("%s", p->next ? " " : "");
  }
  printf(")\n");
  fflush(stdout);
}  /* p_i2list */

/*************
 *
 *   i2list_count()
 *
 *************/

/* DOCUMENTATION
This routine returns the length of a I2list.
*/

/* PUBLIC */
int i2list_count(I2list p)
{
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}  /* i2list_count */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*************
 *
 *   i3list_member()
 *
 *************/

/* DOCUMENTATION
This function checks if a triple of integers is a member of a I3list.
*/

/* PUBLIC */
BOOL i3list_member(I3list lst, int i, int j, int k)
{
  if (lst == NULL)
    return FALSE;
  else if (lst->i == i && lst->j == j && lst->k == k)
    return TRUE;
  else
    return i3list_member(lst->next, i, j, k);
}  /* i3list_member */

/*************
 *
 *   i3list_append()
 *
 *************/

/* DOCUMENTATION
This routine appends an integer to a I3list.  The updated I3list
is returned.
*/

/* PUBLIC */
I3list i3list_append(I3list lst, int i, int j, int k)
{
  if (lst == NULL) {
    I3list g = get_i3list();
    g->i = i;
    g->j = j;
    g->k = k;
    g->next = NULL;
    return g;
  }
  else {
    lst->next = i3list_append(lst->next, i, j, k);
    return lst;
  }
}  /* i3list_append */

/*************
 *
 *   i3list_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts an integer triple as the first member of a I3list.
The updated I3list is returned.
*/

/* PUBLIC */
I3list i3list_prepend(I3list lst, int i, int j, int k)
{
  I3list g = get_i3list();
  g->i = i;
  g->j = j;
  g->k = k;
  g->next = lst;
  return g;
}  /* i3list_prepend */

/*************
 *
 *   zap_i3list(p)
 *
 *************/

/* DOCUMENTATION
This routine frees a I3list (the whole list).
*/

/* PUBLIC */
void zap_i3list(I3list p)
{
  I3list curr, prev;

  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_i3list(prev);
  }
}  /* zap_i3list */

/*************
 *
 *   rev_app_i3list(p1, p2)
 *
 *************/

static
I3list rev_app_i3list(I3list p1, I3list p2)
{
  I3list p3;

  if (p1 == NULL)
    return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_i3list(p3, p1));
  }
}  /* rev_app_i3list */

/*************
 *
 *     reverse_i3list(p1)
 *
 *************/

/* DOCUMENTATION
This routine reverses a I3list.  The 3list is reversed in-place,
so you should not refer to the argument after calling this routine.
A good way to use it is like this:
<PRE>
p = reverse_i3list(p);
</PRE>
*/

/* PUBLIC */
I3list reverse_i3list(I3list p)
{
  return rev_app_i3list(p, NULL);
}  /* reverse_i3list */

/*************
 *
 *   copy_i3list(p)
 *
 *************/

/* DOCUMENTATION
This routine copies a I3list (the whole 3list) and returns the copy.
*/

/* PUBLIC */
I3list copy_i3list(I3list p)
{
  I3list start, p1, p2;

  start = NULL;
  p2 = NULL;
  for ( ; p; p = p->next) {
    p1 = get_i3list();
    p1->i = p->i;
    p1->j = p->j;
    p1->k = p->k;
    p1->next = NULL;
    if (p2)
      p2->next = p1;
    else
      start = p1;
    p2 = p1;
  }
  return(start);
}  /* copy_i3list */

/*************
 *
 *   i3list_count()
 *
 *************/

/* DOCUMENTATION
This routine returns the length of a I3list.
*/

/* PUBLIC */
int i3list_count(I3list p)
{
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}  /* i3list_count */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*************
 *
 *   alist_insert()
 *
 *************/

/* DOCUMENTATION
Alists (association list) for integers.
Insert key/value pairs.  With assoc(key), retreive value.
If a key has more than one value, the most recent is returned.
It an key is not in the alist, INT_MIN is returned.
An alist can be freed with zap_i2list(alist).
This is not efficient, because no hashing is done; lookups are linear.
*/

/* PUBLIC */
I2list alist_insert(I2list p, int key, int val)
{
  return i2list_prepend(p, key, val);
}  /* alist_insert */

/*************
 *
 *   assoc()
 *
 *************/

/* DOCUMENTATION
See alist_insert.
*/

/* PUBLIC */
int assoc(I2list p, int key)
{
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->j;
  else
    return assoc(p->next, key);
}  /* assoc */

/*************
 *
 *   alist2_insert()
 *
 *************/

/* DOCUMENTATION
Alist2 (association list) for pairs of integers.
Insert key/<value-a,value-b> pairs.
With assoc2a(key), retreive value-a.
With assoc2b(key), retreive value-b.
If a key has more than one value pair, the most recent is returned.
It a key is not in the alist2, INT_MIN is returned.
An alist2 can be freed with zap_i3list(alist2).
*/

/* PUBLIC */
I3list alist2_insert(I3list p, int key, int a, int b)
{
  return i3list_prepend(p, key, a, b);
}  /* alist2_insert */

/*************
 *
 *   assoc2a()
 *
 *************/

/* DOCUMENTATION
See alist2_insert.
*/

/* PUBLIC */
int assoc2a(I3list p, int key)
{
  /* assume number of members is multiple of 3 */
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->j;
  else
    return assoc2a(p->next, key);
}  /* assoc2a */

/*************
 *
 *   assoc2b()
 *
 *************/

/* DOCUMENTATION
See alist2_insert.
*/

/* PUBLIC */
int assoc2b(I3list p, int key)
{
  /* assume number of members is multiple of 3 */
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->k;
  else
    return assoc2b(p->next, key);
}  /* assoc2a */

/*************
 *
 *   alist2_remove()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I3list alist2_remove(I3list p, int key)
{
  /* assume number of members is multiple of 3 */

  if (p == NULL)
    return NULL;
  else {
    p->next= alist2_remove(p->next, key);
    if (p->i == key) {
      I3list a = p;
      p = p->next;
      free_i3list(a);
    }
    return p;
  }
}  /* alist2_remove */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
  Multiset of integers is implemented with I2list, that is,
  list of pairs of integers: <value, number-of-occurrences>.
 */

/*************
 *
 *   i2list_multimember()
 *
 *************/

/* DOCUMENTATION
Is <i,n> a multimember of multiset b?
*/

/* PUBLIC */
BOOL i2list_multimember(I2list b, int i, int n)
{
  if (b == NULL)
    return FALSE;
  else if (i == b->i)
    return n <= b->j;
  else
    return i2list_multimember(b->next, i, n);
}  /* i2list_multimember */

/*************
 *
 *   i2list_multisubset()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL i2list_multisubset(I2list a, I2list b)
{
  if (a == NULL)
    return TRUE;
  else if (!i2list_multimember(b, a->i, a->j))
    return FALSE;
  else
    return i2list_multisubset(a->next, b);
}  /* i2list_multisubset */

/*************
 *
 *   multiset_add_n()
 *
 *************/

/* DOCUMENTATION
Add n occurrences of i to multiset a.
*/

/* PUBLIC */
I2list multiset_add_n(I2list a, int i, int n)
{
  if (a == NULL) {
    a = get_i2list();
    a->i = i;
    a->j = n;
  }
  else if (a->i == i)
    a->j += n;
  else
    a->next = multiset_add_n(a->next, i, n);
  return a;
}  /* multiset_add_n */

/*************
 *
 *   multiset_add()
 *
 *************/

/* DOCUMENTATION
Add 1 occurrence of i to multiset a.
*/

/* PUBLIC */
I2list multiset_add(I2list a, int i)
{
  return multiset_add_n(a, i, 1);
}  /* multiset_add */

/*************
 *
 *   multiset_union()
 *
 *************/

/* DOCUMENTATION
The result is constructed from the arguments,
so do not refer to either of the arguments after the call.
That is, both arguments are "used up".
*/

/* PUBLIC */
I2list multiset_union(I2list a, I2list b)
{
  I2list p;
  for (p = b; p; p = p->next)
    a = multiset_add_n(a, p->i, p->j);
  zap_i2list(b);
  return a;
}  /* multiset_union */

/*************
 *
 *   multiset_to_set()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist multiset_to_set(I2list m)
{
  if (m == NULL)
    return NULL;
  else {
    Ilist p = get_ilist();
    p->i = m->i;
    p->next = multiset_to_set(m->next);
    return p;
  }
}  /* multiset_to_set */

/*************
 *
 *   multiset_occurrences()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int multiset_occurrences(I2list m, int i)
{
  I2list a = i2list_member(m, i);
  return (a == NULL ? 0 : a->j);
}  /* multiset_occurrences */

