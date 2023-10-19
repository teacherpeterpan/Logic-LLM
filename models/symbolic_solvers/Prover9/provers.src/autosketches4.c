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

#define PROVER_NAME     "Autosketches"
#include "../VERSION_DATE.h"

#include "provers.h"

/*************
 *
 *   attach_sketch_hint_labels()
 *
 *   For each clause in a Plist, if it has no labels, append one,
 *   e.g., label(S4_H5), meaning sketch 4 hint 5.
 *
 *************/

static
void attach_sketch_hint_labels(int sketch_count, Plist hints)
{
  Plist p;
  int hint_count;
  for (p = hints, hint_count = 1; p; p = p->next, hint_count++) {
    Topform c = p->v;
    if (!exists_attribute(c->attributes, label_att())) {
      char s[100];
      sprintf(s, "S%d_H%d", sketch_count, hint_count);
      c->attributes = set_string_attribute(c->attributes, label_att(), s);
    }
  }
}  /* attach_sketch_hint_labels */

/*************
 *
 *    main -- autosketches4
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;

  int use_expanded_proofs_flag;     // special-purpose option
  Plist assumptions;                // special-purpose list

  /************************ General initialization ***************************/

  print_banner(argc, argv, PROVER_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);

  /************************ Initialize our own options and lists *************/

  use_expanded_proofs_flag = init_flag("use_expanded_proofs", TRUE);
  accept_list("extra_assumptions",  FORMULAS,  FALSE, &assumptions);

  /************************ Read the input ***********************************/

  input = std_prover_init_and_input(argc, argv,
			   TRUE,           // clausify
			   TRUE,           // echo input to stdout
			   KILL_UNKNOWN);  // unknown flags/parms are fatal

  // Clausify the assumptions (always needed).

  assumptions = embed_formulas_in_topforms(assumptions, TRUE);
  assumptions = process_input_formulas(assumptions, TRUE);

  // tell search() to return expanded proofs as well as ordinary proofs
  input->xproofs = flag(use_expanded_proofs_flag);
  
  /******************** Search for a proof *******************************/

  {
    int sketch_count = 0;
    double total_user_seconds = 0.0;    // running total for all searches
    double total_system_seconds = 0.0;  // running total for all searches

    Plist original_sos   = input->sos;
    Plist original_hints = input->hints;
    
    Plist assumptions_work = copy_plist(assumptions);       // shallow
    Plist hints_work       = copy_plist(original_hints);    // shallow

    while (TRUE) {  // exit from within loop

      Prover_results results;

      Plist sos_work = plist_cat(copy_plist(original_sos),
				 copy_plist(assumptions_work));  // shallow

      fprintf(stdout,"\nStarting a search with assumptions:\n\n");
      fwrite_clause_list(stdout, assumptions_work, "extra_assumptions", CL_FORM_STD);

      // note that sos_work and hints_work change as we iterate

      input->sos   = sos_work;
      input->hints = hints_work;

      results = forking_search(input);   // SEARCH

      total_user_seconds   += results->user_seconds;
      total_system_seconds += results->system_seconds;

      if (results->proofs == NULL) {
	printf("\nTotal user_CPU=%.2f, system_CPU=%.2f.\n",
	       total_user_seconds, total_system_seconds);
	exit_with_message(stdout, results->return_code);
      }

      else if (assumptions_work == NULL) {
	printf("The preceding proof was found with no extra assumptions, "
	       "so we are done.\n");
	printf("\nTotal user_CPU=%.2f, system_CPU=%.2f.\n",
	       total_user_seconds, total_system_seconds);
	exit_with_message(stdout, results->return_code);
      }

      zap_plist(sos_work);      // shallow zap
	
      // append proof to hints

      {
	Plist assumps_in_proof, inputs_in_proof, new_hints;

	sketch_count++;

	if (flag(use_expanded_proofs_flag))
	  new_hints = copy_clauses_ija(results->xproofs->v); // deep copy
	else
	  new_hints = copy_clauses_ija(results->proofs->v);  // deep copy

	attach_sketch_hint_labels(sketch_count, new_hints);  // label(S3_H45)

	hints_work = plist_cat(hints_work, new_hints); // shallow, uses up args

	// get assumptions that occur as input clauses in the proof

	inputs_in_proof = input_clauses(results->proofs->v);       // shallow
	assumps_in_proof = intersect_clauses(assumptions_work,
					     inputs_in_proof);  // shallow

	if (assumps_in_proof == NULL) {
	  printf("\nThe following extra assumptions were available for\n"
		 "the preceding proof, but they were not used.\n"
		 "Therefore we are done.\n\n");
	  fwrite_clause_list(stdout, assumptions_work,
			     "extra_assumptions_not_used", CL_FORM_BARE);
			     
	  printf("\nTotal user_CPU=%.2f, system_CPU=%.2f.\n",
		 total_user_seconds, total_system_seconds);
	  exit_with_message(stdout, MAX_PROOFS_EXIT);
	}
	else {
	  Topform assump = assumps_in_proof->v;  // remove this one
	  printf("\nThe preceding proof uses the following extra_assmumptions.\n"
		 "The first of these will be removed.\n\n");
	  fwrite_clause_list(stdout, assumps_in_proof,
			     "extra_assumptions_in_proof", CL_FORM_BARE);
	  assumptions_work = plist_remove(assumptions_work, assump);
	}
	zap_plist(inputs_in_proof);   // shallow zap
	zap_plist(assumps_in_proof);  // shallow zap
      }
      zap_prover_results(results);
    }  // end of while loop
  }

  exit(1);  // to satisfy the compiler (won't be called)

}  // main
