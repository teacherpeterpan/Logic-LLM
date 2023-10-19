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

#include "listterm.h"

/* Private definitions and types */

#define CONS_SYM  "$cons"
#define NIL_SYM   "$nil"

/*************
 *
 *   get_nil_term()
 *
 *************/

/* DOCUMENTATION
Allocate and return an empty listterm.
*/

/* PUBLIC */
Term get_nil_term()
{
  return get_rigid_term(NIL_SYM, 0);
}  /* get_nil_term */

/*************
 *
 *   listterm_cons()
 *
 *************/

/* DOCUMENTATION
This routine returns the "cons" of two terms.
The two terms are not copied.
*/

/* PUBLIC */
Term listterm_cons(Term t1, Term t2)
{
  Term c = get_rigid_term(CONS_SYM, 2);
  ARG(c,0) = t1;
  ARG(c,1) = t2;
  return c;
}  /* listterm_cons */

/*************
 *
 *   cons_term()
 *
 *************/

/* DOCUMENTATION
This function checks if a term is a "cons", that is,
arity 2 with the official "cons" symbol.   
*/

/* PUBLIC */
BOOL cons_term(Term t)
{
  return is_symbol(SYMNUM(t), CONS_SYM, 2);
}  /* cons_term */

/*************
 *
 *   nil_term()
 *
 *************/

/* DOCUMENTATION
This function checks if a term is a "nil", that is,
arity 0 with the official "nil" symbol.   
*/

/* PUBLIC */
BOOL nil_term(Term t)
{
  return is_symbol(SYMNUM(t), NIL_SYM, 0);
}  /* nil_term */

/*************
 *
 *   proper_listterm()
 *
 *************/

/* DOCUMENTATION
This function checks if a term is a proper listterm,
that is, a nil_term(), or a cons_term() whose tail
is a proper_listterm().
*/

/* PUBLIC */
BOOL proper_listterm(Term t)
{
  if (nil_term(t))
    return TRUE;
  else if (cons_term(t))
    return proper_listterm(ARG(t,1));
  else
    return FALSE;
}  /* proper_listterm */

/*************
 *
 *   listterm_append()
 *
 *************/

/* DOCUMENTATION
This routine appends an element to a listterm.  The resulting
listterm is returned.  Neither the list nor the element is copied.
You should not refer to the argument "list" after calling this
routine---a good way to call it is like this:
<PRE>list = listterm_append(list, element)</PRE>
<P>
If "list" is not a proper_listterm(), the result will be
well-formed, but it might not be what you expect.
*/

/* PUBLIC */
Term listterm_append(Term list, Term element)
{
  if (!cons_term(list))
    return listterm_cons(element, list);
  else {
    ARG(list,1) = listterm_append(ARG(list,1), element);
    return list;
  }
}  /* listterm_append */

/*************
 *
 *   listterm_length()
 *
 *************/

/* DOCUMENTATION
This function returns the length of a listterm.
*/

/* PUBLIC */
int listterm_length(Term t)
{
  if (!cons_term(t))
    return 0;
  else {
    return 1 + listterm_length(ARG(t,1));
  }
}  /* listterm_length */

/*************
 *
 *   listterm_i()
 *
 *************/

/* DOCUMENTATION
Return the i-th member, counting from 1, of a listterm.
If there are less than i members, return NULL.
*/

/* PUBLIC */
Term listterm_i(Term lst, int i)
{
  if (!cons_term(lst))
    return NULL;
  else if (i == 1)
    return ARG(lst,0);
  else
    return listterm_i(ARG(lst,1), i-1);
}  /* listterm_i */

/*************
 *
 *   listterm_member()
 *
 *************/

/* DOCUMENTATION
This function checks if Term t is a member of a listterm (Term lst).
*/

/* PUBLIC */
BOOL listterm_member(Term t, Term lst)
{
  if (cons_term(lst)) {
    if (term_ident(t, ARG(lst,0)))
      return TRUE;
    else
      return listterm_member(t, ARG(lst,1));
  }
  else
    return FALSE;
}  /* listterm_member */

/*************
 *
 *   listterm_to_tlist()
 *
 *************/

/* DOCUMENTATION
Given a proper listterm (e.g, [a,b,c]), return a Plist
of the members.  The members are not copied.
*/

/* PUBLIC */
Plist listterm_to_tlist(Term t)
{
  if (!proper_listterm(t))
    return NULL;
  else {
    Plist p = NULL;
    while (cons_term(t)) {
      p = plist_append(p, ARG(t, 0));
      t = ARG(t,1);
    }
    return p;
  }
}  /* listterm_to_tlist */

/*************
 *
 *   listterm_zap()
 *
 *************/

/* DOCUMENTATION
Free a list structure, but do not free its members.
*/

/* PUBLIC */
void listterm_zap(Term t)
{
  if (!cons_term(t))
    zap_term(t);
  else {
    listterm_zap(ARG(t,1));
    free_term(t);
  }
}  /* listterm_zap */

/*************
 *
 *   rev2()
 *
 *************/

static
Term rev2(Term t, Term done)
{
  if (!cons_term(t))
    return done;
  else
    return rev2(ARG(t,1), listterm_cons(ARG(t,0), done));
}  /* rev2 */

/*************
 *
 *   listterm_reverse()
 *
 *************/

/* DOCUMENTATION
Reverse a listterm.  A new list structure is created, but
the members are not copied.  The old list structure is freed.
*/

/* PUBLIC */
Term listterm_reverse(Term t)
{
  Term reversed = rev2(t, get_nil_term());
  listterm_zap(t);
  return reversed;
}  /* listterm_reverse */

