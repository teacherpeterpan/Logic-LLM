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

#include "clauses.h"

/* Private definitions and types */

/*
 * memory management
 */

/*************
 *
 *   clause_member_plist()
 *
 *************/

/* DOCUMENTATION
Is a clause a member of a Plist?  A deep identity check is done (clause_ident).
*/

/* PUBLIC */
Topform clause_member_plist(Plist p, Topform c)
{
  while (p) {
    Topform d = p->v;
    if (clause_ident(c->literals, d->literals))
      return p->v;
    p = p->next;
  }
  return NULL;
}  /* clause_member_plist */

/*************
 *
 *   intersect_clauses()
 *
 *************/

/* DOCUMENTATION
Intersect 2 Plists of clauses.
The order of the result is determined by the order of the first list.
A deep identity check is done (clause_ident).
*/

/* PUBLIC */
Plist intersect_clauses(Plist a, Plist b)
{
  if (a == NULL)
    return NULL;
  else {
    Plist c = intersect_clauses(a->next, b);
    if (clause_member_plist(b, a->v))
      return plist_prepend(c, a->v);
    else
      return c;
  }
}  /* intersect_clauses */

/*************
 *
 *   max_clause_weight()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses, return the weight of the heaviest clause.
The weight field of the clause is used.
*/

/* PUBLIC */
int max_clause_weight(Plist p)
{
  if (p == NULL)
    return INT_MIN;
  else {
    int max_rest = max_clause_weight(p->next);
    Topform c = p->v;
    return IMAX(c->weight,max_rest);
  }
}  /* max_clause_weight */

/*************
 *
 *   max_clause_symbol_count()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses, return the symbol_count of a clause
with most symbols.
*/

/* PUBLIC */
int max_clause_symbol_count(Plist p)
{
  if (p == NULL)
    return INT_MIN;
  else {
    int max_rest = max_clause_symbol_count(p->next);
    Topform c = p->v;
    return IMAX(clause_symbol_count(c->literals), max_rest);
  }
}  /* max_clause_symbol_count */

/*************
 *
 *   nonneg_clauses()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses, return the subset of nonnegative clauses.
*/

/* PUBLIC */
Plist nonneg_clauses(Plist clauses)
{
  Plist nonneg = NULL;
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    if (!negative_clause(c->literals))
      nonneg = plist_append(nonneg, c);
  }
  return nonneg;
}  /* nonneg_clauses */

/*************
 *
 *   all_clauses_horn()
 *
 *************/

/* DOCUMENTATION
Is every clause in the Plist a Horn clause?
*/

/* PUBLIC */
BOOL all_clauses_horn(Plist l)
{
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (!horn_clause(c->literals))
      return FALSE;
  }
  return TRUE;
}  /* all_clauses_horn */

/*************
 *
 *   all_clauses_unit()
 *
 *************/

/* DOCUMENTATION
Is every clause in the Plist a unit clause?
*/

/* PUBLIC */
BOOL all_clauses_unit(Plist l)
{
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (!unit_clause(c->literals))
      return FALSE;
  }
  return TRUE;
}  /* all_clauses_unit */

/*************
 *
 *   all_clauses_positive()
 *
 *************/

/* DOCUMENTATION
Is every clause in the Plist a unit clause?
*/

/* PUBLIC */
BOOL all_clauses_positive(Plist l)
{
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (!positive_clause(c->literals))
      return FALSE;
  }
  return TRUE;
}  /* all_clauses_positive */

/*************
 *
 *   neg_nonunit_clauses()
 *
 *************/

/* DOCUMENTATION
How many negative nonunits are in a Plist of clauses?
*/

/* PUBLIC */
int neg_nonunit_clauses(Plist l)
{
  int n = 0;
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (negative_clause(c->literals) && number_of_literals(c->literals) > 1)
      n++;
  }
  return n;
}  /* neg_nonunit_clauses */

/*************
 *
 *   negative_clauses()
 *
 *************/

/* DOCUMENTATION
How many negative clauses are in a Plist of clauses?
*/

/* PUBLIC */
int negative_clauses(Plist l)
{
  int n = 0;
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (negative_clause(c->literals))
      n++;
  }
  return n;
}  /* negative_clauses */

/*************
 *
 *   most_literals()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses,
what is the maximum number of literals in a clause.
*/

/* PUBLIC */
int most_literals(Plist clauses)
{
  int max = -1;
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    int n = number_of_literals(c->literals);
    max = IMAX(max,n);
  }
  return max;
}  /* most_literals */

/*************
 *
 *   pos_equality_in_clauses()
 *
 *************/

/* DOCUMENTATION
Does the Plist contain a clause with a positive equality literal?
*/

/* PUBLIC */
BOOL pos_equality_in_clauses(Plist clauses)
{
  if (clauses == NULL)
    return FALSE;
  else {
    Topform c = clauses->v;
    if (contains_pos_eq(c->literals))
      return TRUE;
    else
      return pos_equality_in_clauses(clauses->next);
  }
}  /* pos_equality_in_clauses */

/*************
 *
 *   equality_in_clauses()
 *
 *************/

/* DOCUMENTATION
Does the Plist contain a clause with a positive equality literal?
*/

/* PUBLIC */
BOOL equality_in_clauses(Plist clauses)
{
  if (clauses == NULL)
    return FALSE;
  else {
    Topform c = clauses->v;
    if (contains_eq(c->literals))
      return TRUE;
    else
      return equality_in_clauses(clauses->next);
  }
}  /* equality_in_clauses */

