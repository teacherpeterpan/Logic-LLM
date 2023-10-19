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
#include "../ladr/demod.h"

#define PROGRAM_NAME    "rewriter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a file of demodulators (arg 1) and a\n"
"stream of terms (stdin).  Rewritten terms are sent to stdout.\n"
"The demodulators are not checked for termination.\n"
"For example,\n\n"
"   rewriter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations and associativity/commutativity.\n"
"Example file of demodulators:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    assoc_comm(^).\n"
"    assoc_comm(v).\n"
"    formulas(demodulators).\n"
"    x ^ x = x.\n"
"    x ^ (x v y) = x.\n"
"    x v x = x.\n"
"    x v (x ^ y) = x.\n"
"    end_of_list.\n\n";

int main(int argc, char **argv)
{
  FILE *head_fp;
  Ilist just;
  Plist demodulators, p;
  Mindex idx;
  Term t;
  int rewritten = 0;
  BOOL verbose = string_member("verbose", argv, argc);;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc) ||
      argc < 2) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  head_fp = fopen(argv[1], "r");
  if (head_fp == NULL)
    fatal_error("demodulator file can't be opened for reading");

  t = read_commands(head_fp, stderr, verbose, KILL_UNKNOWN);

  if (!is_term(t, "clauses", 1) && !is_term(t, "formulas", 1))
    fatal_error("formulas(demodulators) not found");

  /* Read list of demodulators. */

  demodulators = read_clause_list(head_fp, stderr, TRUE);

  fclose(head_fp);

  /* AC-canonicalize and index the demodulators. */

  if (assoc_comm_symbols() || comm_symbols())
    idx = mindex_init(DISCRIM_WILD, BACKTRACK_UNIF, 0);
  else
    idx = mindex_init(DISCRIM_BIND, ORDINARY_UNIF, 0);

  for (p = demodulators; p != NULL; p = p->next) {
    /* assume positive equality unit */
    Topform d = p->v;
    Literals lit = d->literals;
    Term alpha = lit->atom->args[0];
    mark_oriented_eq(lit->atom);  /* don not check for termination */
    if (assoc_comm_symbols())
      ac_canonical(lit->atom, -1);
    mindex_update(idx, alpha, INSERT);
  }

  if (verbose)
    fwrite_clause_list(stdout, demodulators, "demodulators", CL_FORM_BARE);

  /* Read and demodulate terms. */

  t = read_term(stdin, stderr);

  while (t != NULL) {
    rewritten++;
    if (verbose) {
      fprintf(stdout, "\nBefore:   "); fwrite_term_nl(stdout, t);
    }

    if (assoc_comm_symbols())
      ac_canonical(t, -1);
    just = NULL;
    t = demodulate(t, idx, &just, FALSE);

    if (verbose)
      fprintf(stdout, "After:    ");

    fwrite_term_nl(stdout, t);
    fflush(stdout);

    zap_ilist(just);
    zap_term(t);
    t = read_term(stdin, stderr);
  }

  printf("%% %s %s: rewrote %d terms with %d rewrite steps in %.2f seconds.\n",
	 PROGRAM_NAME, argv[1], rewritten, demod_rewrites(), user_seconds());
    
  exit(0);

}  /* main */

