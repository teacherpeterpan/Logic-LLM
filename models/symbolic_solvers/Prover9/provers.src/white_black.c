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

#include "white_black.h"
#include "semantics.h"
#include "../ladr/clause_eval.h"
#include "../ladr/interp.h"

/* Private definitions and types */

static Plist White_rules = NULL;
static Plist Black_rules = NULL;
static BOOL  Rule_needs_semantics = FALSE;

/*************
 *
 *   init_white_black()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_white_black(Plist white, Plist black)
{
  Plist p;
  for (p = white; p; p = p->next) {
    Clause_eval ce = compile_clause_eval_rule(p->v);
    if (ce == NULL)
      fatal_error("Error in \"keep\" rule");
    else {
      White_rules = plist_append(White_rules, ce);
      if (rule_contains_semantics(ce))
	Rule_needs_semantics = TRUE;
    }
  }
  for (p = black; p; p = p->next) {
    Clause_eval ce = compile_clause_eval_rule(p->v);
    if (ce == NULL)
      fatal_error("Error in \"delete\" rule");
    else {
      Black_rules = plist_append(Black_rules, ce);
      if (rule_contains_semantics(ce))
	Rule_needs_semantics = TRUE;
    }
  }
}  /* init_white_black */

/*************
 *
 *   new_rule_int()
 *
 *************/

static
Term new_rule_int(char *property, Ordertype order, int value)
{
  Term t;
  char *s = NULL;
  switch(order) {
  case LESS_THAN: s = "<";  break;
  case LESS_THAN_OR_SAME_AS: s = "<=";  break;
  case SAME_AS: s = "=";  break;
  case GREATER_THAN_OR_SAME_AS: s = ">=";  break;
  case GREATER_THAN: s = ">";  break;
  default: fatal_error("new_rule_int, bad relation");
  }
  t = get_rigid_term(s, 2);
  ARG(t,0) = get_rigid_term(property, 0);
  ARG(t,1) = int_to_term(value);
  return t;
}  /* new_rule_int */

/*************
 *
 *   new_rule_double()
 *
 *************/

static
Term new_rule_double(char *property, Ordertype order, double value)
{
  Term t;
  char *s = NULL;
  switch(order) {
  case LESS_THAN: s = "<";  break;
  case LESS_THAN_OR_SAME_AS: s = "<=";  break;
  case SAME_AS: s = "=";  break;
  case GREATER_THAN_OR_SAME_AS: s = ">=";  break;
  case GREATER_THAN: s = ">";  break;
  default: fatal_error("new_rule_double, bad relation");
  }
  t = get_rigid_term(s, 2);
  ARG(t,0) = get_rigid_term(property, 0);
  ARG(t,1) = double_to_term(value);
  return t;
}  /* new_rule_double */

/*************
 *
 *   delete_rules_from_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist delete_rules_from_options(Prover_options opt)
{
  Plist p = NULL;

  if (parm(opt->max_weight) != DBL_LARGE)
    p=plist_append(p,new_rule_double("weight",
			      GREATER_THAN,
			      floatparm(opt->max_weight)));

  if (parm(opt->max_vars) != parm_default(opt->max_vars))
    p=plist_append(p,new_rule_int("variables",
			      GREATER_THAN,
			      parm(opt->max_vars)));

  if (parm(opt->max_depth) != parm_default(opt->max_depth))
    p=plist_append(p,new_rule_int("depth",
			      GREATER_THAN,
			      parm(opt->max_depth)));

  if (parm(opt->max_literals) != parm_default(opt->max_literals))
    p=plist_append(p,new_rule_int("literals",
			      GREATER_THAN,
			      parm(opt->max_literals)));
  return p;
}  /* delete_rules_from_options */

/*************
 *
 *   black_tests()
 *
 *************/

/* DOCUMENTATION
Return TRUE if the clause satisfies any of the "black" rules.
*/

/* PUBLIC */
BOOL black_tests(Topform c)
{
  Plist p;
  if (Rule_needs_semantics)
    set_semantics(c);  /* in case not yet evaluated */

  for (p = Black_rules; p; p = p->next) {
    if (eval_clause_in_rule(c, p->v))
      return TRUE;
  }
  return FALSE;
}  /* black_tests */

/*************
 *
 *   white_tests()
 *
 *************/

/* DOCUMENTATION
Return TRUE if the clause satisfies any of the "white" rules.
*/

/* PUBLIC */
BOOL white_tests(Topform c)
{
  Plist p;
  if (Rule_needs_semantics)
    set_semantics(c);  /* in case not yet evaluated */
  for (p = White_rules; p; p = p->next) {
    if (eval_clause_in_rule(c, p->v))
      return TRUE;
  }
  return FALSE;
}  /* white_tests */
