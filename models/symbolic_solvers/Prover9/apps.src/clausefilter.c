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

#define PROGRAM_NAME    "clausefilter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a file of interpretations (arg 1) and a stream of\n"
"formulas (stdin).  Formulas that pass the given test (arg 2) are sent\n"
"to stdout. The tests are true_in_all, true_in_some, false_in_all,\n"
"false_in_some.\n"
"For example,\n\n"
"   clausefilter interps true_in_all < clauses.in > clauses.out\n\n";

static Ordertype interp_compare(Interp a, Interp b)
{
  if (interp_size(a) < interp_size(b))
    return LESS_THAN;
  else if (interp_size(a) > interp_size(b))
    return GREATER_THAN;
  else
    return SAME_AS;
}  /* interp_compare */

/*************
 *
 *   sort_interps()
 *
 *************/

static
void sort_interps(Plist interps)
{
  int n = plist_count(interps);
  void **a = malloc(n * sizeof(void *));
  Plist p;
  int i = 0;

  for (p = interps; p; p = p->next)
    a[i++] = p->v;
  merge_sort(a, n, (Ordertype (*)(void*,void*)) interp_compare);
  i = 0;
  for (p = interps; p; p = p->next)
    p->v = a[i++];
}  /* sort_interps */

/*************
 *
 *   find_interp(clause, list-of-interpretations, models-flag)
 *
 *   Look for an interpretation that models (or doesn't model) a given clause.
 *   That is, return the first interpretation in which the clause is true
 *   (not true); return NULL if the clause is false (true) in all the
 *   interpretations.
 *
 *************/

Interp find_interp(Topform c, Plist interps,
		   BOOL models, BOOL check_evaluable)
{
  Plist p = interps;
  BOOL value;

  while (p != NULL) {
    if (check_evaluable && !evaluable_topform(c, p->v))
      ;  /* skip this evaluation */
    else {
      value = eval_topform(c, p->v);  /* works also for non-clauses */
      if ((models && value) || (!models && !value))
	return p->v;
    }
    p = p->next;
  }
  return NULL;
}  /* find_interp */

enum {TRUE_IN_ALL, TRUE_IN_SOME, FALSE_IN_ALL, FALSE_IN_SOME};

int main(int argc, char **argv)
{
  FILE *interp_fp;
  Topform c;
  Term t;
  Interp interp;
  Plist interps = NULL;
  int operation = -1;
  BOOL commands = string_member("commands", argv, argc);
  BOOL ignore_nonevaluable = string_member("ignore_nonevaluable", argv, argc);
  unsigned long int checked = 0;
  unsigned long int passed = 0;
  int i;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc) ||
      argc < 3) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  if (string_member("true_in_all", argv, argc))
    operation = TRUE_IN_ALL;
  else if (string_member("true_in_some", argv, argc))
    operation = TRUE_IN_SOME;
  else if (string_member("false_in_all", argv, argc))
    operation = FALSE_IN_ALL;
  else if (string_member("false_in_some", argv, argc))
    operation = FALSE_IN_SOME;
  else
    fatal_error("clausefilter, need argument {true,false}_in_{all,some}");

  init_standard_ladr();
  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
  i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */

  interp_fp = fopen(argv[1], "r");
  if (interp_fp == NULL)
    fatal_error("clausefilter, interp file cannot be opened for reading");

  t = read_term(interp_fp, stderr);  /* get first interpretation */

  while (t != NULL) {
    interp = compile_interp(t, FALSE);
    interps = plist_append(interps, interp);
    zap_term(t);
    t = read_term(interp_fp, stderr);
  }
  fclose(interp_fp);

  sort_interps(interps);  /* puts smallest ones first */

  if (commands) {
    t = read_commands(stdin, stdout, FALSE, KILL_UNKNOWN);
    if (!is_term(t, "clauses", 1) && !is_term(t, "formulas", 1))
      fatal_error("formulas(...) not found");
  }
  
  /* Evaluate each formula/clause on stdin. */

  c = read_clause_or_formula(stdin, stderr);

  while (c != NULL && !end_of_list_clause(c)) {
    
    checked++;

    interp = find_interp(c, interps,
			 operation == TRUE_IN_SOME ||
			 operation == FALSE_IN_ALL,
			 ignore_nonevaluable);

    if ((interp && (operation==TRUE_IN_SOME || operation==FALSE_IN_SOME)) ||
        (!interp && (operation==TRUE_IN_ALL || operation==FALSE_IN_ALL))) {
      passed++;
      fwrite_clause(stdout, c, CL_FORM_BARE);  /* ok for nonclausal formulas */
    }
    zap_topform(c);
    c = read_clause_or_formula(stdin, stderr);
  }
  
  printf("%% %s %s %s: checked %lu, passed %lu, %.2f seconds.\n",
	 PROGRAM_NAME, argv[1], argv[2], checked, passed, user_seconds());

  exit(passed > 0 ? 0 : 1);
}  /* main */

