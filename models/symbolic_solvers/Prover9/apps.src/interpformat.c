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

/* Format an interpretation in various ways
 */

#include "../ladr/top_input.h"
#include "../ladr/interp.h"
#include "../ladr/banner.h"

#define PROGRAM_NAME    "modelformat"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program reads interpretations in standard format\n"
"(from stdin or with -f <file>).  The input can be just interps\n"
"(with or without list(interpretations)) or a Mace4 output file.\n"
"It and takes a command-line argument saying how to print the interps:\n\n"
"    standard    : one line per operation\n"
"    standard2   : standard, with binary operations in a square (default)\n"
"    portable    : list of lists, suitable for parsing by Python, GAP, etc.\n"
"    tabular     : as nice tables\n"
"    raw         : similar to standard, but without punctuation\n"
"    cooked      : as terms, e.g., f(0,1)=2\n"
"    tex         : formatted for LaTeX\n"
"    xml         : XML\n\n"
"Also, argument \"output '<operations>'\" is accepted.\n\n";

enum {STANDARD, STANDARD2, PORTABLE, TABULAR, RAW, COOKED, TEX, XML};

/*************
 *
 *   read_next_section()
 *
 *************/

static
String_buf read_next_section(FILE *fp)
{
  char line[1000], *s;  /* the first 999 chars of the line */
  String_buf sb = get_string_buf();
  BOOL ok = FALSE;

  s = fgets(line, 1000, fp);
  while (s && !substring("==== end of ", s)) {
    if (ok)
      sb_append(sb, s);
    else if (initial_substring("====", s))
      ok = TRUE;
    s = fgets(line, 1000, fp);
  }

  if (!s)
    fatal_error("read_next_section, \"==== end of \" not found");

  return sb;
}  /* read_next_section */

/*************
 *
 *   read_mace4_input()
 *
 *************/

static
String_buf read_mace4_input(FILE *fp)
{
  char line[1000], *s;  /* the first 999 chars of the line */
  String_buf sb = get_string_buf();
  BOOL ok = FALSE;

  s = fgets(line, 1000, fp);
  while (s && !substring("==== end of input", s)) {
    if (!ok && (initial_substring("clauses(", s) ||
		initial_substring("formulas(", s))) {
      if (sb_size(sb) != 0)
	sb_append(sb, "\n");  /* no newline before first list */
      ok = TRUE;
    }
    
    if (ok) {
      sb_append(sb, s);
      if (initial_substring("end_of_list.", s))
	ok = FALSE;
    }
      
    s = fgets(line, 1000, fp);
  }

  if (!s)
    fatal_error("read_mace4_input, \"==== end of input\" not found");

  return sb;
}  /* read_mace4_input */

/*************
 *
 *   read_to_line()
 *
 *************/

static
BOOL read_to_line(FILE *fp, char *str)
{
  char line[1000], *s;  /* the first 999 chars of the line */
  
  s = fgets(line, 1000, fp);
  while (s && !substring(str, line))
    s = fgets(line, 1000, fp);

  return s != NULL;
}  /* read_to_line */

/*************
 *
 *   next_interp()
 *
 *************/

static
Term next_interp(FILE *fp, BOOL mace4_file)
{
  if (mace4_file) {
    if (read_to_line(fp, "= MODEL ="))
      return read_term(fp, stderr);
    else
      return NULL;
  }
  else {
    Term t = read_term(fp, stderr);
    if (t == NULL)
      return NULL;
    else if (is_term(t, "terms", 1) ||
	     is_term(t, "list", 1) ||
	     end_of_list_term(t)) {
      zap_term(t);
      return next_interp(fp, FALSE);
    }
    else
      return t;
  }
}  /* next_interp */

int main(int argc, char **argv)
{
  BOOL mace4_file;  /* list of interps or mace4 output */
  BOOL wrap;        /* enclose output in list(interpretations). */
  int type, rc, count;
  Term t;
  String_buf heading, mace4_input;
  FILE *fin = stdin;
  char *filename = NULL;
  Plist output_strings = NULL;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }
  else if (string_member("standard", argv, argc))
    type = STANDARD;
  else if (string_member("standard2", argv, argc))
    type = STANDARD2;
  else if (string_member("portable", argv, argc))
    type = PORTABLE;
  else if (string_member("tabular", argv, argc))
    type = TABULAR;
  else if (string_member("raw", argv, argc))
    type = RAW;
  else if (string_member("cooked", argv, argc))
    type = COOKED;
  else if (string_member("tex", argv, argc))
    type = TEX;
  else if (string_member("xml", argv, argc))
    type = XML;
  else {
    type = STANDARD2;  /* default */
  }

  wrap = string_member("wrap", argv, argc);

  rc = which_string_member ("output", argv, argc);
  if (rc == -1)
    rc = which_string_member ("-output", argv, argc);
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal_error("interpformat: missing \"output\" argument");
    else
      output_strings = split_string(argv[rc+1]);
  }

  rc = which_string_member("-f", argv, argc);
  if (rc == -1)
    fin = stdin;
  else if (rc+1 >= argc)
    fatal_error("file name missing");
  else {
    filename = argv[rc+1];
    fin = fopen(filename, "r");
    if (fin == NULL) {
      char s[100];
      sprintf(s, "file %s not found", argv[rc+1]);
      fatal_error(s);
    }
  }

  /* Input can be any of 3 types:
       1. stream of interps
       2. list of interps, surrounded by list(interpretations) .. end_of_list
       3. Mace4 output file, with "= MODEL =" ... "= end of model ="
     See next_interp().
  */

  rc = getc(fin);
  mace4_file = (rc == '=');
  if (!mace4_file)
    ungetc(rc, fin);

  init_standard_ladr();
  simple_parse(TRUE);

  if (mace4_file) {
    heading = read_next_section(fin);
    mace4_input = read_mace4_input(fin);
  }
  else {
    heading = NULL;
    mace4_input = NULL;
  }

  /* Okay, read and format the interps. */

  if (type == XML) {
    printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    printf("\n<!DOCTYPE interps SYSTEM \"interp3.dtd\">\n");
    printf("\n<?xml-stylesheet type=\"text/xsl\" href=\"interp3.xsl\"?>\n");
    printf("\n<interps>\n");
    if (fin != stdin)
      printf("\n<source>%s</source>\n", filename);
    if (heading) {
      printf("\n<heading><![CDATA[\n");
      fprint_sb(stdout, heading);
      printf("]]></heading>\n");
    }
    if (mace4_input) {
      printf("\n<input><![CDATA[\n");
      fprint_sb(stdout, mace4_input);
      printf("]]></input>\n");
    }
  }

  if (wrap)
    printf("list(interpretations).\n");

  t = next_interp(fin, mace4_file);
  count = 0;

  while (t != NULL) {
    Interp a;
    count++;

    if (output_strings != NULL)
      interp_remove_others(t, output_strings);

    a = compile_interp(t, TRUE);

    if (type == STANDARD)
      fprint_interp_standard(stdout, a);
    else if (type == STANDARD2)
      fprint_interp_standard2(stdout, a);
    else if (type == PORTABLE) {
      printf("%s\n", count == 1 ? "[" : ",");
      fprint_interp_portable(stdout, a);
    }
    else if (type == TABULAR)
      fprint_interp_tabular(stdout, a);
    else if (type == COOKED)
      fprint_interp_cooked(stdout, a);
    else if (type == RAW)
      fprint_interp_raw(stdout, a);
    else if (type == TEX)
      fprint_interp_tex(stdout, a);
    else if (type == XML)
      fprint_interp_xml(stdout, a);
    else
      fprint_interp_standard2(stdout, a);

    zap_interp(a);
    zap_term(t);

    t = next_interp(fin, mace4_file);
  }

  if (type == XML)
    printf("\n</interps>\n");
  else if (type == PORTABLE)
    printf("\n]\n");

  if (wrap)
    printf("end_of_list.\n");

  exit(count > 0 ? 0 : 2);
}  /* main */

