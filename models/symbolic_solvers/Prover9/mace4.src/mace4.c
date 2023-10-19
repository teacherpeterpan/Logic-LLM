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

#include "../ladr/banner.h"

#include <signal.h>

/*************
 *
 *   init_attrs()
 *
 *************/

void init_attrs(void)
{
  // This will allow these attributes to occur on clauses.
  // Mace4 will ignore these attributes.

  int id;
  id = register_attribute("label",         STRING_ATTRIBUTE);
  id = register_attribute("bsub_hint_wt",     INT_ATTRIBUTE);
  id = register_attribute("answer",          TERM_ATTRIBUTE);
  id = register_attribute("action",          TERM_ATTRIBUTE);
  id = register_attribute("action2",         TERM_ATTRIBUTE);
}  /* init_attrs */

/*************
 *
 *   mace4_sig_handler()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mace4_sig_handler(int condition)
{
  printf("\nmace4_sig_handler: condition %d", condition);
  switch (condition) {
  case SIGSEGV:
    p_stats();
    mace4_exit(MACE_SIGSEGV_EXIT);
    break;
  case SIGINT:
    p_stats();
    mace4_exit(MACE_SIGINT_EXIT);
    break;
  case SIGUSR1:
    p_stats();
    fflush(stdout);
    break;
  default: fatal_error("mace4_sig_handler, unknown signal");
  }
}  /* mace4_sig_handler */

/*************
 *
 *   process_distinct_terms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist process_distinct_terms(Plist distinct)
{
  Plist p;
  Plist forms = NULL;
  for (p = distinct; p; p = p->next) {
    Term list = p->v;
    if (!proper_listterm(list))
      fatal_error("process_distinct_terms: lists must be proper, e.g., [a,b,c].\n");
    while (!nil_term(list)) {
      Term a = ARG(list,0);
      Term rest = ARG(list,1);
      while (!nil_term(rest)) {
	Term b = ARG(rest,0);
	Term neq = build_unary_term_safe(not_sym(),
					 build_binary_term_safe(eq_sym(),
								copy_term(a),
								copy_term(b)));
	Formula f = term_to_formula(neq);
	zap_term(neq);
	forms = plist_append(forms, f);
	rest = ARG(rest, 1);
      }
      list = ARG(list, 1);
    }
  }
  return forms;
}  /* process_distinct_terms */

/*************
 *
 *   read_mace4_input()
 *
 *************/

static
Plist read_mace4_input(int argc, char **argv, BOOL allow_unknown_things,
		      Mace_options opt)
{
  Plist wild_formulas, goals;
  Plist distinct_lists, distinct_forms;
  Plist wild_terms, hints;  /* we won't use these */

  // Tell the top_input package what lists to accept and where to put them.

  // Accept hints, but they will not be used.

  accept_list("hints", FORMULAS, TRUE, &hints);

  // Accept goals; these are negated individually (each must be falsified)

  accept_list("goals", FORMULAS, FALSE, &goals);

  // Accept lists of distinct items

  accept_list("distinct", TERMS, FALSE, &distinct_lists);

  // Accept any other clauses and formulas.  Each must be true.

  accept_list("",    FORMULAS, FALSE, &wild_formulas);

  // Accept any terms.  These will not be used.

  accept_list("",      TERMS,    FALSE, &wild_terms);

  // Read commands such as set, clear, op, lex.
  // Read lists, filling in variables given to the accept_list calls.

  print_separator(stdout, "INPUT", TRUE);

  read_all_input(argc, argv, stdout, TRUE,
		 allow_unknown_things ? WARN_UNKNOWN : KILL_UNKNOWN);

  if (wild_terms)
    printf("%%   term list(s) ignored\n");
  if (hints)
    printf("%%   hints list(s) ignored\n");

  process_command_line_args(argc, argv, opt);

  print_separator(stdout, "end of input", TRUE);

  if (!option_dependencies_state()) {
    /* This might be needed in the future. */
    printf("\n%% Enabling option dependencies (ignore applies only on input).\n");
    enable_option_dependencies();
  }

  distinct_forms = process_distinct_terms(distinct_lists);
  wild_formulas = plist_cat(wild_formulas, distinct_forms);

  wild_formulas = embed_formulas_in_topforms(wild_formulas, TRUE);
  goals = embed_formulas_in_topforms(goals, FALSE);

  // Clausify 

  print_separator(stdout, "PROCESS NON-CLAUSAL FORMULAS", TRUE);
  printf("\n%% Formulas that are not ordinary clauses:\n");

  wild_formulas = process_input_formulas(wild_formulas, TRUE);
  goals = process_goal_formulas(goals, TRUE);  /* negates goals */

  print_separator(stdout, "end of process non-clausal formulas", TRUE);

  wild_formulas = plist_cat(wild_formulas, goals);

  return wild_formulas;
}  /* read_mace4_input */

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  struct mace_options opt;
  Plist clauses;
  Mace_results results;

  /* Following says whether to ignore unregognized set/clear/assigns. */
  BOOL prover_compatability_mode = member_args(argc, argv, "-c");

  init_standard_ladr();
  init_mace_options(&opt);  /* We must do this before calling usage_message. */
  init_attrs();  

  if (member_args(argc, argv, "help") ||
      member_args(argc, argv, "-help")) {
    usage_message(stderr, &opt);
    exit(1);
  }

  print_banner(argc, argv, PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);
  set_program_name(PROGRAM_NAME);   /* for conditional input */

  signal(SIGINT,  mace4_sig_handler);
  signal(SIGUSR1, mace4_sig_handler);
  signal(SIGSEGV, mace4_sig_handler);

  clauses = read_mace4_input(argc, argv, prover_compatability_mode, &opt);
			     
  print_separator(stdout, "CLAUSES FOR SEARCH", TRUE);
  fwrite_clause_list(stdout, clauses, "mace4_clauses", CL_FORM_BARE);
  print_separator(stdout, "end of clauses for search", TRUE);

  results = mace4(clauses, &opt);

  mace4_exit(results->return_code);  /* print messages and exit */

  exit(0);  /* won't happen */

}  /* main */
