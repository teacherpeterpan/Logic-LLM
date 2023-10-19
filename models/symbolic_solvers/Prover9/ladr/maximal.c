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

#include "maximal.h"

/* Private definitions and types */

int Maximal_flag         = -1;  /* termflag to mark maximal literals */
int Maximal_signed_flag  = -1;  /* to mark maximal literals within sign */
int Selected_flag        = -1;  /* to mark selected literals */

/*************
 *
 *   init_maximal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_maximal(void)
{
  if (Maximal_flag != -1)
    fatal_error("init_maximal, called more than once");
  Maximal_flag = claim_term_flag();
  Maximal_signed_flag = claim_term_flag();
  Selected_flag = claim_term_flag();
}  /* init_maximal */

/*************
 *
 *   greater_literals()
 *
 *************/

static
BOOL greater_literals(Literals l1, Literals l2)
{
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Ordertype p = sym_precedence(SYMNUM(a1), SYMNUM(a2));
  if (p == GREATER_THAN)
    return TRUE;
  else if (p == LESS_THAN)
    return FALSE;
  else if (SYMNUM(a1) != SYMNUM(a2))
    return FALSE;
  else if (is_eq_symbol(SYMNUM(a1)))
    return greater_multiset_current_ordering(a1, a2);
  else
    return term_greater(a1, a2, FALSE);  /* LPO, RPO, KBO */
}  /* greater_literals */

/*************
 *
 *   max_lit_test()
 *
 *************/

/* DOCUMENTATION
Test if a literal is maximal in a clause (w.r.t. others literals
of the either sign).
This version does not use a flag.
*/

/* PUBLIC */
BOOL max_lit_test(Literals lits, Literals lit)
{
  /* If there is a greater literal of ANY sign, return FALSE. */
  Literals l2 = lits;
  BOOL maximal = TRUE;
  while (l2 && maximal) {
    if (lit != l2 && greater_literals(l2, lit))
      maximal = FALSE;
    else
      l2 = l2->next;
  }
  return maximal;
}  /* max_lit_test */

/*************
 *
 *   max_signed_lit_test()
 *
 *************/

/* DOCUMENTATION
Test if a literal is maximal in a clause (w.r.t. others literals
of the same sign).
This version does not use a flag.
*/

/* PUBLIC */
BOOL max_signed_lit_test(Literals lits, Literals lit)
{
  /* If there is a greater literal of the same sign, return FALSE. */
  Literals l2 = lits;
  BOOL maximal = TRUE;
  while (l2 && maximal) {
    if (lit != l2 && lit->sign == l2->sign && greater_literals(l2, lit))
      maximal = FALSE;
    else
      l2 = l2->next;
  }
  return maximal;
}  /* max_signed_lit_test */

/*************
 *
 *   mark_maximal_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_maximal_literals(Literals lits)
{
  Literals lit;

  if (Maximal_flag == -1)
    fatal_error("mark_maximal_literals, init_maximal() was not called");
  
  /* Note: we mark the atom, not the literal. */

  for (lit = lits; lit; lit = lit->next) {
    if (max_lit_test(lits, lit))
      term_flag_set(lit->atom, Maximal_flag);
    if (max_signed_lit_test(lits, lit))
      term_flag_set(lit->atom, Maximal_signed_flag);
  }
}  /* mark_maximal_literals */

/*************
 *
 *   maximal_literal()
 *
 *************/

/* DOCUMENTATION
Check if a literal is maximal in the clause that contains it.
The argument "check" should be FLAG_CHECK (check the flag only)
or FULL_CHECK (do the full comparicon).
*/

/* PUBLIC */
BOOL maximal_literal(Literals lits, Literals lit, int check)
{
  if (check == FLAG_CHECK)
    return term_flag(lit->atom, Maximal_flag);
  else
    return max_lit_test(lits, lit);
}  /* maximal_literal */

/*************
 *
 *   maximal_signed_literal()
 *
 *************/

/* DOCUMENTATION
Check if a literal is maximal in the clause that contains it.
This only checks a flag.  It does not compute maximality.
*/

/* PUBLIC */
BOOL maximal_signed_literal(Literals lits, Literals lit, int check)
{
  if (check == FLAG_CHECK)
    return term_flag(lit->atom, Maximal_signed_flag);
  else
    return max_signed_lit_test(lits, lit);
}  /* maximal_signed_literal */

/*************
 *
 *   number_of_maximal_literals()
 *
 *************/

/* DOCUMENTATION
Return the number of maximal literals.  This checks a flag only.
*/

/* PUBLIC */
int number_of_maximal_literals(Literals lits, int check)
{
  int n = 0;
  Literals lit;
  for (lit = lits; lit; lit = lit->next) {
    if (maximal_literal(lits, lit, check))
      n++;
  }
  return n;
}  /* number_of_maximal_literals */

/*************
 *
 *   mark_selected_literal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_selected_literal(Literals lit)
{
  term_flag_set(lit->atom, Selected_flag);
}  /* mark_selected_literal */

/*************
 *
 *   mark_selected_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_selected_literals(Literals lits, char *selection)
{
  Literals lit;

  if (Selected_flag == -1)
    fatal_error("mark_selected_literals, init_maximal() was not called");
  
  /* Note: we mark the atom, not the literal. */

  for (lit = lits; lit; lit = lit->next) {
    if (!lit->sign) {
      if (str_ident(selection, "all_negative"))
	mark_selected_literal(lit);
      else if (str_ident(selection, "max_negative")) {
	if (maximal_signed_literal(lits, lit, FLAG_CHECK))
	  mark_selected_literal(lit);
      }
    }
  }
}  /* mark_maximal_literals */

/*************
 *
 *   selected_literal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL selected_literal(Literals lit)
{
  return term_flag(lit->atom, Selected_flag);
}  /* selected_literal */

/*************
 *
 *   exists_selected_literal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL exists_selected_literal(Literals lits)
{
  Literals lit;
  for (lit = lits; lit; lit = lit->next)
    if (selected_literal(lit))
      return TRUE;
  return FALSE;
}  /* exists_selected_literal */

/*************
 *
 *   copy_selected_literal_marks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void copy_selected_literal_marks(Literals a, Literals b)
{
  if (!a || !b)
    return;
  else {
    copy_selected_literal_marks(a->next, b->next);
    if (selected_literal(a))
      mark_selected_literal(b);
  }
}  /* copy_selected_literal_marks */


