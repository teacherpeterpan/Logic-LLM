#include "clause.h"
#include "memory2.h"

/* Private definitions and types */

#define VAR_ARRAY_SIZE    100  /* for renumbering variables */

/*
 * memory management
 */

static unsigned Literal_gets, Literal_frees;
static unsigned Clause_gets, Clause_frees;

#define BYTES_LITERAL sizeof(struct literal)
#define PTRS_LITERAL BYTES_LITERAL%BPP == 0 ? BYTES_LITERAL/BPP : BYTES_LITERAL/BPP + 1

#define BYTES_CLAUSE sizeof(struct clause)
#define PTRS_CLAUSE BYTES_CLAUSE%BPP == 0 ? BYTES_CLAUSE/BPP : BYTES_CLAUSE/BPP + 1

/*************
 *
 *   Literal get_literal()
 *
 *************/

static
Literal get_literal(void)
{
  Literal p = get_mem(PTRS_LITERAL);
  Literal_gets++;
  return(p);
}  /* get_literal */

/*************
 *
 *    free_literal()
 *
 *************/

static
void free_literal(Literal p)
{
  free_mem(p, PTRS_LITERAL);
  Literal_frees++;
}  /* free_literal */

/*************
 *
 *   Clause get_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Clause get_clause(void)
{
  Clause p = get_mem(PTRS_CLAUSE);
  Clause_gets++;
  return(p);
}  /* get_clause */

/*************
 *
 *    free_clause()
 *
 *************/

static
void free_clause(Clause p)
{
  free_mem(p, PTRS_CLAUSE);
  Clause_frees++;
}  /* free_clause */

/*************
 *
 *   fprint_clause_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clause package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_clause_mem(FILE *fp, int heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_LITERAL;
  fprintf(fp, "literal (%4d)      %11u%11u%11u%9.1f K\n",
          n, Literal_gets, Literal_frees,
          Literal_gets - Literal_frees,
          ((Literal_gets - Literal_frees) * n) / 1024.);

  n = BYTES_CLAUSE;
  fprintf(fp, "clause (%4d)       %11u%11u%11u%9.1f K\n",
          n, Clause_gets, Clause_frees,
          Clause_gets - Clause_frees,
          ((Clause_gets - Clause_frees) * n) / 1024.);

}  /* fprint_clause_mem */

/*************
 *
 *   p_clause_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clause package.
*/

/* PUBLIC */
void p_clause_mem()
{
  fprint_clause_mem(stdout, 1);
}  /* p_clause_mem */

/*
 *  end of memory management
 */
/*************
 *
 *    zap_literal(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a list of literal.
*/

/* PUBLIC */
void zap_literal(Literal l)
{
  zap_term(l->atom);
  free_literal(l);
}  /* zap_clause */

/*************
 *
 *    zap_literals(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a list of literals.
*/

/* PUBLIC */
void zap_literals(Literal l)
{
  if (l) {
    zap_literals(l->next);
    zap_literal(l);
  }
}  /* zap_clause */

/*************
 *
 *    zap_clause(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a clause (but not any justification list or
attributes).  The caller should make sure that nothing (e.g., indexes)
refer to the clause or any of its subterms.
<P>
If the clause has a justification or an ID, use the higher-level
routine delete_clause(c) instead.
*/

/* PUBLIC */
void zap_clause(Clause c)
{
  zap_literals(c->literals);
  zap_attributes(c->attributes);
  free_clause(c);
}  /* zap_clause */

/*************
 *
 *   fprint_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints a clause to a file.
*/

/* PUBLIC */
void fprint_clause(FILE *fp, Clause c)
{
  Literal lit;

  if (c->id > 0)
    fprintf(fp, "%d: ", c->id);

  if (c->literals == NULL)
    fprintf(fp, "%s", FALSE_SYM);
  else {
    for (lit = c->literals; lit != NULL; lit = lit->next) {
      if (!lit->sign)
	fprintf(fp, "%s", NOT_SYM);
      fprint_term(fp, lit->atom);
#if 0
      if (maximal_literal_check(lit))
	fprintf(fp, "[max]");
#endif
      if (lit->next != NULL)
	fprintf(fp, " %s ", OR_SYM);
    }
  }
  fprintf(fp, ".\n");
  fflush(fp);
}  /* fprint_clause */

/*************
 *
 *   p_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints a clause to stdout.
*/

/* PUBLIC */
void p_clause(Clause c)
{
  fprint_clause(stdout, c);
}  /* p_clause */

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
Literal new_literal(int sign, Term atom)
{
  Literal lit = get_literal();
  lit->sign = sign;
  lit->atom = atom;
  return lit;
}  /* new_literal */

/*************
 *
 *   append_literal()
 *
 *************/

/* DOCUMENTATION
This routine appends a literal to a clause.
*/

/* PUBLIC */
void append_literal(Clause c, Literal lit)
{
  Literal l = c->literals;

  if (l == NULL) {
    c->literals = lit;
  }
  else {
    while (l->next != NULL)
      l = l->next;
    l->next = lit;
  }
  lit->next = NULL;
}  /* append_literal */

/*************
 *
 *   clause_length()
 *
 *************/

/* DOCUMENTATION
This routine returns the length of a clause, which is the sum of
the lengths of its atoms.
*/

/* PUBLIC */
int clause_length(Clause c)
{
  int n = 0;
  Literal lit;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    n += symbol_count(lit->atom);
  return n;
}  /* clause_length */

/*************
 *
 *   term_to_literals()
 *
 *************/

static
Literal term_to_literals(Term t, Literal lits)
{
  Literal l;

  if (COMPLEX(t) && is_term(t, OR_SYM, 2)) {
    /* Traverse term right-to-left and add to the
     * front of the clause, so order is preserved.
     */
    l = term_to_literals(t->args[1], lits);
    l = term_to_literals(t->args[0], l);
  }
  else {
    l = get_literal();
    l->next = lits;
    l->sign = !(COMPLEX(t) && is_term(t, NOT_SYM, 1));
    if (l->sign)
      l->atom = copy_term(t);
    else
      l->atom = copy_term(t->args[0]);
  }
  return(l);
}  /* term_to_literals */

/*************
 *
 *   term_to_clause()
 *
 *************/

/* DOCUMENTATION
This routine takes a Term t (presumably a disjunction with binary
symbol OR_SYM), and constructs a Clause.  The Clause is entirely new.
<P>
The main use of this routine is intended to be as follows: a
Term representing a clause is parsed (using mixfix notation)
from the input, then here it is copied translated into
a Clause data structure.
*/

/* PUBLIC */
Clause term_to_clause(Term t)
{
  Clause c = get_clause();
  Term t_start;

  if (COMPLEX(t) && is_term(t, ATTRIB_SYM, 2)) {
    c->attributes = term_to_attributes(ARG(t,1), ATTRIB_SYM);
    t_start = ARG(t,0);
  }
  else
    t_start = t;

  c->literals = term_to_literals(t_start, NULL);
  return(c);
}  /* term_to_clause */

/*************
 *
 *   literals_to_term()
 *
 *************/

static
Term literals_to_term(Literal l)
{
  Term t;
  if (l->sign)
    t = copy_term(l->atom);
  else {
    t = get_rigid_term(NOT_SYM, 1);
    t->args[0] = copy_term(l->atom);
  }
  if (l->next) {
    Term d = get_rigid_term(OR_SYM, 2);
    d->args[0] = t;
    d->args[1] = literals_to_term(l->next);
    return d;
  }
  else
    return t;
}  /* literals_to_term */

/*************
 *
 *   clause_to_term()
 *
 *************/

/* DOCUMENTATION
This routine takes a Clause and returns an entirely new Term
which represents the clause.  The disjunction symbol for the
term is binary OR_SYM, and the negation symbols is NOT_SYM.
*/

/* PUBLIC */
Term clause_to_term(Clause c)
{
  Term lits;

  if (c->literals == NULL)
    lits = get_rigid_term(FALSE_SYM, 0);
  else
    lits = literals_to_term(c->literals);

  if (c->attributes == NULL)
    return lits;
  else
    return build_binary_term(str_to_sn(ATTRIB_SYM, 2),
			     lits,
			     attributes_to_term(c->attributes, ATTRIB_SYM));
}  /* clause_to_term */

/*************
 *
 *   literals_to_term_x()
 *
 *************/

static
Term literals_to_term_x(Literal l)
{
  Term t;
  if (l->sign)
    t = l->atom;
  else {
    t = get_rigid_term(NOT_SYM, 1);
    t->args[0] = l->atom;
  }
  if (l->next) {
    Term d = get_rigid_term(OR_SYM, 2);
    d->args[0] = t;
    d->args[1] = literals_to_term_x(l->next);
    t = d;
  }
  free_literal(l);
  return t;
}  /* literals_to_term_x */

/*************
 *
 *   clause_to_term_x()
 *
 *************/

/* DOCUMENTATION
This routine takes a Clause and returns its Term form.
The disjunction symbol for the
term is binary OR_SYM, and the negation symbols is NOT_SYM.
<P>
This version destroys the clause.
*/

/* PUBLIC */
Term clause_to_term_x(Clause c)
{
  Term t;
  if (c->literals != NULL)
    t = literals_to_term_x(c->literals);
  else
    t = get_rigid_term(FALSE_SYM, 0);
  free_clause(c);  /* literals are already freed */
  return t;
}  /* clause_to_term_x */

/*************
 *
 *   clause_set_variables()
 *
 *************/

/* DOCUMENTATION
This routine traverses a clause and changes the constants
that should be variables, into variables.  On input, the clause
should have no variables.  The new variables are numbered
0, 1, 2 ... according the the first occurrence, reading from the
left.
<P>
A fatal error occurs if there are more than max_vars variables.
<P>
The intended is use is for input clauses that
are built without regard to variable/constant distinction.
*/

/* PUBLIC */
void clause_set_variables(Clause c, int max_vars)
{
  char *a[VAR_ARRAY_SIZE], **vmap;
  int i;
  Literal lit;

  if (max_vars > VAR_ARRAY_SIZE)
    vmap = malloc((max_vars * sizeof(char *)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    vmap[i] = NULL;

  for (lit = c->literals; lit != NULL; lit = lit->next) {
    Term a = lit->atom;
    for (i = 0; i < a->arity; i++) 
      a->args[i] = set_vars_recurse(a->args[i], vmap, max_vars);
  }

  /* Now do any answer literals (with the same vmap). */

  set_vars_attributes(c->attributes, vmap, max_vars);
  
  if (max_vars > VAR_ARRAY_SIZE)
    free(vmap);
}  /* clause_set_variables */

/*************
 *
 *   renumber_variables()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables of a clause.  The variables are
renumbered 0, 1, 2 ... according the the first occurrence, reading
from the left.
<P>
If there are more than max_vars distinct variables,
a fatal error occurs.
<P>
The intended is use is for inferred clauses that
may contain variable indexes greater than max_vars.
*/

/* PUBLIC */
void renumber_variables(Clause c, int max_vars)
{
  int a[VAR_ARRAY_SIZE], *vmap;
  int i;
  Literal lit;

  if (max_vars > VAR_ARRAY_SIZE)
    vmap = malloc((max_vars * sizeof(int)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    a[i] = -1;

  for (lit = c->literals; lit != NULL; lit = lit->next)
    lit->atom = renum_vars_recurse(lit->atom, vmap, max_vars);

  /* Now do any inheritable attributes (with the same vmap). */

  renumber_vars_attributes(c->attributes, vmap, max_vars);

  if (max_vars > VAR_ARRAY_SIZE)
    free(vmap);
}  /* renumber_variables */

/*************
 *
 *   term_renumber_variables()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables of a term.  The variables are
renumbered 0, 1, 2 ... according the the first occurrence, reading
from the left.
<P>
If there are more than max_vars distinct variables,
a fatal error occurs.
<P>
Do not use this to renumber variables of a clause (see renumber_variables).
*/

/* PUBLIC */
void term_renumber_variables(Term t, int max_vars)
{
  int a[VAR_ARRAY_SIZE], *vmap;
  int i;

  if (max_vars > VAR_ARRAY_SIZE)
    vmap = malloc((max_vars * sizeof(int)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    a[i] = -1;

  t = renum_vars_recurse(t, vmap, max_vars);
  
  if (max_vars > VAR_ARRAY_SIZE)
    free(vmap);
}  /* term_renumber_variables */

/*************
 *
 *   positive_literals()
 *
 *************/

/* DOCUMENTATION
This function returns the number of positive literals in a clause.
*/

/* PUBLIC */
int positive_literals(Clause c)
{
  Literal lit;
  int n = 0;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    if (lit->sign)
      n++;
  return n;
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
int negative_literals(Clause c)
{
  Literal lit;
  int n = 0;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    if (!lit->sign)
      n++;
  return n;
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
BOOL positive_clause(Clause c)
{
  return negative_literals(c) == 0;
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
BOOL any_clause(Clause c)
{
  return TRUE;
}  /* positive_clause */

/*************
 *
 *   negative_clause()
 *
 *************/

/* DOCUMENTATION
This function checks if all of the literals of a clause are negative.
*/

/* PUBLIC */
BOOL negative_clause(Clause c)
{
  return positive_literals(c) == 0;
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
BOOL mixed_clause(Clause c)
{
  return (positive_literals(c) >= 1 && negative_literals(c) >= 1);
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
int number_of_literals(Clause c)
{
  return positive_literals(c) + negative_literals(c);
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
BOOL unit_clause(Clause c)
{
  return c->literals != NULL && c->literals->next == NULL;
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
BOOL horn_clause(Clause c)
{
  return positive_literals(c) <= 1;
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
BOOL definite_clause(Clause c)
{
  return positive_literals(c) == 1;
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
int greatest_variable_in_clause(Clause c)
{
  Literal lit;
  int max, v;
    
  for (max = -1, lit = c->literals; lit != NULL; lit = lit->next) {
    v = greatest_variable(lit->atom);
    max = (v > max ? v : max);
  }
  return(max);
}  /* greatest_variable_in_clause */

/*************
 *
 *   upward_clause_links()
 *
 *************/

/* DOCUMENTATION
In the given Clause c, make the "container" field of each subterm
point to c.
*/

/* PUBLIC */
void upward_clause_links(Clause c)
{
  Literal lit;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    upward_term_links(lit->atom, c);
}  /* upward_clause_links */

/*************
 *
 *   copy_clause()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The container field of each nonvariable subterm points
to the clause.
*/

/* PUBLIC */
Clause copy_clause(Clause c)
{
  Literal l;
  Clause c2 = get_clause();

  for (l = c->literals; l != NULL; l = l->next) {
    Literal l2 = get_literal();
    l2->sign = l->sign;
    l2->atom = copy_term(l->atom);
    append_literal(c2, l2);
  }
  upward_clause_links(c2);
  return c2;
}  /* copy_clause */

/*************
 *
 *   copy_clause_with_flags()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
All termflags are copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Clause copy_clause_with_flags(Clause c)
{
  Literal l;
  Clause c2 = get_clause();

  for (l = c->literals; l != NULL; l = l->next) {
    Literal l2 = get_literal();
    l2->sign = l->sign;
    l2->atom = copy_term_with_flags(l->atom);
    append_literal(c2, l2);
  }
  upward_clause_links(c2);
  return c2;
}  /* copy_clause_with_flags */

/*************
 *
 *   copy_clause_with_flag()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The given termflag is copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Clause copy_clause_with_flag(Clause c, int flag)
{
  Literal l;
  Clause c2 = get_clause();

  for (l = c->literals; l != NULL; l = l->next) {
    Literal l2 = get_literal();
    l2->sign = l->sign;
    l2->atom = copy_term_with_flag(l->atom, flag);
    append_literal(c2, l2);
  }
  upward_clause_links(c2);
  return c2;
}  /* copy_clause_with_flag */

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
int literal_number(Clause c, Literal l)
{
  int n = 1;
  Literal lit = c->literals;

  while (lit != NULL && lit != l) {
    n++;
    lit = lit->next;
  }
  return (lit == NULL ? 0 : n);
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
int atom_number(Clause c, Term atom)
{
  int n = 1;
  Literal lit = c->literals;

  while (lit != NULL && lit->atom != atom) {
    n++;
    lit = lit->next;
  }
  return (lit == NULL ? 0 : n);
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
Literal ith_literal(Clause c, int i)
{
  Literal lit = c->literals;
  int n = 1;
  while (lit != NULL && n < i) {
    lit = lit->next;
    n++;
  }
  return lit;
}  /* ith_literal */

/*************
 *
 *   true_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL true_term(Term t)
{
  return is_term(t, TRUE_SYM, 0);
}  /* true_term */

/*************
 *
 *   false_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL false_term(Term t)
{
  return is_term(t, FALSE_SYM, 0);
}  /* false_term */

/*************
 *
 *   tautology()
 *
 *************/

/* DOCUMENTATION
This routine returns TRUE if the clause has complementary literals
or if it has any literals of the form $T, -$F.
*/

/* PUBLIC */
BOOL tautology(Clause c)
{
  Literal l1;
  for (l1 = c->literals; l1; l1 = l1->next) {
    BOOL sign = l1->sign;
    Term a = l1->atom;
    if (sign && true_term(a))
      return TRUE;
    else if (!sign && false_term(a))
      return TRUE;
    else {
      /* Check for a complementary occurring after this literal. */
      Literal l2;
      for (l2 = l1->next; l2; l2 = l2->next) {
	if (sign != l2->sign && term_ident(a, l2->atom))
	  return TRUE;
      }
    }
  }
  return FALSE;
}  /* tautology */

/*************
 *
 *   inherit_attributes()
 *
 *************/

/* DOCUMENTATION
This takes two parent clauses and their associated
substitutions, and a child clause.  All inheritable
attributes on the parents are instantiated and
appended to the child's attributes.
*/

/* PUBLIC */
void inherit_attributes(Clause parent1, Context subst1,
			Clause parent2, Context subst2,
			Clause child)
{
  Attribute a1 = inheritable_att_instances(parent1->attributes, subst1);
  Attribute a2 = inheritable_att_instances(parent2->attributes, subst2);
  child->attributes = cat_att(child->attributes, cat_att(a1, a2));
}  /* inherit_attributes */

/*************
 *
 *   function_symbols_in_clause()
 *
 *************/

/* DOCUMENTATION
Collect the multiset of function symbols in a clause.
*/

/* PUBLIC */
Ilist function_symbols_in_clause(Clause c, Ilist g)
{
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next) {
    int i;
    for (i = 0; i < ARITY(lit->atom); i++) {
      g = symbols_in_term(ARG(lit->atom,i), g);
    }
  }
  return g;
}  /* function_symbols_in_clause */

/*************
 *
 *   relation_symbols_in_clause()
 *
 *************/

/* DOCUMENTATION
Collect the multiset of relation symbols in a clause.
*/

/* PUBLIC */
Ilist relation_symbols_in_clause(Clause c, Ilist g)
{
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next)
    g = ilist_prepend(g, SYMNUM(lit->atom));
  return g;
}  /* relation_symbols_in_clause */

/*************
 *
 *   symbol_occurrences_in_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int symbol_occurrences_in_clause(Clause c, int symnum)
{
  int n = 0;
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next)
    n += symbol_occurrences(lit->atom, symnum);
  return n;
}  /* symbol_occurrences_in_clause */

/*************
 *
 *   remove_null_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literal remove_null_literals(Literal l)
{
  if (l == NULL)
    return NULL;
  else {
    l->next = remove_null_literals(l->next);
    if (l->atom != NULL)
      return l;
    else {
      Literal m = l->next;
      free_literal(l);
      return m;
    }
  }
}  /* remove_null_literals */

/*************
 *
 *   new_atom_nodes()
 *
 *************/

/* DOCUMENTATION
Replace the atom nodes in a clause.  The new atom
nodes have the property that they are greater in the
FPA ordering than any other terms currently in use.
*/

/* PUBLIC */
void new_atom_nodes(Clause c)
{
  Literal l;
  for (l = c->literals; l; l = l->next)
    l->atom = new_term_top(l->atom);
}  /* new_atom_nodes */

/*************
 *
 *   new_atoms()
 *
 *************/

/* DOCUMENTATION
Replace the atoms and all subterms in a clause.  The new terms
have the property that they are greater in the
FPA ordering than any other terms currently in use.
Also, all of the new subterms are contiguous in memory.
*/

/* PUBLIC */
void new_atoms(Clause c)
{
  Literal l;
  for (l = c->literals; l; l = l->next)
    l->atom = entirely_new_term(l->atom);
}  /* new_atoms */

/*************
 *
 *   first_negative_literal()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literal first_negative_literal(Clause c)
{
  Literal lit = c->literals;
  while (lit && lit->sign)
    lit = lit->next;
  return lit;
}  /* first_negative_literal */

/*************
 *
 *   constants_in_clause()
 *
 *************/

/* DOCUMENTATION
Given a clause, return the set of constants therein.
*/

/* PUBLIC */
Plist constants_in_clause(Clause c)
{
  Plist p = NULL;
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next)
    p = constants_in_term(lit->atom, p);
  return p;
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
BOOL clause_ident(Clause c1, Clause c2)
{
  Literal l1, l2;

  for (l1 = c1->literals, l2 = c2->literals;
       l1 && l2;
       l1 = l1->next, l2 = l2->next) {
    if (l1->sign != l2->sign || !term_ident(l1->atom, l2->atom)) {
      return FALSE;
    }
  }
  return (l1 == NULL && l2 == NULL);
}  /* clause_ident */

