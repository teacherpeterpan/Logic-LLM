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

/* Format proofs in various ways
 */

#include "../ladr/top_input.h"
#include "../ladr/xproofs.h"
#include "../ladr/ivy.h"
#include "../ladr/banner.h"

#define PROGRAM_NAME "directproof"
#include "../VERSION_DATE.h"

static char Help_string[] = "";
static int Transformations = 0;

#define BUF_MAX 1000

/*************
 *
 *   read_heading()
 *
 *************/

static
String_buf read_heading(FILE *fp)
{
  char line[BUF_MAX], *s;  /* the first BUF_MAX-1 chars of the line */
  String_buf sb = get_string_buf();
  int i = 0;

  s = fgets(line, BUF_MAX, fp);
  while (s && !substring("= end of head =", s)) {
    if (i != 0) {
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
void read_program_input(FILE *fp)
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
      if (!tlist_member(cmd, lang_commands)) {  /* don't print duplicates */
	fwrite_term_nl(stdout, cmd);
	lang_commands = plist_prepend(lang_commands, cmd);
      }
    }
    else if (substring("set(prolog_style_variables)", line)) {
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
    if (comment) {
      fprintf(fp, "\n<comments><![CDATA[\n");
      fprint_sb(fp, comment);
      fprintf(fp, "]]></comments>\n");
    }
  }
  else if (format == CL_FORM_IVY) {
    fprintf(fp, "\n;; BEGINNING OF PROOF OBJECT\n");
    fprintf(fp, "(\n");
  }
  else {
    print_separator(stdout, "PROOF", TRUE);
    if (comment) {
      fprintf(fp, "\n%% -------- Comments from original proof --------\n");
      fprint_sb(stdout, comment);
      fprintf(fp, "\n");
    }
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
 *   horn_eq_clauses()
 *
 *************/

/* DOCUMENTATION
Is every clause in the Plist an equality unit (pos or neg)?
*/

/* PUBLIC */
BOOL horn_eq_clauses(Plist l)
{
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = p->v;
    if (!string_attribute_member(c->attributes, label_att(), "non_clause") &&
	c->literals) {
      if (!horn_clause(c->literals) || !only_eq(c->literals)) {
	return FALSE;
      }
    }
  }
  return TRUE;
}  /* horn_eq_clauses */

/*************
 *
 *   pos_bin_res()
 *
 *************/

/* DOCUMENTATION
If it is a resolution (binary, hyper, ur), is it positive binary?
*/

/* PUBLIC */
BOOL pos_bin_res(Topform c, Plist proof)
{
  if (!c->justification)
    return TRUE;
  else if (c->justification->type == HYPER_RES_JUST ||
	   c->justification->type == UR_RES_JUST)
    return FALSE;
  else if (c->justification->type != BINARY_RES_JUST)
    return TRUE;
  else {
    Ilist parents = get_parents(c->justification, FALSE);
    Topform a = proof_id_to_clause(proof, parents->i);
    Topform b = proof_id_to_clause(proof, parents->next->i);
    BOOL ok = (positive_clause(a->literals) ||
	       positive_clause(b->literals));
    zap_ilist(parents);
    return ok;
  }
}  /* pos_bin_res */

/*************
 *
 *   all_resolutions_pos_binary()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL all_resolutions_pos_binary(Plist proof)
{
  Plist p;
  for (p = proof; p; p = p->next) {
    if (!pos_bin_res(p->v, proof))
      return FALSE;
  }
  return TRUE;
}  /* all_resolutions_pos_binary */

/*************
 *
 *   all_paramodulations_unit()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL all_paramodulations_unit(Plist proof)
{
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c = p->v;
    if (c->justification->type == PARA_JUST &&
	number_of_literals(c->literals) != 1)
      return FALSE;
  }
  return TRUE;
}  /* all_paramodulations_unit */

/*************
 *
 *   first_pos_parent()
 *
 *************/

static
Topform first_pos_parent(Topform c, Plist proof)
{
  Ilist parents = get_parents(c->justification, FALSE);
  Ilist p;
  for (p = parents; p; p = p->next) {
    Topform c = proof_id_to_clause(proof, p->i);
    if (positive_clause(c->literals))
      return c;
  }
  return NULL;
}  /* first_pos_parent */

/*************
 *
 *   first_neg_parent()
 *
 *************/

static
Topform first_neg_parent(Topform c, Plist proof)
{
  Ilist parents = get_parents(c->justification, FALSE);
  Ilist p;
  for (p = parents; p; p = p->next) {
    Topform c = proof_id_to_clause(proof, p->i);
    if (negative_clause(c->literals))
      return c;
  }
  return NULL;
}  /* first_neg_parent */

/*************
 *
 *   bidirectional()
 *
 *************/

static
BOOL bidirectional(Plist proof)
{
  Topform e = plist_last(proof);
  Topform n1 = first_neg_parent(e, proof);
  if (n1 && !n1->is_formula) {
    Topform n2 = first_neg_parent(n1, proof);
    return n2 && !n1->is_formula;
  }
  else
    return FALSE;
}  /* bidirectional */

/*************
 *
 *   last_two_steps()
 *
 *************/

static
void last_two_steps(Plist proof, Topform *e,
		    Topform *p1, Topform *n1,
		    Topform *p2, Topform *n2)
{
  *e = plist_last(proof);
  *p1 = first_pos_parent(*e, proof);
  *n1 = first_neg_parent(*e, proof);
  *p2 = first_pos_parent(*n1, proof);
  *n2 = first_neg_parent(*n1, proof);
}  /* last_two_steps */

/*************
 *
 *   xx_recurse()
 *
 *************/

static
Term xx_recurse(Term t, Ilist pos, int *n)
{
  if (pos == NULL) {
    Term v = get_variable_term(*n);
    (*n)++;
    return v;
  }
  else {
    int i;
    Term new = get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++) {
      Term arg;
      if (i == pos->i-1)
	arg = xx_recurse(ARG(t,i), pos->next, n);
      else {
	arg = get_variable_term(*n);
	(*n)++;
      }
      ARG(new,i) = arg;
    }
    return new;
  }
}  /* xx_recurse */

/*************
 *
 *   xx_instance()
 *
 *************/

static
Topform xx_instance(Literals lits, Ilist pos)
{
  Topform new = get_topform();
  int n = 0;
  Literals lit = ith_literal(lits, pos->i);
  Term t1 = xx_recurse(ARG(lit->atom, pos->next->i-1), pos->next->next, &n);
  Term e = get_rigid_term_like(lit->atom);
  ARG(e,0) = t1;
  ARG(e,1) = copy_term(t1);
  new->literals = append_literal(new->literals, new_literal(1, e));
  new->justification = input_just();
  return new;
}  /* xx_instance */

/*************
 *
 *   instance_from_position_recurse()
 *
 *************/

static
Term instance_from_position_recurse(Term t1, Term t2, Ilist pos, int n)
{
  if (VARIABLE(t1)) {
    Term s = xx_recurse(t2, pos, &n);
    return listterm_cons(copy_term(t1), s);
  }
  else {
    return instance_from_position_recurse(ARG(t1,pos->i-1),
					  ARG(t2,pos->i-1),
					  pos->next, n);
  }
}  /* instance_from_position_recurse */

/*************
 *
 *   instance_from_position()
 *
 *************/

static
Term instance_from_position(Topform c1, Topform c2, Ilist pos)
{
  Term t1 = ith_literal(c1->literals, pos->i)->atom;
  Term t2 = ith_literal(c2->literals, pos->i)->atom;
  int n = greatest_variable_in_clause(c1->literals) + 1;
  return instance_from_position_recurse(t1, t2, pos->next, n);
}  /* instance_from_position */

/*************
 *
 *   forward_proof()
 *
 *************/

static
Plist forward_proof(Plist proof)
{
  int next_id;

  if (!horn_eq_clauses(proof))
    fatal_error("forward_proof, all clauses must be Horn equality");
  else if (!all_paramodulations_unit(proof))
    fatal_error("forward_proof, all paramodulations must be unit");
  else if (!all_resolutions_pos_binary(proof))
    fatal_error("forward_proof, all resolutions must be positive binary");
  
  next_id = greatest_id_in_proof(proof) + 1;

  while (bidirectional(proof)) {
    Topform e, p1, n1, p2, n2;
    Transformations++;
    /* print_proof(stdout, proof, NULL, CL_FORM_STD, NULL, 0); */
    last_two_steps(proof, &e, &p1, &n1, &p2, &n2);
    if (!p1 && has_copy_flip_just(n1)) {
      /* CASE 0: flip neg clause, then conflict with x=x. */
      /* We can simply remove the flip inference. */
      proof = plist_remove(proof, n1);
      /* Change justification on empty clause. */
      e->justification->u.id = n2->id;
    }
    else if (p1 && has_copy_flip_just(n1)) {
      /* CASE 1: flip negative clause, then unit conflict. */
      if (e->justification->u.lst->next->i == -1 ||
	  e->justification->u.lst->next->next->next->i == -1) {
	/* CASE 1a: Unit conflict has an implicit flip.
	   double flips cancel, so we can just remove n1. */
	Topform empty = try_unit_conflict(p1, n2);
	if (!empty)
	  fatal_error("forward_proof, case 1a failed");
	empty->id = next_id++;
	proof = plist_remove(proof, n1);
	proof = plist_remove(proof, e);
	delete_clause(n1);
	delete_clause(e);
	proof = plist_append(proof, empty);
      }
      else {
	/* CASE 1b: change flip from neg to pos clause. */
	Topform flipped_pos, empty;
	int n = n1->justification->next->u.id;  /* flipped literal */
	flipped_pos = copy_inference(p1);
	flip_eq(flipped_pos->literals->atom, n);
	flipped_pos->id = next_id++;
	/* Now, flipped_pos and n2 should give unit conflict. */
	empty = try_unit_conflict(flipped_pos, n2);
	if (!empty)
	  fatal_error("forward_proof, case 1b failed");
	empty->id = next_id++;
	/* Remove 2 clauses and append 2 clauses. */
	proof = plist_remove(proof, n1);
	proof = plist_remove(proof, e);
	delete_clause(n1);
	delete_clause(e);
	proof = plist_append(proof, flipped_pos);
	proof = plist_append(proof, empty);
      }
    }
    else if (p1 && n1->justification->type == PARA_JUST &&
	     n1->justification->next == NULL) {
      /* CASE 3: para into negative clause, then unit conflict. */
      Ilist p2_pos;
      Ilist n2_pos;
      Term target;
      Topform new1, empty;
      Ilist from_pos;
      if (e->justification->u.lst->next->i == -1 ||
	  e->justification->u.lst->next->next->next->i == -1) {
	/* The unit conflict involved an implicit flip.  We'll handle that by
	   explicitly flipping the positive equality first. */
	Topform flipped_pos = copy_inference(p1);
	flip_eq(flipped_pos->literals->atom, 1);
	flipped_pos->id = next_id++;
	proof = plist_append(proof, flipped_pos);
	p1 = flipped_pos;
      }

      p2_pos = n1->justification->u.para->from_pos;
      n2_pos = n1->justification->u.para->into_pos;
      target = term_at_position(p1->literals, n2_pos);
      if (!target) {
	/* Into_term does not exist, so we have to instantiate p1. */
	Term pair = instance_from_position(p1, n1, n2_pos);
	Topform p1i = copy_clause(p1);
	p1i->literals->atom = subst_term(p1i->literals->atom,
					 ARG(pair,0),
					 ARG(pair,1));
	upward_clause_links(p1i);
	p1i->justification = instance_just(p1, plist_append(NULL,pair));
	p1i->id = next_id++;
	proof = plist_append(proof, p1i);
	p1 = p1i;
      }
      from_pos = copy_ilist(p2_pos);
      from_pos->next->i = (p2_pos->next->i == 1 ? 2 : 1);
      new1 = para_pos2(p2, from_pos, p1, n2_pos);
      zap_ilist(from_pos);
      new1->id = next_id++;
      empty = try_unit_conflict(new1, n2);
      if (!empty) {
	fatal_error("directproof failed (unit conflict case 3)");
      }
      empty->id = next_id++;
      proof = plist_remove(proof, n1);
      proof = plist_remove(proof, e);
      delete_clause(n1);
      delete_clause(e);
      proof = plist_append(proof, new1);
      proof = plist_append(proof, empty);
    }  /* case 3 */
    else if (!p1 &&
	     e->justification->type == COPY_JUST &&
	     e->justification->next &&
	     e->justification->next->type == XX_JUST) {
      /* CASE 2: conflict with x=x. */
      Ilist p2_pos = n1->justification->u.para->from_pos;
      Ilist n2_pos = n1->justification->u.para->into_pos;
      Topform positive, empty;
      if (ilist_count(n2_pos) > 2) {
	/* into subterm */
	Topform xxi = xx_instance(n2->literals,
				  n1->justification->u.para->into_pos);
	Topform new1, empty;
	Ilist from_pos = copy_ilist(p2_pos);
	from_pos->next->i = (p2_pos->next->i == 1 ? 2 : 1);
	xxi->id = next_id++;
	new1 = para_pos2(p2, from_pos, xxi, n2_pos);
	zap_ilist(from_pos);
	new1->id = next_id++;
	empty = try_unit_conflict(new1, n2);
	if (!empty)
	  fatal_error("directproof failed (unit conflict case 3a)");
	empty->id = next_id++;
	proof = plist_remove(proof, n1);
	proof = plist_remove(proof, e);
	delete_clause(n1);
	delete_clause(e);
	proof = plist_append(proof, xxi);
	proof = plist_append(proof, new1);
	proof = plist_append(proof, empty);
      }
      else {
	/* into top */
	if (p2_pos->next->i != n2_pos->next->i) {
	  /* left->right or right->left, so we flip the positive parent. */
	  Topform flipped_pos = copy_inference(p2);
	  flip_eq(flipped_pos->literals->atom, 1);
	  flipped_pos->id = next_id++;
	  proof = plist_append(proof, flipped_pos);
	  positive = flipped_pos;
	}
	else
	  positive = p2;
	/* Now, positive and n2 should give unit conflict. */
	empty = try_unit_conflict(positive, n2);
	if (!empty)
	  fatal_error("directproof failed (unit conflict case 3b)");
	empty->id = next_id++;
	/* replace empty. */
	proof = plist_remove(proof, n1);
	proof = plist_remove(proof, e);
	delete_clause(n1);
	delete_clause(e);
	proof = plist_append(proof, empty);
      }
    }  /* case 2 */
    else {
      fprintf(stderr, "CASE not handled, transformation incomplete.\n");
      printf("CASE not handled:\n");
      if (p2)
	{ printf("p2: "); f_clause(p2); }
      if (n2)
	{ printf("n2: "); f_clause(n2); }
      if (p1)
	{ printf("p1: "); f_clause(p1); }
      if (n1)
	{ printf("n1: "); f_clause(n1); }
      if (e)
	{ printf("e:  "); f_clause(e); }
      fatal_error("forward_proof, case not handled");
    }
  }
  return proof;
}  /* forward_proof */

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  int rc, n, number_of_proofs;
  BOOL found;
  Plist p, c;
  int label_attr, answer_attr, props_attr;
  FILE *fin = stdin;
  char *filename = NULL;

  String_buf heading;       /* first few lines of the file */
  Plist proofs = NULL;      /* all of the proofs */
  Plist comments = NULL;    /* the corresponding comments */

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("%s", Help_string);
    exit(1);
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
  
  init_standard_ladr();
  label_attr  = register_attribute("label",  STRING_ATTRIBUTE);
  answer_attr = register_attribute("answer", TERM_ATTRIBUTE);
  props_attr  = register_attribute("props", TERM_ATTRIBUTE);
  declare_term_attribute_inheritable(answer_attr);

  /* Ok, start reading the input. */

  heading = read_heading(fin);

  print_separator(stdout, PROGRAM_NAME, FALSE);
  fprint_sb(stdout, heading);
  print_separator(stdout, "end of head", FALSE);

  read_program_input(fin);

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

  for (p = proofs, c = comments, n = 1; p; p = p->next, c = c->next, n++) {
    I3list jmap = NULL;
    Plist proof = p->v;
    String_buf comment = c->v;

    Plist xproof = expand_proof(proof, &jmap);
    Plist xnproof;
    /* delete_clauses(xproof); */

    Plist xnfproof;
    zap_i3list(jmap);
    /* delete_clauses(proof); */

    xnproof = copy_and_renumber_proof(xproof, 1);
    /* delete_clauses(xproof); */

    xnfproof = forward_proof(xnproof);
    /* delete_clauses(xnproof); */

    print_proof(stdout, xnfproof, comment, CL_FORM_STD, NULL, n);
    /* delete_clauses(xnfproof); */
  }

  if (number_of_proofs > 0) {
    printf("\n%% Directproof did %d transformation(s) on %d proof(s).\n",
	   Transformations, number_of_proofs);
    exit(0);
  }
  else
    exit(2);
}  /* main */
