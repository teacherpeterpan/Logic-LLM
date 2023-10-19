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

#define PROVER_NAME     "Prover9"
#include "../VERSION_DATE.h"

#include "provers.h"

/*************
 *
 *    main -- basic prover
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  Prover_results results;

  print_banner(argc, argv, PROVER_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);
  set_program_name(PROVER_NAME);   /* for conditional input */

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    TRUE,           // clausify formulas
			    TRUE,           // echo input to stdout
			    KILL_UNKNOWN);  // unknown flags/parms are fatal

  /***************** Search for a proof **************************************/

  results = search(input);

  /***************** Print result message and exit ***************************/

  if (results->return_code == MAX_PROOFS_EXIT) {
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

  exit_with_message(stdout, results->return_code);
  exit(1);  // to satisfy the compiler (won't be called)
}  // main
