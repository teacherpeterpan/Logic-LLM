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

#define PROVER_NAME     "MProver"
#include "../VERSION_DATE.h"

#include "../ladr/banner.h"
#include "../ladr/clausify.h"
#include "search.h"
#include "utilities.h"
#include "../mace4.src/msearch.h"

/*************
 *
 *   disprover() -- look for a model of the clauses
 *
 *   Return TRUE if a model is found.  The lists of clauses
 *   are not changed.
 *
 *************/

static
BOOL disprover(Plist usable, Plist sos, Plist demodulators,
	       int max_sec)
{
  struct mace_options opt;
  Plist clauses = NULL;
  Mace_results results;

  // collect clauses (shallow, leaving lists unchanged by using plist_cat2)

  clauses = plist_cat2(clauses, usable);
  clauses = plist_cat2(clauses, sos);
  clauses = plist_cat2(clauses, demodulators);

  // set up the standard mace4 options

  init_mace_options(&opt);

  // modify the mace4 options

  assign_parm(opt.max_seconds, parm(max_sec), TRUE);
  assign_parm(opt.iterate_up_to, 100, TRUE);
  set_flag(opt.print_models_portable, TRUE);
  set_flag(opt.return_models, TRUE);

  // call mace4

  results = mace4(clauses, &opt);

  if (results->success) {
    fprint_interp_2(stdout, results->models->v);
  }

  zap_plist(clauses);  // shallow zap

  return (results->success);

}  // disprover

/*************
 *
 *    main -- basic prover
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  Prover_results results;  

  int disprover_flag, disprover_max_sec_flag;  // local options

  print_banner(argc, argv, PROVER_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);

  /************************ Initialize our own options ***********************/

  disprover_flag = init_flag("disprover",TRUE);
  disprover_max_sec_flag = init_parm("disprover_max_seconds",5,0,INT_MAX);

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    TRUE,           // echo input to stdout
			    KILL_UNKNOWN);  // unknown flags/parms are fatal

  /******************** Search for a counterexample **********************/

  if (flag(disprover_flag)) {
    printf("\n%% Searching for counterexample.");
    if (disprover(input->usable, input->sos, input->demods,
		  disprover_max_sec_flag))
      mace4_exit(MAX_MODELS_EXIT);  // call mace4_exit() with mace4 codes!
    else
      printf("\n%% Failed to find counterexample.");
  }
    
  /******************** Search for a proof *******************************/

  fprintf(stderr, "\nStarting proof search.\n");

  results = search(input);
      
  exit_with_message(stdout, results->return_code);

  exit(1);  // to satisfy the compiler (won't be called)

}  // main
