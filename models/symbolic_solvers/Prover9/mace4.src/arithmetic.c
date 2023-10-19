/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "msearch.h"

extern int Negation_flag;  /* for dealing with mclause */
extern Term *Domain;
extern int Domain_size;

/* Private definitions and types */

static int Sum_sn;
static int Prod_sn;
static int Neg_sn;
static int Div_sn;
static int Mod_sn;
static int Min_sn;
static int Max_sn;
static int Abs_sn;
static int Domain_size_sn;
static int Lt_sn;
static int Le_sn;
static int Gt_sn;
static int Ge_sn;
static int Eq_sn;

static BOOL *Arith_op_sn;  /* array to know if sn is arithmetic operation */
static BOOL *Arith_rel_sn; /* array to know if sn is arithmetic relation */
static int Arith_sn_size = 0;   /* size of array */

/*************
 *
 *   init_arithmetic()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_arithmetic(void)
{
  Arith_sn_size = greatest_symnum() + 20;
  Arith_op_sn = calloc(Arith_sn_size, sizeof(BOOL));
  Arith_rel_sn = calloc(Arith_sn_size, sizeof(BOOL));

  Arith_op_sn[Sum_sn = str_to_sn("+", 2)] = TRUE;
  Arith_op_sn[Prod_sn = str_to_sn("*", 2)] = TRUE;
  Arith_op_sn[Neg_sn = str_to_sn("-", 1)] = TRUE;
  Arith_op_sn[Div_sn = str_to_sn("/", 2)] = TRUE;
  Arith_op_sn[Mod_sn = str_to_sn("mod", 2)] = TRUE;
  Arith_op_sn[Min_sn = str_to_sn("min", 2)] = TRUE;
  Arith_op_sn[Max_sn = str_to_sn("max", 2)] = TRUE;
  Arith_op_sn[Abs_sn = str_to_sn("abs", 1)] = TRUE;
  Arith_op_sn[Domain_size_sn = str_to_sn("domain_size", 0)] = TRUE;

  Arith_rel_sn[Le_sn = str_to_sn("<=", 2)] = TRUE;
  Arith_rel_sn[Lt_sn = str_to_sn("<", 2)] = TRUE;
  Arith_rel_sn[Ge_sn = str_to_sn(">=", 2)] = TRUE;
  Arith_rel_sn[Gt_sn = str_to_sn(">", 2)] = TRUE;
  Arith_rel_sn[Eq_sn = str_to_sn("=", 2)] = TRUE;

  set_assoc_comm("+", TRUE);
  set_assoc_comm("*", TRUE);

  declare_parse_type("+",        490, INFIX_RIGHT);
  declare_parse_type("*",        470, INFIX_RIGHT);
  declare_parse_type("/",        460, INFIX);
  declare_parse_type("mod",      460, INFIX);

}  /* init_arithmetic */

/*************
 *
 *   modulo()
 *
 *************/

/* DOCUMENTATION
In C, the % (remainder) operation is not defined for negative operands.
Also there is a distinction between the "remainder" and "modulo" operations
for negative operands:

  A   B   A/B   A rem B  A mod B

 14   5    2       4        4
-14   5   -2      -4        1
 14  -5   -2       4       -1 
-14  -5    2      -4       -4

*/

/* PUBLIC */
int modulo(int a, int b)
{
  if (b == 0)
    return INT_MAX;
  else if (b > 0) {
    if (a >= 0)
      return a % b;                  /* a >= 0, b > 0 */
    else
      return -(abs(a) % b) + b;      /* a <  0, b > 0 */
  }
  else {
    if (a >= 0)
      return (a % abs(b)) + b;       /* a >= 0, b < 0 */
    else
      return -(abs(a) % abs(b));     /* a <  0, b < 0 */
  }
}  /* modulo */

/*************
 *
 *   domain_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL domain_term(Term t, int domain_size)
{
  return VARIABLE(t) && VARNUM(t) < domain_size;
}  /* domain_term */

/*************
 *
 *   arith_op_sn()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL arith_op_sn(int i)
{
  if (i >= Arith_sn_size)
    return FALSE;
  else
    return Arith_op_sn[i];
}  /* arith_op_sn */

/*************
 *
 *   arith_rel_sn()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL arith_rel_sn(int i)
{
  if (i >= Arith_sn_size)
    return FALSE;
  else
    return Arith_rel_sn[i];
}  /* arith_rel_sn */

/*************
 *
 *   arith_op_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL arith_op_term(Term t)
{
  return !VARIABLE(t) && arith_op_sn(SYMNUM(t));
}  /* arith_op_term */

/*************
 *
 *   arith_rel_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL arith_rel_term(Term t)
{
  return !VARIABLE(t) && arith_rel_sn(SYMNUM(t));
}  /* arith_rel_term */

/*************
 *
 *   arith_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL arith_term(Term t)
{
  if (VARIABLE(t))
    return TRUE;
  else
    return arith_op_term(t) || arith_rel_term(t);
}  /* arith_term */

/*************
 *
 *   arith_quasi_evaluable()
 *
 *************/

/* DOCUMENTATION
Similar to arith_evaluable(), except that division by 0 is not checked.
*/

/* PUBLIC */
BOOL arith_quasi_evaluable(Term t)
{
  if (!arith_term(t))
    return FALSE;
  else if (VARIABLE(t))
    return TRUE;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      if (!arith_quasi_evaluable(ARG(t,i)))
	return FALSE;
    return TRUE;
  }
}  /* arith_quasi_evaluable */

/*************
 *
 *   arith_evaluate()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int arith_evaluate(Term t, BOOL *evaluated)
{
  if (!arith_term(t)) {
    *evaluated = FALSE;
    return 0;
  }

  if (VARIABLE(t))
    return VARNUM(t);
  else {
    int sn = SYMNUM(t);

    if (sn == Div_sn || sn == Mod_sn) {
      int d = arith_evaluate(ARG(t,1), evaluated);
      if (d == 0) {
	*evaluated = FALSE;
	return 0;
      }
      else if (sn == Div_sn)
	return arith_evaluate(ARG(t,0), evaluated) / d;
      else
	return modulo(arith_evaluate(ARG(t,0), evaluated), d);
    }

    else if (sn == Sum_sn)
      return arith_evaluate(ARG(t,0), evaluated) + arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Prod_sn)
      return arith_evaluate(ARG(t,0), evaluated) * arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Neg_sn)
      return -arith_evaluate(ARG(t,0), evaluated);
    else if (sn == Abs_sn)
      return abs(arith_evaluate(ARG(t,0), evaluated));
    else if (sn == Domain_size_sn)
      return Domain_size;
    else if (sn == Min_sn) {
      int a0 = arith_evaluate(ARG(t,0), evaluated);
      int a1 = arith_evaluate(ARG(t,1), evaluated);
      return IMIN(a0,a1);
    }
    else if (sn == Max_sn) {
      int a0 = arith_evaluate(ARG(t,0), evaluated);
      int a1 = arith_evaluate(ARG(t,1), evaluated);
      return IMAX(a0,a1);
    }
    else if (sn == Lt_sn)
      return arith_evaluate(ARG(t,0), evaluated) <  arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Le_sn)
      return arith_evaluate(ARG(t,0), evaluated) <= arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Gt_sn)
      return arith_evaluate(ARG(t,0), evaluated) >  arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Ge_sn)
      return arith_evaluate(ARG(t,0), evaluated) >= arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Eq_sn)
      return arith_evaluate(ARG(t,0), evaluated) == arith_evaluate(ARG(t,1), evaluated);
    else {
      fatal_error("arith_evaluate, operation not handled");
      return INT_MIN;
    }
  }
}  /* arith_evaluate */

/*************
 *
 *   arith_eval()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int arith_eval(Term t, BOOL *evaluated)
{
  *evaluated = TRUE;
  return arith_evaluate(t, evaluated);
}  /* arith_eval */

/*************
 *
 *   top_safe() -- for clauses whose nats are constants
 *
 *************/

static
BOOL top_safe(Term t, int domain_size)
{
  if (VARIABLE(t))
    return TRUE;
  else if (CONSTANT(t))
    return natural_constant_term(t) < domain_size;
  else if (arith_op_term(t) || arith_rel_term(t))
    return FALSE;
  else
    return TRUE;
}  /* top_safe */

/*************
 *
 *   all_safe() - nothing in the term or any of the subterms involves
 *                arithmetic or nats out of range.
 *             - for clauses whose nats are constants
 *
 *************/

static
BOOL all_safe(Term t, int domain_size)
{
  if (VARIABLE(t))
    return TRUE;
  else if (!top_safe(t, domain_size))
    return FALSE;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (!all_safe(ARG(t,i), domain_size))
	return FALSE;
    }
    return TRUE;
  }
}  /* all_safe */

/*************
 *
 *   all_ordinary_nodes_safe() -- for clauses whose nats are constants
 *
 *************/

static
BOOL all_ordinary_nodes_safe(Term t, int domain_size)
{
  if (VARIABLE(t) || CONSTANT(t))
    return TRUE;
  else if (arith_rel_term(t) || arith_op_term(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (!all_ordinary_nodes_safe(ARG(t,i), domain_size))
	return FALSE;
    }
    return TRUE;
  }
  else
    return all_safe(t, domain_size);
}  /* all_ordinary_nodes_safe */

/*************
 *
 *   non_arith() -- for clauses whose nats are constants
 *
 *************/

static
BOOL non_arith(Term t)
{
  if (VARIABLE(t))
    return FALSE;
  else if (CONSTANT(t))
    return natural_constant_term(t) < 0;
  else if (arith_rel_term(t) || arith_op_term(t))
    return FALSE;
  else
    return TRUE;
}  /* non_arith */

/*************
 *
 *   atom_safe() -- for clauses whose nats are constants
 *
 *************/

static
BOOL atom_safe(Term atom, int domain_size)
{
  if (SYMNUM(atom) == Eq_sn) {
    /* special case, because = is sometimes arith, sometimes not */
    Term a = ARG(atom,0);
    Term b = ARG(atom,1);
    if (non_arith(a) && natural_constant_term(b) >= domain_size)
      return FALSE;
    else if (non_arith(b) && natural_constant_term(a) >= domain_size)
      return FALSE;
    else
      return all_ordinary_nodes_safe(atom, domain_size);
    }
  else
    return all_ordinary_nodes_safe(atom, domain_size);
}  /* atom_safe */

/*************
 *
 *   ok_for_arithmetic()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL ok_for_arithmetic(Plist clauses, int domain_size)
{
  /* Domain elements and other integers are CONSTANTS!!! */
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    Literals lit;
    for (lit = c->literals; lit; lit = lit->next) {
      if (!atom_safe(lit->atom, domain_size))
	return FALSE;
    }
  }
  return TRUE;
}  /* ok_for_arithmetic */

/*************
 *
 *   distrib()
 *
 *************/

static
Term distrib(Term t)
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = distrib(ARG(t,i));
    
    if (SYMNUM(t) != Prod_sn)
      return t;
    else {
      if (SYMNUM(ARG(t,1)) == Sum_sn) {
	/* a*(b+c) */
	Term a = ARG(t,0);
	Term b = ARG(ARG(t,1),0);
	Term c = ARG(ARG(t,1),1);
	free_term(ARG(t,1));
	free_term(t);
	return build_binary_term(Sum_sn,
				 distrib(build_binary_term(Prod_sn, a, b)),
				 distrib(build_binary_term(Prod_sn, copy_term(a), c)));
      }
      else if (SYMNUM(ARG(t,0)) == Sum_sn) {
	/* (b+c)*a */
	Term a = ARG(t,1);
	Term b = ARG(ARG(t,0),0);
	Term c = ARG(ARG(t,0),1);
	free_term(ARG(t,0));
	free_term(t);
	return build_binary_term(Sum_sn,
				 distrib(build_binary_term(Prod_sn, b, a)),
				 distrib(build_binary_term(Prod_sn, c, copy_term(a))));
      }
      else
	return t;
    }
  }
}  /* distrib */

/*************
 *
 *   qsimp()
 *
 *************/

static
Term qsimp(Term t)
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    BOOL all_args_ints = TRUE;
    for (i = 0; i < ARITY(t); i++) {
      ARG(t,i) = qsimp(ARG(t,i));
      if (!(VARIABLE(ARG(t,i)) ||
	    (SYMNUM(ARG(t,i)) == Neg_sn && VARIABLE(ARG(ARG(t,i),0)))))
	all_args_ints = FALSE;
    }

    if (all_args_ints) {
      BOOL evaluated;
      int i = arith_eval(t, &evaluated);
      if (evaluated) {
	zap_term(t);
	if (i >= 0)
	  return get_variable_term(i);
	else
	  return build_unary_term(Neg_sn, get_variable_term(-i));
      }
      else
	return t;
    }
    else {
      if (SYMNUM(t) != Prod_sn && VARIABLE(ARG(t,0)) && VARNUM(ARG(t,0)) == 0) {
	/* 0*x to 0 */
	zap_term(t);
	return get_variable_term(0);
      }
      else if (SYMNUM(t) != Sum_sn &&
	       SYMNUM(ARG(t,1)) == Neg_sn &&
	       term_ident(ARG(t,0),ARG(ARG(t,1),0))) {
	/* x + -x to 0 */
	zap_term(t);
	return get_variable_term(0);
      }
      else
	return t;
    }
  }
}  /* qsimp */

/*************
 *
 *   arith_rel_quasi_eval()
 *
 *************/

static
BOOL arith_rel_quasi_eval(Term atom)
{
  /* This is an initial version for testing only. */
  if (SYMNUM(atom) == Eq_sn) {
    BOOL negated = NEGATED(atom);
    Term atom2 = copy_term(atom);
    BOOL val;

    atom2 = distrib(atom2);
    // printf("after distrib:  "); fwrite_term_nl(stdout, atom2);
    ac_canonical2(atom2, -1, term_compare_vcp);
    // printf("after AC canon: "); fwrite_term_nl(stdout, atom2);

    ARG(atom2,0) = qsimp(ARG(atom2,0));
    ARG(atom2,1) = qsimp(ARG(atom2,1));

    // printf("after qsimp: "); fwrite_term_nl(stdout, atom2);

    if (term_ident(ARG(atom2,0), ARG(atom2,1)))
      val = negated ? FALSE : TRUE;
    else
      val = FALSE;

    zap_term(atom2);
    return val;
  }
  else
    return FALSE;
}  /* arith_rel_quasi_eval */

/*************
 *
 *   check_with_arithmetic()
 *
 *************/

/* DOCUMENTATION
Return TRUE iff all clauses are true.  There can be arithmeic
terms that need to be evaluated.
*/

/* PUBLIC */
BOOL check_with_arithmetic(Plist ground_clauses)
{
  Plist p;
  for (p = ground_clauses; p; p = p->next) {
    Mclause c = p->v;
    if (!c->subsumed) {
      /* look for an arithmetic term and evaluate it */
      BOOL clause_is_true = FALSE;
      int i;
      for (i = 0; i < c->numlits && !clause_is_true; i++) {
	Term atom = LIT(c, i);
	if (arith_quasi_evaluable(atom)) {
	  if (arith_rel_quasi_eval(atom))
	    clause_is_true = TRUE;
	}
	else if (!FALSE_TERM(atom) && !TRUE_TERM(atom)) {
	  /* non-arithmetic lits should have been reduced to TRUE or FALSE */
	  fprintf(stderr, "ERROR, model reported, but clause not true!\n");
	  fprintf(stdout, "ERROR, model reported, but clause not true! ");
	  p_mclause(c);
	  fatal_error("check_with_arithmetic, clause not reduced");
	}
      }  /* literals loop */
      if (!clause_is_true)
	return FALSE;
    }  /* non-subsumed clause */
  }  /* clauses loop */
  return TRUE;
}  /* check_with_arithmetic */

