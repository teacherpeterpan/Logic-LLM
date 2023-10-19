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

/* Take a stream of interpretations and remove the isomorphic ones.
 */

#include "../ladr/top_input.h"
#include "../ladr/interp.h"

#define PROGRAM_NAME    "isofilter0"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program reads a stream of interpretations (from stdin) and removes\n"
"isomorphic ones.\n\n"
"Interpretations look like this:\n\n"
"interpretation(2, [\n"
"        function(A, [1]),\n"
"        function(e(_,_), [1,0,0,1]),\n"
"        relation(P(_), [0,1])]).\n\n"
"Argument \"ignore_constants\" is accepted.\n"
"Argument \"wrap\" is accepted.\n"
"Argument \"check '<operations>'\" is accepted.\n"
"Argument \"output '<operations>'\" is accepted.\n";

static BOOL iso_member(Interp a, Plist interps)
{
  if (interps == NULL)
    return FALSE;
  else if (isomorphic_interps(a, interps->v, TRUE))
    return TRUE;
  else
    return iso_member(a, interps->next);
}  /* iso_member */

int main(int argc, char **argv)
{
  Term t;
  Plist interps = NULL;
  int interps_read = 0;
  int interps_kept = 0;
  BOOL ignore_constants = FALSE;
  BOOL wrap = FALSE;  /* surround output with list(interpretations) */
  Plist check_strings = NULL;
  Plist output_strings = NULL;
  int rc, i;

  init_standard_ladr();

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  if (string_member("ignore_constants", argv, argc) ||
      string_member("-ignore_constants", argv, argc))
    ignore_constants = TRUE;

  rc = which_string_member("check", argv, argc);
  if (rc == -1)
    rc = which_string_member("-check", argv, argc);
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal_error("isofilter: missing \"check\" argument");
    else
      check_strings = split_string(argv[rc+1]);
  }

  rc = which_string_member("output", argv, argc);
  if (rc == -1)
    rc = which_string_member("-output", argv, argc);
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal_error("isofilter: missing \"output\" argument");
    else
      output_strings = split_string(argv[rc+1]);
  }

  if (string_member("wrap", argv, argc) ||
      string_member("-wrap", argv, argc))
    wrap = TRUE;

  /* Input is a stream of interpretations. */

  if (wrap)
    printf("list(interpretations).\n\n");

  t = read_term(stdin, stderr);

  while (t != NULL) {
    Interp a, c;
    Term twork;
    interps_read++;

    if (ignore_constants)
      interp_remove_constants(t);  /* constants not checked or output */

    twork = copy_term(t);

    if (check_strings)
      interp_remove_others(twork, check_strings);
      
    a = compile_interp(twork, FALSE);
    c = normal_interp(a);
    if (iso_member(c, interps))
      zap_interp(c);
    else {
      /* print the original interp */

      Interp b;
      if (output_strings)
	interp_remove_others(t, output_strings);
      
      b = compile_interp(t, FALSE);
      fprint_interp_standard2(stdout, b);
      fflush(stdout);
      zap_interp(b);
      interps = plist_append(interps, c);  /* keep the normal interp */
      interps_kept++;
    }
    zap_interp(a);
    zap_term(t);
    zap_term(twork);
    t = read_term(stdin, stderr);

    if (interps_read % 1000 == 0)
      fprintf(stderr, "%s: %d interps read, %d kept\n",
	      PROGRAM_NAME, interps_read, interps_kept);
  }

  printf("%% %s", PROGRAM_NAME);
  for(i = 1; i < argc; i++)
    printf(" %s", argv[i]);

  printf(": input=%d, kept=%d, ", interps_read, interps_kept);
  printf("checks=%lu, perms=%lu, ", iso_checks(), iso_perms());
  printf("%.2f seconds.\n", user_seconds());

  if (wrap)
    printf("\nend_of_list.\n");

  exit(0);
}  /* main */

