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

#include "clause_eval.h"
#include "just.h"
#include "interp.h"

/*
(signs of literals)    positive, negative, mixed
(matches a hint)       hint
(semantics)            true, false
                       not_evaluated (e.g., too many variables)
(other properties)     has_equality, horn, definite, unit, initial
(for integer or double n)
                       weight = n, weight < n, weight > n,
                       vars = n, vars < n, vars > n,
                       depth = n, depth < n, depth > n,
                       literals = n, literals < n, literals > n,
                       level = n, level < n, level > n
*/

/* Private definitions and types */

enum { CL_EVAL_AND,
       CL_EVAL_OR,
       CL_EVAL_NOT,
       CL_EVAL_ALL,

       CL_EVAL_LESS,
       CL_EVAL_LESS_EQUAL,
       CL_EVAL_GREATER,
       CL_EVAL_GREATER_EQUAL,
       CL_EVAL_EQUAL,

       CL_EVAL_POSITIVE,
       CL_EVAL_NEGATIVE,
       CL_EVAL_MIXED,

       CL_EVAL_HINT,

       CL_EVAL_TRUE,
       CL_EVAL_FALSE,

       CL_EVAL_HAS_EQUALITY,
       CL_EVAL_HORN,
       CL_EVAL_DEFINITE,
       CL_EVAL_UNIT,

       CL_EVAL_INITIAL,
       CL_EVAL_RESOLVENT,
       CL_EVAL_UR_RESOLVENT,
       CL_EVAL_HYPER_RESOLVENT,
       CL_EVAL_FACTOR,
       CL_EVAL_PARAMODULANT,
       CL_EVAL_BACK_DEMODULANT,
       CL_EVAL_SUBSUMER,

       CL_EVAL_WEIGHT,
       CL_EVAL_VARIABLES,
       CL_EVAL_DEPTH,
       CL_EVAL_LITERALS,
       CL_EVAL_LEVEL
 };  /* type of clause eval nodes */

/* Following for the commpiled form of an evaluation rule. */

struct clause_eval {
  int          type;
  Clause_eval  left;       /* for AND, OR, NOT */
  Clause_eval  right;      /* for AND, OR */
  Ordertype    relation;   /* <, >, = */
  double       test_val;        /* for comparison, e.g., weight < 3 */
};  /* struct clause_eval */

/*
 * memory management
 */

#define PTRS_EVAL_RULE CEILING(sizeof(struct clause_eval), BYTES_POINTER)
static unsigned Clause_eval_gets, Clause_eval_frees;

/*************
 *
 *   Clause_eval get_clause_eval()
 *
 *************/

static
Clause_eval get_clause_eval(void)
{
  Clause_eval p = get_cmem(PTRS_EVAL_RULE);
  Clause_eval_gets++;
  return(p);
}  /* get_clause_eval */

/*************
 *
 *    free_clause_eval()
 *
 *************/

static
void free_clause_eval(Clause_eval p)
{
  free_mem(p, PTRS_EVAL_RULE);
  Clause_eval_frees++;
}  /* free_clause_eval */

/*
 * end memory management
 */

/*************
 *
 *   zap_clause_eval_rule()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_clause_eval_rule(Clause_eval p)
{
  if (p->type == CL_EVAL_AND || p->type == CL_EVAL_OR) {
    zap_clause_eval_rule(p->left);
    zap_clause_eval_rule(p->right);
  }
  else if (p->type == CL_EVAL_OR)
    zap_clause_eval_rule(p->left);
  
  free_clause_eval(p);
}  /* zap_clause_eval_rule */

/*************
 *
 *   compile_clause_eval_rule()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clause_eval compile_clause_eval_rule(Term t)
{
  Clause_eval p = get_clause_eval();

  if (is_term(t, "&", 2)) {
    p->type = CL_EVAL_AND;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
    p->right = compile_clause_eval_rule(ARG(t,1));
    if (p->right == NULL)
      return NULL;
  }

  else if (is_term(t, "|", 2)) {
    p->type = CL_EVAL_OR;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
    p->right = compile_clause_eval_rule(ARG(t,1));
    if (p->right == NULL)
      return NULL;
  }

  else if (is_term(t, "-", 1)) {
    p->type = CL_EVAL_NOT;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
  }

  else if (is_term(t, "all",  0))
    p->type = CL_EVAL_ALL;
  else if (is_term(t, "positive",  0))
    p->type = CL_EVAL_POSITIVE;
  else if (is_term(t, "negative",  0))
    p->type = CL_EVAL_NEGATIVE;
  else if (is_term(t, "mixed",  0))
    p->type = CL_EVAL_MIXED;

  else if (is_term(t, "true",  0))
    p->type = CL_EVAL_TRUE;
  else if (is_term(t, "false",  0))
    p->type = CL_EVAL_FALSE;

  else if (is_term(t, "has_equality",  0))
    p->type = CL_EVAL_HAS_EQUALITY;
  else if (is_term(t, "horn",  0))
    p->type = CL_EVAL_HORN;
  else if (is_term(t, "definite",  0))
    p->type = CL_EVAL_DEFINITE;
  else if (is_term(t, "unit",  0))
    p->type = CL_EVAL_UNIT;
  else if (is_term(t, "hint",  0))
    p->type = CL_EVAL_HINT;

  else if (is_term(t, "initial",  0))
    p->type = CL_EVAL_INITIAL;
  else if (is_term(t, "resolvent",  0))
    p->type = CL_EVAL_RESOLVENT;
  else if (is_term(t, "hyper_resolvent",  0))
    p->type = CL_EVAL_HYPER_RESOLVENT;
  else if (is_term(t, "ur_resolvent",  0))
    p->type = CL_EVAL_UR_RESOLVENT;
  else if (is_term(t, "factor",  0))
    p->type = CL_EVAL_FACTOR;
  else if (is_term(t, "paramodulant",  0))
    p->type = CL_EVAL_PARAMODULANT;
  else if (is_term(t, "back_demodulant",  0))
    p->type = CL_EVAL_BACK_DEMODULANT;
  else if (is_term(t, "subsumer",  0))
    p->type = CL_EVAL_SUBSUMER;

  else if (is_term(t, "<",  2) ||
	   is_term(t, ">",  2) ||
	   is_term(t, "<=", 2) ||
	   is_term(t, ">=", 2) ||
	   is_term(t, "=",  2)) {
    Term a0 = ARG(t,0);
    Term a1 = ARG(t,1);
    if (is_term(a0, "weight",  0))
      p->type = CL_EVAL_WEIGHT;
    else if (is_term(a0, "variables",  0))
      p->type = CL_EVAL_VARIABLES;
    else if (is_term(a0, "depth",  0))
      p->type = CL_EVAL_DEPTH;
    else if (is_term(a0, "literals",  0))
      p->type = CL_EVAL_LITERALS;
    else if (is_term(a0, "level",  0))
      p->type = CL_EVAL_LEVEL;
    else
      return NULL;

    if (!term_to_number(a1, &(p->test_val)))
      return NULL;
    
    if (is_term(t, "<",  2))
      p->relation = LESS_THAN;
    else if (is_term(t, ">",  2))
      p->relation = GREATER_THAN;
    else if (is_term(t, "<=", 2))
      p->relation = LESS_THAN_OR_SAME_AS;
    else if (is_term(t, ">=", 2))
      p->relation = GREATER_THAN_OR_SAME_AS;
    else if (is_term(t, "=",  2))
      p->relation = SAME_AS;
  }
  else
    return NULL;
  return p;
}  /* compile_clause_eval_rule */

/*************
 *
 *   eval_clause_in_rule()
 *
 *************/

/* DOCUMENTATION
The following properties have fields in the clause, and if they
hold, those flags should already be set: matching_hint, weight, semantics.
*/

/* PUBLIC */
BOOL eval_clause_in_rule(Topform c, Clause_eval p)
{
  Literals lits = c->literals;

  switch (p->type) {

  case CL_EVAL_AND:
    return
      eval_clause_in_rule(c, p->left) &&
      eval_clause_in_rule(c, p->right);
  case CL_EVAL_OR:
    return
      eval_clause_in_rule(c, p->left) ||
      eval_clause_in_rule(c, p->right);
  case CL_EVAL_NOT:
    return
      !eval_clause_in_rule(c, p->left);

  case CL_EVAL_ALL:
    return TRUE;
  case CL_EVAL_POSITIVE:
    return positive_clause(lits);
  case CL_EVAL_NEGATIVE:
    return negative_clause(lits);
  case CL_EVAL_MIXED:
    return mixed_clause(lits);

  case CL_EVAL_HINT:
    return c->matching_hint != NULL;

  case CL_EVAL_HAS_EQUALITY:
    return contains_eq(lits);
  case CL_EVAL_HORN:
    return horn_clause(lits);
  case CL_EVAL_DEFINITE:
    return definite_clause(lits);
  case CL_EVAL_UNIT:
    return unit_clause(lits);

  case CL_EVAL_INITIAL:
    return c->initial;
  case CL_EVAL_RESOLVENT:
    return primary_just_type(c, BINARY_RES_JUST);
  case CL_EVAL_HYPER_RESOLVENT:
    return primary_just_type(c, HYPER_RES_JUST);
  case CL_EVAL_UR_RESOLVENT:
    return primary_just_type(c, UR_RES_JUST);
  case CL_EVAL_FACTOR:
    return primary_just_type(c, FACTOR_JUST);
  case CL_EVAL_PARAMODULANT:
    return primary_just_type(c, PARA_JUST);
  case CL_EVAL_BACK_DEMODULANT:
    return primary_just_type(c, BACK_DEMOD_JUST);
  case CL_EVAL_SUBSUMER:
    return c->subsumer;

  case CL_EVAL_TRUE:
    return
      c->semantics == SEMANTICS_TRUE ||
      c->semantics == SEMANTICS_NOT_EVALUABLE;
  case CL_EVAL_FALSE:
    return c->semantics == SEMANTICS_FALSE;

  case CL_EVAL_WEIGHT:
  case CL_EVAL_VARIABLES:
  case CL_EVAL_DEPTH:
  case CL_EVAL_LITERALS:
  case CL_EVAL_LEVEL: {
    double val = 0;
    switch (p->type) {
    case CL_EVAL_WEIGHT:    val = c->weight;                  break;
    case CL_EVAL_VARIABLES: val = number_of_variables(lits);  break;
    case CL_EVAL_DEPTH:     val = literals_depth(lits);       break;
    case CL_EVAL_LITERALS:  val = number_of_literals(lits);   break;
    case CL_EVAL_LEVEL:     val = clause_level(c);            break;
    }
    switch (p->relation) {
    case LESS_THAN:                return val <  p->test_val;
    case GREATER_THAN:             return val >  p->test_val;
    case LESS_THAN_OR_SAME_AS:     return val <= p->test_val;
    case GREATER_THAN_OR_SAME_AS:  return val >= p->test_val;
    case SAME_AS:                  return val == p->test_val;
    default: fatal_error("eval_clause_in_rule, bad relation");
    }
  }
  default: fatal_error("eval_clause_in_rule, unknown operation");
  }  /* outer switch */
  return FALSE;
}  /* eval_clause_in_rule */

/*************
 *
 *   rule_contains_semantics()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL rule_contains_semantics(Clause_eval p)
{
  if (p->type == CL_EVAL_AND || p->type == CL_EVAL_OR) {
    return (rule_contains_semantics(p->left) ||
	    rule_contains_semantics(p->right));
  }
  else if (p->type == CL_EVAL_OR)
    return rule_contains_semantics(p->left);
  else
    return p->type == CL_EVAL_TRUE || p->type == CL_EVAL_FALSE;
}  /* rule_contains_semantics */

