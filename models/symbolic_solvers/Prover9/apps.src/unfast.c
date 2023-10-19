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

#define PROGRAM_NAME    "unfast"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a stream of fastparse terms and writes them\n"
"as ordinary terms.  The following symbols/arities are recognized.\n"
"arity 2: [=mjfd+*/]; arity 1: [cgi-~']; variables are r--z.  Other symbols\n"
"are constants.  Input should be one term per line, terminated\n"
"with a period, without spaces.  Lines starting with % are echoed.\n\n";

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  Topform c;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  fast_set_defaults();

  c = fast_read_clause(stdin, stderr);
  while (c != NULL) {
    fwrite_clause(stdout, c, CL_FORM_BARE);
    fflush(stdout);
    zap_topform(c);
    c = fast_read_clause(stdin, stderr);
  }
  exit(0);
}  /* main */
