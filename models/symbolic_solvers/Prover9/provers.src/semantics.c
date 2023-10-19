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

#include "semantics.h"

/* Private definitions and types */

enum { FALSE_IN_ALL,
       FALSE_IN_SOME
};

static Plist Compiled_interps = NULL;
static int Eval_limit = 0;
static BOOL False_in_all;
static Clock Eval_clock;

/*************
 *
 *   init_semantics()
 *
 *************/

/* DOCUMENTATION
Take a list of terms that represent interpretations,
compile them, and store them locally.
*/

/* PUBLIC */
void init_semantics(Plist interp_terms, Clock eval_clock,
		    char *type, int eval_limit, int eval_var_limit)
{
  Plist p;
  int max_domain_size = 0;
  
  for (p = interp_terms; p; p = p->next) {
    Interp a = compile_interp(p->v, FALSE);
    max_domain_size = IMAX(max_domain_size, interp_size(a));
    Compiled_interps = plist_prepend(Compiled_interps, a);
  }
  Compiled_interps = reverse_plist(Compiled_interps);
  if (str_ident(type, "false_in_all"))
    False_in_all = TRUE;
  else if (str_ident(type, "false_in_some"))
    False_in_all = FALSE;
  else
    fatal_error("init_semantics, bad type");
  if (eval_var_limit == -1)
    Eval_limit = eval_limit;
  else {
    Eval_limit = int_power(max_domain_size, eval_var_limit);
    printf("eval_limit reset to %d.\n", Eval_limit);
  }
  Eval_clock = eval_clock;
}  /* init_semantics */

/*************
 *
 *   eval_limit_ok()
 *
 *************/

static
BOOL eval_limit_ok(Interp p, int number_of_vars)
{
  if (Eval_limit == -1)
    return TRUE;
  else {
    int evals_required = int_power(interp_size(p), number_of_vars);
    return evals_required <= Eval_limit;
  }
}  /* eval_limit_ok */

/*************
 *
 *   eval_in_interps()
 *
 *************/

/* DOCUMENTATION
Evaluate a clause c in the compiled interpretations.  The
parameter "type" should be "false_in_all" or "false_in_some".
<p>
If evaluation would be too expensive (eval_limit), or if
a the clause contains an uninterpreted symbol, the clause
gets the valuse TRUE for that interpretation.
<p>
Finally, if c->semantics gets FALSE, and there are some
interpretations, the clause gets the attribute "false"
(which is intended to be used only for printing).
*/

/* PUBLIC */
void eval_in_interps(Topform c)
{
  if (Compiled_interps == NULL) {
    /* There are no interps, so use default interp: positive literals TRUE. */
    if (negative_clause(c->literals))
      c->semantics = SEMANTICS_FALSE;
    else
      c->semantics = SEMANTICS_TRUE;
  }
  else {

    int num_vars = number_of_variables(c->literals);

    if (!c->normal_vars)
      renumber_variables(c, MAX_VARS);

    if (False_in_all) {
      /* False_in_all:
	   SEMANTICS_FALSE: false in all interps (evaluable in all)
	   SEMANTICS_TRUE:  true in at least one interp
	   SEMANTICS_NOT_EVALUABLE: otherwise
       */
      Plist p = Compiled_interps;
      c->semantics = SEMANTICS_FALSE;
      while (p && c->semantics != SEMANTICS_TRUE) {
	Interp x = p->v;
	if (!eval_limit_ok(x, num_vars) || !evaluable_topform(c, x))
	  c->semantics = SEMANTICS_NOT_EVALUABLE;
	else if (eval_literals(c->literals, x))
	  c->semantics = SEMANTICS_TRUE;
	p = p->next;
      }
    }
      
    else {
      /* False_in_some:
	   SEMANTICS_FALSE: false in at lease one interp
	   SEMANTICS_TRUE:  true in all interps (evaluable in all)
	   SEMANTICS_NOT_EVALUABLE: otherwise
       */
      Plist p = Compiled_interps;
      c->semantics = SEMANTICS_TRUE;
      while (p && c->semantics != SEMANTICS_FALSE) {
	Interp x = p->v;
	if (!eval_limit_ok(x, num_vars) || !evaluable_topform(c, x))
	  c->semantics = SEMANTICS_NOT_EVALUABLE;
	else if (!eval_literals(c->literals, x))
	  c->semantics = SEMANTICS_FALSE;
	p = p->next;
      }
    }

    if (c->semantics == SEMANTICS_FALSE)
      c->attributes = set_string_attribute(c->attributes,label_att(),"false");
  }
}  /* eval_in_interps */

/*************
 *
 *   set_semantics()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_semantics(Topform c)
{
  if (c->semantics == SEMANTICS_NOT_EVALUATED) {
    clock_start(Eval_clock);
    eval_in_interps(c);
    clock_stop(Eval_clock);
  }
}  /* set_semantics */

/*************
 *
 *   update_semantics_new_constant()
 *
 *************/

/* DOCUMENTATION
A new constant, say c_0, has just been introduced with
a clause, say f(x,x) = c_0.
This routine adds c_0 to all of the compiled interpretations.
It is given the same value as f(0,0).
*/

/* PUBLIC */
void update_semantics_new_constant(Topform c)
{
  Term alpha = ARG(c->literals->atom, 0);
  Term beta = ARG(c->literals->atom, 1);
  Plist p;
  int n = biggest_variable(alpha);
  int *vals = malloc((n+1) * sizeof(int));
  int i;

  for (i = 0; i <= n; i++)
    vals[i] = 0;

  for (p = Compiled_interps, i = 1; p; p = p->next, i++) {
    Interp x = p->v;
    int val = eval_term_ground(alpha, x, vals);
    update_interp_with_constant(x, beta, val);
    printf("NOTE: updating interpretation %d: %s=%d.\n",
	   i, sn_to_str(SYMNUM(beta)), val);
  }

  free(vals);
}  /* update_semantics_new_constant */

