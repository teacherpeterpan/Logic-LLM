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

#define PROGRAM_NAME    "sigtest"
#include "../VERSION_DATE.h"

static char Help_string[] =

"\nHelp\n";

int main(int argc, char **argv)
{
  FILE *clause_fp;
  Term t;
  int i;
  Interp interp;
  int checked = 0;
  Plist topforms;
  BOOL commands = string_member("commands", argv, argc);

  if (string_member("help", argv, argc) ||
      string_member("help", argv, argc) ||
      argc < 1) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  clause_fp = fopen(argv[1], "r");
  if (clause_fp == NULL)
    fatal_error("interpfilter: clause file cannot be opened for reading");

  init_standard_ladr();
  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
  i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */

  if (commands) {
    t = read_commands(clause_fp, stderr, FALSE, KILL_UNKNOWN);
    if (!is_term(t, "topforms", 1) && !is_term(t, "formulas", 1))
      fatal_error("interpfilter: formulas(...) not found");
  }
  
  topforms = read_clause_or_formula_list(clause_fp, stderr);
  fclose(clause_fp);

  /* Print each interpretation on stdin that satisfies the query. */

  t = read_term(stdin, stderr);
  while (t != NULL) {
    Topform c;
    Plist p;
    int i;
    checked++;
    interp = compile_interp(t, FALSE);

    /* printf("Interp %d: ", checked);*/
    for (p = topforms, i = 1; p; p = p->next, i++) {
      int n;
      c = p->v;
      n = eval_literals_false_instances(c->literals, interp);
      printf(" c%d: %3d,", i, n);
    }
    printf("\n");
    zap_interp(interp);
    zap_term(t);
    t = read_term(stdin, stderr);
  }

  exit(0);
}  /* main */

