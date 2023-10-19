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
#include "../ladr/clause_eval.h"
#include "../ladr/weight.h"

#define PROGRAM_NAME    "test_clause_eval"
#include "../VERSION_DATE.h"

int main(int argc, char **argv)
{
  FILE *rule_fp;
  Topform c;
  Term t;
  int i;
  Clause_eval compiled_rule;
  Plist rules = NULL;

  init_standard_ladr();
  init_weight(NULL, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0);
  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
  i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */

  rule_fp = fopen(argv[1], "r");
  if (rule_fp == NULL)
    fatal_error("test_clause_eval, rule file cannot be opened for reading");

  t = read_term(rule_fp, stderr);  /* get first rule */

  while (t != NULL) {
    compiled_rule = compile_clause_eval_rule(t);
    rules = plist_append(rules, compiled_rule);
    fwrite_term_nl(stdout, t);
    zap_term(t);
    t = read_term(rule_fp, stderr);
  }
  fclose(rule_fp);

  /* Evaluate each clause on stdin. */

  c = read_clause(stdin, stderr);

  while (c != NULL && !end_of_list_clause(c)) {
    Plist p;
    c->weight = clause_weight(c->literals);
    for (p = rules; p; p = p->next) {
      Clause_eval rule = p->v;
      BOOL result = eval_clause_in_rule(c, rule);
      printf("%d  (wt=%.3f) : ", result, c->weight);
      fwrite_clause(stdout, c, CL_FORM_BARE);
    }
    printf("\n");
    
    zap_topform(c);
    c = read_clause(stdin, stderr);
  }
  
  exit(0);
}  /* main */

