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

#include "msearch.h"

/* External variables defined in mace4.c. */

extern Mace_options Opt;

extern Symbol_data *Sn_to_mace_sn;

extern struct cell *Cells;

extern int Domain_size;
extern Term *Domain; /* array of terms representing (shared) domain elements */

extern int Relation_flag;  /* term flag */
extern int Negation_flag;  /* term flag */

extern int Eq_sn;
extern int Or_sn;
extern int Not_sn;

extern struct mace_stats Mstats;

/*************
 *
 *   eterm()
 *
 *   Check if a term is evaluable.  If so, set the id.
 *   For example, f(k,j,i) gives k*n*n + j*n + i + base.
 *   If the term is not evaluable, set the id to -1.
 *
 *************/

BOOL eterm(Term t, int *pid)
{
  *pid = -1;  /* We must return -1 if the term is not evaluable. */
  if (t == NULL || VARIABLE(t) || arith_rel_term(t) || arith_op_term(t))
    return FALSE;
  else {
    int i;
    int mult = 1;
    int id = Sn_to_mace_sn[SYMNUM(t)]->base;
    for (i = ARITY(t)-1; i >= 0; i--) {
      if (!VARIABLE(ARG(t,i)))
        return FALSE;
      else
        id += VARNUM(ARG(t,i)) * mult;
      mult *= Domain_size;
    }
    *pid = id;
    return TRUE;
  }
}  /* eterm */

/*************
 *
 *   decode_eterm_id()
 *
 *   Build and return the e-term corresponding to the given ID.
 *   This inverse of eterm_id.  If it is temporary, make sure
 *   to call zap_mterm(t) when finished with it.
 *
 *************/

Term decode_eterm_id(int id)
{
  /* Assume the id is in range. */
  Symbol_data s = Cells[id].symbol;
  Term t = get_rigid_term_dangerously(s->sn, s->arity);
  int n = Domain_size;
  int x = id - s->base;
  int i;
  for (i = s->arity - 1; i >= 0; i--) {
    int p = int_power(n, i);
    int e = x / p;
    ARG(t, (s->arity-1) - i) = Domain[e];
    x = x % p;
  }
  return t;
}  /* decode_eterm_id */

/*************
 *
 *   pvalues_check()
 *
 *   If there is exactly 1 possible value, return it; else return NULL;
 *
 *************/

static
Term pvalues_check(Term *a, int n)
{
  Term b = NULL;
  int i;
  for (i = 0; i < n; i++) {
    if (a[i] != NULL) {
      if (b != NULL)
	return NULL;
      else
	b = a[i];
    }
  }
  if (b == NULL)
    fatal_error("pvalues_check: no possible values\n");
  return b;
}  /* pvalues_check */

/*************
 *
 *   nterm_check_and_process()
 *
 *   Given a new unit (which is not an ASSIGNMENT or ELIMINATION),
 *   check to see if it is a NEAR_ASSIGNMENT or NEAR_ELIMINATION.
 *   If so, insert it into the index (which updates the stack),
 *   and put it into the job list.
 *
 *   Note that these operations occur even if the some of the
 *   individual operations are disbaled.  However, this routine
 *   should not be called if all negative propagation is disabled.
 *
 *************/

static
void nterm_check_and_process(Term lit, Mstate state)
{
  int pos;
  int id;
  int neg = NEGATED(lit);
  int eq = EQ_TERM(lit);
  int type = (neg && eq ? NEAR_ELIMINATION : NEAR_ASSIGNMENT);
  if (eq) {
    Term a0 = ARG(lit,0);
    Term a1 = ARG(lit,1);

    if (VARIABLE(a1) && nterm(a0, &pos, &id)) {
      insert_negprop_eq(lit, a0, VARNUM(a1), state);
      job_prepend(state, type, id, a0, a1, pos);
    }
    else if (VARIABLE(a0) && nterm(a1, &pos, &id)) {
      insert_negprop_eq(lit, a1, VARNUM(a0), state);
      job_prepend(state, type, id, a1, a0, pos);
    }
  }
  else if (nterm(lit, &pos, &id)) {
    insert_negprop_noneq(lit, state);
    job_prepend(state, NEAR_ASSIGNMENT, id, lit,
		(neg ? Domain[0] : Domain[1]), pos);
  }
}  /* nterm_check_and_process */

/*************
 *
 *   new_assignment()
 *
 *   If a contradiction is found, set state->ok to FALSE.
 *
 *************/

static
void new_assignment(int id, Term value, Mstate state)
{
  if (Cells[id].value == NULL) {
    /* Note that alpha of the new rule is indexed, so the
       rule will rewrite itself.  That IS what we want. */
    state->stack = update_and_push((void **) &(Cells[id].value),
				   value, state->stack);
    if (flag(Opt->trace)) {
      printf("\t\t\t\t\t");
      fwrite_term(stdout, Cells[id].eterm);
      printf(" = %d\n", VARNUM(value));
    }
    job_prepend(state, ASSIGNMENT, id, NULL, NULL, -1);
    Mstats.propagations++;      
    return;
  }
  else if (Cells[id].value == value)
    return;   /* ok: we already have the rule */
  else {
    /* contradiction: we have an incompatible rule */
    if (flag(Opt->trace)) {
      printf("\t\t\t\t\t");
      fwrite_term(stdout, Cells[id].eterm);
      printf(" = %d BACKUP!\n", VARNUM(value));
    }
    state->ok = FALSE;
    return;
  }
}  /* new_assignment */

/*************
 *
 *   new_elimination()
 *
 *   If a contradiction is found, set state->ok to FALSE.
 *
 *************/

void new_elimination(int id, Term beta, Mstate state)
{
  if (Cells[id].value == beta) {
    if (flag(Opt->trace)) {
      printf("\t\t\t\t\t");
      fwrite_term(stdout, Cells[id].eterm);
      printf(" != %d BACKUP!\n", VARNUM(beta));
    }
    state->ok = FALSE;   /* contradiction: cell already has that value! */
    return;
  }
  else if (Cells[id].value != NULL)
    return;   /* ok: cell already has a (different) value */ 
  else if (Cells[id].possible[VARNUM(beta)] == NULL)
    return;   /* ok: already crossed off */
  else {
    /* New unit f(1,2) != 3.  Cross it off and push for negprop. */
    Term value;
    Mstats.cross_offs++;
    state->stack=update_and_push((void **) &(Cells[id].possible[VARNUM(beta)]),
				 NULL, state->stack);
    if (flag(Opt->trace)) {
      printf("\t\t\t\t\t");
      fwrite_term(stdout, Cells[id].eterm);
      printf(" != %d\n", VARNUM(beta));
    }
    if (flag(Opt->negprop))
      job_prepend(state, ELIMINATION, id, NULL, beta, -1);

    value = pvalues_check(Cells[id].possible, Domain_size);
    if (value == NULL)
      return;  /* ok: nothing more to do */
    else {
      Mstats.rules_from_neg++;
      new_assignment(id, value, state);
    }
  }
}  /* new_elimination */

/*************
 *
 *   process_clause() -- process new unit clauses
 *
 *   There are several things to check for:
 *     (1) contradiction in the form of a FALSE clause;
 *     (2) contradiction in the form of a rule inconsistent with an existing
 *         rule (this would eventually produce a FALSE clause, but why wait?);
 *     (3) a new assignment,  e.g., f(1,2)=3, P(4), ~P(5);
 *     (4) a new elimination, e.g., f(1,2)!=3;
 *     (5) near assignment, near elimination (for negative propagation).
 *
 *   If a contradiction is found, set state->ok to FALSE.
 *
 *************/

static
void process_clause(Mclause c, Mstate state)
{
  if (c->subsumed)
    return;
  else if (c->u.active == 0) {
    if (flag(Opt->trace))
      printf("\t\t\t\t\t** BACKUP **\n");
    state->ok = FALSE;
    return;
  }
  else if (c->u.active != 1)
    return;   /* nonunit, so do nothing */
  else {
    /* OK, we have a nonsubsumed unit. */
    Term lit, beta;
    BOOL negated, eq;
    int id;
    int i = 0;
    while (FALSE_TERM(LIT(c,i)))
      i++;
    
    lit = LIT(c,i);
    negated = NEGATED(lit);
    eq = EQ_TERM(lit);

#if 0
    printf("process_clause 1: ");
    p_matom(lit);
#endif

    if (!eq && eterm(lit, &id))
      beta = Domain[negated ? 0 : 1]; /* P(1,2,3) or ~P(1,2,3) */
    else if (eq && eterm(ARG(lit,0),&id) && VARIABLE(ARG(lit,1)))
      beta = ARG(lit,1);  /* f(1,2)=3 or f(1,2)!=3 */
    else if (eq && eterm(ARG(lit,1),&id) && VARIABLE(ARG(lit,0)))
      beta = ARG(lit,0);  /* 3=f(1,2) or 3!=f(1,2) */
    else {
      if (flag(Opt->negprop))
	/* If it is an nterm, index and insert into job list. */
	nterm_check_and_process(lit, state);
      return;  /* We cannot do anything else with the unit. */
    }

    if (eq && negated)
      new_elimination(id, beta, state);  /* f(1,2) != 3 */
    else
      new_assignment(id, beta, state);   /* f(1,2) = 3, P(0), ~P(0) */
  }
}  /* process_clause */

/*************
 *
 *   handle_literal()
 *
 *************/

static
Mclause handle_literal(Term lit, Term result, Mstate state)
{
  Mclause clause_to_process = NULL;
  Mclause parent_clause = lit->container;
  /* evaluable eterm literal -- this will rewrite to TRUE or FALSE */
  int pos = lit_position(parent_clause, lit);
  BOOL negated = NEGATED(lit);
  Mstats.rewrite_bools++;
  /* Result should be either 0 or 1, because lit is a literal.
     if the literal is negated, negate the result. */
  if (negated)
    result = (result == Domain[0] ? Domain[1] : Domain[0]);
  state->stack = update_and_push((void **) &(LIT(parent_clause,pos)),
				 result, state->stack);
				    
  /* Now we have to update fields in the clause. */
  if (FALSE_TERM(result)) {
    /* decrement the count of active literals */
    state->stack = update_and_push((void **) &(parent_clause->u.active),
				   (void *) (parent_clause->u.active-1),
				   state->stack);
    clause_to_process = parent_clause;
  }
  else
    /* mark clause as subsumed */
    state->stack = update_and_push((void **) &(parent_clause->subsumed),
				   (void *) TRUE, state->stack);
  return clause_to_process;
}  /* handle_literal */

/*************
 *
 *   propagate_positive()
 *
 *   Propagate a positive assignment.  This includes negated non-equality
 *   atoms.  For example, ~P(1,2,3) is thought of as p(1,2,3) = FALSE.
 *
 *   If a contradiction is found, set state->ok to FALSE.
 *
 *************/

static
void propagate_positive(int id, Mstate state)
{
  Term t;
#if 0
  printf("propagage_positive: "); p_term(Cells[id].eterm);
#endif
  for (t = Cells[id].occurrences; t != NULL; t = t->u.vp) {
    /* foreach term the rule applies to */
    Term curr = t;
    Mclause clause_to_process;
    BOOL index_it;
    /* The following loop iterates up toward the root of the clause,
       rewriting terms.  We stop when we get to a literal, an eterm that
       cannot be rewritten (we then index the eterm in this case), or when
       we get to a non-eterm. */
#if 0
    printf("rewriting: "); p_mclause(containing_mclause(curr));
#endif
    while (!LITERAL(curr) &&            /* stop if literal */
	   !arith_op_term(curr) &&      /* stop if arithmetic term */
	   eterm(curr, &id) &&          /* stop if not eterm */
	   Cells[id].value != NULL) {   /* stop if eterm not evaluable*/
      Term result = Cells[id].value;
      Term parent = curr->container;
      int pos = arg_position(parent, curr);
      state->stack = update_and_push((void **) &(ARG(parent,pos)),
				     result, state->stack);
      Mstats.rewrite_terms++;
      curr = parent;
    }  /* while rewriting upward */

#if 0
    printf("done:      "); p_mclause(containing_mclause(curr));
#endif

    clause_to_process = NULL;  /* set to possible new rule */
    index_it = FALSE;        /* should curr be indexed? */

    if (arith_rel_term(curr) || arith_op_term(curr)) {
      Term parent_lit = containing_mliteral(curr);
      Mclause parent_clause = parent_lit->container;
      if (!parent_clause->subsumed) {
	BOOL evaluated;
	int b = arith_eval(parent_lit, &evaluated);
	if (evaluated) {
	  Term result;
	  if (b != 0 && b != 1)
	    fatal_error("propagate_positive, arith_eval should be Boolean");
	  result = (b ? Domain[1] : Domain[0]);
	  clause_to_process = handle_literal(parent_lit, result, state);
	}
	else if (EQ_TERM(curr))
	  clause_to_process = parent_clause;
      }
    }
    else if (!LITERAL(curr)) {
      /* curr is a term */
      Term parent = curr->container;
      if (id != -1)
	index_it = TRUE;  /* curr is a non-evaluable eterm */
      /* If curr is 1 or 2 steps away from a literal, process it. */
      if (LITERAL(parent))
	clause_to_process = parent->container;
      else {
	parent = parent->container;
	if (LITERAL(parent))
	  clause_to_process = parent->container;
      }
    }
    else {
      /* curr is a literal (equality or nonquality, positive or negative) */
      Mclause parent_clause = curr->container;
      if (!eterm(curr, &id))
	/* non-eterm literal */
	clause_to_process = parent_clause;
      else if (Cells[id].value == NULL) {
	/* non-evaluable eterm literal */
	index_it = TRUE;
	clause_to_process = parent_clause;
      }
      else if (!parent_clause->subsumed) {
	clause_to_process = handle_literal(curr, Cells[id].value, state);
      }
    }  /* literal */

    if (index_it) {
      /* curr is an evaluable term or literal, e.g., f(1,2), but there is
	 no rule for it. Therefore, we index it so that it can be
	 found in case a rule appears later. */
      Mstats.indexes++;
      state->stack = update_and_push((void **) &(curr->u.vp),
				     Cells[id].occurrences, state->stack);
      state->stack = update_and_push((void **) &(Cells[id].occurrences),
				     curr, state->stack);
    }

    if (clause_to_process != NULL) {
      process_clause(clause_to_process, state);
      if (!state->ok)
	return;
    }
  }  /* foreach occurrence (container) of the cell just assigned */
}  /* propagate_positive */

/*************
 *
 *   propagate()
 *
 *   Do all of the jobs in the Mstate.  If a contradiction is found,
 *   clean up the Mstate by flushing any undone jobs and restoring
 *   from the stack.
 *
 *************/

static
void propagate(Mstate state)
{
  while (state->ok && state->first_job != NULL) {
    /* Negative propagation is applied to all types.
       Positive propagation is applied to ASSIGNMENT only.
    */
    int type = state->first_job->type;
    int id = state->first_job->id;
    Term alpha = state->first_job->alpha;
    Term beta = state->first_job->beta;
    int pos = state->first_job->pos;
    job_pop(state);

    if (type == ASSIGNMENT)
      propagate_positive(id, state);

    if (state->ok && flag(Opt->negprop))
      propagate_negative(type, id, alpha, beta, pos, state);
  }

  if (!state->ok) {
    zap_jobs(state);
    restore_from_stack(state->stack);
    state->stack = NULL;
  }
}  /* propagate */

/*************
 *
 *   assign_and_propagate()
 *
 *   Make an assignment, and propagate its effects.
 *   Return the stack of events that occur.  If the propagation
 *   gives a contradiction, return NULL.
 *
 *************/

Estack assign_and_propagate(int id, Term value)
{
  Estack tmp_stack;
  Mstate state = get_mstate();

  if (Cells[id].value == value)
    fatal_error("assign_and_propagate: repeated assignment");
  if (Cells[id].value != NULL)
    fatal_error("assign_and_propagate: contradictory assignment");

  /* First make the assignment and initialize the job list. */

  state->stack = update_and_push((void **) &(Cells[id].value), value, NULL);
  job_prepend(state, ASSIGNMENT, id, NULL, NULL, -1);

  /* Process the job list (which can grow during propagation). */

  propagate(state);

  /* Return the stack (which is NULL iff we have a contradiction). */
  
  tmp_stack = state->stack;
  free_mstate(state);
  return tmp_stack;
}  /* assign_and_propagate */

/*************
 *
 *   process_initial_clause()
 *
 *   This routine processes the initial ground clauses.  This includes
 *   checking for the empty clause, checking for unit conflicts, and
 *   unit propagation.  If a contradiction is found, state->ok is set
 *   to FALSE.
 *
 *************/

void process_initial_clause(Mclause c, Mstate state)
{
  process_clause(c, state);  /* handles empty clause and nonsubsumed units */

  if (state->ok && state->first_job != NULL)
    propagate(state);  /* Process_clause pushed a job, so we propagate it. */
}  /* process_initial_clause */
