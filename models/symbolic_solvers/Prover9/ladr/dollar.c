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

#include "dollar.h"
#include "just.h"
#include "parautil.h"

/* Private definitions and types */

struct rule {
  Topform c;
  Term alpha;
  Term beta;
  Term condition;
  struct rule *next;
};

enum {
  SUM_OP=1, PROD_OP, DIV_OP, MOD_OP, MIN_OP, MAX_OP, ABS_OP,
  NEG_OP,
  LT_OP, LE_OP, GT_OP, GE_OP,      /* arithmetic comparison */
  LLT_OP, LLE_OP, LGT_OP, LGE_OP,  /* lexical comparison */
  AND_OP, OR_OP,
  AND2_OP, OR2_OP,
  IF_OP,
  ID_OP, NID_OP,                   /* for all terms, including integers */
  VAR_OP, CONST_OP, GROUND_OP};

static int Symbols_size;
static int *Op_codes;
static struct rule **Rules;
static int Local_evals;

/*
 * memory management
 */

/*************
 *
 *   init_dollar_eval()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_dollar_eval(Clist rules)
{
  Clist_pos p;

  Symbols_size = greatest_symnum() + 1;
  Op_codes = calloc(Symbols_size, sizeof(int));
  
  Op_codes[str_to_sn("+", 2)]        = SUM_OP;
  Op_codes[str_to_sn("*", 2)]        = PROD_OP;
  Op_codes[str_to_sn("/", 2)]        = DIV_OP;
  Op_codes[str_to_sn("mod", 2)]      = MOD_OP;
  Op_codes[str_to_sn("min", 2)]      = MIN_OP;
  Op_codes[str_to_sn("max", 2)]      = MAX_OP;
  Op_codes[str_to_sn("abs", 2)]      = ABS_OP;
  Op_codes[str_to_sn("-", 1)]        = NEG_OP;

  Op_codes[str_to_sn("<", 2)]        = LT_OP;
  Op_codes[str_to_sn("<=", 2)]       = LE_OP;
  Op_codes[str_to_sn(">", 2)]        = GT_OP;
  Op_codes[str_to_sn(">=", 2)]       = GE_OP;

  Op_codes[str_to_sn("@<", 2)]       = LLT_OP;
  Op_codes[str_to_sn("@<=", 2)]      = LLE_OP;
  Op_codes[str_to_sn("@>", 2)]       = LGT_OP;
  Op_codes[str_to_sn("@>=", 2)]      = LGE_OP;

  Op_codes[str_to_sn("==", 2)]       = ID_OP;
  Op_codes[str_to_sn("!==", 2)]      = NID_OP;

  Op_codes[str_to_sn("variable", 1)] = VAR_OP;
  Op_codes[str_to_sn("constant", 1)] = CONST_OP;
  Op_codes[str_to_sn("ground", 1)]   = GROUND_OP;

  Op_codes[str_to_sn("&&", 2)]       = AND2_OP;
  Op_codes[str_to_sn("||", 2)]       = OR2_OP;
  Op_codes[str_to_sn("&", 2)]        = AND_OP;
  Op_codes[str_to_sn("|", 2)]        = OR_OP;
  Op_codes[str_to_sn("if", 3)]       = IF_OP;

  Rules = calloc(Symbols_size, sizeof(void *));

  for (p = rules->last; p; p = p->prev) {  /* backward */
    Topform c = p->c;
    Term rule = c->literals->atom;
    Term alpha = NULL, beta, condition;
    if (number_of_literals(c->literals) != 1)
      fatal_error("demodulator has too many literals");

    mark_oriented_eq(c->literals->atom);  /* ok if not eq */

    if (!c->literals->sign) {
      condition = NULL;
      alpha = rule;
      beta  = get_rigid_term(false_sym(), 0);
    }
    else if (is_term(rule, "->", 2) &&
	     (eq_term(ARG(rule,1)) || is_term(ARG(rule,1), "<->", 2))) {
      condition = ARG(rule,0);
      alpha = ARG(ARG(rule,1),0);
      beta  = ARG(ARG(rule,1),1);
    }
    else if (is_term(rule, "<-", 2) &&
	     (eq_term(ARG(rule,0)) || is_term(ARG(rule,0), "<->", 2))) {
      condition = ARG(rule,1);
      alpha = ARG(ARG(rule,0),0);
      beta  = ARG(ARG(rule,0),1);
    }
    else if (is_term(rule, "<->", 2) || eq_term(rule)) {
      condition = NULL;
      alpha = ARG(rule,0);
      beta  = ARG(rule,1);
    }
    else {
      /* Assume it's an atomic formula to be rewritten to $T. */
      condition = NULL;
      alpha = rule;
      beta  = get_rigid_term(true_sym(), 0);
    }
    {
      int symnum = SYMNUM(alpha);
      if (symnum >= Symbols_size)
	fatal_error("init_dollar_eval, symnum too big");
      struct rule *r = malloc(sizeof(struct rule));
      r->c = c;
      r->alpha = alpha;
      r->beta = beta;
      r->condition = condition;
      r->next = Rules[symnum];  /* insert at beginning */
      Rules[symnum] = r;
    }
  }
}  /* init_dollar_eval */

/*************
 *
 *   evaluable_predicate()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_predicate(int symnum)
{
  if (symnum >= Symbols_size)
    return FALSE;
  else if (Rules[symnum])
    return TRUE;
  else if (Op_codes[symnum])
    return TRUE;
  else
    return FALSE;
}  /* evaluable_predicate */

/*************
 *
 *   dollar_eval()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term dollar_eval(Term t)
{
  if (SYMNUM(t) < 0 || SYMNUM(t) >= Symbols_size)
    return NULL;
  else {
    int op_code = Op_codes[SYMNUM(t)];
    if (op_code == 0)
      return NULL;
    else {
      int i0, i1;
      BOOL b0, b1;
      Term result = NULL;
      switch (op_code) {

	/* INT x INT -> INT */

      case SUM_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 + i1);
	break;
      case PROD_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 * i1);
	break;
      case DIV_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 / i1);
	break;
      case MOD_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 % i1);
	break;
      case MIN_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 < i1 ? i0 : i1);
	break;
      case MAX_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = int_to_term(i0 > i1 ? i1 : i1);
	break;

	/* INT -> INT */

      case ABS_OP:
	if (term_to_int(ARG(t,0), &i0))
	  result = int_to_term(i0 >= 0 ? i0 : -i0);
	break;

	/* INT -> INT, BOOL->BOOL */

      case NEG_OP:
	if (term_to_int(ARG(t,0), &i0))
	  result = int_to_term(-i0);
	else if (term_to_bool(ARG(t,0), &b0))
	  result = bool_to_term(!b0);
	break;

	/* INT x INT -> BOOL */

      case LT_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = bool_to_term(i0 < i1);
	break;
      case LE_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = bool_to_term(i0 <= i1);
	break;
      case GT_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = bool_to_term(i0 > i1);
	break;
      case GE_OP:
	if (term_to_int(ARG(t,0), &i0) && term_to_int(ARG(t,1), &i1))
	  result = bool_to_term(i0 >= i1);
	break;

	/* BOOL x BOOL -> BOOL */

	/* Ok if one of the args to be non-Bool, e.g., ($T & junk) = junk */

      case AND_OP:
      case AND2_OP:
	if (term_to_bool(ARG(t,0), &b0)) {
	  if (b0)
	    result = copy_term(ARG(t,1));
	  else
	    result = bool_to_term(FALSE);
	}
	else if (term_to_bool(ARG(t,1), &b1)) {
	  if (b1)
	    result = copy_term(ARG(t,0));
	  else
	    result = bool_to_term(FALSE);
	}
	break;

      case OR_OP:
      case OR2_OP:
	if (term_to_bool(ARG(t,0), &b0)) {
	  if (b0)
	    result = bool_to_term(TRUE);
	  else
	    result = copy_term(ARG(t,1));
	}
	else if (term_to_bool(ARG(t,1), &b1)) {
	  if (b1)
	    result = bool_to_term(TRUE);
	  else
	    result = copy_term(ARG(t,0));
	}
	break;

	/* Term x Term -> BOOL */

      case ID_OP:
	result = bool_to_term(term_ident(ARG(t,0), ARG(t,1)));
	break;

      case NID_OP:
	result = bool_to_term(!term_ident(ARG(t,0), ARG(t,1)));
	break;

	/* INT x INT -> BOOL */

      case LLT_OP:
	result = bool_to_term(term_compare_basic(ARG(t,0),ARG(t,1)) == LESS_THAN);
	break;
      case LLE_OP: {
	Ordertype r = term_compare_basic(ARG(t,0),ARG(t,1));
	result = bool_to_term(r == LESS_THAN || r == SAME_AS);
	break;
      }
      case LGT_OP:
	result = bool_to_term(term_compare_basic(ARG(t,0),ARG(t,1)) == GREATER_THAN);
	break;
      case LGE_OP: {
	Ordertype r = term_compare_basic(ARG(t,0),ARG(t,1));
	result = bool_to_term(r == GREATER_THAN || r == SAME_AS);
	break;
      }

	/* Term -> BOOL */

      case VAR_OP:
	result = bool_to_term(VARIABLE(ARG(t,0)));
	break;

      case CONST_OP:
	result = bool_to_term(CONSTANT(ARG(t,0)));
	break;

      case GROUND_OP:
	result = bool_to_term(ground_term(ARG(t,0)));
	break;

	/* else error */

      default:
	printf("bad opcode is %d\n", op_code);
	fatal_error("dollar_eval: bad opcode");
      }
      return result;
    }
  }
}  /* dollar_eval */

static Term rewrite(Term t, int flag, I3list *steps);  /* mutual recursion */

/*************
 *
 *   rewrite_top()
 *
 *************/

static
Term rewrite_top(Term t, int flag, I3list *steps)
{
  Term t1 = dollar_eval(t);
  if (t1 != NULL) {
    zap_term(t);
    Local_evals++;
    return t1;
  }
  else if (SYMNUM(t) >= Symbols_size)
    return t;
  else if (Rules[SYMNUM(t)] == NULL)  /* we know it's not a variable */
    return t;
  else {
    struct rule *r;
    Context c = get_context();
    Trail tr;
    for (r = Rules[SYMNUM(t)]; r; r = r->next) {
      Term alpha = r->alpha;
      Term beta  = r->beta;
      Term condition = r->condition;
      tr = NULL;
      if (match(alpha, c, t, &tr)) {
	BOOL ok;
	if (condition == NULL)
	  ok = TRUE;
	else {
	  Term condition_rewritten = rewrite(apply(condition, c), flag, steps);
	  ok = true_term(condition_rewritten);
	  zap_term(condition_rewritten);
	  }
	if (ok) {
	  Term contractum = apply_demod(beta, c, flag);
	  undo_subst(tr);
	  free_context(c);
	  zap_term(t);
	  if (!i3list_member(*steps, r->c->id, 0, 1))
	    *steps = i3list_prepend(*steps, r->c->id, 0, 1);  /* for just. */
	  return(rewrite(contractum, flag, steps));
	}
	else
	  undo_subst(tr);
      }
    }
    free_context(c);
    return t;  /* not rewritten */
  }
}  /* rewrite_top */

/*************
 *
 *   rewrite()
 *
 *************/

static
Term rewrite(Term t, int flag, I3list *steps)
{
  if (term_flag(t, flag) || VARIABLE(t))
    return t;
  else {
    int op_code = (SYMNUM(t) < Symbols_size ? Op_codes[SYMNUM(t)] : -1);
    int i;

    switch (op_code) {

    /* There are a few cases where we don't evaluate all args first. */

    /* a & b, a && b */

    case AND_OP:
    case AND2_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (true_term(ARG(t,0))) {
	Term tmp = ARG(t,1);
	zap_term(ARG(t,0));
	free_term(t);
	return rewrite(tmp, flag, steps);
      }
      else if (false_term(ARG(t,0))) {
	zap_term(t);
	return bool_to_term(FALSE);
      }
      break;

    /* a | b, a || b */

    case OR_OP:
    case OR2_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (false_term(ARG(t,0))) {
	Term tmp = ARG(t,1);
	zap_term(ARG(t,0));
	free_term(t);
	return rewrite(tmp, flag, steps);
      }
      else if (true_term(ARG(t,0))) {
	zap_term(t);
	return bool_to_term(TRUE);
      }
      break;
      
    /* if(cond, then_part, else_part) */

    case IF_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (true_term(ARG(t,0))) {
	Term tmp = ARG(t,1);
	zap_term(ARG(t,0));
	zap_term(ARG(t,2));
	free_term(t);
	return rewrite(tmp, flag, steps);
      }
      else if (false_term(ARG(t,0))) {
	Term tmp = ARG(t,2);
	zap_term(ARG(t,0));
	zap_term(ARG(t,1));
	free_term(t);
	return rewrite(tmp, flag, steps);
      }
      break;
    }

    /* rewrite subterms */

    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = rewrite(ARG(t,i), flag, steps);

    /* rewrite top */

    t = rewrite_top(t, flag, steps);

    term_flag_set(t, flag);  /* Mark as fully demodulated. */
    return t;
  }
}  /* rewrite */

/*************
 *
 *   rewrite_with_eval()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void rewrite_with_eval(Topform c)
{
  int flag = claim_term_flag();
  Literals lit;
  I3list steps = NULL;
  Local_evals = 0;

  for (lit = c->literals; lit; lit = lit->next) {
    lit->atom = rewrite(lit->atom, flag, &steps);
    term_flag_clear_recursively(lit->atom, flag);
    if (evaluable_predicate(SYMNUM(lit->atom))) {
      fprintf(stdout, "Fails to evaluate: "); p_term(lit->atom);
      fatal_error("rewrite_with_eval: evaluable_formula fails to evaluate");
    }
  }

  if (steps != NULL) {
    steps = reverse_i3list(steps);
    c->justification = append_just(c->justification, demod_just(steps));
  }

  if (Local_evals > 0)
    c->justification = append_just(c->justification, eval_just(Local_evals));

  release_term_flag(flag);
}  /* rewrite_with_eval */

