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

// ***********************************************************************
// NewAuto (new autosketches implementation, work in progress)
//
// Last updated:  2006-may-24
// ***********************************************************************

#define PROVER_NAME     "NewAuto"
#include "../VERSION_DATE.h"

#include "provers.h"  // includes LADR and search definitions

// autosketches
int NewAuto(Prover_input input, Plist assumptions);

/*************
 *
 *    main -- NewAuto
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  int return_code;

  // new input list
  Plist assumptions;
  accept_list("extra_assumptions",  FORMULAS,  FALSE, &assumptions);

  print_banner(argc, argv, PROVER_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    TRUE,           //clausify
			    TRUE,           // echo input to stdout
			    KILL_UNKNOWN);  // unknown flags/parms are fatal

  // Clausify the assumptions (always needed).

  assumptions = embed_formulas_in_topforms(assumptions, TRUE);
  assumptions = process_input_formulas(assumptions, TRUE);

  /***************** Search for a proof **************************************/

  return_code = NewAuto(input, assumptions);

  /***************** Print result message and exit ***************************/

  // The return code is from the most recent execution of the prover.

  if (return_code == MAX_PROOFS_EXIT) {
    printf("\nTHEOREM PROVED\n");
    if (!flag(input->options->quiet))
      fprintf(stderr, "\nTHEOREM PROVED\n");
  }
  else {
    // Note that we fail if we found less than max_proofs proofs.
    printf("\nSEARCH FAILED\n");
    if (!flag(input->options->quiet))
      fprintf(stderr, "\nSEARCH FAILED\n");
  }

  exit_with_message(stdout, return_code);
  exit(1);  // to satisfy the compiler (won't be called)
}  // main


int NewAuto(Prover_input input, Plist assumptions)
{
   // save copy of input sos, since the input structure will be
   // used for temporary working lists that include assumptions
   Plist original_sos = copy_plist(input->sos);
   
   // assumptions are in one of three states:
   //
   //    active    -- active assumptions, available to the prover
   //
   //    current   -- current candidate for elimination, not available
   //                 to the prover
   //
   //    deleted   -- the current candidate gets deleted when proof is 
   //                 found (i.e., not using it);  we could keep a list
   //                 list of these for more extensive backtracking, but
   //                 for now I am literally deleting these
   
   
   // the current candidate, NULL for the first proof iteration only
   Topform assumption_current = NULL;

   // the active assumptions, initially the full input set
   Plist assumptions_active = copy_plist(assumptions);
   
   // bookeeping to determine which assumptions participated in a proof
   Plist proof;
   Plist expanded_proof;
   Plist inputs_in_proof;
   Plist assumptions_in_proof = NULL;  // recheck need for this initialization
   
   // check memory management for successive calls
   Prover_results results;

   int return_code;
   int sketches_count = 0;
   int list_size;
   int done = FALSE;

   // used to help construct expanded proofs for new hints
   I3list jmap = NULL;

   while (!done)
   {
      sketches_count++ ;

      fprintf(stdout,"\n***********************************************\n");
      fprintf(stdout,"AUTO SKETCHES ITERATION %d\n", sketches_count);
      fprintf(stdout,"***********************************************\n");

      list_size = plist_count(assumptions_active);
      fprintf(stdout,"\nStarting a search with %d assumptions:\n", list_size);
      fwrite_clause_list(stdout, assumptions_active, "assumptions_active", CL_FORM_STD);

      if (list_size == 0)
         fprintf(stdout,"\nNote: Attempt with no assumptions!\n\n");

      // temporary working sos list (probably should zap list first)
      input->sos = copy_plist(original_sos);
      input->sos = plist_cat(input->sos, copy_plist(assumptions_active));
   
      // execute prover
      results = forking_search(input);
      return_code = results->return_code;

      // proof found
      if (results->proofs != NULL)
      {
         // find list of assumptions actually used
   
         proof = results->proofs->v;  // use first proof only

         inputs_in_proof = input_clauses(proof);
         assumptions_in_proof = intersect_clauses(assumptions_active,
                                                  inputs_in_proof); 
                                           
         zap_plist(inputs_in_proof);

         if (assumptions_in_proof == NULL)
         {
            printf("\nProof is complete (no extra assumptions used).\n");
            done = TRUE;
         }
         else // proof with assumptions
         {
            list_size = plist_count(assumptions_in_proof);
            printf("\nSuccessful proof using the following %d assumptions:\n",
               list_size);

            fwrite_clause_list(stdout, assumptions_in_proof,
               "assumptions_in_proof", CL_FORM_STD);

            // add expanded proof as new hints
            expanded_proof = expand_proof(proof, &jmap);

            // I think this is safe, since expand is deep copy.
            // delete_clauses(proof);
            zap_prover_results(results);
            results = NULL;

            list_size = plist_count(expanded_proof);
            printf("\nIncluding %d new hint clauses:\n", list_size);
            fwrite_clause_list(stdout, expanded_proof,
               "new_hints", CL_FORM_BARE);

            input->hints = plist_cat(input->hints,expanded_proof);
            expanded_proof = NULL;
         
            // An assumption has been successfully eliminated.  We delete
            // this assumption permanently for now.  In the future, we
            // might want to maintain a "deleted_assumptions" list to
            // be able to backtrack in case of failure.
            //
            // The first proof, with all assumptions, is a special case.

            if (assumption_current != NULL)
               delete_clause(assumption_current);

            // move one from assumptions_in_proof to current 
            assumption_current = assumptions_in_proof->v;
            assumptions_in_proof = plist_pop(assumptions_in_proof);
            fprintf(stdout,"\nAttempt to eliminate one assumption:\n\n   ");
            fwrite_clause(stdout,assumption_current,CL_FORM_BARE);
   
            // eliminate current from active (a different data structure
            // may allow us to avoid this search, but it should be fine)
            assumptions_active
               = plist_remove(assumptions_active, assumption_current);
         }
      }
      else // no proof
      {
         // no proof with full input list of assumptions
         if (assumption_current == NULL)
         {
            printf("No proof with full list of assumptions, so fail.\n");
            done = TRUE;
         }
   
         // no other candidates to eliminate
         else if (assumptions_in_proof == NULL)
         {
            printf("Unable to eliminate an assumption, so fail.\n");
            done = TRUE;
         }
   
         // try to eliminate different assumption from most recent proof
         else
         {
            printf("\nTry to eliminate a different assumption.\n");

            fprintf(stdout,"\nReturn eliminated assumption to active pool:\n\n   ");
            fwrite_clause(stdout,assumption_current,CL_FORM_BARE);

            // return current to active pool
            assumptions_active
               = plist_append(assumptions_active, assumption_current);

            list_size = plist_count(assumptions_in_proof);
            printf("\n%d candidate assumptions to eliminate:\n", list_size);
            fwrite_clause_list(stdout, assumptions_in_proof,
               "candidates", CL_FORM_BARE);

            // move one from assumptions_in_proof to current 
            assumption_current = assumptions_in_proof->v;
            assumptions_in_proof = plist_pop(assumptions_in_proof);
            fprintf(stdout,"\nAttempt to eliminate next assumption:\n\n   ");
            fwrite_clause(stdout,assumption_current,CL_FORM_BARE);

            // eliminate current from active
            assumptions_active = plist_remove(assumptions_active,
               assumption_current);
         }
      }
   }

   printf("\nAutosketches completes after %d iterations.\n", sketches_count);

   // return code from most recent execution of prover
   return return_code;

} // NewAuto
