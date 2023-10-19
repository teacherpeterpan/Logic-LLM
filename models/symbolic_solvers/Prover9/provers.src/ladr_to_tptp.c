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

#define PROVER_NAME     "LADR_to_TPTP"
#include "../VERSION_DATE.h"

/*************
 *
 *    main
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  Plist sos, usable, demods, goals, all, p;
  Ilist fp_syms = NULL;
  I2list map;

  BOOL quote_bad_tptp_syms = string_member("-q", argv, argc);

  parenthesize(string_member("-p", argv, argc));

  /* This is similar to the way Prover9 initializes and reads its input. */

  input = std_prover_init_and_input(0, NULL,
			    FALSE,            // clausify formulas
			    FALSE,            // echo input to stdout
			    IGNORE_UNKNOWN);  // for unknown flags/parms

  /* Transform the LADR formulas to TPTP terms. */

  sos    = ladr_list_to_tptp_list(input->sos,    "sos", "axiom");
  usable = ladr_list_to_tptp_list(input->usable, "usable", "axiom");
  demods = ladr_list_to_tptp_list(input->demods, "demods", "axiom");
  goals  = ladr_list_to_tptp_list(input->goals,  "goals", "conjecture");

  /* Collect all of the TPTP terms into one list. */

  all = NULL;
  all = plist_cat2(all, sos);
  all = plist_cat2(all, usable);
  all = plist_cat2(all, demods);
  all = plist_cat2(all, goals);

  /* Collect all of the function and predicate symbols. */

  fp_syms = NULL;
  for (p = all; p; p = p->next) {
    Term t = p->v;
    Ilist a = syms_in_form(ARG(t,2), str_ident(sn_to_str(SYMNUM(t)), "cnf"));
    fp_syms = ilist_union(fp_syms, a);
  }

  /* Determine which symbols are bad for TPTP and create a map to new ones. */

  map = map_for_bad_tptp_syms(fp_syms, quote_bad_tptp_syms);

  /* Replace the bad symbols. */

  if (map != NULL) {
    I2list b;
    printf("\n%% The LADR formulas contain function or predicate symbols\n"
	   "%% that are not legal TPTP symbols, and we have replaced those\n"
	   "%% symbols with new symbols.  Here is the list of the unaccepted\n"
	   "%% symbols and the corresponding replacements.\n%%\n");
    for (b = map; b; b = b->next)
      printf("%%   (arity %d) %8s    %s\n",
	     sn_to_arity(b->j),
	     sn_to_str(b->i),
	     sn_to_str(b->j));
    printf("\n");
    for (p = all; p; p = p->next) {
      Term t = p->v;
      ARG(t,2) = replace_bad_tptp_syms_form(ARG(t,2), 
				  str_ident(sn_to_str(SYMNUM(t)),"cnf"),
				  map);
						      
    }
  }

  /* Change settings for TPTP output. */

  set_variable_style(PROLOG_STYLE);
  clear_parse_type_for_all_symbols();
  declare_tptp_output_types();

  /* Output the TPTP terms. */

  for (p = all; p; p = p->next)
    fwrite_term_nl(stdout, p->v);

  exit(1);  //
}  // main
