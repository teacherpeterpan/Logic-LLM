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
#include "../ladr/interp.h"

#define PROGRAM_NAME    "clausetester"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a file of interpretations (arg 1) and a stream of\n"
"clauses (stdin).  For each clause, it tells interpretations in which\n"
"the clause is true.\n\n"
"For example,\n\n"
"    clausetester interps < clauses.in\n\n";

int main(int argc, char **argv)
{
  FILE *interp_fp;
  Topform c;
  Term t;
  Interp interp;
  Plist interps = NULL;
  int icount = 0;
  int ccount = 0;
  int *counters, i;
  BOOL commands = string_member("commands", argv, argc);

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc) ||
      argc < 2) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  interp_fp = fopen(argv[1], "r");
  if (interp_fp == NULL)
    fatal_error("interpretation file cannot be opened for reading");

  init_standard_ladr();

  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
  i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */

  t = read_term(interp_fp, stderr);  /* get first interpretation */

  while (t != NULL) {
    icount++;
    interp = compile_interp(t, FALSE);
    interps = plist_append(interps, interp);
    zap_term(t);
    t = read_term(interp_fp, stderr);
  }
  fclose(interp_fp);

  counters = calloc(icount, sizeof(int *));

  if (commands) {
    t = read_commands(stdin, stdout, FALSE, KILL_UNKNOWN);
    if (!is_term(t, "clauses", 1) && !is_term(t, "formulas", 1))
      fatal_error("formulas(...) not found");
  }

  /* Evaluate each clause on stdin. */

  c = read_clause_or_formula(stdin, stderr);

  while (c != NULL && !end_of_list_clause(c)) {

    Plist p = interps;
    Term t = topform_to_term(c);
    ccount++;
    fwrite_term(stdout, t);
    zap_term(t);
    printf(".  %%");

    i = 0;
    while (p != NULL) {
      if (eval_topform(c, p->v)) {  /* works also for non-clauses */
	counters[i]++;
	printf(" %2d", i+1);
      }
      i++;
      p = p->next;
    }
    printf("\n");
    fflush(stdout);
    zap_topform(c);
    c = read_clause_or_formula(stdin, stderr);
  }

  for (i = 0; i < icount; i++)
    printf("%% interp %d models %d of %d clauses.\n",
	   i+1, counters[i], ccount) ;
  
  exit(0);
}  /* main */

