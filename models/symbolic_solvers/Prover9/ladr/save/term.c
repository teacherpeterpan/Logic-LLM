#include "term.h"

/*
  Sharing Variables.  In the original design, no term sharing occurred.
  This allows compact data structures and simpler algorithms for
  processiong terms.  Then we tried sharing variables only (using the
  same data structures).

  In most cases of practical work, nearly all of the leaves in the
  clause spaces are variables (as opposed to constants).  Assuming most
  function applications are binary, this means that variables take up
  about half of the term storage.  We can save all of this by sharing
  variables.

  All terms have a containment field, which points to the containing
  clause.  (If we wished to have terms point to immediate superterms, we
  could do that instead.)  We need containment, because indexing returns
  terms, and we have to get from those terms to the containing clauses.
  BUT, we don't index variables, so we can do without containment
  pointers in variables.

  Making the change to shared variables was easy.  We introduced
  an array of term pointers, static Term Shared_variables[MAX_VNUM], and
  changed get_variable_term to return one of those instead of
  a fresh one.  Also, we eliminated the routine set_variable, which
  changes the variable number of a variable term.  If you want to
  do that now (with shared or with nonshared variables), just free
  the old one and get a new one.

  With this code, you can switch back and forth with conditional
  compilation: #define SHARE_VARIABLES.

  Things to be careful about:

  (1) Don't change variable numbers of variable terms.  Just use
  get_variable_term and free_term.
  (2) Don't use the container field of variables.

  If you observe those rules, I think everything will be okay; in
  particular, you should get the same results with and without
  SHARE_VARIABLES, except for memory usage.

  January 29, 2003.
*/

/* Private definitions and types */

#define SHARE_VARIABLES

#ifdef SHARE_VARIABLES
static Term Shared_variables[MAX_VNUM];
#endif

#ifdef TERM_ID_FIELD
static int Term_id_count = 0;
#endif

/*
 * memory management
 */

static unsigned Term_gets, Term_frees;

#define BYTES_TERM sizeof(struct term)
#define PTRS_TERM BYTES_TERM%BPP == 0 ? BYTES_TERM/BPP : BYTES_TERM/BPP + 1

/*************
 *
 *   Term get_term(arity)
 *
 *************/

static
Term get_term(int arity)
{
  Term p = get_mem(PTRS_TERM);
  p->args = get_mem(arity);
  p->arity = arity;
#ifdef TERM_ID_FIELD
  p->id = ++Term_id_count;
#endif
  Term_gets++;
  return(p);
}  /* get_term */

/*************
 *
 *    free_term()
 *
 *************/

/* DOCUMENTATION
This routine frees a term node only.  If you wish to recursively
free all of the subterms as well, call zap_term(t) instead.
*/

/* PUBLIC */
void free_term(Term p)
{
#ifdef SHARE_VARIABLES
  if (VARIABLE(p))
    return;  /* variables are never freed, because they are shared */
#endif
  free_mem(p->args, p->arity);
  free_mem(p, PTRS_TERM);
  Term_frees++;
}  /* free_term */

/*************
 *
 *   fprint_term_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for Terms.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_term_mem(FILE *fp, BOOL heading)
{
  int n;
  
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");
  
  n = sizeof(struct term);
  fprintf(fp, "term (%4d)         %11u%11u%11u%9.1f K\n",
	  n, Term_gets, Term_frees, Term_gets - Term_frees,
	  ((Term_gets - Term_frees) * n) / 1024.);
  
  /* end of printing for each type */
  
}  /* fprint_term_mem */

/*************
 *
 *   p_term_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints memory usage statistics for Terms to stdout.
*/

/* PUBLIC */
void p_term_mem(void)
{
  fprint_term_mem(stdout, 1);
}  /* p__mem */

/*
 *  end of memory management
 */

/*************
 *
 *   get_variable_term()
 *
 *************/

/* DOCUMENTATION
This routine returns a term of type VARIABLE.
The index of the variable is set to var_num, which  should
be an integer >= 0.
*/

/* PUBLIC */
Term get_variable_term(int var_num)
{
#ifdef SHARE_VARIABLES
  if (var_num < 0 || var_num > MAX_VNUM)
    fatal_error("get_variable_term: var_num too big");

  if (Shared_variables[var_num] == NULL) {
    Term t = get_term(0);
    t->private_symbol = var_num;
    Shared_variables[var_num] = t;
  }
  return Shared_variables[var_num];
#else
  Term t = get_term(0);

  if (var_num < 0 || var_num > MAX_VAR)
    fatal_error("get_variable_term,  var_num out of range.");
  t->private_symbol = var_num;
  return t;
#endif
}  /* get_variable_term */

/*************
 *
 *   zap_variables()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_variables(void)
{
#ifdef SHARE_VARIABLES
  int i;
  for (i = 0; i < MAX_VNUM; i++) {
    Term t = Shared_variables[i];
    if (t) {
      /* Trick free_term into really freeing it by making */
      /* it into a constant. */
      t->private_symbol = -1;
      free_term(t);
      Shared_variables[i] = NULL;
    }
  }
#endif
}  /* zap_variables */

/*************
 *
 *   get_rigid_term_like()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns a term node with the same
symbol and arity as the given Term t.
*/

/* PUBLIC */
Term get_rigid_term_like(Term t)
{
  Term t1 = get_term(t->arity);
  t1->private_symbol = t->private_symbol;
  return t1;
}  /* get_rigid_term_like */

/*************
 *
 *   get_rigid_term()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns a term node with the given
symbol and arity.  If you already have a similar term node, say t,
(containing the symbol and arity you need) call get_rigid_term_like(t)
instead.
*/

/* PUBLIC */
Term get_rigid_term(char *sym, int arity)
{
  Term t1 = get_term(arity);
  t1->private_symbol = -str_to_sn(sym, arity);
  return t1;
}  /* get_rigid_term */

/*************
 *
 *   get_rigid_term_dangerously()
 *
 *************/

/* DOCUMENTATION
This routine can be used to allocate a term node if all you have is
the symbol ID and arity.  <I>If the arity is not correct
for the symbol ID, terrible things will happen!</I> 
<P>
If you have a similar term, use get_rigid_term_like() instead.
If you can afford the time to access the symbol table,
use sn_to_str() and get_rigid_term() instead.
*/

/* PUBLIC */
Term get_rigid_term_dangerously(int symnum, int arity)
{
  Term t1 = get_term(arity);
  t1->private_symbol = -symnum;
  return t1;
}  /* get_rigid_term_dangerously */

/*************
 *
 *    zap_term(term)
 *
 *    Free a term and all of its subterms.
 *
 *************/

/* DOCUMENTATION
This routine frees a term t and all of its subterms.  You should not
refer to t after calling zap_term(t).
*/

/* PUBLIC */
void zap_term(Term t)
{
  int i;
  for (i = 0; i < t->arity; i++)
    zap_term(t->args[i]);
  free_term(t);
}  /* zap_term */

/*************
 *
 *    int term_ident(term1, term2) -- Compare two terms.
 *
 *    If identical return 1); else return 0.  The bits
 *    field is not checked.
 *
 *************/

/* DOCUMENTATION
This function checks if two terms are identical.  Only the 
structure and symbols are checked---any extra fields such as
bits or u are NOT checked.
*/

/* PUBLIC */
BOOL term_ident(Term t1, Term t2)
{
  if (t1->private_symbol != t2->private_symbol)
    return 0;
  else {
    int i;
    for (i = 0; i < t1->arity; i++)
      if (!term_ident(t1->args[i], t2->args[i]))
	return 0;
    return 1;
  }
}  /* term_ident */  

/*************
 *
 *    Term copy_term(term) -- Return a copy of the term.
 *
 *    The bits field is not copied.
 *
 *************/

/* DOCUMENTATION
This routine copies a term.  Only the symbols and structure
are copied---any extra fields such as bits or u are
NOT copied.
*/

/* PUBLIC */
Term copy_term(Term t)
{
  if (VARIABLE(t))
    return get_variable_term(VARNUM(t));
  else {
    int i;
    Term t2 = get_rigid_term_like(t);
    for (i = 0; i < t->arity; i++)
      t2->args[i] = copy_term(t->args[i]);
    return t2;
  }
}  /* copy_term */

/*************
 *
 *    int ground_term(t) -- is a term ground?
 *
 *************/

/* DOCUMENTATION
This function checks if a term is ground, that is, has no variables.
*/

/* PUBLIC */
BOOL ground_term(Term t)
{
  if (VARIABLE(t))
    return FALSE;
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      if (!ground_term(t->args[i]))
	return FALSE;
    return TRUE;
  }
}  /* ground_term */

/*************
 *
 *   biggest_variable()
 *
 *************/

/* DOCUMENTATION
This routine returns the greatest variable index of any variable int
the given term t.  If t is ground, -1 is returned.
*/

/* PUBLIC */
int biggest_variable(Term t)
{
  if (VARIABLE(t))
    return VARNUM(t);
  else {
    int i, max, v;
    for (max = -1, i = 0; i < t->arity; i++) {
      v = biggest_variable(t->args[i]);
      max = (v > max ? v : max);
    }
    return max;
  }
}  /* biggest_variable */

/*************
 *
 *    symbol_count
 *
 *************/

/* DOCUMENTATION
This routine returns the total number of symbols (i.e., the number of
nodes) in the given term t.
*/

/* PUBLIC */
int symbol_count(Term t)
{
  int i;
  int count = 0;
  for (i = 0; i < ARITY(t); i++)
    count += symbol_count(ARG(t,i));
  return count+1;
}  /* symbol_count  */

/*************
 *
 *     int occurs_in(t1, t2) -- Does t1 occur in t2 (including t1==t2)?
 *
 *     term_ident is used to check identity.
 *
 *************/

/* DOCUMENTATION
This function checks if Term t2 is identical to a subterm of Term t1,
including the case term_ident(t1,t2).  All identity checks are done
with term_ident(), so extra fields such as bits or u are not
checked.
*/

/* PUBLIC */
BOOL occurs_in(Term t1, Term t2)
{
  if (term_ident(t1, t2))
    return TRUE;
  else {
    int i;
    for (i = 0; i < t2->arity; i++)
      if (occurs_in(t1, t2->args[i]))
	return TRUE;
    return FALSE;
  }
}  /* occurs_in */

/*************
 *
 *  fprint_term(fp, t)
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a term.  A newline is NOT printed.
*/

/* PUBLIC */
void fprint_term(FILE *fp, Term t)
{
  if (VARIABLE(t))
    fprintf(fp, "v%d", VARNUM(t));
  else {
    fprint_sym(fp, SYMNUM(t));
    if (COMPLEX(t)) {
      int i;
      fprintf(fp, "(");
      for (i = 0; i < ARITY(t); i++) {
	fprint_term(fp, ARG(t,i));
	if (i < ARITY(t)-1)
	  fprintf(fp, ",");
      }
      fprintf(fp, ")");
    }
  }
  fflush(fp);
}  /* print_term */

/*************
 *
 *  sprint_term(sb, t)
 *
 *************/

/* DOCUMENTATION
This (recursive) routine appends the string representation of a term to
a String_buf.  A newline is not included.
*/

/* PUBLIC */
void sprint_term(String_buf sb, Term t)
{
  if (VARIABLE(t)) {
    char s[MAX_NAME];
    sprintf(s, "v%d", VARNUM(t));
    sb_append(sb, s);
  }
  else {
    sprint_sym(sb, SYMNUM(t));
    if (COMPLEX(t)) {
      int i;
      sb_append(sb, "(");
      for (i = 0; i < t->arity; i++) {
	sprint_term(sb, t->args[i]);
	if (i < t->arity-1)
	  sb_append(sb, ",");
      }
      sb_append(sb, ")");
    }
  }
}  /* sprint_term */

/*************
 *
 *  p_term(t)
 *
 *************/

/* DOCUMENTATION
This routine prints a term, followed by '\n' and fflush, to stdout.
If you don't want the newline, use fprint_term() instead.
If you want the term put into a string, use sprint_term() instead.
*/

/* PUBLIC */
void p_term(Term t)
{
  fprint_term(stdout, t);
  printf("\n");
  fflush(stdout);
}  /* p_term */

/*************
 *
 *  all_args_vars(t)
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if all argumets of Term t are VARIABLEs.
(It is true also if t is a VARIABLE.)
*/

/* PUBLIC */
BOOL all_args_vars(Term t)
{
  if (VARIABLE(t))
    return TRUE;
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      if (!VARIABLE(t->args[i]))
	return FALSE;
    return TRUE;
  }
}  /* all_args_vars */

/*************
 *
 *    build_binary_term()
 *
 *************/

/* DOCUMENTATION
Build and return a binary term with SYMNUM sn, first term a1, and
second term a2.
<P>
WARNING: if sn is not a binary symbol, bad things will happen!
*/

/* PUBLIC */
Term build_binary_term(int sn, Term a1, Term a2)
{
    Term t = get_rigid_term_dangerously(sn, 2);
    t->args[0] = a1;
    t->args[1] = a2;
    return(t);
}  /* build_binary_term */

/*************
 *
 *    build_unary_term()
 *
 *************/

/* DOCUMENTATION
Build and return a unary term with SYMNUM sn and argument term a.
<P>
WARNING: if sn is not a unary symbol, bad things will happen!
*/

/* PUBLIC */
Term build_unary_term(int sn, Term a)
{
    Term t = get_rigid_term_dangerously(sn, 1);
    t->args[0] = a;
    return(t);
}  /* build_unary_term */

/*************
 *
 *   subst_term()
 *
 *************/

/* DOCUMENTATION
In term t, replace all occurrences of Term target with <I>copies of</I>
Term replacement.  Free all of the replaced terms;
*/

/* PUBLIC */
Term subst_term(Term t, Term target, Term replacement)
{
  if (term_ident(t, target)) {
    zap_term(t);
    return copy_term(replacement);
  }
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      t->args[i] = subst_term(t->args[i], target, replacement);
    return t;
  }
}  /* subst_term */

/*************
 *
 *   subst_var_term()
 *
 *************/

/* DOCUMENTATION
In Term t, replace all CONSTANT terms containing SYMNUM symnum
with a variable containing VARNUM varnum.  Free the replaced constants
and return the result.
*/

/* PUBLIC */
Term subst_var_term(Term t, int symnum, int varnum)
{
  if (CONSTANT(t) && SYMNUM(t) == symnum) {
    Term v = get_variable_term(varnum);
    zap_term(t);
    return v;
  }
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      t->args[i] = subst_var_term(t->args[i], symnum, varnum);
    return t;
  }
}  /* subst_var_term */

/*************
 *
 *   greatest_variable()
 *
 *************/

/* DOCUMENTATION
This routine returns the greatest variable index in a term.
If the term is ground, -1 is returned.
*/

/* PUBLIC */
int greatest_variable(Term t)
{
  if (VARIABLE(t))
    return VARNUM(t);
  else {
    int i, max, v;
    for (max = -1, i = 0; i < t->arity; i++) {
      v = greatest_variable(t->args[i]);
      max = (v > max ? v : max);
    }
    return max;
  }
}  /* greatest_variable */

/*************
 *
 *   greatest_symnum_in_term()
 *
 *************/

/* DOCUMENTATION
This function returns the greatest SYMNUM (of a CONSTANT or COMPLEX term)
in the given Term t.
If the term is a VARIABLE, return -1.
*/

/* PUBLIC */
int greatest_symnum_in_term(Term t)
{
  if (VARIABLE(t))
    return -1;
  else {
    int max = SYMNUM(t);
    int i;
    for (i = 0; i < t->arity; i++) {
      int sm = greatest_symnum_in_term(t->args[i]);
      max = (sm > max ? sm : max);
    }
    return max;
  }
}  /* greatest_symnum_in_term */

/*************
 *
 *   upward_term_links()
 *
 *************/

/* DOCUMENTATION
In the given Term t, make the "container" field of t and each subterm,
except possibly variables, point to (void *) p.  (We're experimenting
with shared variables.)
*/

/* PUBLIC */
void upward_term_links(Term t, void *p)
{
  int i;
#ifdef SHARE_VARIABLES
  if (!VARIABLE(t))
    t->container = p;
#else
  t->container = p;
#endif
  for (i = 0; i < t->arity; i++)
    upward_term_links(t->args[i], p);
}  /* upward_term_links */

/*************
 *
 *   occurrences()
 *
 *************/

/* DOCUMENTATION
This function returns the number of occurrences of Term target in Term t.
The checks are made with term_ident().
*/

/* PUBLIC */
int occurrences(Term t, Term target)
{
  if (term_ident(t, target))
    return 1;
  else {
    int n = 0;
    int i;
    for (i = 0; i < t->arity; i++)
      n += occurrences(t->args[i], target);
    return n;
  }
}  /* occurrences */

/*************
 *
 *   trm_set_vars_recurse()
 *
 *   There might be another (static) copy of this routine in clause.c.
 *
 *************/

static
Term trm_set_vars_recurse(Term t, char **varnames, int max_vars)
{
  if (CONSTANT(t)) {
    char *name = sn_to_str(SYMNUM(t));
    if (variable_name(name)) {
      int i = 0;
      while (i < max_vars && varnames[i] != NULL && varnames[i] != name)
	i++;
      if (i == max_vars) 
	fatal_error("trm_set_vars_recurse: max_vars");
      else {
	if (varnames[i] == NULL)
	  varnames[i] = name;
	free_term(t);
	t = get_variable_term(i);
      }
    }
  }
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      t->args[i] = trm_set_vars_recurse(t->args[i], varnames, max_vars);
  }
  return t;
}  /* trm_set_vars_recurse */

/*************
 *
 *   term_set_variables()
 *
 *************/

/* DOCUMENTATION
This routine traverses a term and changes the constants
that should be variables, into variables.  On input, the term
should have no variables.  The new variables are numbered
0, 1, 2 ... according the the first occurrence, reading from the
left.
<P>
A fatal error occurs if there are more than max_vars variables.
<P>
<I>If you are dealing with clauses, use clause_set_variables()
instead.</I>
*/

#define VAR_ARRAY_SIZE    100

/* PUBLIC */
void term_set_variables(Term t, int max_vars)
{
  char *a[VAR_ARRAY_SIZE], **vmap;
  int i;

  if (max_vars > VAR_ARRAY_SIZE)
    vmap = malloc((max_vars * sizeof(char *)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    vmap[i] = NULL;

  for (i = 0; i < t->arity; i++) 
    t->args[i] = trm_set_vars_recurse(t->args[i], vmap, max_vars);

  if (max_vars > VAR_ARRAY_SIZE)
    free(vmap);
}  /* term_set_variables */

/*************
 *
 *   nat_to_term()
 *
 *************/

/* DOCUMENTATION
This routine takes a nonnegative integer and returns
a constant Term with the string representation of the
integer as the constant symbol.
*/

/* PUBLIC */
Term nat_to_term(int n)
{
  char s[100];
  if (n < 0)
    fatal_error("nat_to_term: negative term");
  itoa(n, s);
  return get_rigid_term(s, 0);
}  /* nat_to_term */

/*************
 *
 *   natural_constant(t)
 *
 *************/

/* DOCUMENTATION
This routine takes a term, and if the term represents
an nonnegative integer, that integer is returned;
otherwise, -1 is returned.
*/

/* PUBLIC */
int natural_constant(Term t)
{
  if (!CONSTANT(t))
    return -1;
  else {
    int i;
    if (!str_to_int(sn_to_str(SYMNUM(t)), &i))
      return -1;
    else if (i < 0)
      return -1;
    else
      return i;
  }
}  /* natural_constant */

/*************
 *
 *   arg_position()
 *
 *************/

/* DOCUMENTATION
If the given terms are in a parent-child relatioship,
return the argument position (index) of the child.
Otherwise, return -1.
*/

/* PUBLIC */
int arg_position(Term parent, Term child)
{
  int i;
  for (i = 0; i < ARITY(parent); i++) {
    if (ARG(parent,i) == child)
      return i;
  }
  return -1;
}  /* arg_position */

/*************
 *
 *   is_term()
 *
 *************/

/* DOCUMENTATION
Does term t have the the given symbol and arity?
*/

/* PUBLIC */
BOOL is_term(Term t, char *str, int arity)
{
  return t != NULL && is_symbol(SYMNUM(t), str, arity);
}  /* is_term */

/*************
 *
 *   is_constant()
 *
 *************/

/* DOCUMENTATION
Is term t a specific constant?
*/

/* PUBLIC */
BOOL is_constant(Term t, char *str)
{
  return is_term(t, str, 0);
}  /* is_constant */

/*************
 *
 *   term_symbol()
 *
 *************/

/* DOCUMENTATION
Return the print string associated with the given nonvariable term.
If the term is a variable, return NULL.
*/

/* PUBLIC */
char *term_symbol(Term t)
{
  return VARIABLE(t) ? NULL : sn_to_str(SYMNUM(t));
}  /* term_symbol */

/*************
 *
 *   term_to_int()
 *
 *************/

/* DOCUMENTATION
Given a term, see if it represents an integer.
If so, set *result to the integer and return TRUE.
If not, return FALSE.
<P>
The term representation of a negative integer is
the function symbol "-" applied to a nonnegative integer.
*/

/* PUBLIC */
BOOL term_to_int(Term t, int *result)
{
  if (CONSTANT(t)) {
    return str_to_int(sn_to_str(SYMNUM(t)), result); 
  }
  else if (is_term(t, "-", 1)) {
    if (!CONSTANT(ARG(t,0)))
      return FALSE;
    else {
      if (str_to_int(sn_to_str(SYMNUM(ARG(t,0))), result)) {
	*result = -(*result);
	return TRUE;
      }
      else return FALSE;
    }
  }
  else
    return FALSE;
}  /* term_to_int */

/*************
 *
 *   symbols_in_term()
 *
 *************/

/* DOCUMENTATION
This routine collects the multiset of nonvariable symbols in a term.
An Ilist of symbol IDs (symnums) is returned
*/

/* PUBLIC */
Ilist symbols_in_term(Term t, Ilist g)
{
  if (!VARIABLE(t)) {
    int i;
    g = ilist_prepend(g, SYMNUM(t));
    for (i = 0; i < ARITY(t); i++)
      g = symbols_in_term(ARG(t,i), g);
  }
  return g;
}  /* symbols_in_term */

/*************
 *
 *   new_term_top()
 *
 *************/

/* DOCUMENTATION
Replace the top node of a term with a new node.
The new node has an address greater than any other
term currently in use.  The container pointer
and private_flags are copied.
<P>
If TERM_ID_FIELD is defined, the new node gets
a term ID greater than any other currently in use.
<P>
The purpose of this routine is to get the term
ready for FPA indexing.
<P>
Call it like this:  x->t = new_term_top(x->t);
*/

/* PUBLIC */
Term new_term_top(Term t1)
{
#if 1
  return t1;
#else
  if (VARIABLE(t1))
    return t1;
  else {
    Term t2 = get_new_term(ARITY(t1));
    int i;
    for (i = 0; i < ARITY(t1); i++)
      ARG(t2,i) = ARG(t1,i);
    t2->private_symbol = t1->private_symbol;
    t2->container = t1->container;
    t2->private_flags = t1->private_flags;
    free_term(t1);
    return t2;
  }
#endif
}  /* new_term_top */

/*************
 *
 *   entirely_new_term()
 *
 *************/

/* DOCUMENTATION
The purpose of this routine is to get the term
ready for FPA indexing.
<P>
Call it like this:  x->t = entirely_new_term(x->t);
*/

/* PUBLIC */
Term entirely_new_term(Term t)
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = entirely_new_term(ARG(t,i));
    return new_term_top(t);
  }
}  /* entirely_new_term */

/*************
 *
 *   renum_vars_recurse()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables of a term.  It is assumed
that vmap has been filled with -1 on the initial call and that
the size of vmap is at least max_vars.
<P>
This returns a Term instead of being void, in case the
given term is itself a variable.  (Recall that variables
may be shared, so we can't just change a variable's index.
*/

/* PUBLIC */
Term renum_vars_recurse(Term t, int vmap[], int max_vars)
{
  if (VARIABLE(t)) {
    int i = 0;
    while (i < max_vars && vmap[i] != -1 && vmap[i] != VARNUM(t))
      i++;
    if (i == max_vars) 
      fatal_error("renum_vars_recurse: too many variables");

    if (vmap[i] == -1)
      vmap[i] = VARNUM(t);
    free_term(t);
    return get_variable_term(i);
  }
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      t->args[i] = renum_vars_recurse(t->args[i], vmap, max_vars);
    return t;
  }
}  /* renum_vars_recurse */

/*************
 *
 *   set_vars_recurse()
 *
 *************/

/* DOCUMENTATION
This routine sets the variables of a term.  It is assumed
that vnames has been filled with NULL on the initial call and that
the size of vnames is at least max_vars.
<P>
This returns a Term instead of being void, in case the
given term is itself becomes a variable.
*/

/* PUBLIC */
Term set_vars_recurse(Term t, char *vnames[], int max_vars)
{
  if (CONSTANT(t)) {
    char *name = sn_to_str(SYMNUM(t));
    if (variable_name(name)) {
      int i = 0;
      while (i < max_vars && vnames[i] != NULL && vnames[i] != name)
	i++;
      if (i == max_vars) 
	fatal_error("set_vars_recurse: max_vars");
      else {
	if (vnames[i] == NULL)
	  vnames[i] = name;
	free_term(t);
	t = get_variable_term(i);
      }
    }
  }
  else {
    int i;
    for (i = 0; i < t->arity; i++)
      t->args[i] = set_vars_recurse(t->args[i], vnames, max_vars);
  }
  return t;
}  /* set_vars_recurse */

/*************
 *
 *   multiset_of_vars()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist multiset_of_vars(Term t, Plist vars)
{
  if (VARIABLE(t))
    return plist_prepend(vars, t);
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      vars = multiset_of_vars(ARG(t,i), vars);
    return vars;
  }
}  /* multiset_of_vars */

/*************
 *
 *   multiset_of_variables()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist multiset_of_variables(Term t)
{
  return multiset_of_vars(t, NULL);
}  /* multiset_of_variables */

/*************
 *
 *   variables_subset()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL variables_subset(Term t1, Term t2)
{
  Plist t1_vars = multiset_of_variables(t1);
  Plist t2_vars = multiset_of_variables(t2);
  BOOL ok = plist_subset(t1_vars, t2_vars);
  zap_plist(t1_vars);
  zap_plist(t2_vars);
  return ok;
}  /* variables_subset */

/*************
 *
 *   term_at_pos()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term term_at_pos(Term t, Ilist pos)
{
  if (pos == NULL)
    return t;
  else {
    if (pos->i > ARITY(t))
      fatal_error("term_at_pos, position out of range");
    return term_at_pos(ARG(t,pos->i - 1), pos->next);
  }
}  /* term_at_pos */

/*************
 *
 *   pos_of_subterm()
 *
 *************/

static
Ilist pos_of_subterm(Term t, Term subterm)
{
  if (VARIABLE(t))
    return NULL;
  else if (t == subterm)
    /* We need to let the caller know that we found it,
       and we also need to return the position vector (NULL).
       The easiest way I can see to do that is to return
       a non-NULL position consisting of a "terminator"
       which will have to be removed later.
    */
    return ilist_prepend(NULL, INT_MAX);  /* terminator */
  else {
    int i;
    Ilist p = NULL;
    for (i = 0; i < ARITY(t) && p == NULL; i++)
      p = pos_of_subterm(ARG(t, i), subterm);
    return p ? ilist_prepend(p, i) : NULL;
  }
}  /* pos_of_subterm */

/*************
 *
 *   position_of_subterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist position_of_subterm(Term t, Term subterm)
{
  Ilist pos = pos_of_subterm(t, subterm);
  if (pos == NULL)
    return NULL;
  else {
    return ilist_remove_last(pos);
  }
}  /* position_of_subterm */

/*************
 *
 *   symbol_occurrences()
 *
 *************/

/* DOCUMENTATION
Return the number of occurrences of a symbol in a term.
*/

/* PUBLIC */
int symbol_occurrences(Term t, int symnum)
{
  if (VARIABLE(t))
    return 0;
  else {
    int n = (SYMNUM(t) == symnum ? 1 : 0);
    int i;
    for (i = 0; i < ARITY(t); i++)
      n += symbol_occurrences(ARG(t,i), symnum);
    return n;
  }
}  /* symbol_occurrences */

