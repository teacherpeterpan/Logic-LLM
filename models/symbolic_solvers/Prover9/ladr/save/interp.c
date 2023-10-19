#include "interp.h"

/* TO DO:
   1. Get rid of arity limits.
*/

/* Private definitions and types */

struct interp {
  Term  t;        /* the term representation (not always present) */
  int   size;     /* domain size */
  /* Array of pointers to tables, one for each constant, function,
   * or predicate symbol (indexed by symbol number).  The arity and
   * print symbol can be obtained from the symbol number.
   */
  int num_tables;      /* number of tables */
  int **tables;
  int *types;          /* type of tables[i]: FUNCTION or RELATION */

  int *occurrences;    /* number of occurrences of each element */
  Interp next;         /* for avail list */
};

/* We're using 1D arrays to for higher dimensions. */

#define I2(n,i,j)     ((i) * (n) + (j))
#define I3(n,i,j,k)   ((i) * (n) * (n) + (j) * (n) + (k))

#define UNDEFINED 0
#define FUNCTION  1
#define RELATION  2

/* statistics */

static long unsigned Iso_checks = 0;
static long unsigned Iso_perms = 0;

/*
 * memory management
 */

static unsigned Interp_gets, Interp_frees;

#define BYTES_INTERP sizeof(struct interp)
#define PTRS_INTERP BYTES_INTERP%BPP == 0 ? BYTES_INTERP/BPP : BYTES_INTERP/BPP + 1

/*************
 *
 *   Interp get_interp()
 *
 *************/

static
Interp get_interp(void)
{
  Interp p = get_mem(PTRS_INTERP);
  Interp_gets++;
  return(p);
}  /* get_interp */

/*************
 *
 *    free_interp()
 *
 *************/

static
void free_interp(Interp p)
{
  free_mem(p, PTRS_INTERP);
  Interp_frees++;
}  /* free_interp */

/*************
 *
 *   fprint_interp_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the interp package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_interp_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_INTERP;
  fprintf(fp, "interp (%4d)       %11u%11u%11u%9.1f K\n",
          n, Interp_gets, Interp_frees,
          Interp_gets - Interp_frees,
          ((Interp_gets - Interp_frees) * n) / 1024.);

}  /* fprint_interp_mem */

/*************
 *
 *   p_interp_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the interp package.
*/

/* PUBLIC */
void p_interp_mem()
{
  fprint_interp_mem(stdout, TRUE);
}  /* p_interp_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   int_power()
 *
 *************/

static int int_power(int n, int exp)
{
  if (exp <= 0)
    return 1;
  else
    return n * int_power(n, exp-1);
}  /* int_power */

/*************
 *
 *   compile_interp()
 *
 *************/

/* DOCUMENTATION
This routine takes a term representing an interpretation and
builds a data structure that allows fast evaluation of clauses
and formulas w.r.t. the interpretation.  Here is an example of
the term form of an interpretation.
<PRE>
   interpretation(3, [
       function(e,      [0]),
       function(*(_,_), [0,1,2,1,2,0,2,0,1]),
       relation(p,      [1])
       relation(q(_),   [1,0,1])
       ])
</PRE>
The given Term t is not changed.
*/

/* PUBLIC */
Interp compile_interp(Term t)
{
  Interp p;
  int number_of_ops, domain_size, arity;
  int i, j, n, symnum, val, max, rc;
  BOOL function = FALSE;
  int *table; 
  Term operations, lst;

  if (t->arity != 2) 
    fatal_error("compile_interp, bad arity.");

  rc = str_to_int(sn_to_str(SYMNUM(t->args[0])), &domain_size);

  if (!rc || domain_size < 1)
    fatal_error("compile_interp, domain size out of range.");

  operations = t->args[1];

  number_of_ops = listterm_length(operations);
  if (number_of_ops == 0)
    fatal_error("compile_interp, no operations.");

  /* Get the largest symnum, so we can get an table array big enough. */

  max = 0;
  for (i = 1; i <= number_of_ops; i++) {
    Term f = listterm_i(operations, i);
    if (f->arity != 2 || VARIABLE(f->args[0]))
      fatal_error("compile_interp, bad operation.");
    symnum = SYMNUM(f->args[0]);
    max = symnum > max ? symnum : max;
  }

  p = get_interp();
  p->t = copy_term(t);
  p->size = domain_size;
  p->num_tables = max+1;

  p->occurrences = malloc(domain_size * sizeof(int));
  for (i = 0; i < domain_size; i++)
    p->occurrences[i] = 0;

  p->tables  = malloc(p->num_tables * sizeof(int *));
  p->types   = malloc(p->num_tables * sizeof(int));

  for (i = 0; i < p->num_tables; i++) {
    p->tables[i] = NULL;
    p->types[i] = UNDEFINED;
  }

  for (i = 1; i <= number_of_ops; i++) {
    Term f = listterm_i(operations, i);
    if (is_symbol(SYMNUM(f), "function", 2))
      function = TRUE;
    else if (is_symbol(SYMNUM(f), "relation", 2) || 
	     is_symbol(SYMNUM(f), "predicate", 2)) {
      function = FALSE;
    }
    
    else
      fatal_error("compile_interp, bad function/relation");

    symnum = SYMNUM(f->args[0]);
    arity = ARITY(f->args[0]);

    /* n = domain_size^arity */

    for (j = 0, n = 1; j < arity; j++)
      n = n * domain_size;

    lst = f->args[1];
    if (listterm_length(lst) != n)
      fatal_error("compile_interp, bad list.");

    p->types[symnum] = (function ? FUNCTION : RELATION);
    p->tables[symnum] = malloc(n * sizeof(int));
    table = p->tables[symnum];

    for (j = 0; j < n; j++, lst = lst->args[1]) {
      rc = str_to_int(sn_to_str(SYMNUM(lst->args[0])), &val);
      if (!rc)
	fatal_error("compile_interp, bad domain elemnt.");
      else if (function && (val < 0 || val > domain_size-1))
	fatal_error("compile_interp, function element out of range.");
      else if (!function && (val < 0 || val > 1))
	fatal_error("compile_interp, relation element out of range.");
      else {
	table[j] = val;
	if (function)
	  p->occurrences[val]++;
      }
    }
  }
  return p;
}  /* compile_interp */

/*************
 *
 *   transpose_binary()
 *
 *************/

/* DOCUMENTATION
This routine takes a term representing an interpretation and
(destructively) transposes all of the binary functions and relations.
It is assumed that the interpretation is well-formed.  You can check
well-formedness first by calling compile_interp().
*/

/* PUBLIC */
void transpose_binary(Term t)
{
  int number_of_ops, n;
  int i, rc;
  Term operations;

  rc = str_to_int(sn_to_str(SYMNUM(t->args[0])), &n);
  operations = t->args[1];
  number_of_ops = listterm_length(operations);

  for (i = 1; i <= number_of_ops; i++) {
    Term f = listterm_i(operations, i);  /* e.g., function(j(_,_), [0,1,1,0]) */
    if (f->args[0]->arity == 2) {
      int j, k;
      Term lst = f->args[1];             /* e.g., [0,1,1,0] */
      for (j = 0; j < n; j++) {
	for (k = j+1; k < n; k++) {
	  Term t1 = listterm_i(lst, j*n+k+1);
	  Term t2 = listterm_i(lst, k*n+j+1);
	  int tmp = t1->private_symbol;
	  t1->private_symbol = t2->private_symbol;
	  t2->private_symbol = tmp;
	}
      }
    }
  }
}  /* transpose_binary */

/*************
 *
 *   zap_interp()
 *
 *************/

/* DOCUMENTATION
Free a compiled interpretation.
*/

/* PUBLIC */
void zap_interp(Interp p)
{
  int i;

  free(p->occurrences);
  free(p->types);

  for (i = 0; i < p->num_tables; i++)
    if (p->tables[i] != NULL)
      free(p->tables[i]);

  free(p->tables);

  if (p->t != NULL)
    zap_term(p->t);
  free_interp(p);
}  /* zap_interp */

/*************
 *
 *   fprint_interp_tex()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in a form that might be useful as input to LaTeX.
*/

/* PUBLIC */
void fprint_interp_tex(FILE *fp, Interp p)
{
  int n = p->size;
  int i;
  BOOL first = TRUE;

  fprintf(fp, "\\begin{table}[H]  \\centering %% size %d\n", p->size);
  
  for (i = 0; i < p->num_tables; i++) {  /* arity 0 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    if (table != NULL && arity == 0) {
      if (first)
	first = FALSE;
      else
	fprintf(fp, " \\hspace{.5cm}\n");
      fprintf(fp, "%s: %d", sn_to_str(i), table[0]);
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 1 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    if (table != NULL && arity == 1) {
      int j;
      if (first)
	first = FALSE;
      else
	fprintf(fp, " \\hspace{.5cm}\n");
      fprintf(fp, "\\begin{tabular}{r|");
      for (j = 0; j < n; j++)
	fprintf(fp, "r");
      fprintf(fp, "}\n");
      fprintf(fp, "%s: & ", sn_to_str(i));
      for (j = 0; j < n; j++)
	fprintf(fp, "%d%s", j, j < n-1 ? " & " : "\\\\\n\\hline\n   & ");
      for (j = 0; j < n; j++)
	fprintf(fp, "%d%s", table[j], j < n-1 ? " & " : "\n");
      fprintf(fp, "\\end{tabular}");
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 2 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    if (table != NULL && arity == 2) {
      int j, k;
      if (first)
	first = FALSE;
      else
	fprintf(fp, " \\hspace{.5cm}\n");
      fprintf(fp, "\\begin{tabular}{r|");
      for (j = 0; j < n; j++)
	fprintf(fp, "r");
      fprintf(fp, "}\n");
      fprintf(fp, "%s: & ", sn_to_str(i));
      for (j = 0; j < n; j++)
	fprintf(fp, "%d%s", j, j < n-1 ? " & " : "\\\\\n\\hline\n");
      for (j = 0; j < n; j++) {
	fprintf(fp, "    %d & ", j);
	for (k = 0; k < n; k++) {
	  fprintf(fp,
		  "%d%s",
		  table[(n*j) + k], 
		  k < n-1 ? " & " : (j < n-1 ? " \\\\\n" : "\n"));
	}
      }
      fprintf(fp, "\\end{tabular}");
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity > 2 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    if (table != NULL && arity > 2) {
      fprintf(fp, "\n\n%% table for arity %d %s %s not printed.\n\n",
	      arity,
	      p->types[i] == FUNCTION ? "function" : "relation",
	      sn_to_str(i));
    }
  }
  fprintf(fp, "\n\\caption{ }\n");
  fprintf(fp, "\\end{table}\n");
}  /* fprint_interp_tex */

/*************
 *
 *   fprint_interp()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in portable form, with each operation on a separate line.
*/

/* PUBLIC */
void fprint_interp(FILE *fp, Interp p)
{
  int i;

  fprintf(fp, "interpretation(%d, [\n", p->size);
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j, n;
      int arity = sn_to_arity(i);
      fprintf(fp,"    %s(",p->types[i] == FUNCTION ? "function" : "relation");
      if (arity == 0)
	fprintf(fp, "%s, [",  sn_to_str(i));
      else {
	fprintf(fp, "%s(",  sn_to_str(i));
	for (j = 0; j < arity; j++)
	  fprintf(fp, "_%s",  j == arity-1 ? "), [" : ",");
      }

      for (j = 0, n = 1; j < arity; j++, n = n * p->size);
      for (j = 0; j < n; j++)
	fprintf(fp, "%d%s", table[j], j == n-1 ? "])" : ",");

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL)
	fprintf(fp, ",\n");
      else
	fprintf(fp, "]).\n");
    }
  }
}  /* fprint_interp */

/*************
 *
 *   fprint_interp_2()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in portable form, with each operation on a separate line,
except that binary operations are printed on multiple lines.
*/

/* PUBLIC */
void fprint_interp_2(FILE *fp, Interp p)
{
  int i;

  fprintf(fp, "\ninterpretation(%d, [\n", p->size);
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j, n;
      int arity = sn_to_arity(i);
      BOOL w = (arity == 2 && p->size > 10);
      fprintf(fp,"    %s(",p->types[i] == FUNCTION ? "function" : "relation");
      if (arity == 0)
	fprintf(fp, "%s, [",  sn_to_str(i));
      else {
	fprintf(fp, "%s(",  sn_to_str(i));
	for (j = 0; j < arity; j++)
	  fprintf(fp, "_%s",  j == arity-1 ? "), " : ",");
	fprintf(fp, "[%s",  arity== 2 ? "\n        " : "");
      }

      for (j = 0, n = 1; j < arity; j++, n = n * p->size);
      for (j = 0; j < n; j++) {
	fprintf(fp, w ? "%2d%s" : "%d%s",
		table[j],
		j == n-1 ? "])" : ",");
	if (arity == 2 && (j+1) % p->size == 0 && j != n-1)
	  fprintf(fp, "\n        ");
      }

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL)
	fprintf(fp, ",\n");
      else
	fprintf(fp, "]).\n");
    }
  }
}  /* fprint_interp_2 */

/*************
 *
 *   p_interp()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) a compiled interpretation,
in portable form, with each operation on a separate line.
*/

/* PUBLIC */
void p_interp(Interp p)
{
  fprint_interp(stdout, p);
}  /* p_interp */

/*************
 *
 *   fprint_interp_tabular()
 *
 *************/

/* DOCUMENTATION
This routine pretty prints (to FILE *fp) an interpretation
in tabular (not easily parsable).  Arities > 2 are not pretty.
*/

/* PUBLIC */
void fprint_interp_tabular(FILE *fp, Interp p)
{
  int f, i, j;

  for (f = 0; f < p->num_tables; f++) {
    int *table = p->tables[f];
    if (table != NULL) {
      int n = p->size;
      int arity = sn_to_arity(f);

      fprintf(fp, "\n %s : ", sn_to_str(f));

      if (arity == 0)
	fprintf(fp, "%d\n", table[0]);

      else if (arity == 1) {
	fprintf(fp, "\n        ");
	for (i = 0; i < n; i++)
	  fprintf(fp, "%2d", i);
	fprintf(fp, "\n    ----");
	for (i = 0; i < n; i++)
	  fprintf(fp, "--");
	fprintf(fp, "\n        ");
	for (i = 0; i < n; i++)
	  fprintf(fp, "%2d", table[i]);
	fprintf(fp, "\n");
      }
      else if (arity == 2) {
	fprintf(fp, "\n       |");
	for (i = 0; i < n; i++)
	  fprintf(fp, "%2d", i);
	fprintf(fp, "\n    ---+");
	for (i = 0; i < n; i++)
	  fprintf(fp, "--");
	for (i = 0; i < n; i++) {
	  fprintf(fp, "\n    %2d |", i);
	  for (j = 0; j < n; j++)
	    fprintf(fp, "%2d", table[I2(n,i,j)]);
	}
	fprintf(fp, "\n");
      }
      else {
	int m = int_power(n, arity);
	fprintf(fp, "[");
	for (i = 0; i < m; i++)
	  fprintf(fp, "%d%s", table[i], i == m-1 ? "]\n" : ",");
      }
    }
  }  /* for each function or relation */
}  /* fprint_interp_tabular */

/*************
 *
 *   eval_term_ground()
 *
 *   This version is for clauses, where variables are represented as VARIABLEs.
 *   It works for terms and atoms.  All natural numbers are interpreted as
 *   domain values, and if any are out of range, a fatal error occurs.
 *
 *************/

static
int eval_term_ground(Term t, Interp p, int *vals)
{
  if (VARIABLE(t))
    return vals[VARNUM(t)];
  else {
    int n = p->size;
    int sn = SYMNUM(t);
    int domain_element;

    if (CONSTANT(t) && term_to_int(t, &domain_element)) {
      if (domain_element < 0 || domain_element >= n) {
	printf("ready to abort, bad term: "); p_term(t);
	fatal_error("eval_term_ground, domain element out of range");
      }
      return domain_element;
    }
    else {
      int *table;
      int i, j, mult;

      if (sn >= p->num_tables || p->tables[sn] == NULL) {
	printf("ready to abort, bad term: "); p_term(t);
	fatal_error("eval_term_ground, symbol not in interpretation");
      }

      table = p->tables[sn];

      j = 0;     /* we'll build up the index with j */
      mult = 1;  
      for (i = t->arity-1; i >= 0; i--) {
	int v = eval_term_ground(t->args[i], p, vals);
	j += v * mult;
	mult = mult * n;
      }
      return table[j];
    }
  }
}  /* eval_term_ground */

/*************
 *
 *   eval_clause_ground()
 *
 *   Given a ground clause and an interpretation,
 *   return "at least one literal is true in the interpretation".
 *
 *************/

static
BOOL eval_clause_ground(Clause c, Interp p, int *vals)
{
  Literal lit;
  BOOL atom_val, true_literal;

  true_literal = FALSE;
  for (lit = c->literals; lit && !true_literal; lit = lit->next) {

    if (is_eq_symbol(SYMNUM(lit->atom)))
      atom_val = (eval_term_ground(lit->atom->args[0], p, vals) ==
		  eval_term_ground(lit->atom->args[1], p, vals));
    else
      atom_val = eval_term_ground(lit->atom, p, vals);

    true_literal = (lit->sign ? atom_val : !atom_val);
  }
  return true_literal;
}  /* eval_clause_ground */

/*************
 *
 *   all_recurse()
 *
 *************/

static
BOOL all_recurse(Clause c, Interp p, int *vals, int nextvar, int nvars)
{
  if (nextvar == nvars) {
    return eval_clause_ground(c, p, vals);
  }
  else {
    int i, rc;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      rc = all_recurse(c, p, vals, nextvar+1, nvars);
      if (!rc)
	return FALSE;
    }
    return TRUE;
  }
}  /* all_recurse */

/*************
 *
 *   eval_clause()
 *
 *************/

#define MAX_VARS_EVAL 100

/* DOCUMENTATION
This routine evaluates a clause in an interpretation.
If all instances (over the domain of the interpretation) of the clause
are true in the interpretaion, TRUE is returned.  If any instance
is false, FALSE is returned.
<P>
Note that if the interpretation has d elements and the clause has
v variables, it takes d^v evaluations to verify the clause.
<P>
All natural numbers are interpreted as domain values, and if any
domain values are out of range, a fatal error occurs.
<P>
A fatal error occurs if any constant, function or predicate symbol
in the clause (other than EQ_SYM, which is always built in)
is absent from the interpetation.
*/

/* PUBLIC */
BOOL eval_clause(Clause c, Interp p)
{
  int a[MAX_VARS_EVAL], *vals;
  int nvars;
  BOOL rc;

  nvars = greatest_variable_in_clause(c) + 1;
  if (nvars > MAX_VARS_EVAL)
    vals = malloc((nvars * sizeof(int)));
  else
    vals = a;

  rc = all_recurse(c, p, vals, 0, nvars);

  if (nvars > MAX_VARS_EVAL)
    free(vals);

  return rc;
}  /* eval_clause */

/*************
 *
 *   all_recurse2()
 *
 *************/

static
int all_recurse2(Clause c, Interp p, int *vals, int nextvar, int nvars)
{
  if (nextvar == nvars) {
    return eval_clause_ground(c, p, vals) ? 0 : 1;
  }
  else {
    int i;
    int false_instances = 0;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      false_instances += all_recurse2(c, p, vals, nextvar+1, nvars);
    }
    return false_instances;
  }
}  /* all_recurse2 */

/*************
 *
 *   eval_clause_false_instances()
 *
 *************/

/* DOCUMENTATION
This routine evaluates a clause in an interpretation.
The number of false instances is returned.
*/

/* PUBLIC */
int eval_clause_false_instances(Clause c, Interp p)
{
  int a[MAX_VARS_EVAL], *vals;
  int nvars;
  int false_instances;

  nvars = greatest_variable_in_clause(c) + 1;
  if (nvars > MAX_VARS_EVAL)
    vals = malloc((nvars * sizeof(int)));
  else
    vals = a;

  false_instances = all_recurse2(c, p, vals, 0, nvars);

  if (nvars > MAX_VARS_EVAL)
    free(vals);

  return false_instances;
}  /* eval_clause_false_instances */

/*************
 *
 *   eval_fterm_ground()
 *
 *   This version is for formulas, where variables are
 *   represented as CONSTANTs.
 *   It works for terms and atoms.
 *
 *************/

static
int eval_fterm_ground(Term t, Interp p, int *vals)
{
  if (VARIABLE(t))
    fatal_error("eval_fterm_ground, VARIABLE encountered.");

  if (vals[SYMNUM(t)] != -1)
    return vals[SYMNUM(t)];

  else {
    int n = p->size;
    int sn = SYMNUM(t);
    int domain_element;

    if (CONSTANT(t) && term_to_int(t, &domain_element)) {
      if (domain_element < 0 || domain_element >= n) {
	printf("ready to abort, bad term: "); p_term(t);
	fatal_error("eval_fterm_ground, domain element out of range");
      }
      return domain_element;
    }
    else {
      int *table;
      int i, j, mult;

      if (sn >= p->num_tables || p->tables[sn] == NULL) {
	printf("ready to abort, bad term: "); p_term(t);
	fatal_error("eval_fterm_ground, symbol not in interpretation");
      }

      table = p->tables[sn];

      j = 0;     /* we'll build up the index with j */
      mult = 1;  
      for (i = t->arity-1; i >= 0; i--) {
	int v = eval_term_ground(t->args[i], p, vals);
	j += v * mult;
	mult = mult * n;
      }
      return table[j];
    }
  }
}  /* eval_fterm_ground */

/*************
 *
 *   eval_form()
 *
 *************/

static
BOOL eval_form(Formula f, Interp p, int vals[])
{
  if (f->type == ATOM_FORM) {
    if (is_eq_symbol(SYMNUM(f->atom)))
      return (eval_fterm_ground(f->atom->args[0], p, vals) ==
	      eval_fterm_ground(f->atom->args[1], p, vals));
    else
      return eval_fterm_ground(f->atom, p, vals);
  }
  else if (f->type == ALL_FORM) {
    /* ok if true for every element of domain */
    int i;
    BOOL ok = TRUE;
    int sn = str_to_sn(f->qvar, 0);
    if (vals[sn] != -1)
      fatal_error("eval_form, variable conflict.");
    for (i = 0; i < p->size && ok; i++) {
      vals[sn] = i;
      if (!eval_form(f->kids[0], p, vals))
	ok = FALSE;
    }
    vals[sn] = -1;
    return ok;
  }
  else if (f->type == EXISTS_FORM) {
    /* ok if true for any element of domain */
    int i;
    BOOL ok = FALSE;
    int sn = str_to_sn(f->qvar, 0);
    if (vals[sn] != -1)
      fatal_error("eval_form, variable conflict.");
    for (i = 0; i < p->size && !ok; i++) {
      vals[sn] = i;
      if (eval_form(f->kids[0], p, vals))
	ok = TRUE;
    }
    vals[sn] = -1;
    return ok;
  }
  else if (f->type == AND_FORM) {
    int i;
    BOOL ok = TRUE;
    for (i = 0; i < f->arity && ok; i++)
      if (!eval_form(f->kids[i], p, vals))
	ok = FALSE;
    return ok;
  }
  else if (f->type == OR_FORM) {
    int i;
    BOOL ok = FALSE;
    for (i = 0; i < f->arity && !ok; i++)
      if (eval_form(f->kids[i], p, vals))
	ok = TRUE;
    return ok;
  }
  else if (f->type == NOT_FORM) {
    return !eval_form(f->kids[0], p, vals);
  }
  else if (f->type == IFF_FORM) {
    return (eval_form(f->kids[0], p, vals) == eval_form(f->kids[1], p, vals));
  }
  else if (f->type == IMP_FORM) {
    return (!eval_form(f->kids[0], p, vals) || eval_form(f->kids[1], p, vals));
  }
  else if (f->type == IMPBY_FORM) {
    return (eval_form(f->kids[0], p, vals) || !eval_form(f->kids[1], p, vals));
  }
  else {
    fatal_error("eval_form, bad formula.");
    return 0;  /* to please the compiler */
  }
}  /* eval_form */

/*************
 *
 *   eval_formula()
 *
 *************/

/* DOCUMENTATION
This routine evaluates a formula in an interpretation.
There is no restriction on the structure of the formula.
However,
<I>quantified variables must be named in such a way that</I>
when a quantifier binds a variable, say (all x F), then
no quantifier in F can rebind x.  The routine eliminate_rebinding()
can be called to transform a formula, if necessary, so that it
satisfies the rule.
<P>
All natural numbers are interpreted as domain values, and if any
domain values are out of range, a fatal error occurs.
<P>
A fatal error occurs if any constant, function or predicate symbol
in the formula (other than EQ_SYM, which is always built in)
is absent from the interpetation.
*/

/* PUBLIC */
BOOL eval_formula(Formula f, Interp p)
{
  int a[MAX_VARS_EVAL], *vals;
  int nsyms, i;
  BOOL rc;

  nsyms = greatest_symnum_in_formula(f) + 1;
  if (nsyms > MAX_VARS_EVAL)
    vals = malloc((nsyms * sizeof(int)));
  else
    vals = a;

  for (i = 0; i < nsyms; i++)
    vals[i] = -1;

  rc = eval_form(f, p, vals);

  if (nsyms > MAX_VARS_EVAL)
    free(vals);

#if 0
  if (rc)
    printf("Formula is TRUE in this interpretation.\n");
  else {
    printf("Formula is FALSE in this interpretation.\n");
  }
#endif
  return rc;
}  /* eval_formula */

/*************
 *
 *   copy_interp()
 *
 *************/

/* DOCUMENTATION
This routine copies an interpretation.  We don't check for errors.
*/

/* PUBLIC */
Interp copy_interp(Interp p)
{
  int i;
  Interp q = get_interp();

  q->t = copy_term(p->t);
  q->size = p->size;
  q->num_tables = p->num_tables;

  q->occurrences = malloc(q->size * sizeof(int));
  for (i = 0; i < q->size; i++)
    q->occurrences[i] = p->occurrences[i];

  q->types = malloc(q->num_tables * sizeof(int));
  for (i = 0; i < q->num_tables; i++)
    q->types[i] = p->types[i];

  q->tables = malloc(q->num_tables * sizeof(int *));
  for (i = 0; i < q->num_tables; i++)
    q->tables[i] = NULL;

  for (i = 0; i < q->num_tables; i++)
    if (p->tables[i] != NULL) {
      int arity = sn_to_arity(i);
      int n = 1;
      int *ptable, *qtable, j;
      for (j = 0; j < arity; j++)
	n = n * p->size;
      q->tables[i] = malloc(n * sizeof(int));
      ptable = p->tables[i];
      qtable = q->tables[i];
      for (j = 0; j < n; j++)
	qtable[j] = ptable[j];
    }

  return q;
}  /* copy_interp */

/*************
 *
 *   permute_interp()
 *
 *************/

/* DOCUMENTATION
This routine returns a permutation of an interpretation.
The permuted interpretation does not contain the term
representation (because it would be nontrivial to construct it).
*/

/* PUBLIC */
Interp permute_interp(Interp source, int *p)
{
  Interp dest = copy_interp(source);
  int n = source->size;
  int f;
  for (f = 0; f < source->num_tables; f++) {
    if (source->tables[f] != NULL) {
      int *st = source->tables[f];
      int *dt =   dest->tables[f];
      int arity = sn_to_arity(f);
      BOOL function = (source->types[f] == FUNCTION);
      if (arity == 0)
	dt[0] = (function ? p[st[0]] : st[0]);
      else if (arity == 1) {
	int i;
	for (i = 0; i < n; i++)
	  dt[p[i]] = (function ? p[st[i]] : st[i]);
      }
      else if (arity == 2) {
	int i, j;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++)
	    dt[I2(n,p[i],p[j])] = (function
				   ? p[st[I2(n,i,j)]]
				   : st[I2(n,i,j)]);
      }
      else if (arity == 3) {
	int i, j, k;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++)
	    for (k = 0; k < n; k++)
	      dt[I3(n,p[i],p[j],p[k])] = (function
					  ? p[st[I3(n,i,j,k)]]
					  : st[I3(n,i,j,k)]);
      }
      else
	fatal_error("permute_interp: arity > 3");
    }
  }
  {
    int i;
    for (i = 0; i < n; i++)
      dest->occurrences[p[i]] = source->occurrences[i];
  }
  /* The term representation is no longer correct. */
  zap_term(dest->t);
  dest->t = NULL;
  return dest;
}  /* permute_interp */

/*************
 *
 *   ident_interp_perm()
 *
 *************/

/* DOCUMENTATION
Is interpretation B identical to a given permutation of interpretation A?
If so, then A and B are isomorphic.  It is assumed that A and B
are the same size and have the same symbols.
*/

/* PUBLIC */
BOOL ident_interp_perm(Interp a, Interp b, int *p)
{
  int n = a->size;
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *at =   a->tables[f];
      int *bt =   b->tables[f];
      int arity = sn_to_arity(f);
      BOOL function = (a->types[f] == FUNCTION);
      if (arity == 0) {
	if (bt[0] != (function ? p[at[0]] : at[0]))
	  return FALSE;
      }
      else if (arity == 1) {
	int i;
	for (i = 0; i < n; i++) {
	  if (bt[p[i]] != (function ? p[at[i]] : at[i]))
	    return FALSE;
	}
      }
      else if (arity == 2) {
	int i, j;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++) {
	    if (bt[I2(n,p[i],p[j])] != (function
					? p[at[I2(n,i,j)]]
					: at[I2(n,i,j)]))
	      return FALSE;
	  }
      }
      else if (arity == 3) {
	int i, j, k;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++)
	    for (k = 0; k < n; k++) {
	      if (bt[I3(n,p[i],p[j],p[k])] != (function
					       ? p[at[I3(n,i,j,k)]]
					       : at[I3(n,i,j,k)]))
		return FALSE;
	    }
      }
      else
	fatal_error("ident_interp_perm: arity > 3");
    }
  }
  return TRUE;
}  /* ident_interp_perm */

/*************
 *
 *   canon_interp()
 *
 *************/

/* DOCUMENTATION
This routine returns a canonicalized copy of an interpretation.
Canonical interpretations are used to speed up isomorphism checking.
<P>
Consider, in all of the function tables, the number of occurrences of each
element.  If we have a size-4 interpretation with a binary function,
a unary function, and a constant, the occurrence array might be
something like [7,6,4,4], meaning that there are 7 occurrences of
0, 6, occurrences of 1, 4 occurrences of 2, and 4 occurrences of 3.
If the occurrence array is nonincreasing, the interpretation is in
a canonical form.  (Canonical forms are not unique; otherwise,
isomorphism checking would be trivial.)
<P>
If 2 canonical interpretations have different occurrence arrays,
they cannot be isomorphic.  That's the first check we make.
<P>
When checking whether two interpretations are isomorphic, we
look at permutations of one of the interpretations, and we can
use occurrence array to eliminate some of the permutations.
Examples: with occurrence array [7,6,4,4], we 
look at only 2 permutations: (0,1,2,3) and (0,1,3,2);
with occurrence array [4,4,4,3,3], we'd look at 12 (6*2)
permutations instead of 120 (5!).
*/

/* PUBLIC */
Interp canon_interp(Interp a)
{
  int i;
  int *occ  = malloc(sizeof(int) * a->size);  /* remember to free this */
  int *perm = malloc(sizeof(int) * a->size);  /* remember to free this */
  int size = a->size;
  Interp can;

  /* Determine the permutation we'll use to canonicalize
     the interpretation. */

  for (i = 0; i < size; i++)
    occ[i] = a->occurrences[i];

  for (i = 0; i < size; i++) {
    int max = -1;
    int index_of_max = -1;
    int j;
    for (j = 0; j < size; j++) {
      if (occ[j] > max) {
	index_of_max = j;
	max = occ[j];
      }
    }
    perm[index_of_max] = i;
    occ[index_of_max] = -1;
  }

  free(occ);  /* This is now useless (all members are -1). */

  /* Apply the permutation to the interpretation. */
  
  can = permute_interp(a, perm);
  free(perm);
  return can;
}  /* canon_interp */

/*************
 *
 *   iso_interp_recurse()
 *
 *************/

#define ISWAP(x,y) {int t = x; x = y; y = t;}

static BOOL iso_interp_recurse(int *p, int k, int n,
				   Interp a, Interp b, BOOL canon)
{
  int i;
  if (k == n) {
    /* We have a permutation. */
    Iso_perms++;
    return ident_interp_perm(a, b, p);
  }
  else {
    /* Continue building permutations. */
    if (iso_interp_recurse(p, k+1, n, a, b, canon))
      return TRUE;
    for (i = k+1; i < n; i++) {
      /* If canonical, and if i and k have different number
	 of occurrences, don't swap them. */
      if (!canon || a->occurrences[i] == a->occurrences[k]) {
	ISWAP(p[k], p[i]);
	if (iso_interp_recurse(p, k+1, n, a, b, canon))
	  return TRUE;
	ISWAP(p[k], p[i]);
      }
    }
    return FALSE;
  }
}  /* iso_interp_recurse */

/*************
 *
 *   isomorphic_canon_interps()
 *
 *************/

/* DOCUMENTATION
Are interpretations A and B isomorphic?  We assume they are
compatible (same operations/arities).  If the flag canon
is set, it is assumed that both interps were produced by
canon_interp(); this allows some optimization.
*/

/* PUBLIC */
BOOL isomorphic_interps(Interp a, Interp b, BOOL canon)
{
  int i;
  BOOL isomorphic;
  int *perm;

  if (a->size != b->size)
    return FALSE;

  /* If canonical, make sure the interps have the same occurrence-type. */

  if (canon) {
    for (i = 0; i < a->size; i++)
      if (a->occurrences[i] != b->occurrences[i])
	return FALSE;
  }

  Iso_checks++;

  perm = malloc(sizeof(int) * a->size);  /* remember to free this */

  /* Initialize perm to the trivial permutation. */

  for (i = 0; i < a->size; i++)
    perm[i] = i;

  isomorphic = iso_interp_recurse(perm, 0, a->size, a, b, canon);
  free(perm);
  return isomorphic;
}  /* isomorphic_canon_interps */

/*************
 *
 *   interp_size()
 *
 *************/

/* DOCUMENTATION
Return the domain size of an interpretation.   
*/

/* PUBLIC */
int interp_size(Interp a)
{
  return a->size;
}  /* interp_size */

/*************
 *
 *   interp_table()
 *
 *************/

/* DOCUMENTATION
Given a symbol and arity, return the corresponding table.xc
*/

/* PUBLIC */
int *interp_table(Interp p, char *sym, int arity)
{
  int f;
  for (f = 0; f < p->num_tables; f++)
    if (is_symbol(f, sym, arity))
      return p->tables[f];
  return NULL;
}  /* interp_table */

/*************
 *
 *   iso_checks()
 *
 *************/

/* DOCUMENTATION
Return the number of isomorphism checks.  For canonical checks,
ones where the occurrence-types don't match are not counted.
*/

/* PUBLIC */
long unsigned iso_checks(void)
{
  return Iso_checks;
}  /* iso_checks */

/*************
 *
 *   iso_perms()
 *
 *************/

/* DOCUMENTATION
Return the number of permutations seen during isomorphism checks.
*/

/* PUBLIC */
long unsigned iso_perms(void)
{
  return Iso_perms;
}  /* iso_perms */
