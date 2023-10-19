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

#include "foffer.h"

/* Private definitions and types */

/*************
 *
 *   construct_problem() -- formulas are not copied!
 *
 *************/

static
Formula construct_problem(Plist assumptions, Plist goals)
{
  Plist p;
  Plist hyps = NULL;
  Plist concs = NULL;
  Formula hyp;
  Formula conc;
  Formula problem;

  /* If there are goals, construct the statement
     conjunction-of-assumptions -> conjunction-of-goals.

     If there are no goals, construct
     conjunction-of-assumptions -> FALSE.
  */

  /* Build conjunction of assumptions.  If empty, result is TRUE */

  for (p = assumptions; p; p = p->next) {
    Formula f = universal_closure(formula_copy(p->v));
    hyps = plist_append(hyps, f);
  }
  hyp  = formulas_to_conjunction(hyps);
  zap_plist(hyps);   /* shallow */

  /* Goals. */

  if (goals) {
    for (p = goals; p; p = p->next) {
      Formula f = universal_closure(formula_copy(p->v));
      concs = plist_append(concs, f);
    }
    conc = formulas_to_conjunction(concs);
    zap_plist(concs);  /* shallow */
  }
  else
    conc = formulas_to_disjunction(NULL);  /* FALSE */

  problem = imp(hyp, conc);

  return problem;
}  /* construct_problem */

/*************
 *
 *   reduce_problem()
 *
 *************/

static
Formula reduce_problem(Formula f)
{
  /* Formula f is a negated conjecture.  We have to un-negate it,
     reduce it, then re-negate it.  If the reduction succeeds, the
     returned formula is always a flat disjuncion, even if it has
     0 (meaning FALSE) or 1 members. */
  f = miniscope_formula(negate(f), 500);  /* bogo_tick limit, about 50/sec */
  if (f == NULL)
    return NULL;
  else
    return make_disjunction(nnf(negate(f)));
}  /* reduce_problem */

/*************
 *
 *   foffer_clausify()
 *
 *************/

static
Plist foffer_clausify(Formula f)
{
  Plist clauses = clausify_formula(f);
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    c->justification = input_just();
  }
  return clauses;
}  /* foffer_clausify */

/*************
 *
 *   foffer()
 *
 *   Attempt to reduce the problem to independent subproblems and
 *   refute all of the subproblems.  If we fail to reduce the problem,
 *   or we decide not to run the subproblems, return -1; otherwise,
 *   return the ordinary "search" return code from the last subproblem
 *   that was run.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int foffer(Prover_input input)
{
  double children_user_seconds = 0.0;    // running total for all children
  double children_system_seconds = 0.0;  // running total for all children
  int i, nnf_size, cnf_max;
  int n_max, c_max;
  int max_seconds = parm(input->options->max_seconds);
  int return_code;

  Formula f0 = negate(construct_problem(input->sos, input->goals));
  Formula f  = formula_copy(f0);

  // Now f is the NEGATION of the conjecture.

  f = nnf2(f, DISJUNCTION);      // we do this only to get the sizes
  nnf_size = formula_size(f);    // number of Formula nodes
  cnf_max = cnf_max_clauses(f);  // maximum number of clauses in CNF

  printf("\nAttempting problem reduction;"
	 " original problem has <nnf_size,cnf_max> = <%d,%d>.\n",
	 nnf_size, cnf_max);
  fflush(stdout);

  // Problem reduction returns an equivalent formula (or NULL if it fails).

  f = reduce_problem(f);

  if (f == NULL) {
    fprintf(stdout,"\nProblem reduction (%.2f seconds) failed to terminate.\n",
	   user_seconds());
    fprintf(stderr,"\nProblem reduction (%.2f seconds) failed to terminate.\n",
	   user_seconds());
    return -1;
  }
  
  // Formula f is disjunction, even if it has 0 or 1 members; f is still
  // the denial, so members of disjunction are (denied) subproblems.

  // Print sizes of subproblems and get maximum sizes.

  n_max = c_max = 0;
  printf("\nProblem reduction (%.2f sec) gives %d independent subproblems: (",
	 user_seconds(), f->arity);
  for (i = 0; i < f->arity; i++) {
    int n = formula_size(f->kids[i]);
    int c = cnf_max_clauses(f->kids[i]);
    if (!flag(input->options->quiet))
      printf(" <%d,%d>", n, c);
    n_max = IMAX(n_max,n);
    c_max = IMAX(c_max,c);
  }
  printf(" ).\n");
  fflush(stdout);

  // If both maximums are greater than originals, don't reduce.

  printf("\nMax nnf_size of subproblems is %d; max cnf_max is %d.\n",
	 n_max, c_max);
  if (n_max > nnf_size && c_max > cnf_max) {
    fprintf(stdout, "\nProblem reduction failed, because there appear to be\n"
	   "subproblems more complex than the original problem.\n");
    fprintf(stderr, "\nProblem reduction failed, because there appear to be\n"
	   "subproblems more complex than the original problem.\n");
    return -1;
  }

  if (f->arity == 1) {
    fprintf(stderr, "\nNOTE: Problem reduction gives one subproblem that\n"
	    "may be more complex than the original problem.  We are "
	    "proceeding anyway.\n");
    fprintf(stdout, "\nNOTE: Problem reduction gives one subproblem that\n"
	    "may be more complex than the original problem.  We are "
	    "proceeding anyway.\n");
    fflush(stdout);
    fflush(stderr);
  }

  // Attack the subproblems.

  skolem_check(FALSE);  /* don't check that Skolem symbols are new */
  
  for (i = 0, return_code = MAX_PROOFS_EXIT;
       i < f->arity && return_code == MAX_PROOFS_EXIT;
       i++) {

    Prover_results results;

    if (i == 0)
      print_separator(stdout, "FOF REDUCTION MULTISEARCH", TRUE);
    else
      print_separator(stdout, "continuing FOF reduction multisearch", TRUE);
    
    if (flag(input->options->print_initial_clauses)) {
      printf("\nSubproblem %d of %d (negated):\n", i+1, f->arity);
      p_formula(f->kids[i]);
    }
    else
      printf("\nStarting Subproblem %d of %d.\n", i+1, f->arity);
    fflush(stdout);

    input->sos = foffer_clausify(f->kids[i]);

    if (max_seconds != -1) {
      int seconds_used = (children_user_seconds + user_seconds());
      assign_parm(input->options->max_seconds,
		  IMAX(max_seconds - seconds_used, 0),
		  FALSE);
      printf("\nMax_seconds is %d for this subproblem.\n",
	     parm(input->options->max_seconds));
      fflush(stdout);
    }

    results = forking_search(input);

    return_code = results->return_code;
    children_user_seconds   += results->user_seconds;
    children_system_seconds += results->system_seconds;

    zap_prover_results(results);
    delete_clauses(input->sos);
    input->sos = NULL;
    decommission_skolem_symbols();
    skolem_reset();
  }

  print_separator(stdout, "end of multisearch", TRUE);
  
  if (return_code == MAX_PROOFS_EXIT)
    printf("\nAll %d subproblems have been proved, so we are done.\n", i);
  else
    printf("\nSearch failed on subproblem %d.\n", i);

  printf("\nTotal user_CPU=%.2f, system_CPU=%.2f, wall_clock=%u.\n",
	 children_user_seconds + user_seconds(),
	 children_system_seconds + system_seconds(),
	 wallclock());

  fflush(stdout);
  return return_code;
}  /* foffer */

