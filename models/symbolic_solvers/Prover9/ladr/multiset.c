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

#include "multiset.h"

/* Private definitions and types */

/*************
 *
 *   num_occurrences()
 *
 *************/

static
int num_occurrences(Term t, Term a[], int n)
{
  int i, count;
  for (i = 0, count = 0; i < n; i++)
    if (term_ident(a[i], t))
      count++;
  return count;
}  /* num_occurrences */

/*************
 *
 *   set_of_more_occurrences() 
 *
 *   Return the list of elements with more occurrences in a1.
 *
 *************/

static
Plist set_of_more_occurrences(Term a1[], int n1, Term a2[], int n2)
{
  Plist answer = NULL;
  int i, j;
    
  for (i = 0; i < n1; i++) {
    Term e1 = a1[i];

    /* Check if this is the first occurrence of e1. */

    for (j = 0; j != i && !term_ident(e1, a1[j]); j++);

    if (i == j && num_occurrences(e1,a1,n1) > num_occurrences(e1,a2,n2))
      answer = plist_prepend(answer, e1);
  }
  return answer;
}  /* set_of_more_occurrences */

/*************
 *
 *   greater_multiset()
 *
 *   a1 >> a2 iff 
 *      (1) there is an element with more occurrences in a1; and
 *      (2) for each element with more occurrences in a2,
 *          there is a greater element with more occurrences in a1.
 *
 *************/

/* DOCUMENTATION
Given two arrays of terms, check if the first is greater
in the multiset extension of the ordering given by comp_proc.
*/

/* PUBLIC */
BOOL greater_multiset(Term a1[], int n1, Term a2[], int n2,
		      BOOL (*comp_proc) (Term, Term, BOOL),
		      BOOL lex_order_vars)
{
  Plist s1, s2, p1, p2;
  BOOL ok;
  s1 = set_of_more_occurrences(a1, n1, a2, n2);  /* more occurrences in a1 */
  s2 = set_of_more_occurrences(a2, n2, a1, n1);  /* more occurrences in a2 */
  /*
   * return (s1 not empty and foreach p2 in s2
   * there is an p1 in s1 such that p1 > p2).
   */
  if (s1 == NULL)
    ok = FALSE;
  else {
    for (p2 = s2, ok = TRUE; p2 && ok; p2 = p2->next)
      for (p1 = s1, ok = FALSE; p1 && !ok; p1 = p1->next)
	ok = (*comp_proc)(p1->v, p2->v, lex_order_vars);
  }
  zap_plist(s1);
  zap_plist(s2);
  return ok;
}  /* greater_multiset */

