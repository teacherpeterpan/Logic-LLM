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

#include "tlist.h"

/* Private definitions and types */

/*************
 *
 *   zap_tlist()
 *
 *************/

/* DOCUMENTATION
Zap each term in the list and free the list.  If you want to free the
list only, without zapping the terms, use zap_plist(p) instead.
*/

/* PUBLIC */
void zap_tlist(Plist p)
{
  if (p != NULL) {
    zap_tlist(p->next);
    zap_term(p->v);
    free_plist(p);
  }
}  /* zap_tlist */

/*************
 *
 *   tlist_remove()
 *
 *************/

/* DOCUMENTATION
Remove a term from a Plist.  Term_ident() is used, and the
term in the list is zapped.
*/

/* PUBLIC */
Plist tlist_remove(Term t, Plist p)
{
  if (p == NULL)
    return NULL;
  else {
    p->next = tlist_remove(t, p->next);

    if (term_ident(t, p->v)) {
      Plist next = p->next;
      zap_term(p->v);
      free_plist(p);
      return next;
    }
    else
      return p;
  }
}  /* tlist_remove */

/*************
 *
 *   tlist_union()
 *
 *************/

/* DOCUMENTATION
Return the union of two Plists of terms.  If the inputs
are sets, the output is a set.  Do not refer the inputs
after the call.  Duplicates are zapped.
*/

/* PUBLIC */
Plist tlist_union(Plist a, Plist b)
{
  if (a == NULL)
    return b;
  else if (tlist_member(a->v, b)) {
    Plist c = tlist_union(a->next,b );
    zap_term(a->v);
    free_plist(a);
    return c;
  }
  else {
    a->next = tlist_union(a->next, b);
    return a;
  }
}  /* tlist_union */

/*************
 *
 *   constants_in_term()
 *
 *************/

/* DOCUMENTATION
Given a term t, return a plist containing the set of constants in t.
*/

/* PUBLIC */
Ilist constants_in_term(Term t, Ilist p)
{
  if (VARIABLE(t))
    return p;
  else if (CONSTANT(t)) {
    if (ilist_member(p, SYMNUM(t)))
      return p;
    else
      return ilist_prepend(p, SYMNUM(t));
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      p = constants_in_term(ARG(t, i), p);
    return p;
  }
}  /* constants_in_term */

/*************
 *
 *   tlist_copy()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist tlist_copy(Plist p)
{
  if (p == NULL)
    return NULL;
  else {
    Term t = p->v;
    Plist new = get_plist();
    new->v = copy_term(t);
    new->next = tlist_copy(p->next);
    return new;
  }
}  /* tlist_copy */

