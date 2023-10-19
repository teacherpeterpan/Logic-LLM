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

#include "../ladr/top_input.h"
#include "../ladr/dollar.h"

#define PROGRAM_NAME    "rewriter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a file of rewrite rules (arg 1) and a\n"
"stream of terms (stdin).  Rewritten terms are sent to stdout.\n"
"The rewrite rules are not checked for termination.\n"
"For example,\n\n"
"   rewriter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations.\n"
"Example file of rewrite rules:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    formulas(demodulators).\n"
"    end_of_list.\n\n";

int main(int argc, char **argv)
{
  FILE *head_fp;
  Plist rules, p;
  Term t;
  BOOL verbose = string_member("verbose", argv, argc);;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc) ||
      argc < 2) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();
  init_dollar_eval();

  head_fp = fopen(argv[1], "r");
  if (head_fp == NULL)
    fatal_error("rewrite rule file can't be opened for reading");

  t = read_commands(head_fp, stderr, verbose, KILL_UNKNOWN);

  if (!is_term(t, "list", 1))
    fatal_error("list(rewrite_rules) not found");

  /* Read list of rules. */

  rules = read_term_list(head_fp, stderr);

  fclose(head_fp);

  for (p = rules; p != NULL; p = p->next) {
    Term rule = p->v;
    if (is_term(rule, "=", 2) ||
	is_term(rule, "<->", 2) ||
	(is_term(rule, "->", 2) && is_term(ARG(rule,1), "=", 2))) {
      term_set_variables(rule, MAX_VARS);
    }
    else {
      fprintf(stdout, "bad rewrite rule: "); fwrite_term_nl(stdout, rule);
      fprintf(stderr, "bad rewrite rule: "); fwrite_term_nl(stderr, rule);
      fatal_error("bad rewrite fule");
    }
  }

  if (verbose)
    fwrite_term_list(stdout, rules, "rewrite_rules");

  /* Read and demodulate terms. */

  t = read_term(stdin, stderr);

  while (t != NULL) {
    if (verbose) {
      fprintf(stdout, "\nBefore:   "); fwrite_term_nl(stdout, t);
    }

    t = programmed_rewrite(t, rules);

    if (verbose)
      fprintf(stdout, "After:    ");

    fwrite_term_nl(stdout, t);
    fflush(stdout);

    zap_term(t);
    t = read_term(stdin, stderr);
  }

  exit(0);

}  /* main */

