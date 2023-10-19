#define PROVER_NAME     "Ploop"
#define PROVER_VERSION  "4"
#define VERSION_DATE    "04 Feb 2005"

#include "prover9.h"
#include "poptions.h"
#include "attributes.h"
#include "auto.h"
#include "search.h"

/*************
 *
 *    void print_banner(argc, argv)
 *
 *************/

static
void print_banner(int argc, char **argv)
{
  int i;
  printf("----- %s %s, %s -----\n", PROVER_NAME, PROVER_VERSION, VERSION_DATE);
  printf("Process %d was started by %s on %s,\n%s",
	 my_process_id(), username(), hostname(), get_date());
	 
  printf("The command was \"");
  for(i = 0; i < argc; i++)
    printf("%s%s", argv[i], (i < argc-1 ? " " : ""));
  printf("\".\n\n");
}  // print_banner

/*************
 *
 *   process_basic_options()
 *
 *************/

static
void process_basic_options(void)
{
  // Tell the clock package if the user doesn't want timing.
  if (!flag(CLOCKS))
    disable_clocks();

  // Tell the memory package when and how to quit.
  set_max_megs(parm(MAX_MEGS));
  set_max_megs_proc(max_megs_exit);

}  // process_basic_options

/*************
 *
 *    main -- autosketches!
 *
 *************/

int main(int argc, char **argv)
{
  int use_hints_flag;               // local option
  int use_expanded_proofs_flag;     // local option

  /************************ General initialization ***************************/

  print_banner(argc, argv);         // local routine

  init_standard_ladr();             // LADR routine

  init_prover_options();            // local routine
  init_prover_attributes();         // local routine

  signal(SIGINT,  sig_handler);     // system routine
  signal(SIGUSR1, sig_handler);     // system routine
  signal(SIGSEGV, sig_handler);     // system routine

  /************************ Initialize our own options ***********************/

  // Local options must be set up AFTER calling init_prover_options().

  use_hints_flag = next_available_flag_id();
  init_flag(use_hints_flag, "use_hints", FALSE);

  use_expanded_proofs_flag = next_available_flag_id();
  init_flag(use_expanded_proofs_flag, "use_expanded_proofs", FALSE);

  /************************ Read the input ***********************************/

  {
    Plist usable, sos, demodulators, goals, hints, defns;
    Plist usable_formulas, sos_formulas, defns_formulas, goals_formulas;
    Plist actions, weights;

    Plist candidates;

    // Tell the top_input package what lists to accept and where to put them.

    accept_list("usable",       CLAUSES,  &usable);
    accept_list("sos",          CLAUSES,  &sos);
    accept_list("demodulators", CLAUSES,  &demodulators);
    accept_list("goals",        CLAUSES,  &goals);
    accept_list("hints",        CLAUSES,  &hints);
    accept_list("candidates",   CLAUSES,  &candidates);
    defns = NULL;

    accept_list("usable",       FORMULAS, &usable_formulas);
    accept_list("sos",          FORMULAS, &sos_formulas);
    accept_list("goals",        FORMULAS, &goals_formulas);
    accept_list("definitions",  FORMULAS, &defns_formulas);

    accept_list("actions",      TERMS,    &actions);
    accept_list("weights",      TERMS,    &weights);

    // Read commands such as set, clear, op, lex.
    // Read lists, filling in variables given to the accept_list calls.

    read_all_input(argc, argv, stdout, TRUE, WARN_UNKNOWN);

    printf("\n%% Finished reading the input.\n");

    process_basic_options();

    // Clausify any formulas and append to the corresponding clause lists.

    usable = plist_cat(usable, clausify_formulas(usable_formulas));
    sos    = plist_cat(sos,    clausify_formulas(sos_formulas));
    goals  = plist_cat(goals,  clausify_formulas(goals_formulas));
    defns  = plist_cat(defns,  clausify_formulas(defns_formulas));

    // Order the symbols.

    symbol_order(usable, sos, demodulators, goals, defns);

    // Maybe set options based on the structure of the clauses.

    process_auto_options(sos, usable, goals, defns);

    /******************** Search for a proof *******************************/

    {
      double total_user_seconds = 0.0;    // running total for all searches
      double total_system_seconds = 0.0;  // running total for all searches

      Plist failures = NULL;
      Plist successes = NULL;

      Plist p;

      for (p = candidates; p != NULL; p = p->next) {

	Clause candidate = p->v;
	Search_results results;

	sos = plist_append(sos, candidate);

	fprintf(stdout,"\nStarting a search with candidate:\n\n");
	fwrite_clause(stdout, candidate, TRUE, TRUE);

	// note that sos and hints change as we iterate

	results = forking_search(usable, sos, demodulators,
				 goals, defns, hints,
				 weights, actions);

	total_user_seconds   += results->user_seconds;
	total_system_seconds += results->system_seconds;

	sos = plist_remove_last(sos);  // remove candidate

	if (results->proofs == NULL) {
	  failures = plist_append(failures, candidate);
	  fprintf(stdout,"\nFailure: ");
	  fwrite_clause(stdout, candidate, TRUE, TRUE);
	}
	else {
	  successes = plist_append(successes, candidate);
	  fprintf(stdout,"\nSuccess: ");
	  fwrite_clause(stdout, candidate, TRUE, TRUE);

	  if (flag(use_hints_flag)) {
	    // append proof to hints
	    Plist proof, new_hints;
	    proof = results->proofs->v;  // use first proof only

	    if (flag(use_expanded_proofs_flag))
	      new_hints = proof_to_xproof(proof);   // all new clauses (deep)
	    else
	      new_hints = copy_clauses_ija(proof);  // deep copy

	    hints = plist_cat(hints, new_hints); // uses up args
	  }
	}
	zap_search_results(results);

      }  // end of for loop

      fwrite_clause_list(stdout, successes, "successes");
      fwrite_clause_list(stdout, failures, "failures");

      printf("\nTotal user_CPU=%.2f, system_CPU=%.2f.\n",
	     total_user_seconds, total_system_seconds);

    }  // end of search block
  }  // end of read/search block
  
  exit(0);

}  // main
