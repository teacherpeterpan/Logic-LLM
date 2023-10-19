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

#include "literals.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_LITERALS PTRS(sizeof(struct literals))
static unsigned Literals_gets, Literals_frees;

/*************
 *
 *   Literals get_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals get_literals(void)
{
  Literals p = get_cmem(PTRS_LITERALS);
  Literals_gets++;
  return(p);
}  /* get_literals */

/*************
 *
 *    free_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_literals(Literals p)
{
  free_mem(p, PTRS_LITERALS);
  Literals_frees++;
}  /* free_literals */

/*************
 *
 *   fprint_literals_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clause package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_literals_mem(FILE *fp, int heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct literals);
  fprintf(fp, "literals (%4d)      %11u%11u%11u%9.1f K\n",
          n, Literals_gets, Literals_frees,
          Literals_gets - Literals_frees,
          ((Literals_gets - Literals_frees) * n) / 1024.);

}  /* fprint_literals_mem */

/*************
 *
 *   p_literals_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clause package.
*/

/* PUBLIC */
void p_literals_mem()
{
  fprint_literals_mem(stdout, 1);
}  /* p_literals_mem */

/*
 *  end of memory management
 */

/*************
 *
 *    zap_literal(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a literal.
*/

/* PUBLIC */
void zap_literal(Literals l)
{
  zap_term(l->atom);
  free_literals(l);
}  /* zap_literal */

/*************
 *
 *    zap_literals(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a list of literals.
*/

/* PUBLIC */
void zap_literals(Literals l)
{
  if (l) {
    zap_literals(l->next);
    zap_literal(l);
  }
}  /* zap_literals */

/*************
 *
 *   new_literal()
 *
 *************/

/* DOCUMENTATION
This routine takes a sign (Boolean) and a Term atom, and returns
a literal.  The atom is not copied.
*/

/* PUBLIC */
Literals new_literal(int sign, Term atom)
{
  Literals lit = get_literals();
  lit->sign = sign;
  lit->atom = atom;
  return lit;
}  /* new_literal */

/*************
 *
 *   copy_literal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals copy_literal(Literals lit)
{
  return new_literal(lit->sign, copy_term(lit->atom));
}  /* copy_literal */

/*************
 *
 *   append_literal()
 *
 *************/

/* DOCUMENTATION
This routine appends a literal to a list of literals.
*/

/* PUBLIC */
Literals append_literal(Literals lits, Literals lit)
{
  if (lits == NULL)
    return lit;
  else {
    lits->next = append_literal(lits->next, lit);
    return lits;
  }
}  /* append_literal */

/*************
 *
 *   term_to_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals term_to_literals(Term t, Literals lits)
{
  Literals l;

  if (is_term(t, false_sym(), 0))
    return lits;  /* translates to nothing */
  else if (is_term(t, or_sym(), 2)) {
    /* Traverse term right-to-left and add to the
     * front of the clause, so order is preserved.
     */
    l = term_to_literals(ARG(t,1), lits);
    l = term_to_literals(ARG(t,0), l);
  }
  else {
    l = get_literals();
    l->next = lits;
    l->sign = !(COMPLEX(t) && is_term(t, not_sym(), 1));
    if (l->sign)
      l->atom = copy_term(t);
    else
      l->atom = copy_term(ARG(t,0));
  }
  return(l);
}  /* term_to_literals */

/*************
 *
 *   literal_to_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term literal_to_term(Literals l)
{
  Term t;
  if (l->sign)
    t = copy_term(l->atom);
  else {
    t = get_rigid_term(not_sym(), 1);
    ARG(t,0) = copy_term(l->atom);
  }
  return t;
}  /* literal_to_term */

/*************
 *
 *   literals_to_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term literals_to_term(Literals l)
{
  Term t = literal_to_term(l);
  if (l->next) {
    Term d = get_rigid_term(or_sym(), 2);
    ARG(d,0) = t;
    ARG(d,1) = literals_to_term(l->next);
    return d;
  }
  else
    return t;
}  /* literals_to_term */

/*************
 *
 *   lits_to_term() -- do not copy atoms!
 *
 *************/

/* DOCUMENTATION
This routine converts a nonempty list of literals into a term.
This version does not copy atoms; it constructs new term
nodes only for the NOT and OR structure at the top of the clause.
Use free_lits_to_term() to free terms constructed with this routine.
*/

/* PUBLIC */
Term lits_to_term(Literals l)
{
  Term t;

  if (l->sign)
    t = l->atom;
  else {
    t = get_rigid_term_dangerously(not_symnum(), 1);
    ARG(t,0) = l->atom;
  }
  if (l->next) {
    Term d = get_rigid_term_dangerously(or_symnum(), 2);
    ARG(d,0) = t;
    ARG(d,1) = lits_to_term(l->next);
    t = d;
  }
  return t;
}  /* lits_to_term */

/*************
 *
 *   free_lits_to_term() -- do not free atoms!
 *
 *************/

/* DOCUMENTATION
This routine is to be used with terms constructed by lits_to_term().
*/

/* PUBLIC */
void free_lits_to_term(Term t)
{
  if (SYMNUM(t) == not_symnum())
    free_term(t);
  else if (SYMNUM(t) == or_symnum()) {
    free_lits_to_term(ARG(t,0));
    free_lits_to_term(ARG(t,1));
    free_term(t);
  }
}  /* free_lits_to_term */

/*************
 *
 *   positive_literals()
 *
 *************/

/* DOCUMENTATION
This function returns the number of positive literals in a clause.
*/

/* PUBLIC */
int positive_literals(Literals lits)
{
  if (lits == NULL)
    return 0;
  else if (lits->sign)
    return 1 + positive_literals(lits->next);
  else
    return positive_literals(lits->next);
}  /* positive_literals */

/*************
 *
 *   negative_literals()
 *
 *************/

/* DOCUMENTATION
This function returns the number of negative literals in a clause.
*/

/* PUBLIC */
int negative_literals(Literals lits)
{
  if (lits == NULL)
    return 0;
  else if (!lits->sign)
    return 1 + negative_literals(lits->next);
  else
    return negative_literals(lits->next);
}  /* negative_literals */

/*************
 *
 *   positive_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if all of the literals of a clause are positive.
*/

/* PUBLIC */
BOOL positive_clause(Literals lits)
{
  return negative_literals(lits) == 0;
}  /* positive_clause */

/*************
 *
 *   any_clause()
 *
 *************/

/* DOCUMENTATION
This function is always TRUE.  (It it intended to be used as an argument.)
*/

/* PUBLIC */
BOOL any_clause(Literals lits)
{
  return TRUE;
}  /* any_clause */

/*************
 *
 *   negative_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if all of the literals of a clause are negative.
*/

/* PUBLIC */
BOOL negative_clause(Literals lits)
{
  return positive_literals(lits) == 0;
}  /* negative_clause */

/*************
 *
 *   mixed_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause has at least one positive and
at least one negative literal.
*/

/* PUBLIC */
BOOL mixed_clause(Literals lits)
{
  return (positive_literals(lits) >= 1 &&
	  negative_literals(lits) >= 1);
}  /* mixed_clause */

/*************
 *
 *   number_of_literals()
 *
 *************/

/* DOCUMENTATION
This function returns the number of literals in a clause.
*/

/* PUBLIC */
int number_of_literals(Literals lits)
{
  if (lits == NULL)
    return 0;
  else
    return 1 + number_of_literals(lits->next);
}  /* number_of_literals */

/*************
 *
 *   unit_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause has exactly one literal.
*/

/* PUBLIC */
BOOL unit_clause(Literals lits)
{
  return number_of_literals(lits) == 1;
}  /* unit_clause */

/*************
 *
 *   horn_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause has at most one positive literal.   
*/

/* PUBLIC */
BOOL horn_clause(Literals lits)
{
  return positive_literals(lits) <= 1;
}  /* horn_clause */

/*************
 *
 *   definite_clause()
 *
 *************/

/* DOCUMENTATION
This Boolean function checks if a clause has exactly one positive literal.   
*/

/* PUBLIC */
BOOL definite_clause(Literals lits)
{
  return positive_literals(lits) == 1;
}  /* definite_clause */

/*************
 *
 *   greatest_variable_in_clause(c)
 *
 *************/

/* DOCUMENTATION
This routine returns the greatest variable index in a clause.
If the clause is ground, -1 is returned.
*/

/* PUBLIC */
int greatest_variable_in_clause(Literals lits)
{
  if (lits == NULL)
    return -1;
  else {
    int max_this = greatest_variable(lits->atom);
    int max_rest = greatest_variable_in_clause(lits->next);
    return IMAX(max_this, max_rest);
  }
}  /* greatest_variable_in_clause */

/*************
 *
 *   vars_in_clause(c)
 *
 *************/

/* DOCUMENTATION
This routine returns the set of variables (as a Plist) in a clause.
*/

/* PUBLIC */
Plist vars_in_clause(Literals lits)
{
  if (lits == NULL)
    return NULL;
  else
    return set_of_vars(lits->atom, vars_in_clause(lits->next));
}  /* vars_in_clause */

/*************
 *
 *   varnums_in_clause(c)
 *
 *************/

/* DOCUMENTATION
This routine returns the set of variable indexes (as an Ilist) in a clause.
*/

/* PUBLIC */
Ilist varnums_in_clause(Literals lits)
{
  Plist vars = vars_in_clause(lits);
  Ilist varnums = NULL;
  Plist p;
  for (p = vars; p; p = p->next) {
    Term var = p->v;
    varnums = ilist_append(varnums, VARNUM(var));
  }
  zap_plist(vars);
  return varnums;
}  /* varnums_in_clause */

/*************
 *
 *   number_of_variables(c)
 *
 *************/

/* DOCUMENTATION
This routine returns number of (distinct) variables in a clause.
*/

/* PUBLIC */
int number_of_variables(Literals lits)
{
  Plist vars = vars_in_clause(lits);
  int n = plist_count(vars);
  zap_plist(vars);
  return n;
}  /* number_of_variables */

/*************
 *
 *   ground_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL ground_clause(Literals lits)
{
  return greatest_variable_in_clause(lits) == -1;
}  /* ground_clause */

/*************
 *
 *   copy_literals()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The container field of each nonvariable subterm points
to the clause.
*/

/* PUBLIC */
Literals copy_literals(Literals lits)
{
  if (lits == NULL)
    return NULL;
  else {
    Literals new = get_literals();
    new->sign = lits->sign;
    new->atom = copy_term(lits->atom);
    new->next = copy_literals(lits->next);
    return new;
  }
}  /* copy_literals */

/*************
 *
 *   copy_literals_with_flags()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
All termflags are copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Literals copy_literals_with_flags(Literals lits)
{
  if (lits == NULL)
    return NULL;
  else {
    Literals new = get_literals();
    new->sign = lits->sign;
    new->atom = copy_term_with_flags(lits->atom);
    new->next = copy_literals(lits->next);
    return new;
  }
}  /* copy_literals_with_flags */

/*************
 *
 *   copy_literals_with_flag()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The given termflag is copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Literals copy_literals_with_flag(Literals lits, int flag)
{
  if (lits == NULL)
    return NULL;
  else {
    Literals new = get_literals();
    new->sign = lits->sign;
    new->atom = copy_term_with_flag(lits->atom, flag);
    new->next = copy_literals(lits->next);
    return new;
  }
}  /* copy_literals_with_flag */

/*************
 *
 *   literal_number()
 *
 *************/

/* DOCUMENTATION
Given a clause and a literal, return the position of the literal
(counting from 1) in the clause.  The check is by pointer only.
If the literal does not occur in the clause, 0 is returned.
*/

/* PUBLIC */
int literal_number(Literals lits, Literals lit)
{
  if (lits == NULL)
    return 0;
  else if (lits == lit)
    return 1;
  else {
    int n = literal_number(lits->next, lit);
    return n == 0 ? 0 : n+1;
  }
}  /* literal_number */

/*************
 *
 *   atom_number()
 *
 *************/

/* DOCUMENTATION
Given a clause and an atom, return the position of the atom
(counting from 1) in the clause.  The check is by pointer only.
If the atom does not occur in the clause, 0 is returned.
*/

/* PUBLIC */
int atom_number(Literals lits, Term atom)
{
  if (lits == NULL)
    return 0;
  else if (lits->atom == atom)
    return 1;
  else {
    int n = atom_number(lits->next, atom);
    return n == 0 ? 0 : n+1;
  }
}  /* atom_number */

/*************
 *
 *   ith_literal()
 *
 *************/

/* DOCUMENTATION
Return the i-th literal of a clause, counting from 1.
Return NULL if i is out of range.
*/

/* PUBLIC */
Literals ith_literal(Literals lits, int i)
{
  if (lits == NULL)
    return NULL;
  else if (i == 1)
    return lits;
  else
    return ith_literal(lits->next, i-1);
}  /* ith_literal */

/*************
 *
 *   true_clause()
 *
 *************/

/* DOCUMENTATION
Does the clause contain a literal $T?
(This does not check for complementary literals, -$F, or x=x.)
*/

/* PUBLIC */
BOOL true_clause(Literals lits)
{
  if (lits == NULL)
    return FALSE;
  else if (lits->sign && true_term(lits->atom))
    return TRUE;
  else
    return true_clause(lits->next);
}  /* true_clause */

/*************
 *
 *   complementary_scan()
 *
 *************/

static
BOOL complementary_scan(Literals lits, Literals lit)
{
  if (lits == NULL)
    return FALSE;
  else if (lits->sign != lit->sign && term_ident(lits->atom, lit->atom))
    return TRUE;
  else
    return complementary_scan(lits->next, lit);
}  /* complementary_scan */

/*************
 *
 *   tautology()
 *
 *************/

/* DOCUMENTATION
This routine returns TRUE if the clause has complementary literals
or if it has any literals of the form $T, -$F.
This dos not check for x=x.
*/

/* PUBLIC */
BOOL tautology(Literals lits)
{
  if (lits == NULL)
    return FALSE;
  else if (lits->sign && true_term(lits->atom))
    return TRUE;
  else if (!lits->sign && false_term(lits->atom))
    return TRUE;
  else if (complementary_scan(lits->next, lits))
    return TRUE;
  else
    return tautology(lits->next);
}  /* tautology */

/*************
 *
 *   symbol_occurrences_in_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int symbol_occurrences_in_clause(Literals lits, int symnum)
{
  if (lits == NULL)
    return 0;
  else
    return
      symbol_occurrences(lits->atom, symnum) +
      symbol_occurrences_in_clause(lits->next, symnum);
}  /* symbol_occurrences_in_clause */

/*************
 *
 *   remove_null_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals remove_null_literals(Literals l)
{
  if (l == NULL)
    return NULL;
  else {
    l->next = remove_null_literals(l->next);
    if (l->atom != NULL)
      return l;
    else {
      Literals m = l->next;
      free_literals(l);
      return m;
    }
  }
}  /* remove_null_literals */

/*************
 *
 *   first_literal_of_sign()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals first_literal_of_sign(Literals lits, BOOL sign)
{
  if (lits == NULL)
    return NULL;
  else if (lits->sign == sign)
    return lits;
  else
    return first_literal_of_sign(lits->next, sign);
}  /* first_literal_of_sign */

/*************
 *
 *   constants_in_clause()
 *
 *************/

/* DOCUMENTATION
Given a clause, return the set of symnums for constants therein.
*/

/* PUBLIC */
Ilist constants_in_clause(Literals lits)
{
  if (lits == NULL)
    return NULL;
  else {
    Ilist p = constants_in_clause(lits->next);
    return constants_in_term(lits->atom, p);
  }
}  /* constants_in_clause */

/*************
 *
 *   clause_ident()
 *
 *************/

/* DOCUMENTATION
Identical clauses, including order of literals and variable numbering.
*/

/* PUBLIC */
BOOL clause_ident(Literals lits1, Literals lits2)
{
  if (lits1 == NULL)
    return lits2 == NULL;
  else if (lits2 == NULL)
    return FALSE;
  else if (lits1->sign != lits2->sign)
    return FALSE;
  else if (!term_ident(lits1->atom, lits2->atom))
    return FALSE;
  else
    return clause_ident(lits1->next, lits2->next);
}  /* clause_ident */

/*************
 *
 *   clause_symbol_count()
 *
 *************/

/* DOCUMENTATION
Disjunction and negation signs are not included in the count.
*/

/* PUBLIC */
int clause_symbol_count(Literals lits)
{
  if (lits == NULL)
    return 0;
  else
    return symbol_count(lits->atom) + clause_symbol_count(lits->next);
}  /* clause_symbol_count */

/*************
 *
 *   clause_depth()
 *
 *************/

/* DOCUMENTATION
Disjunction and negation signs are not included in the count.
That is, return the depth of the deepest atomic formula.
*/

/* PUBLIC */
int clause_depth(Literals lits)
{
  if (lits == NULL)
    return 0;
  else {
    int depth_this = term_depth(lits->atom);
    int depth_rest = clause_depth(lits->next);
    return IMAX(depth_this, depth_rest);
  }
}  /* clause_depth */

/*************
 *
 *   pos_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if a literal is a positive equality
for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
BOOL pos_eq(Literals lit)
{
  return lit->sign && eq_term(lit->atom);
}  /* pos_eq */

/*************
 *
 *   neg_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if a literal is a positive equality
for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
BOOL neg_eq(Literals lit)
{
  return lit->sign == FALSE && eq_term(lit->atom);
}  /* neg_eq */

/*************
 *
 *   pos_eq_unit()
 *
 *************/

/* DOCUMENTATION
This function checks if a list of Literals is a positive equality unit
for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
BOOL pos_eq_unit(Literals lits)
{
  return (unit_clause(lits) &&
	  lits->sign &&
	  eq_term(lits->atom));
}  /* pos_eq_unit */

/*************
 *
 *   neg_eq_unit()
 *
 *************/

/* DOCUMENTATION
This function checks if a list of Literals is a negative equality unit.
*/

/* PUBLIC */
BOOL neg_eq_unit(Literals lits)
{
  return (unit_clause(lits) &&
	  !lits->sign &&
	  eq_term(lits->atom));
}  /* neg_eq_unit */

/*************
 *
 *   contains_pos_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause contains a positive equality
literal for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
BOOL contains_pos_eq(Literals lits)
{
  if (lits == NULL)
    return FALSE;
  else if (pos_eq(lits))
    return TRUE;
  else
    return contains_pos_eq(lits->next);
}  /* contains_pos_eq */

/*************
 *
 *   contains_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause contains an equality
literal (positive or negative) for the purposes of
paramodulation and demodulation.
*/

/* PUBLIC */
BOOL contains_eq(Literals lits)
{
  if (lits == NULL)
    return FALSE;
  else if (eq_term(lits->atom))
    return TRUE;
  else
    return contains_eq(lits->next);
}  /* contains_eq */

/*************
 *
 *   only_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if a clause contains only equality
literals (positive or negative).
*/

/* PUBLIC */
BOOL only_eq(Literals lits)
{
  if (lits == NULL)
    return TRUE;
  else if (!eq_term(lits->atom))
    return FALSE;
  else
    return only_eq(lits->next);
}  /* only_eq */

/*************
 *
 *   literals_depth()
 *
 *************/

/* DOCUMENTATION
This function returns the maximum depth of a list of literals.
Negation signs are not counted, and P(a) has depth 1.
*/

/* PUBLIC */
int literals_depth(Literals lits)
{
  if (lits == NULL)
    return 0;
  else {
    int m = literals_depth(lits->next);
    int n = term_depth(lits->atom);
    return IMAX(m, n);
  }
}  /* literals_depth */

/*************
 *
 *   term_at_position()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term term_at_position(Literals lits, Ilist pos)
{
  if (lits == NULL || pos == NULL)
    return NULL;
  else {
    Literals lit = ith_literal(lits, pos->i);
    Term t = term_at_pos(lit->atom, pos->next);
    return t;
  }
}  /* term_at_position */

/*************
 *
 *   pos_predicates()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist pos_predicates(Ilist p, Literals lits)
{
  Literals l;
  for (l = lits; l; l = l->next) {
    if (l->sign && ! ilist_member(p, SYMNUM(l->atom)))
      p = ilist_prepend(p, SYMNUM(l->atom));
  }
  return p;
}  /* pos_predicates */

