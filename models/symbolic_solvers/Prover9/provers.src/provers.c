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

#include "provers.h"

#include <unistd.h>  /* for getopt */
#include <signal.h>

/* Private definitions and types */

static char Help_string[] = 
"\nUsage: prover9 [-h] [-x] [-p] [-t <n>] [-f <files>]\n"
"\n"
"  -h         Help.  Also see http://www.cs.unm.edu/~mccune/prover9/\n"
"  -x         set(auto2).  (enhanced auto mode)\n"
"  -p         Fully parenthesize output.\n"
"  -t n       assign(max_seconds, n).  (overrides ordinary input)\n"
"  -f files   Take input from files instead of from standard input.\n"
"\n";

struct arg_options {
  BOOL parenthesize_output;
  BOOL auto2;
  int  max_seconds;
  BOOL files;
};

/*************
 *
 *   get_command_line_args()
 *
 * This gets the command-line options and stores them in a structure
 * for later processing.  If an option is not recognized, the help
 * string is printed and then exit(1).
 *
 *************/

static
struct arg_options get_command_line_args(int argc, char **argv)
{
  extern char *optarg;
  int c;
  struct arg_options opts = {FALSE, FALSE, INT_MAX, FALSE};

  /* getopt() option string

     Initial colon: unrecognized options will not cause errors.

     No colons after option:  no argument.
     One colon after option:  argument required.
     Two colon after options: argument optional. (GNU extension! Don't use it!)
  */
  
  while ((c = getopt(argc, argv,":hapxt:f")) != EOF) {
    switch (c) {
    case 'x':
      opts.auto2 = TRUE;
      break;
    case 'p':
      opts.parenthesize_output = TRUE;
      break;
    case 't':
      opts.max_seconds = atoi(optarg);
      break;
    case 'f':  /* input files */
      opts.files = TRUE;
      break;
    case 'h':
    default:
      printf("%s", Help_string);
      exit(1);
      break;
    }
  }
  if (FALSE) {
    int i;
    for (i = 0; i < argc; i++)
      printf("arg: %s\n", argv[i]);
  }
    
  return opts;
}  /* get_command_line_args */

/*************
 *
 *   max_megs_exit()
 *
 *   This is intended to be called from the memory package.
 *
 *************/

static
void max_megs_exit(void)
{
  fprint_all_stats(stdout, "all");
  exit_with_message(stdout, MAX_MEGS_EXIT);
}  /* max_megs_exit */

/*************
 *
 *   process_command_line_args_1()
 *
 *   This is called BEFORE the input (clauses, etc) are read.  This is
 *   intended for high-level options that affect how input is read.
 *
 *************/

static
void process_command_line_args_1(struct arg_options command_opt,
				 Prover_options prover_opt)
{
  if (command_opt.auto2) {
    printf("\n%% From the command line: set(auto2).\n");
    set_flag(prover_opt->auto2, TRUE);
  }

  if (command_opt.parenthesize_output) {
    printf("\n%% From the command line: parenthesize output.\n");
    parenthesize(TRUE);  /* tell the parsing/printing package */
  }

}  /* process_command_line_args_1 */

/*************
 *
 *   process_command_line_args_2()
 *
 * This is called AFTER the input (clauses, etc) are read.  This allows
 * command-line args to override settings in the ordinary input.
 *
 *************/

static
void process_command_line_args_2(struct arg_options command_opt,
				 Prover_options prover_opt)
{
  if (command_opt.max_seconds != INT_MAX) {
    int n = command_opt.max_seconds;
    int id = prover_opt->max_seconds;
    printf("\n%% From the command line: assign(%s, %d).\n",
	   parm_id_to_str(id), n);
    assign_parm(id, n, TRUE);
  }

  set_max_megs(parm(prover_opt->max_megs));
  set_max_megs_proc(max_megs_exit);

}  /* process_command_line_args_2 */

/*************
 *
 *   prover_sig_handler()
 *
 *************/

static
void prover_sig_handler(int condition)
{
  printf("\nProver catching signal %d.\n", condition);
  switch (condition) {
  case SIGSEGV:
    fprint_all_stats(stdout, "all");
    exit_with_message(stdout, SIGSEGV_EXIT);
    break;
  case SIGINT:
    fprint_all_stats(stdout, "all");
    exit_with_message(stdout, SIGINT_EXIT);
    break;
  case SIGUSR1:
    report(stdout, "");
    report(stderr, "");
    break;
  default: fatal_error("prover_sig_handler, unknown signal");
  }
}  /* prover_sig_handler */

/*************
 *
 *   std_prover_init_and_input()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Prover_input std_prover_init_and_input(int argc, char **argv,
				       BOOL clausify,
				       BOOL echo,
				       int unknown_action)
{
  Prover_input pi = calloc(1, sizeof(struct prover_input));

  struct arg_options opts = get_command_line_args(argc, argv);

  init_standard_ladr();

  pi->options = init_prover_options();

  init_prover_attributes();

  signal(SIGINT,  prover_sig_handler);
  signal(SIGUSR1, prover_sig_handler);
  signal(SIGSEGV, prover_sig_handler);

  // Tell the top_input package what lists to accept and where to put them.

  accept_list("sos",          FORMULAS, FALSE, &(pi->sos));
  accept_list("assumptions",  FORMULAS, FALSE, &(pi->sos));  // Synonym for sos
  accept_list("goals",        FORMULAS, FALSE, &(pi->goals));
  accept_list("usable",       FORMULAS, FALSE, &(pi->usable));
  accept_list("demodulators", FORMULAS, FALSE, &(pi->demods));
  accept_list("hints",        FORMULAS, TRUE,  &(pi->hints));

  accept_list("actions",         TERMS, FALSE, &(pi->actions));
  accept_list("weights",         TERMS, FALSE, &(pi->weights));
  accept_list("kbo_weights",     TERMS, FALSE, &(pi->kbo_weights));
  accept_list("interpretations", TERMS, FALSE, &(pi->interps));
  accept_list("given_selection", TERMS, FALSE, &(pi->given_selection));
  accept_list("keep",            TERMS, FALSE, &(pi->keep_rules));
  accept_list("delete",          TERMS, FALSE, &(pi->delete_rules));

  if (echo)
    print_separator(stdout, "INPUT", TRUE);

  process_command_line_args_1(opts, pi->options);  // high-level, e.g., auto2

  // Read commands such as set, clear, op, lex.
  // Read lists, filling in variables given to the accept_list calls.

  read_all_input(argc, argv, stdout, echo, unknown_action);

  if (echo)
    print_separator(stdout, "end of input", TRUE);
  process_command_line_args_2(opts, pi->options);  // others, which override

  if (!option_dependencies_state()) {
    printf("\n%% Enabling option dependencies (ignore applies only on input).\n");
    enable_option_dependencies();
  }

  if (clausify) {
    Plist denials;

    pi->sos    = embed_formulas_in_topforms(pi->sos, TRUE);
    pi->usable = embed_formulas_in_topforms(pi->usable, TRUE);
    pi->demods = embed_formulas_in_topforms(pi->demods, TRUE);
    pi->hints  = embed_formulas_in_topforms(pi->hints, TRUE);
    pi->goals  = embed_formulas_in_topforms(pi->goals, FALSE);

    if (flag(pi->options->expand_relational_defs)) {
      Plist defs, nondefs, p;
      separate_definitions(pi->sos, &defs, &nondefs);
      pi->sos = NULL;
      print_separator(stdout, "EXPAND RELATIONAL DEFINITIONS", TRUE);
      if (defs) {
	Plist results, rewritten, defs2;

	printf("\n%% Relational Definitions:\n");
	for (p = defs; p; p = p->next) {
	  Topform tf = p->v;
	  assign_clause_id(tf);
	  fwrite_clause(stdout, tf, CL_FORM_STD);
	}

	/* Expand definitions w.r.t. themselves. */
	process_definitions(defs, &results, &defs2, &rewritten);
	if (results != NULL)
	  fatal_error("Circular relational definitions");

	defs = defs2;

	if (rewritten) {
	  printf("\n%% Relational Definitions, Expanded:\n");
	  for (p = defs; p; p = p->next)
	    if (!has_input_just(p->v))
	      fwrite_clause(stdout, p->v, CL_FORM_STD);
	}

	results = NULL;
	expand_with_definitions(nondefs, defs, &results, &rewritten);
	pi->sos = reverse_plist(results);

	results = NULL;
	expand_with_definitions(pi->usable, defs, &results, &rewritten);
	pi->usable = reverse_plist(results);

	results = NULL;
	expand_with_definitions(pi->hints, defs, &results, &rewritten);
	pi->hints = reverse_plist(results);

	results = NULL;
	expand_with_definitions(pi->goals, defs, &results, &rewritten);
	pi->goals = reverse_plist(results);

	for (p = defs; p; p = p->next)
	  append_label_attribute(p->v, "non_clause");

	if (rewritten) {
	  printf("\n%% Formulas Being Expanded:\n");
	  rewritten = reverse_plist(rewritten);
	  for (p = rewritten; p; p = p->next) {
	    Topform tf = p->v;
	    fwrite_clause(stdout, tf, CL_FORM_STD);
	    append_label_attribute(tf, "non_clause");
	  }
	}
	/* After this point, the "defs" and "rewritten" formulas
	   will be accessible only from the Clause ID table, e.g.,
	   for inclusion in proofs.
	 */
      }
      else {
	printf("\n%% No relational definitions were found.\n");
	pi->sos = nondefs;
      }
      print_separator(stdout, "end of expand relational definitions", TRUE);
    }

    print_separator(stdout, "PROCESS NON-CLAUSAL FORMULAS", TRUE);
    if (echo)
      printf("\n%% Formulas that are not ordinary clauses:\n");

    pi->sos    = process_input_formulas(pi->sos, echo);
    pi->usable = process_input_formulas(pi->usable, echo);
    pi->demods = process_demod_formulas(pi->demods, echo);
    pi->hints  = process_input_formulas(pi->hints, echo);
    denials    = process_goal_formulas(pi->goals, echo);

    pi->goals = NULL;

    /* move to denials (negated goals) to the end of sos */

    pi->sos = plist_cat(pi->sos, denials);

    print_separator(stdout, "end of process non-clausal formulas", TRUE);
  }

  return pi;
}  /* std_prover_init_and_input */


