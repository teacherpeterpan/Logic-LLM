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

#define PROVER_NAME     "FOF-Prover9"
#include "../VERSION_DATE.h"

#include "provers.h"
#include "foffer.h"

/*************
 *
 *    main -- basic prover + FOF problem reduction
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  int return_code;

  print_banner(argc, argv, PROVER_NAME, PROGRAM_VERSION, PROGRAM_DATE, FALSE);

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    FALSE,          // don't clausify
			    TRUE,           // echo input to stdout
			    KILL_UNKNOWN);  // unknown flags/parms are fatal

  if (input->usable || input->demods)
    fatal_error(PROVER_NAME ": all input clauses must be in sos, assumptions, "
		"or goals list");
    
  /***************** Search for a proof **************************************/

  if (flag(input->options->auto_denials)) {
    clear_flag(input->options->auto_denials, TRUE);
    printf("\n%% clear(auto_denials), because it is incompatiable with "
	   "FOF reduction.\n");
  }

  /***************** Search for a proof **************************************/

  return_code = foffer(input);

  /***************** Print result message and exit ***************************/

  if (return_code == MAX_PROOFS_EXIT) {
    printf("\nTHEOREM PROVED\n");
    if (!flag(input->options->quiet))
      fprintf(stderr, "\nTHEOREM PROVED\n");
  }
  else {
    printf("\nSEARCH FAILED\n");
    if (!flag(input->options->quiet))
      fprintf(stderr, "\nSEARCH FAILED\n");
  }

  exit_with_message(stdout, return_code);
  exit(1);  // to satisfy the compiler (won't be called)
}  // main
