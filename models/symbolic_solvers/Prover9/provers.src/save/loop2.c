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
 *   deny_unit()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clause deny_unit(Clause c)
{
  Plist p;
  Clause d;
  Formula f;

  if (!unit_clause(c))
    fatal_error("deny_unit: clause not unit");

  f = clause_to_formula(c);  /* deep */
  f = universal_closure(f);
  f = negate(f);
  p = clausify_formula(f);   /* deep */
  d = p->v;
  zap_plist(p);
  zap_formula(f);
  return d;
}  /* deny_unit */

/*************
 *
 *    main -- loop2
 *
 *************/

int main(int argc, char **argv)
{
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

      Plist a, b;

      skolem_check(FALSE);  /* don't check if Skolem symbols are already in use. */

      for (a = candidates; a != NULL; a = a->next) {
	for (b = candidates; b != NULL; b = b->next) {
	  if (a != b) {

	    Search_results results;

	    Clause hypothesis = a->v;
	    Clause conclusion = b->v;

	    Clause denial = deny_unit(conclusion);
	    reset_skolem_symbols(1, 1);
	    denial->justification = input_just();
	    denial->attributes = copy_attributes(conclusion->attributes);

	    sos = plist_append(sos, hypothesis);
	    goals = plist_append(goals, denial);

	    fprintf(stdout,"\nStarting a search with hypothesis: ");
	    fwrite_clause(stdout, hypothesis, TRUE, TRUE);

	    fprintf(stdout,"                       denial: ");
	    fwrite_clause(stdout, denial, TRUE, TRUE);

	    // note that sos and goals change as we iterate

	    results = search(usable, sos, demodulators,
				     goals, defns, hints,
				     weights, actions);
	    exit(1);
	    
	    total_user_seconds   += results->user_seconds;
	    total_system_seconds += results->system_seconds;

	    sos   = plist_remove_last(sos);    // remove hypothesis
	    goals = plist_remove_last(goals);  // remove denial

	    printf("\n%s -> %s: %s\n",
		   get_string_attribute(hypothesis->attributes, LABEL_ATT, 1),
		   get_string_attribute(conclusion->attributes, LABEL_ATT, 1),
		   results->proofs ? "SUCCESS" : "FAILURE");
	    
	    zap_search_results(results);
	    
	  }
	} // inner loop
      } // outer loop
      
      printf("\nTotal user_CPU=%.2f, system_CPU=%.2f.\n",
	     total_user_seconds, total_system_seconds);

    }  // end of search block
  }  // end of read/search block
  
  exit(0);

}  // main
