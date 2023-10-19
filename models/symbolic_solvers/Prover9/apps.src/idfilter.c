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

#define PROGRAM_NAME    "idfilter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a file of demodulators (arg 1) and a\n"
"stream of equations (stdin).  Each equation is rewritten.\n"
"If it rewrites to an instance of x=x, it is discarded;\n"
"otherwise it (in the unrewritten form) is send to stdout.\n"
"For example,\n\n"
"   idfilter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations and associativity/commutativity.\n"
"Example file of demodulators:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    assoc_comm(^).\n"
"    assoc_comm(v).\n"
"    formulas(demodulator).\n"
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
  int tested = 0;
  int passed = 0;
  Uniftype unification_type;
  BOOL fast_parse = string_member("fast", argv, argc);
  BOOL backward = string_member("x", argv, argc);
  BOOL verbose = string_member("verbose", argv, argc);;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc) ||
      argc < 2) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  if (fast_parse)
    fast_set_defaults();  /* Declare the symbols for fastparse. */

  head_fp = fopen(argv[1], "r");
  if (head_fp == NULL)
    fatal_error("demodulator file can't be opened for reading");

  t = read_commands(head_fp, stderr, FALSE, KILL_UNKNOWN);

  if (!is_term(t, "clauses", 1) && !is_term(t, "formulas", 1))
    fatal_error("formulas(demodulators) not found");

  if (assoc_comm_symbols() || comm_symbols())
    unification_type = BACKTRACK_UNIF;
  else
    unification_type = ORDINARY_UNIF;

  /* Read list of demodulators. */

  demodulators = read_clause_list(head_fp, stderr, TRUE);

  fclose(head_fp);

  /* AC-canonicalize and index the demodulators. */

  idx = mindex_init(DISCRIM_WILD, unification_type, 0);
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

  t = term_reader(fast_parse);

  while (t != NULL) {
    Term tcopy = copy_term(t);
    BOOL ident;
    tested++;
    if (verbose) {
      fprintf(stdout, "\nBefore:   "); fwrite_term_nl(stdout, t);
    }

    if (assoc_comm_symbols())
      ac_canonical(tcopy, -1);
    just = NULL;
    tcopy = demodulate(tcopy, idx, &just, FALSE);

    if (verbose) {
      fprintf(stdout, "After:    ");  fwrite_term_nl(stdout, tcopy);
    }

    ident = (is_term(t, "=", 2) && term_ident(ARG(tcopy,0),ARG(tcopy,1)));
    if ((ident && !backward) ||
	(!ident && backward)) {
      passed++;
      term_writer(t, fast_parse);
      fflush(stdout);
    }

    zap_ilist(just);
    zap_term(t);
    zap_term(tcopy);
    t = term_reader(fast_parse);
  }

  printf("%% %s %s %s: tested %d, passed %d in %.2f seconds.\n",
	 PROGRAM_NAME,
	 argv[1], backward ? "x" : "",
	 tested, passed,
	 user_seconds());
    
  exit(0);

}  /* main */

