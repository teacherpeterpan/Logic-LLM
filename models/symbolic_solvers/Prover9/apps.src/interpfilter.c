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

#define PROGRAM_NAME    "interpfilter"
#include "../VERSION_DATE.h"

static char Help_string[] =

"\nThis program takes a file of clauses/formulas (arg 1), a test to\n"
"apply (arg 2), and a stream of interpretations (from stdin).\n"
"The tests are [all_true,some_true,all_false,some_false.  We print\n"
"(to stdout) the interpretations that satisfy the test.\n\n"
"For example, if the set of claues consists of associativity \n"
"and commutativity (AC), we can remove AC models by using the\n"
"some_false test.\n"
"For example,\n\n"
"    interpfilter clauses some_false < interps.in > interps.out\n\n";

/*************
 *
 *  find_clause(topforms, interp)
 *
 *  Return the first clause that has the requested value (TRUE or FALSE)
 *  in the interpretation.  Return NULL on failure.
 *
 *************/

static
Topform find_clause(Plist topforms, Interp interp, BOOL requested_val)
{
  Plist p = topforms;
  while (p != NULL) {
    BOOL val = eval_topform(p->v, interp);
    if (val == requested_val)
      return p->v;
    else
      p = p->next;
  }
  return NULL;
}  /* find_clause */

enum {ALL_TRUE, SOME_TRUE, ALL_FALSE, SOME_FALSE};

int main(int argc, char **argv)
{
  FILE *clause_fp;
  Term t;
  int i;
  Interp interp;
  unsigned long int checked = 0;
  unsigned long int passed = 0;
  Plist topforms;
  int operation = -1;
  BOOL commands = string_member("commands", argv, argc);

  if (string_member("help", argv, argc) ||
      string_member("help", argv, argc) ||
      argc < 3) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  if (string_member("all_true", argv, argc))
    operation = ALL_TRUE;
  else if (string_member("some_true", argv, argc))
    operation = SOME_TRUE;
  else if (string_member("all_false", argv, argc))
    operation = ALL_FALSE;
  else if (string_member("some_false", argv, argc))
    operation = SOME_FALSE;
  else
    fatal_error("interpfilter: operation should be {all,some}_{true,false}");

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
    checked++;
    interp = compile_interp(t, FALSE);
    c = find_clause(topforms, interp, 
		    operation == SOME_TRUE ||
		    operation == ALL_FALSE);

    if ((c  && (operation == SOME_TRUE || operation == SOME_FALSE)) ||
	(!c && (operation == ALL_TRUE  || operation == ALL_FALSE ))) {

      passed++;
      fprint_interp_standard(stdout, interp);
    }
    zap_interp(interp);
    zap_term(t);
    t = read_term(stdin, stderr);
  }

  printf("%% %s %s %s: checked %lu, passed %lu, %.2f seconds.\n",
	 PROGRAM_NAME, argv[1], argv[2], checked, passed, user_seconds());
  
  exit(passed > 0 ? 0 : 1);
}  /* main */

