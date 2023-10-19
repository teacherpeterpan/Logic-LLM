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
#include <unistd.h>

/*************
 *
 *   member_args()
 *
 *************/

BOOL member_args(int argc, char **argv, char *str)
{
  int i;
  for (i = 1; i < argc; i++)
    if (str_ident(argv[i], str))
      return TRUE;
  return FALSE;
}  /* member_args */

/*************
 *
 *   command_line_flag()
 *
 *************/

static
void command_line_flag(int id, char *optarg)
{
  if (!optarg || str_ident(optarg, "1")) {
    set_flag(id, TRUE);
    printf("\n%% From the command line: set(%s).\n", flag_id_to_str(id));
  }
  else if (str_ident(optarg, "0")) {
    clear_flag(id, TRUE);
    printf("\n%% From the command line: clear(%s).\n", flag_id_to_str(id));
  }
  else
    fatal_error("Command-line flag must have 0 or 1 as the value");
}  /* command_line_flag */

/*************
 *
 *   command_line_parm()
 *
 *************/

static
void command_line_parm(int id, char *optarg)
{
  int n = atoi(optarg);
  assign_parm(id, n, TRUE);
  printf("\n%% From the command line: assign(%s, %d).\n", parm_id_to_str(id),n);
}  /* command_line_parm */

/*************
 *
 *   usage_message()
 *
 *************/

void usage_message(FILE *fp, Mace_options opt)
{
  fprintf(fp,

  "%s %s (%s) -- Search for finite models.\n"
  "\n"
  "Input commands, clauses, and formulas are taken from standard input.\n"
  "\n"
  "Command-line options override any settings in the input file.\n"
  "To set or clear a flag, you must give 1 or 0 as the value.\n"
  "\n"
  "Basic Options\n"
  "\n"
  "  -n n : (lower case) parm domain_size (%d).\n"
  "  -N n : (upper case) parm end_size (%d).\n"
  "  -i n : (lower case) parm increment (%d).\n"
  "  -P n : (upper case) flag print_models (%s).\n"
  "  -p n : (lower case) flag print_models_tabular (%s).\n"
  "  -m n : parm max_models (%d).\n"
  "  -t n : parm max_seconds (%d).\n"
  "  -s n : parm max_seconds_per (%d).\n"
  "  -b n : parm max_megs (%d).\n"
  "  -V n : (upper case) flag prolog_style_variables (%s).\n"
  "  -v n : (lower case) flag verbose (%s).\n"
  "\n"
  "Advanced Options\n"
  "\n"
  "  -L n : flag lnh (%s).\n"
  "  -O n : parm selection_order (%d).\n"
  "  -M n : parm Selection_measure (%d).\n"
  "  -G n : flag negprop (%s).\n"
  "  -H n : flag neg_assign (%s).\n"
  "  -I n : flag neg_assign_near (%s).\n"
  "  -J n : flag neg_elim (%s).\n"
  "  -K n : flag neg_elim_near (%s).\n"
  "  -T n : flag trace (%s).\n"
  "  -R n : flag integer_ring (%s).\n"
  "  -q n : flag iterate_primes (%s).\n"
  "  -S n : flag skolems_last (%s).\n"
  "\n"
  "Special Flags (not corresponding to set/clear/assign commands)\n"
  "\n"
  "  -c   : Ignore unrecognized set/clear/assign commands in the input\n"
  "         file.  This is useful for running MACE4 on an input file\n"
  "         designed for another program such as a theorem prover.\n"
  "\n"

	  , PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE,
	  parm(opt->domain_size),
	  parm(opt->iterate_up_to),
	  parm(opt->increment),
	  flag(opt->print_models) ? "set" : "clear",
	  flag(opt->print_models_tabular) ? "set" : "clear",
	  parm(opt->max_models),
	  parm(opt->max_seconds),
	  parm(opt->max_seconds_per),
	  parm(opt->max_megs),
	  flag(prolog_style_variables_id()) ? "set" : "clear",
	  flag(opt->verbose) ? "set" : "clear",
	  flag(opt->lnh) ? "set" : "clear",
	  parm(opt->selection_order),
	  parm(opt->selection_measure),
	  flag(opt->negprop) ? "set" : "clear",
	  flag(opt->neg_assign) ? "set" : "clear",
	  flag(opt->neg_assign_near) ? "set" : "clear",
	  flag(opt->neg_elim) ? "set" : "clear",
	  flag(opt->neg_elim_near) ? "set" : "clear",
	  flag(opt->trace) ? "set" : "clear",
	  flag(opt->integer_ring) ? "set" : "clear",
	  flag(opt->iterate_primes) ? "set" : "clear",
	  flag(opt->skolems_last) ? "set" : "clear"
	  );
}  /* usage_message */

/*************
 *
 *   process_command_line_args()
 *
 *************/

void process_command_line_args(int argc, char **argv,
			       Mace_options opt)
{
  extern char *optarg;
  int c;

  /* No colons:  no argument.
     One colon:  argument required.
     Two colons: argument optional.  (GNU extension!  Don't use it!)
  */
  
  while ((c = getopt(argc, argv,
	     "n:N:m:t:s:b:O:M:p:P:v:L:G:H:I:J:K:T:R:i:q:Q:S:cf:g")) != EOF) {
    switch (c) {
    case 'n':
      command_line_parm(opt->domain_size, optarg);
      break;
    case 'N':
      command_line_parm(opt->end_size, optarg);
      break;
    case 'm':
      command_line_parm(opt->max_models, optarg);
      break;
    case 't':
      command_line_parm(opt->max_seconds, optarg);
      break;
    case 's':
      command_line_parm(opt->max_seconds_per, optarg);
      break;
    case 'b':
      command_line_parm(opt->max_megs, optarg);
      break;
    case 'O':
      command_line_parm(opt->selection_order, optarg);
      break;
    case 'M':
      command_line_parm(opt->selection_measure, optarg);
      break;

    case 'P':
      command_line_flag(opt->print_models, optarg);
      break;
    case 'p':
      command_line_flag(opt->print_models_tabular, optarg);
      break;
#if 0 /* Prolog-style variables cannot be set from the command line. */
    case 'V':
      command_line_flag(prolog_style_variables_id(), optarg);
      break;
#endif
    case 'v':
      command_line_flag(opt->verbose, optarg);
      break;
    case 'L':
      command_line_flag(opt->lnh, optarg);
      break;
    case 'G':
      command_line_flag(opt->negprop, optarg);
      break;
    case 'H':
      command_line_flag(opt->neg_assign, optarg);
      break;
    case 'I':
      command_line_flag(opt->neg_assign_near, optarg);
      break;
    case 'J':
      command_line_flag(opt->neg_elim, optarg);
      break;
    case 'K':
      command_line_flag(opt->neg_elim_near, optarg);
      break;
    case 'T':
      command_line_flag(opt->trace, optarg);
      break;
    case 'R':
      command_line_flag(opt->integer_ring, optarg);
      break;
    case 'i':
      command_line_parm(opt->increment, optarg);
      break;
    case 'q':
      command_line_flag(opt->iterate_primes, optarg);
      break;
    case 'Q':
      command_line_flag(opt->iterate_nonprimes, optarg);
      break;
    case 'S':
      command_line_flag(opt->skolems_last, optarg);
      break;
    case 'c':  /* prover compatability mode */
    case 'f':  /* input files */
    case 'g':  /* tptp syntax */
      /* do nothing---these are handled elsewhere */
      break;

    case '?':
    default:
      usage_message(stderr, opt);
      fatal_error("unrecognized command-line option or missing value (flags take 0 or 1)");
    }
  }
}  /* process_command_line_args */

