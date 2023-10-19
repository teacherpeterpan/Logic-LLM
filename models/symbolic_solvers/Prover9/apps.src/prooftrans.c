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

/* Transform and format proofs in various ways.
 */

#include "../ladr/top_input.h"
#include "../ladr/xproofs.h"
#include "../ladr/ivy.h"
#include "../ladr/banner.h"

#define PROGRAM_NAME "prooftrans"
#include "../VERSION_DATE.h"

static char Help_string[] = 
"\nprooftrans              [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans parents_only [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans xml          [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans ivy                   [renumber]               [-f <file>]\n"
  "prooftrans hints [-label <label>]  [expand] [striplabels] [-f <file>]\n"
  "prooftrans tagged                                         [-f <file>]\n"
"\n";

enum {NO_TRANS, EXPAND_EQ, EXPAND, EXPAND_IVY};   /* proof transformations */

enum {ORDINARY, PARENTS_ONLY, XML, HINTS, IVY, TAGGED};   /* output format */

#define BUF_MAX 1000

/*************
 *
 *   read_heading()
 *
 *************/

static
String_buf read_heading(FILE *fp, int output_format)
{
  char line[BUF_MAX], *s;  /* the first BUF_MAX-1 chars of the line */
  String_buf sb = get_string_buf();
  int i = 0;

  s = fgets(line, BUF_MAX, fp);
  while (s && !substring("= end of head =", s)) {
    if (i != 0) {
      if (output_format == IVY)
	sb_append(sb, ";; ");
      sb_append(sb, s);
    }
    i++;
    s = fgets(line, BUF_MAX, fp);
  }

  if (!s)
    fatal_error("read_heading, \"= end of head =\" not found");

  return sb;
}  /* read_heading */

/*************
 *
 *   read_program_input()
 *
 *************/

static
void read_program_input(FILE *fp, int output_format)
{
  char line[BUF_MAX], *s;       /* the first BUF_MAX-1 chars of the line */
  BOOL in_list = FALSE;      /* parsing list of clauses, formulas, or terms? */
  Plist lang_commands = NULL;  /* in case of multiple identical commands */

  s = fgets(line, BUF_MAX, fp);
  while (s && !substring("= end of input =", s)) {
    if (in_list)
      in_list = !initial_substring("end_of_list.", line);
    else if (initial_substring("clauses(", line) ||
	     initial_substring("formulas(", line) ||
	     initial_substring("terms(", line))
      in_list = TRUE;
    else if (initial_substring("op(", line) ||
	     initial_substring("redeclare(", line)) {
      Term cmd = parse_term_from_string(line);
      if (initial_substring("op(", line))
	process_op(cmd, FALSE, stdout);
      else
	process_redeclare(cmd, FALSE, stdout);
      if (output_format != XML && output_format != IVY) {
	if (!tlist_member(cmd, lang_commands)) {  /* don't print duplicates */
	  fwrite_term_nl(stdout, cmd);
	  lang_commands = plist_prepend(lang_commands, cmd);
	}
      }
    }
    else if (substring("set(prolog_style_variables)", line)) {
      if (output_format != XML && output_format != IVY)
	printf("\nset(prolog_style_variables).");
      set_variable_style(PROLOG_STYLE);
    }
    
    s = fgets(line, BUF_MAX, fp);
  }
  
  if (!s)
    fatal_error("read_program_input, \"= end of input =\" not found");

  zap_plist_of_terms(lang_commands);
  
}  /* read_program_input */

/*************
 *
 *   read_to_line()
 *
 *************/

static
BOOL read_to_line(FILE *fp, char *str)
{
  char line[BUF_MAX], *s;  /* the first BUF_MAX-1 chars of the line */
  
  s = fgets(line, BUF_MAX, fp);
  while (s && !substring(str, line))
    s = fgets(line, BUF_MAX, fp);

  return s != NULL;
}  /* read_to_line */

/*************
 *
 *   print_proof()
 *
 *************/

static
void print_proof(FILE *fp, Plist proof, String_buf comment,
		 int format, I3list jmap, int number)
{
  Plist p;
  int length = proof_length(proof);
  int max_count = max_clause_symbol_count(proof);

  if (format == CL_FORM_XML) {
    fprintf(fp, "\n<proof number=\"%d\" length=\"%d\" max_count=\"%d\">\n",
	   number, length, max_count);
    fprintf(fp, "\n<comments><![CDATA[\n");
    fprint_sb(fp, comment);
    fprintf(fp, "]]></comments>\n");

  }
  else if (format == CL_FORM_IVY) {
    fprintf(fp, "\n;; BEGINNING OF PROOF OBJECT\n");
    fprintf(fp, "(\n");
  }
  else {
    print_separator(stdout, "PROOF", TRUE);
    fprintf(fp, "\n%% -------- Comments from original proof --------\n");
    fprint_sb(stdout, comment);
    fprintf(fp, "\n");
  }
  
  for (p = proof; p; p = p->next)
    fwrite_clause_jmap(stdout, p->v, format, jmap);

  if (format == CL_FORM_XML)
    fprintf(fp, "\n</proof>\n");
  else if (format == CL_FORM_IVY) {
    fprintf(fp, ")\n");
    fprintf(fp, ";; END OF PROOF OBJECT\n");
  }
  else
    print_separator(stdout, "end of proof", TRUE);
}  /* print_proof */

/*************
 *
 *   add_to_hints()
 *
 *************/

static
Plist add_to_hints(Plist hints, Plist proof)
{
  Plist p;
  for (p = proof; p; p = p->next) {
    if (!clause_member_plist(hints, p->v))
      hints = plist_prepend(hints, p->v);
  }
  return hints;
}  /* add_to_hints */

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  int transformation;
  int output_format;
  BOOL renumber_first = FALSE;
  BOOL renumber_last = FALSE;
  BOOL striplabels;
  int clause_format;

  int rc, n, number_of_proofs;
  BOOL found;
  Plist p, c;
  int label_attr, answer_attr, props_attr;
  FILE *fin = stdin;
  char *filename = NULL;
  char *label = NULL;

  String_buf heading;       /* first few lines of the file */
  Plist proofs = NULL;      /* all of the proofs */
  Plist comments = NULL;    /* the corresponding comments */
  Plist hints = NULL;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("%s", Help_string);
    exit(1);
  }

  if (string_member("expand", argv, argc))
    transformation = EXPAND;
  else if (string_member("expand_eq", argv, argc))
    transformation = EXPAND_EQ;
  else
    transformation = NO_TRANS;

  if (string_member("parents_only", argv, argc))
    output_format = PARENTS_ONLY;
  else if (string_member("xml", argv, argc) || 
	   string_member("XML", argv, argc))
    output_format = XML;
  else if (string_member("hints", argv, argc))
    output_format = HINTS;
  else
    output_format = ORDINARY;

  if (string_member("ivy", argv, argc) ||
      string_member("IVY", argv, argc)) {
    transformation = EXPAND_IVY;
    output_format = IVY;
  }

  /* BV(2007-aug-20): recognize tagged proof option */
  if (string_member("tagged", argv, argc) ||
      string_member("TAGGED", argv, argc)) {
    transformation = NO_TRANS;
    output_format = TAGGED;
  }

  striplabels = (string_member("striplabels", argv, argc) ||
		 string_member("-striplabels", argv, argc));
    
  if (string_member("renumber", argv, argc)) {
    if (string_member("expand", argv, argc)) {
      if (which_string_member("renumber", argv, argc) <
	  which_string_member("expand", argv, argc))
	renumber_first = TRUE;
      else
	renumber_last = TRUE;
    }
    else
      renumber_last = TRUE;
  }

  n = which_string_member("-f", argv, argc);
  if (n == -1)
    fin = stdin;
  else if (n+1 >= argc)
    fatal_error("file name missing");
  else {
    filename = argv[n+1];
    fin = fopen(filename, "r");
    if (fin == NULL) {
      char s[100];
      sprintf(s, "file %s not found", argv[n+1]);
      fatal_error(s);
    }
  }
  
  n = which_string_member("-label", argv, argc);
  if (n == -1)
    label = NULL;
  else if (n+1 >= argc)
    label = "";
  else
    label = argv[n+1];

  init_standard_ladr();
  label_attr  = register_attribute("label",  STRING_ATTRIBUTE);
  answer_attr = register_attribute("answer", TERM_ATTRIBUTE);
  props_attr = register_attribute("props", TERM_ATTRIBUTE);
  declare_term_attribute_inheritable(answer_attr);

  if (output_format == XML)
    clause_format = CL_FORM_XML;
  else if (output_format == IVY)
    clause_format = CL_FORM_IVY;
  else if (output_format == PARENTS_ONLY)
    clause_format = CL_FORM_PARENTS;
  else if (output_format == HINTS)
    clause_format = CL_FORM_BARE;
  else if (output_format == TAGGED)
    clause_format = CL_FORM_TAGGED;
  else
    clause_format = CL_FORM_STD;

  /* Ok, start reading the input. */

  heading = read_heading(fin, output_format);

  if (output_format != XML && output_format != HINTS) {
    if (output_format == IVY)
      printf(";; ");
    print_separator(stdout, PROGRAM_NAME, FALSE);
    fprint_sb(stdout, heading);
    if (output_format == IVY)
      printf(";; ");
    print_separator(stdout, "end of head", FALSE);
  }

  read_program_input(fin, output_format);

  if (output_format != XML && output_format != HINTS && output_format != IVY)
    print_separator(stdout, "end of input", TRUE);

  found = read_to_line(fin, "= PROOF =");  /* finishes line */

  number_of_proofs = 0;

  while (found) {
    /* ok, we have a proof */
    String_buf comment = get_string_buf();

    char s[BUF_MAX];
    Plist proof = NULL;
    number_of_proofs++;

    rc = fscanf(fin, "%s", s);         /* "%" or clause id */
    while (str_ident(s, "%")) {
      fgets(s, BUF_MAX, fin);
      /* in case prooftrans is run multiple times */
      if (!substring("Comments from original proof", s)) {
	sb_append(comment, "%");
	sb_append(comment, s);
      }
      /* in case there are any long comment lines (answers) */
      if (s[strlen(s)] != '\n') {
	char c;
	do {
	  c = fgetc(fin);
	  sb_append_char(comment, c);
	} while (c != '\n');
      }
      rc = fscanf(fin, "%s", s);       /* "%" or clause id */
    }

    while (!substring("==========", s)) {  /* separator at end of proof */
      Term clause_term = read_term(fin, stderr);
      Term just_term = read_term(fin, stderr);
      int id;
      Topform cl = term_to_clause(clause_term);
      clause_set_variables(cl, MAX_VARS);
      cl->justification = term_to_just(just_term);
      if (striplabels)
	cl->attributes = delete_attributes(cl->attributes, label_attr);
      if (str_to_int(s, &id))
	cl->id = id;
      else
	fatal_error("clause id is not an integer");

      proof = plist_prepend(proof, cl);

      rc = fscanf(fin, "%s", s);         /* clause id */
    }
    proof = reverse_plist(proof);
    proofs = plist_append(proofs, proof);
    comments = plist_append(comments, comment);
    found = read_to_line(fin, "= PROOF =");
  }

  if (output_format == XML) {
    printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    printf("\n<!DOCTYPE proofs SYSTEM \"proof3.dtd\">\n");
    printf("\n<?xml-stylesheet type=\"text/xsl\" href=\"proof3.xsl\"?>\n");
    printf("\n<proofs number_of_proofs=\"%d\">\n", number_of_proofs);
    if (fin != stdin)
      printf("\n<source>%s</source>\n", filename);
    printf("\n<heading><![CDATA[\n");
    fprint_sb(stdout, heading);
    printf("]]></heading>\n");
  }

  for (p = proofs, c = comments, n = 1; p; p = p->next, c = c->next, n++) {
    I3list jmap = NULL;
    Plist proof = p->v;
    String_buf comment = c->v;

    if (renumber_first) {
      Plist proof2 = copy_and_renumber_proof(proof, 1);
      delete_clauses(proof);
      proof = proof2;
    }

    if (transformation == EXPAND) {
      Plist proof2 = expand_proof(proof, &jmap);
      delete_clauses(proof);
      proof = proof2;
    }

    else if (transformation == EXPAND_IVY) {
      Plist proof2 = expand_proof(proof, &jmap);
      Plist proof3 = expand_proof_ivy(proof2);
      delete_clauses(proof);
      delete_clauses(proof2);
      proof = proof3;
    }

    if (renumber_last) {
      Plist proof2 = copy_and_renumber_proof(proof, 1);
      delete_clauses(proof);
      proof = proof2;
      zap_i3list(jmap);
      jmap = NULL;
    }

    if (output_format == HINTS)
      hints = add_to_hints(hints, proof);  /* add (without dups) to hints */
    else
      print_proof(stdout, proof, comment, clause_format, jmap, n);
  }

  if (output_format == XML)
    printf("\n</proofs>\n");

  else if (output_format == HINTS) {
    int n = 0;
    hints = reverse_plist(hints);

    printf("\nformulas(hints).\n\n");
    printf("%% %d hints from %d proof(s) in file %s, %s\n",
	   plist_count(hints),
	   plist_count(proofs), filename ? filename : "stdin",
	   get_date());

    for (p = hints; p; p = p->next) {

      Topform c = p->v;
      if (label) {
	char s[100];
	/* quote it only if necessary */
	char *q = (ordinary_constant_string(label) ? "" : "\"");
	sprintf(s, "%s%s_%d%s", q, label, ++n, q);
	c->attributes = set_string_attribute(c->attributes, label_attr, s);
      }
      fwrite_clause(stdout, c, CL_FORM_BARE);
    }
    printf("end_of_list.\n");
  }

  if (number_of_proofs > 0)
    exit(0);
  else
    exit(2);
}  /* main */
