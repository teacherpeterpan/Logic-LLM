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

#include "banner.h"

#include <ctype.h>  /* for toupper, tolower */

/* Private definitions and types */

/*
 * memory management
 */

/*************
 *
 *   print_separator()
 *
 *************/

/* DOCUMENTATION
Print the standard separator line.
*/

/* PUBLIC */
void print_separator(FILE *fp, char *str, BOOL initial_newline)
{
  int len = 70;  /* total length of line */
  int n, i;
  char *first_half = "==============================";
  if (initial_newline)
    fprintf(fp, "\n");
  fprintf(fp, "%s %s ", first_half, str);
  n = len - (strlen(first_half) + strlen(str) + 2);
  for (i = 0; i < n; i++)
    fprintf(fp, "=");
  fprintf(fp, "\n");
  fflush(fp);
}  /* print_separator */

/*************
 *
 *    void print_banner(argc, argv, name, version, date, as_comments)
 *
 *************/

/* DOCUMENTATION
Print the standard banner.
*/

/* PUBLIC */
void print_banner(int argc, char **argv,
		  char *name, char *version, char *date,
		  BOOL as_comments)
{
  int i;
  char *com = (as_comments ? "% " : "");

  if (!as_comments)
    print_separator(stdout, name, FALSE);

  printf("%s%s (%d) version %s, %s.\n",
	 com, name, get_bits(), version, date);
  printf("%sProcess %d was started by %s on %s,\n%s%s",
	 com, my_process_id(), username(), hostname(), com, get_date());
	 
  printf("%sThe command was \"", com);
  for(i = 0; i < argc; i++)
    printf("%s%s", argv[i], (i < argc-1 ? " " : ""));
  printf("\".\n");

  if (!as_comments)
    print_separator(stdout, "end of head", FALSE);
  fflush(stdout);
}  /* print_banner */

