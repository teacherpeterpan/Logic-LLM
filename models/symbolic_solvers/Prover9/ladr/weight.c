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

#include "weight.h"
#include "weight2.h"
#include "parse.h"
#include "complex.h"

/* Private definitions and types */

static Plist Rules;

static double Constant_weight;
static double Sk_constant_weight;
static double Not_weight;
static double Or_weight;
static double Prop_atom_weight;
static double Variable_weight;
static double Nest_penalty;
static double Depth_penalty;
static double Var_penalty;
static double Complexity;
static BOOL Not_rules;  /* any rules for not_sym()? */
static BOOL Or_rules;   /* any rules for or_sym()? */

/* Cache the symnums */

static int Eq_sn;      /* equality */
static int Weight_sn;  /* weight function*/

static int Sum_sn;     /* integer arithmetic */
static int Prod_sn;    /* integer arithmetic */
static int Neg_sn;     /* integer arithmetic */
static int Div_sn;     /* integer arithmetic */
static int Max_sn;     /* integer arithmetic */
static int Min_sn;     /* integer arithmetic */
static int Depth_sn;   /* depth */
static int Vars_sn;    /* vars */
static int Call_sn;    /* vars */
static int Avar_sn;    /* anonymous variable */

/*************
 *
 *   weight_beta_check()
 *
 *************/

static
BOOL weight_beta_check(Term b)
{
  if (SYMNUM(b) == Sum_sn ||
      SYMNUM(b) == Prod_sn ||
      SYMNUM(b) == Div_sn ||
      SYMNUM(b) == Min_sn ||
      SYMNUM(b) == Max_sn)
    return weight_beta_check(ARG(b,0)) &&  weight_beta_check(ARG(b,1));
  else if (SYMNUM(b) == Neg_sn)
    return weight_beta_check(ARG(b,0));
  else if (SYMNUM(b) == Depth_sn)
    return TRUE;
  else if (SYMNUM(b) == Vars_sn)
    return TRUE;
  else if (SYMNUM(b) == Call_sn)
    return TRUE;
  else if (SYMNUM(b) == Weight_sn)
    return TRUE;
  else {
    double d;
    if (term_to_number(b, &d))
      return TRUE;
    else {
      printf("weight_rule_check, right side of rule not understood\n");
      return FALSE;
    }
  }
}  /* weight_beta_check */

/*************
 *
 *   weight_rule_check()
 *
 *************/

static
BOOL weight_rule_check(Term rule)
{
  if (!is_eq_symbol(SYMNUM(rule))) {
    printf("weight_rule_check, rule is not equality\n");
    return FALSE;
  }
  else  if (SYMNUM(ARG(rule, 0)) != Weight_sn) {
    printf("weight_rule_check, left side must be weight(...)\n");
    return FALSE;
  }
  else
    return weight_beta_check(ARG(rule, 1));
}  /* weight_rule_check */

/*************
 *
 *   init_weight()
 *
 *************/

/* DOCUMENTATION
Initialize weighting.  The rules are copied.
*/

/* PUBLIC */
void init_weight(Plist rules,
		 double variable_weight,
		 double constant_weight,
		 double not_weight,
		 double or_weight,
		 double sk_constant_weight,
		 double prop_atom_weight,
		 double nest_penalty,
		 double depth_penalty,
		 double var_penalty,
		 double complexity)
{
  Plist p;

  Variable_weight = variable_weight;
  Constant_weight = constant_weight;
  Not_weight = not_weight;
  Or_weight = or_weight;
  Prop_atom_weight = prop_atom_weight;
  Sk_constant_weight = sk_constant_weight;
  Nest_penalty = nest_penalty;
  Depth_penalty = depth_penalty;
  Var_penalty = var_penalty;
  Complexity = complexity;

  /* Cache symbol numbers. */

  Weight_sn  = str_to_sn("weight", 1);
  Eq_sn  = str_to_sn(eq_sym(), 2);

  Sum_sn  = str_to_sn("+", 2);
  Prod_sn = str_to_sn("*", 2);
  Div_sn  = str_to_sn("/", 2);
  Max_sn  = str_to_sn("max", 2);
  Min_sn  = str_to_sn("min", 2);
  Depth_sn  = str_to_sn("depth", 1);
  Vars_sn  = str_to_sn("vars", 1);
  Call_sn  = str_to_sn("call", 2);
  Neg_sn  = str_to_sn("-", 1);
  Avar_sn = str_to_sn("_", 0);

  /* Process the rules. */

  Rules = NULL;
  for (p = rules; p; p = p->next) {
    Term rule = copy_term(p->v);
    if (!weight_rule_check(rule)) {
      p_term(rule);
      fatal_error("init_weight, bad rule");
    }
    else {
      term_set_variables(rule, MAX_VARS);
      Rules = plist_append(Rules, rule);
      if (is_term(ARG(ARG(rule,0),0), not_sym(), 1))
	Not_rules = TRUE;
      if (is_term(ARG(ARG(rule,0),0), or_sym(), 2))
	Or_rules = TRUE;
    }
  }
}  /* init_weight */

/*************
 *
 *   apply_depth()
 *
 *************/

static
int apply_depth(Term t, Context subst)
{
  if (VARIABLE(t))
    return term_depth(subst->terms[VARNUM(t)]);
  else if (CONSTANT(t))
    return 0;
  else {
    int depth = 0;
    int i;
    for (i = 0; i < ARITY(t); i++) {
      int d = apply_depth(ARG(t,i), subst);
      depth = IMAX(d, depth);
    }
    return depth + 1;
  }
}  /* apply_depth */

/*************
 *
 *   weight_calc() -- mutually recursive with weight
 *
 *************/

static
double weight_calc(Term b, Context subst)
{
  /* Apply a rule.  Term b is the right side of the rule. 
     The substitution matches the left side of the rule with the
     term being weighed.

     This routine is recursive, applying arithmetic rules to the
     top of b as much as possible.  When a non-arithmetic expression
     is encountered, the substition is applied to it and then
     it is weighed as usual.
   */
  if (VARIABLE(b)) {
    fatal_error("weight_calc, variable in rule");
    return 0;  /* to please the compiler */
  }
  else if (SYMNUM(b) == Weight_sn) {
    Term b_prime = apply(ARG(b,0), subst);
    Context subst2 = get_context();
    double wt = weight(b_prime, subst2);
    free_context(subst2);
    zap_term(b_prime);
    return wt;
  }
  else if (SYMNUM(b) == Sum_sn)
    return
      weight_calc(ARG(b,0), subst) +
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Prod_sn)
    return
      weight_calc(ARG(b,0), subst) *
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Div_sn)
    return
      weight_calc(ARG(b,0), subst) /
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Max_sn) {
    int w1 = weight_calc(ARG(b,0), subst);
    int w2 = weight_calc(ARG(b,1), subst);
    return IMAX(w1,w2);
  }
  else if (SYMNUM(b) == Min_sn) {
    int w1 = weight_calc(ARG(b,0), subst);
    int w2 = weight_calc(ARG(b,1), subst);
    return IMIN(w1,w2);
  }
  else if (SYMNUM(b) == Neg_sn) {
    return -weight_calc(ARG(b,0), subst);
  }
  else if (SYMNUM(b) == Depth_sn)
    return apply_depth(ARG(b,0), subst);
  else if (SYMNUM(b) == Vars_sn) {
    Term b_prime = apply(ARG(b,0), subst);
    int n = number_of_vars_in_term(b_prime);
    zap_term(b_prime);
    return n;
  }
  else if (SYMNUM(b) == Call_sn) {
    char *prog = term_symbol(ARG(b,0));
    Term b_prime = apply(ARG(b,1), subst);
    double x = call_weight(prog, b_prime);
    zap_term(b_prime);
    return x;
  }
  else {
    double wt;
    if (term_to_number(b, &wt))
      return wt;
    else {
      fatal_error("weight_calc, bad rule");
      return 0;  /* to please the compiler */
    }
  }
}  /* weight_calc */

/*************
 *
 *   weight() -- mutually recursive with weight_calc
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
double weight(Term t, Context subst)
{
  if (VARIABLE(t))
    return Variable_weight;
  else {
    /* Look for a rule to apply. */
    Plist p;
    for (p = Rules; p; p = p->next) {
      Term rule = p->v;           /* weight(f(x)) = 3 + weight(x) */
      Term alpha = ARG(rule,0);
      Term beta  = ARG(rule,1);
      Trail tr = NULL;
      if (match_weight(ARG(alpha,0), subst, t, &tr, Avar_sn)) {
	/* We found a rule.  Now calculate the weight. */
	double wt = weight_calc(beta, subst);
	undo_subst(tr);
	return wt;
      }
    }
    /* Nothing matches; return the default. */
    if (CONSTANT(t)) {
      if (skolem_term(t) && Sk_constant_weight != 1)
	return Sk_constant_weight;
      else if (relation_symbol(SYMNUM(t)))
	return Prop_atom_weight;
      else
	return Constant_weight;
    }
    else {
      /* sum of weights of subterms, plus 1 */
      double wt = 1;
      int i;
      for (i = 0; i < ARITY(t); i++) {
	double arg_wt = weight(ARG(t, i), subst);

	if (Nest_penalty != 0 &&
	    ARITY(t) <= 2 &&
	    SYMNUM(t) == SYMNUM(ARG(t,i)))
	  wt += Nest_penalty;
	
	wt += arg_wt;
      }
      return wt;
    }
  }
}  /* weight */

/*************
 *
 *   clause_weight()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
double clause_weight(Literals lits)
{
  double wt;
  
  if (!Not_rules && !Or_rules) {
    /* There are no rules for OR or NOT, so we don't need to construct a
       Term representation of the clause. */
    Literals lit;
    wt = 0;
    for (lit = lits; lit; lit = lit->next) {
      Context subst = get_context();
      wt += weight(lit->atom, subst);
      free_context(subst);
    }
    wt += (negative_literals(lits) * Not_weight);
    wt += ((number_of_literals(lits)-1) * Or_weight);
  }
  else {
    /* Build a temporary Term representation of the clause and weigh that.
       This is done in case there are weight rules for OR or NOT. */
    Term temp = lits_to_term(lits);
    Context subst = get_context();
    wt = weight(temp, subst);
    free_context(subst);
    free_lits_to_term(temp);

    /* If there are no Not_rules, we have already added one for each not;
       so we undo that and add the correct amount.  Same for Or_rules. */
       
    if (!Not_rules)
      wt += (negative_literals(lits) * (Not_weight - 1));
    if (!Or_rules)
      wt += ((number_of_literals(lits) - 1) * (Or_weight - 1));
  }

  if (Depth_penalty != 0)
    wt += Depth_penalty * literals_depth(lits);

  if (Var_penalty != 0)
    wt += Var_penalty * number_of_variables(lits);
 
  if (Complexity != 0)
    wt += Complexity * (1 - clause_complexity(lits, 4, 0));

  return wt;

}  /* clause_weight */
