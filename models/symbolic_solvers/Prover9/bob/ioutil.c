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

#include "ioutil.h"

/* Private definitions and types */

/*************
 *
 *   fwrite_formula()
 *
 *************/

/* DOCUMENTATION
This routine prints a formula, followed by ".\n" to a file.
This version does not print extra parentheses
(it first translates the formula to a term, then
prints the term, then frees the term).
To print the formula directly, with extra parentheses,
call fprint_formula() instead.
*/

/* PUBLIC */
void fwrite_formula(FILE *fp, Formula f)
{
  Term t = formula_to_term(f);
  if (t == NULL)
    fatal_error("fwrite_formula, formula_to_term returns NULL.");
  fwrite_term_nl(fp, t);
  fflush(fp);
  zap_term(t);
}  /* fwrite_formula */

/*************
 *
 *   read_clause()
 *
 *************/

/* DOCUMENTATION
This routine reads a clause from FILE *fin.
If there are no more clauses in *fin, NULL is returned.
<P>
If any error occurs, a message is sent to FILE *fout and a fatal_error occurs.
<P>
Variables are "set", upward links ar made from all subterms to the
clause, and ac_canonical() is called on each atom.
*/

/* PUBLIC */
Topform read_clause(FILE *fin, FILE *fout)
{
  Term t = read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Topform c = term_to_clause(t);
    zap_term(t);
    clause_set_variables(c, MAX_VARS);  /* fatal if too many vars */
    return c;
  }
}  /* read_clause */

/*************
 *
 *   parse_clause_from_string()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform parse_clause_from_string(char *s)
{
  Term t = parse_term_from_string(s);
  Topform c = term_to_clause(t);
  zap_term(t);
  clause_set_variables(c, MAX_VARS);  /* fatal if too many vars */
  return c;
}  /* parse_clause_from_string */

/*************
 *
 *   end_of_list_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL end_of_list_clause(Topform c)
{
  if (c == NULL)
    return FALSE;
  else if (number_of_literals(c->literals) != 1)
    return FALSE;
  else {
    Term a = c->literals->atom;
    if (!CONSTANT(a))
      return FALSE;
    else
      return is_symbol(SYMNUM(a), "end_of_list", 0);
  }
}  /* end_of_list_clause */

/*************
 *
 *   read_clause_clist()
 *
 *************/

/* DOCUMENTATION
This routine reads a list of clauses from FILE *fin.
If you wish the list to have a name, send a string;
othersize send NULL.  (You can name the list later with name_clist().)
If there are no more clauses in *fin, an empty Clist is returned.
<P>
If any error occurs, a message is sent to FILE *fout and a fatal_error occurs.
*/

/* PUBLIC */
Clist read_clause_clist(FILE *fin, FILE *fout, char *name, BOOL assign_id)
{
  Clist lst = clist_init(name);
  Topform c;

  c = read_clause(fin, fout);
  while (c != NULL && !end_of_list_clause(c)) {
    if (assign_id)
      assign_clause_id(c);
    c->justification = input_just();
    clist_append(c, lst);
    c = read_clause(fin, fout);
  }
  if (c != NULL)
    zap_topform(c);  /* end_of_list_clause */
  return lst;
}  /* read_clause_clist */

/*************
 *
 *   read_clause_list()
 *
 *************/

/* DOCUMENTATION
Read clauses, up to end_of_list (or EOF), and return them in a Plist.
*/

/* PUBLIC */
Plist read_clause_list(FILE *fin, FILE *fout, BOOL assign_id)
{
  Clist a = read_clause_clist(fin, fout, NULL, assign_id);
  return move_clist_to_plist(a);
}  /* read_clause_list */

/*************
 *
 *   sb_write_clause_jmap()
 *
 *************/

/* DOCUMENTATION
This routine writes a clause (in mixfix form, with ".\n") to a String_buf.
*/

/* PUBLIC */
void sb_write_clause_jmap(String_buf sb, Topform c,
			  int format,
			  I3list map)
{
  Term t;
  if (c->compressed)
    t = NULL;
  else
    t = topform_to_term(c);
  
  if (format == CL_FORM_BARE) {
    if (t == NULL)
      sb_append(sb, "clause_is_compressed");
    else
      sb_write_term(sb, t);
    sb_append(sb, ".");
  }
  else {
    if (c->id != 0) {
      sb_append_id(sb, c->id, map);
      sb_append(sb, " ");
    }

    if (t == NULL)
      sb_append(sb, "clause_is_compressed");
    else
      sb_write_term(sb, t);
    sb_append(sb, ".  ");
    if (format == CL_FORM_STD)
      sb_write_just(sb, c->justification, map);
    else {
      /* CL_FORM_PARENTS */
      Ilist parents = get_parents(c->justification, TRUE);
      Ilist p;
      sb_append(sb, "[");
      for (p = parents; p; p = p->next) {
	sb_append_id(sb, p->i, map);
	if (p->next)
	  sb_append(sb, ",");
      }
      sb_append(sb, "].");
    }
  }
  sb_append(sb, "\n");
  if (t)
    zap_term(t);
}  /* sb_write_clause_jmap */

/*************
 *
 *   sb_write_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a clause in mixfix form,
followed by ".\n".
*/

/* PUBLIC */
void sb_write_clause(String_buf sb, Topform c, int format)
{
  sb_write_clause_jmap(sb, c, format, NULL);
}  /* sb_write_clause */

/*************
 *
 *   sb_xml_write_clause_jmap()
 *
 *************/

/* DOCUMENTATION
This routine writes a clause (in mixfix form, with ".\n") to a String_buf.
It is written in XML format.
*/

/* PUBLIC */
void sb_xml_write_clause_jmap(String_buf sb, Topform c, I3list map)
{
  sb_append(sb, "\n  <clause id=\"");
  sb_append_id(sb, c->id, map);
  sb_append(sb, "\"");
  if (c->justification && c->justification->type == GOAL_JUST)
    sb_append(sb, " type=\"goal\"");
  else if (c->justification && c->justification->type == DENY_JUST)
    sb_append(sb, " type=\"deny\"");
  else if (c->justification && c->justification->type == INPUT_JUST)
    sb_append(sb, " type=\"assumption\"");
  else if (c->justification && c->justification->type == CLAUSIFY_JUST)
    sb_append(sb, " type=\"clausify\"");
  else if (c->justification && c->justification->type == EXPAND_DEF_JUST)
    sb_append(sb, " type=\"expand_def\"");
  sb_append(sb, ">\n");
  if (c->compressed)
    sb_append(sb, "    <literal>clause_is_compressed</literal>\n");
  else {
    Literals lit;
    Term atts;
    if (c->literals == NULL) {
      sb_append(sb, "    <literal><![CDATA[\n      ");
      sb_append(sb, false_sym());
      sb_append(sb, "\n    ]]></literal>\n");
    }
    else {
      for (lit = c->literals; lit; lit = lit->next) {
	Term t = literal_to_term(lit);
	sb_append(sb, "    <literal><![CDATA[\n      ");
	sb_write_term(sb, t);
	sb_append(sb, "\n    ]]></literal>\n");
	zap_term(t);
      }
    }
    atts = attributes_to_term(c->attributes, attrib_sym());
    if (atts) {
      Term t = atts;
      while (is_term(t, attrib_sym(), 2)) {
	sb_append(sb, "    <attribute><![CDATA[\n      ");
	sb_write_term(sb, ARG(t,0));
	sb_append(sb, "\n    ]]></attribute>\n");
	t = ARG(t,1);
      }
      sb_append(sb, "    <attribute><![CDATA[\n      ");
      sb_write_term(sb, t);
      sb_append(sb, "\n    ]]></attribute>\n");
      zap_term(atts);
    }
  }
  sb_xml_write_just(sb, c->justification, map);
  
  sb_append(sb, "  </clause>\n");
}  /* sb_xml_write_clause_jmap */

/*************
 *
 *   fwrite_clause_jmap()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a clause in mixfix form,
followed by ".\n".
*/

/* PUBLIC */
void fwrite_clause_jmap(FILE *fp, Topform c, int format, I3list map)
			
{
  if (c == NULL)
    fprintf(fp, "fwrite_clause_jmap: NULL clause\n");
  else {
    String_buf sb = get_string_buf();
    if (format == CL_FORM_XML)
      sb_xml_write_clause_jmap(sb, c, map);
    else if (format == CL_FORM_IVY)
      sb_ivy_write_clause_jmap(sb, c, map);

    /* BV(2007-aug-20): tagged proof format */
    else if (format == CL_FORM_TAGGED)
      sb_tagged_write_clause_jmap(sb, c, format, map);

    else
      sb_write_clause_jmap(sb, c, format, map);
    fprint_sb(fp, sb);
    zap_string_buf(sb);
  }
  fflush(fp);
}  /* fwrite_clause_jmap */

/*************
 *
 *   fwrite_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a clause in mixfix form,
followed by ".\n".
*/

/* PUBLIC */
void fwrite_clause(FILE *fp, Topform c, int format)
{
  fwrite_clause_jmap(fp, c, format, NULL);
}  /* fwrite_clause */

/*************
 *
 *   f_clause()
 *
 *************/

/* DOCUMENTATION
Write a clause to stdout, with id, with justification last.
*/

/* PUBLIC */
void f_clause(Topform c)
{
  fwrite_clause(stdout, c, CL_FORM_STD);
}  /* f_clause */

/*************
 *
 *   fwrite_clause_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a Clist of clauses in mixfix form.
Example:
<PRE>
list(sos).
a = b.
end_of_list.
</PRE>
If the name of the list is "", it is written as list(anonymous).
*/

/* PUBLIC */
void fwrite_clause_clist(FILE *fp, Clist lst, int format)
{
  Clist_pos p;
    
  if (lst->name == NULL || str_ident(lst->name, ""))
    fprintf(fp, "\nformulas(anonymous).\n");
  else
    fprintf(fp, "\nformulas(%s).\n", lst->name);
	  
  for (p = lst->first; p != NULL; p = p->next)
    fwrite_clause(fp, p->c, format);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fwrite_clause_clist */

/*************
 *
 *   fwrite_demod_clist()
 *
 *************/

/* DOCUMENTATION
Similar to fwirte_clause_clist, except that lex-dep demodulators are noted.
*/

/* PUBLIC */
void fwrite_demod_clist(FILE *fp, Clist lst, int format)
{
  Clist_pos p;
    
  if (lst->name == NULL || str_ident(lst->name, ""))
    fprintf(fp, "\nformulas(anonymous).\n");
  else
    fprintf(fp, "\nformulas(%s).\n", lst->name);
	  
  for (p = lst->first; p != NULL; p = p->next) {
    Topform c = p->c;
    fwrite_clause(fp, c, format);
    if (unit_clause(c->literals)&&
	pos_eq(c->literals) &&
	!oriented_eq(c->literals->atom)) {
      fprintf(fp, "        %% (lex-dep)\n");
    }
  }
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fwrite_demod_clist */

/*************
 *
 *   fwrite_clause_list()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a Plist of clauses in mixfix form.
Example:
<PRE>
list(sos).
a = b.
end_of_list.
</PRE>
If the name of the list is "", it is written as list(anonymous).
*/

/* PUBLIC */
void fwrite_clause_list(FILE *fp, Plist lst, char *name, int format)
{
  Plist p;

  if (name == NULL || str_ident(name, ""))
    fprintf(fp, "\nformulas(anonymous).\n");
  else
    fprintf(fp, "\nformulas(%s).\n", name);

  for (p = lst; p != NULL; p = p->next)
    fwrite_clause(fp, p->v, format);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fwrite_clause_list */

/*************
 *
 *   f_clauses()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void f_clauses(Plist p)
{
  fwrite_clause_list(stdout, p, "", CL_FORM_STD);
}  /* f_clauses */

/*************
 *
 *   read_formula()
 *
 *************/

/* DOCUMENTATION
This routine reads a formula from FILE *fin.
If there are no more formulas in *fin, NULL is returned.
<P>
If any error occurs, a message is sent to FILE *fout and a fatal_error occurs.
*/

/* PUBLIC */
Formula read_formula(FILE *fin, FILE *fout)
{
  Term t = read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Formula f = term_to_formula(t);
    zap_term(t);
    return f;
  }
}  /* read_formula */

/*************
 *
 *   end_of_list_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL end_of_list_formula(Formula f)
{
  if (f == NULL)
    return FALSE;
  else if (f->type != ATOM_FORM)
    return FALSE;
  else {
    Term a = f->atom;
    if (!CONSTANT(a))
      return FALSE;
    else
      return is_symbol(SYMNUM(a), "end_of_list", 0);
  }
}  /* end_of_list_formula */

/*************
 *
 *   read_formula_list()
 *
 *************/

/* DOCUMENTATION
This routine reads a list of formulas from FILE *fin.
If there are no more formulas in *fin, NULL is returned.
<P>
If any error occurs, a message is sent to FILE *fout and a fatal_error occurs.
*/

/* PUBLIC */
Plist read_formula_list(FILE *fin, FILE *fout)
{
  Plist p = NULL;
  Formula f;

  f = read_formula(fin, fout);
  while (f != NULL && !end_of_list_formula(f)) {
    p = plist_prepend(p, f);
    f = read_formula(fin, fout);
  }
  if (f != NULL)
    zap_formula(f);
  p = reverse_plist(p);
  return p;
}  /* read_formula_list */

/*************
 *
 *   fwrite_formula_list()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a list of formulas in mixfix form.
Example:
<PRE>
list(sos).
a = b.
end_of_list.
</PRE>
If the name of the list is "", it is written as list(anonymous).
*/

/* PUBLIC */
void fwrite_formula_list(FILE *fp, Plist lst, char *name)
{
  Plist p;

  if (name == NULL || str_ident(name, ""))
    fprintf(fp, "\nformulas(anonymous).\n");
  else
    fprintf(fp, "\nformulas(%s).\n", name);

  for (p = lst; p != NULL; p = p->next)
    fwrite_formula(fp, p->v);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fwrite_formula_list */

/*************
 *
 *   zap_formula_list()
 *
 *************/

/* DOCUMENTATION
Free a Plist of formulas.
*/

/* PUBLIC */
void zap_formula_list(Plist lst)
{
  Plist p = lst;
  while (p != NULL) {
    Plist p2 = p;
    p = p->next;
    zap_formula(p2->v);
    free_plist(p2);
  }
}  /* zap_formula_list */

/*************
 *
 *   end_of_list_term()
 *
 *************/

/* DOCUMENTATION
Check if a term is the constant "end_of_list".
*/

/* PUBLIC */
BOOL end_of_list_term(Term t)
{
  if (t == NULL)
    return FALSE;
  else if (!CONSTANT(t))
    return FALSE;
  else
    return is_symbol(SYMNUM(t), "end_of_list", 0);
}  /* end_of_list_term */

/*************
 *
 *   end_of_commands_term()
 *
 *************/

/* DOCUMENTATION
Check if a term is the constant "end_of_commands".
*/

/* PUBLIC */
BOOL end_of_commands_term(Term t)
{
  if (t == NULL)
    return FALSE;
  else if (!CONSTANT(t))
    return FALSE;
  else
    return is_symbol(SYMNUM(t), "end_of_commands", 0);
}  /* end_of_commands_term */

/*************
 *
 *   read_term_list()
 *
 *************/

/* DOCUMENTATION
This routine reads a list of terms from FILE *fin.
If there are no more terms in *fin, NULL is returned.
<P>
None of the returned terms or their subterms will have type VARIABLE,
even if it looks like a variable (e.g., x).
<P>
If any error occurs, a message is sent to FILE *fout and a fatal_error occurs.
*/

/* PUBLIC */
Plist read_term_list(FILE *fin, FILE *fout)
{
  Plist p = NULL;
  Term t;

  t = read_term(fin, fout);
  while (t != NULL && !end_of_list_term(t)) {
    p = plist_prepend(p, t);
    t = read_term(fin, fout);
  }
  if (t != NULL)
    zap_term(t);
  p = reverse_plist(p);
  return p;
}  /* read_term_list */

/*************
 *
 *   fwrite_term_list()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a list of terms in mixfix form.
Example:
<PRE>
list(sos).
a = b.
end_of_list.
</PRE>
If the name of the list is "" or NULL, it is written as list(anonymous).
*/

/* PUBLIC */
void fwrite_term_list(FILE *fp, Plist lst, char *name)
{
  Plist p;

  if (name == NULL || str_ident(name, ""))
    fprintf(fp, "\nlist(anonymous).\n");
  else
    fprintf(fp, "\nlist(%s).\n", name);

  for (p = lst; p != NULL; p = p->next) {
    fwrite_term(fp, p->v);
    fprintf(fp, ".\n");
  }
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fwrite_term_list */

/*************
 *
 *   term_reader()
 *
 *************/

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
A term is read from stdin (NULL if there are none).  
Errors are fatal.
*/

/* PUBLIC */
Term term_reader(BOOL fast)
{
  if (fast)
    return fast_read_term(stdin, stderr);
  else
    return read_term(stdin, stderr);
}  /* term_reader */

/*************
 *
 *   term_writer()
 *
 *************/

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
The term is written to stdout, with a period and newline.
*/

/* PUBLIC */
void term_writer(Term t, BOOL fast)
{
  if (fast)
    fast_fwrite_term_nl(stdout, t);
  else
    fwrite_term_nl(stdout, t);
  fflush(stdout);
}  /* term_writer */

/*************
 *
 *   clause_reader()
 *
 *************/

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
A clause is read from stdin (NULL if there are none).  
Errors are fatal.
*/

/* PUBLIC */
Topform clause_reader(BOOL fast)
{
  if (fast)
    return fast_read_clause(stdin, stderr);
  else
    return read_clause(stdin, stderr);
}  /* clause_reader */

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
The clause is written to stdout, with a period and newline.
*/

/* PUBLIC */
void clause_writer(Topform c, BOOL fast)
{
  if (fast)
    fast_fwrite_clause(stdout, c);
  else
    fwrite_clause(stdout, c, CL_FORM_BARE);
  fflush(stdout);
}  /* clause_writer */

/*************
 *
 *   term_to_topform2()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform term_to_topform2(Term t)
{
  Formula f = term_to_formula(t);
  Topform tf;
  if (clausal_formula(f)) {
    tf = formula_to_clause(f);
    tf->attributes = copy_attributes(f->attributes);
    zap_formula(f);
    clause_set_variables(tf, MAX_VARS);
  }
  else {
    tf = get_topform();
    tf->is_formula = TRUE;
    tf->attributes = f->attributes;
    f->attributes = NULL;
    tf->formula = universal_closure(f);
  }
  return tf;
}  /* term_to_topform2 */

/*************
 *
 *   read_clause_or_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform read_clause_or_formula(FILE *fin, FILE *fout)
{
  Term t = read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Topform tf = term_to_topform2(t);
    zap_term(t);
    return tf;
  }
}  /* read_clause_or_formula */

/*************
 *
 *   read_clause_or_formula_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist read_clause_or_formula_list(FILE *fin, FILE *fout)
{
  Plist lst = NULL;
  Topform tf = read_clause_or_formula(fin, fout);
  while (tf != NULL && !end_of_list_clause(tf)) {
    lst = plist_prepend(lst, tf);
    tf = read_clause_or_formula(fin, fout);
  }
  if (tf)
    zap_topform(tf);
  return reverse_plist(lst);
}  /* read_clause_or_formula_list */


/* ***************************************************************************
   BV(2007-aug-20):  new functions to support tagged proofs (prooftrans)
   **************************************************************************/

/*************
 *
 *   sb_tagged_write_clause_jmap()
 *
 *************/

/* DOCUMENTATION
This routine writes a clause (in mixfix form, with ".\n") to a String_buf.
*/

/* PUBLIC */
void sb_tagged_write_clause_jmap(String_buf sb, Topform c,
			  int format,
			  I3list map)
{
  Term t;
  if (c->compressed)
    t = NULL;
  else
    t = topform_to_term(c);
  
  if (c->id != 0) {

    /* BV(2007-jul-13) */
    sb_append(sb, "c ");
    
    sb_append_id(sb, c->id, map);
    sb_append(sb, "  ");
  }

  if (t == NULL)
    sb_append(sb, "clause_is_compressed");
  else
    sb_write_term(sb, t);

  sb_append(sb, "\n");

  sb_tagged_write_just(sb, c->justification, map);
  // sb_append(sb, " *** call to sb_tagged_write_just ***\n");
  sb_append(sb, "e\n");

  if (t)
    zap_term(t);
}  /* sb_tagged_write_clause_jmap */
