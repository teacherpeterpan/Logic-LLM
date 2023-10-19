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

#include "accanon.h"

/* Private definitions and types */

/*************
 *
 *    flatten - given an AC term, store arguments in an array
 *
 *    The index (*ip) must be initialized by the calling routine.
 *
 *************/

/* DOCUMENTATION
This routine
*/

/* PUBLIC */
void flatten(Term t, Term *a, int *ip)
{
  Term t1;
  int sn, i;

  sn = SYMNUM(t);
  for (i = 0; i < ARITY(t); i++) {
    t1 = ARG(t,i);
    if (SYMNUM(t1) == sn)
      flatten(t1, a, ip);
    else {
      if (*ip >= MAX_ACM_ARGS) {
	fprint_term(stdout, t);
	fatal_error("flatten, too many arguments.");
      }
      a[*ip] = t1;
      (*ip)++;
    }
  }
}  /* flatten */

/*************
 *
 *    right_associate(t)
 *
 *    Given a term (t) with a binary symbol, say f, right associate the
 *    the binary tree with respect to f.  Do only the top of the tree,
 *    not subtrees under a symbol different from f.  After the reassociation,
 *    the term has the same (physical) top node.
 *
 *************/

static
void right_associate(Term t)
{
  Term fab, a, b, c, d;
  int sn;

  sn = SYMNUM(t);

  if (SYMNUM(ARG(t,1)) == sn)
    right_associate(ARG(t,1));

  if (SYMNUM(ARG(t,0)) == sn) {
    right_associate(ARG(t,0));

    /* Let t be f(f(a,b),c). */

    fab = ARG(t,0);
    a = ARG(fab,0);
    b = ARG(fab,1);
    c = ARG(t,1);

    if (SYMNUM(b) != sn) {
      /*  This is easy---just reassociate. */
      ARG(t,0) = a;
      ARG(t,1) = fab;
      ARG(fab,0) = b;
      ARG(fab,1) = c;
    }
    else {
      /* This is tricky---append the two lists w/o changing top node. */
      d = b;
      while (SYMNUM(ARG(d,1)) == sn)
	d = ARG(d,1);

      ARG(t,0) = a;
      ARG(t,1) = b;
      ARG(fab,0) = ARG(d,1);
      ARG(fab,1) = c;
      ARG(d,1) = fab;
    }
  }
}  /* right_associate */

/*************
 *
 *    ac_canonical2
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void ac_canonical2(Term t, int bit,
		   Ordertype (*term_compare_proc) (Term, Term))
{
  Term args[MAX_ACM_ARGS], work[MAX_ACM_ARGS];
  Term t1;
  int n, i;

  /* if (!Internal_flags[AC_PRESENT]) return; WORK */

  if (is_assoc_comm(SYMNUM(t))) {
    /* Get array of arguments, sort, right assoc tree, then insert. */
    n = 0;
    flatten(t, args, &n);
    for (i = 0; i < n; i++)
      if (bit == -1 || !term_flag(args[i], bit))  /* if not reduced */
	ac_canonical2(args[i], bit, term_compare_proc);

    merge_sort_recurse((void **) args, (void **) work, 0, n-1,
	       (Ordertype (*)(void*,void*)) term_compare_proc);

    right_associate(t);

    for (t1 = t, i = 0; i < n-2; t1 = ARG(t1,1), i++) {
      ARG(t1,0) = args[i];
      /* clear "reduced" flag, because changed. */
      if (bit != -1)
	term_flag_clear(t1, bit);
    }
    ARG(t1,0) = args[n-2];
    ARG(t1,1) = args[n-1];
  }
  else {  /* Top symbol is not AC, so just recurse on arguments. */
    for (i = 0; i < ARITY(t); i++)
      ac_canonical2(ARG(t,i), bit, term_compare_proc);
  }
}  /* ac_canonical2 */

/*************
 *
 *    ac_canonical
 *
 *************/

/* DOCUMENTATION
This routine transforms a term into AC canonical form, which means
that all AC subterms are right associated and sorted.  The routine
term_compare_ncv() is used to compare AC arguments, and the order
is CONSTANT < COMPLEX < VARIABLE; within type, the order is by VARNUM
and lexigocgaphic by SYMNUM.
<P>
Terms can be marked as fully reduced (which implies AC canonical).
The argument "bit" tells which term bit is used for the mark (bit=-1
means to ignore marks).  If such a term is found, it is skipped.
Also, we make sure that any transformed terms are not marked (they
will be AC canonical, but they might not be reduced).
<P>
The top node of the term is not changed, so this is a void routine.  */

/* PUBLIC */
void ac_canonical(Term t, int bit)
{
#if 1
  ac_canonical2(t, bit, term_compare_ncv);
#else
  Term args[MAX_ACM_ARGS], work[MAX_ACM_ARGS];
  Term t1;
  int n, i;

  /* if (!Internal_flags[AC_PRESENT]) return; WORK */

  if (is_assoc_comm(SYMNUM(t))) {
    /* Get array of arguments, sort, right assoc tree, then insert. */
    n = 0;
    flatten(t, args, &n);
    for (i = 0; i < n; i++)
      if (bit == -1 || !term_flag(args[i], bit))  /* if not reduced */
	ac_canonical(args[i], bit);

    merge_sort_recurse((void **) args, (void **) work, 0, n-1,
	       (Ordertype (*)(void*,void*)) term_compare_ncv);

    right_associate(t);

    for (t1 = t, i = 0; i < n-2; t1 = ARG(t1,1), i++) {
      ARG(t1,0) = args[i];
      /* clear "reduced" flag, because changed. */
      if (bit != -1)
	term_flag_clear(t1, bit);
    }
    ARG(t1,0) = args[n-2];
    ARG(t1,1) = args[n-1];
  }
  else {  /* Top symbol is not AC, so just recurse on arguments. */
    for (i = 0; i < ARITY(t); i++)
      ac_canonical(ARG(t,i), bit);
  }
#endif
}  /* ac_canonical */

/*************
 *
 *     check_ac_canonical(t)
 *
 *************/

/* DOCUMENTATION
This Boolean routine simply checks if a term is AC canonical.  It should
only be used if you suspect something is wrong, because it is inefficient
(it copies the term, calls ac_canonical() on the copy, then does term_ident()
with the original term).
*/

/* PUBLIC */
int check_ac_canonical(Term t)
{
  int rc;
  Term t1 = copy_term(t);
  ac_canonical(t1, -1);
  rc = term_ident(t, t1);
  zap_term(t1);
  return(rc);
}  /* check_ac_canonical */

