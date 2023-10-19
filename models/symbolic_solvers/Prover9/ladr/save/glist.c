#include "glist.h"

/*
 * memory management
 */

static unsigned Ilist_gets, Ilist_frees;
static unsigned Plist_gets, Plist_frees;

#define BYTES_ILIST sizeof(struct ilist)
#define PTRS_ILIST BYTES_ILIST%BPP == 0 ? BYTES_ILIST/BPP : BYTES_ILIST/BPP + 1

#define BYTES_PLIST sizeof(struct plist)
#define PTRS_PLIST BYTES_PLIST%BPP == 0 ? BYTES_PLIST/BPP : BYTES_PLIST/BPP + 1

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

  n = BYTES_ILIST;
  fprintf(fp, "ilist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Ilist_gets, Ilist_frees,
          Ilist_gets - Ilist_frees,
          ((Ilist_gets - Ilist_frees) * n) / 1024.);

  n = BYTES_PLIST;
  fprintf(fp, "plist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Plist_gets, Plist_frees,
          Plist_gets - Plist_frees,
          ((Plist_gets - Plist_frees) * n) / 1024.);

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
/*************
 *
 *   ilist_cat()
 *
 *************/

/* DOCUMENTATION
Concatenate two Ilists and return the result.  The result is constructed
from the arguments, so not refer to either of the arguments after the call.
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
This routine frees a Ilist (the whole list).  If the list contains
any pointers, the things to which they point are not freed.
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
Return the members of p1 that are not in p2;
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
 *   ilist_intersect()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist ilist_intersect(Ilist p1, Ilist p2)
{
  if (p1 == NULL)
    return NULL;
  else {
    Ilist r = ilist_intersect(p1->next, p2);
    if (ilist_member(p2, p1->i))
      return ilist_prepend(r, p1->i);
    else
      return r;
  }
}  /* ilist_intersect */

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
}  /* plist_subset */

/*************
 *
 *   plist_cat()
 *
 *************/

/* DOCUMENTATION
Concatenate two Plists and return the result.  The result is constructed
from the arguments, so do not refer to either of the arguments after the call.
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
    fprintf(fp, "%s", p->next ? " " : ")");
  }
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
 *   alist_insert()
 *
 *************/

/* DOCUMENTATION
This is a quick and clean alist (association list) for integers.
Insert key/value pairs.  With assoc(key), retreive value.
If an key has more than one value, the most recent is returned.
It an key is not in the alist, INT_MIN is returned.
An alist can be freed with zap_ilist(alist).
*/

/* PUBLIC */
Ilist alist_insert(Ilist p, int key, int val)
{
  return ilist_prepend(ilist_prepend(p, val), key);
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
int assoc(Ilist p, int key)
{
  /* assume an even number of members */
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->next->i;
  else
    return assoc(p->next->next, key);
}  /* assoc */

/*************
 *
 *   alist2_insert()
 *
 *************/

/* DOCUMENTATION
This is a quick and clean alist2 (association list) for pairs of integers.
Insert key/<value-a,value-b> pairs.
With assoc2a(key), retreive value-a.
With assoc2b(key), retreive value-b.
If a key has more than one value pair, the most recent is returned.
It a key is not in the alist2, INT_MIN is returned.
An alist2 can be freed with zap_ilist(alist2).
*/

/* PUBLIC */
Ilist alist2_insert(Ilist p, int key, int a, int b)
{
  return ilist_prepend(ilist_prepend(ilist_prepend(p, b), a), key);
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
int assoc2a(Ilist p, int key)
{
  /* assume number of members is multiple of 3 */
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->next->i;
  else
    return assoc2a(p->next->next->next, key);
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
int assoc2b(Ilist p, int key)
{
  /* assume number of members is multiple of 3 */
  if (p == NULL)
    return INT_MIN;
  else if (p->i == key)
    return p->next->next->i;
  else
    return assoc2b(p->next->next->next, key);
}  /* assoc2a */

/*************
 *
 *   alist2_remove()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist alist2_remove(Ilist p, int key)
{
  /* assume number of members is multiple of 3 */

  if (p == NULL)
    return NULL;
  else {
    p->next->next->next = alist2_remove(p->next->next->next, key);
    if (p->i == key) {
      Ilist a = p;
      p = p->next->next->next;
      free_ilist(a->next->next);
      free_ilist(a->next);
      free_ilist(a);
    }
    return p;
  }
}  /* alist2_remove */

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

