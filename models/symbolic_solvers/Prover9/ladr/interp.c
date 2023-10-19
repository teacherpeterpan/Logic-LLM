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

#include "interp.h"
#include "ioutil.h"

/* TO DO:
   1. Get rid of arity limits.
*/

/* Private definitions and types */

struct interp {
  Term  t;         /* the term representation (maybe NULL) */
  Term  comments;  /* list of comments (maybe NULL) */
  int   size;      /* domain size */
  /* Array of pointers to tables, one for each constant, function,
   * or predicate symbol (indexed by symbol number).  The arity and
   * print symbol can be obtained from the symbol number.
   */
  int num_tables;      /* number of tables */
  int **tables;
  int *arities;        /* arity of tables[i] */
  int *types;          /* type of tables[i]: FUNCTION or RELATION */

  int *occurrences;    /* number of occurrences of each element */
  int num_discriminators;
  int *discriminator_counts;
  int **profile;               /* [size,profile_components] */
  int num_profile_components;
  int *blocks;                 /* of identical components */
  BOOL incomplete;
};

#define ISWAP(x,y) {int t = x; x = y; y = t;}

/* We're using 1D arrays to for higher dimensions. */

#define I2(n,i,j)     ((i) * (n) + (j))
#define I3(n,i,j,k)   ((i) * (n) * (n) + (j) * (n) + (k))

#define UNDEFINED 0
#define FUNCTION  1
#define RELATION  2

#define MAX_VARS_EVAL 100

/* statistics */

static long unsigned Iso_checks = 0;
static long unsigned Iso_perms = 0;

/*
 * memory management
 */

#define PTRS_INTERP PTRS(sizeof(struct interp))
static unsigned Interp_gets, Interp_frees;

/*************
 *
 *   Interp get_interp()
 *
 *************/

static
Interp get_interp(void)
{
  Interp p = get_cmem(PTRS_INTERP);
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

  n = sizeof(struct interp);
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
 *   trivial_permutation()
 *
 *************/

static
int *trivial_permutation(int n)
{
  int *p = malloc(sizeof(int) * n);
  int i;
  for (i = 0; i < n; i++)
    p[i] = i;
  return p;
}  /* trivial_permutation */

/*************
 *
 *   int_power()
 *
 *************/

/* DOCUMENTATION
Return n^exp.
If exp is negative, return -1.
If the result is too big to fit into an int, return INT_MAX.
*/

/* PUBLIC */
int int_power(int n, int exp)
{
  if (exp < 0)
    return -1;
  else {
    int i;
    int r = 1;
    for (i = 0; i < exp; i++) {
      int x = r * n;
      if (x < r)
	return INT_MAX;  /* overflow */
      r = x;
    }
    return r;
  }
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
Interp compile_interp(Term t, BOOL allow_incomplete)
{
  Interp p;
  int number_of_ops, domain_size, arity;
  int i, j, n, symnum, val, max, rc;
  BOOL function = FALSE;
  int *table; 
  Term comments = NULL;
  Term size = NULL;
  Term operations = NULL;

  if (ARITY(t) == 3) {  /* interpretation(size, comments, ops). */
    size = ARG(t,0);
    comments = ARG(t,1);
    operations = ARG(t,2);
  }
  else
    fatal_error("compile_interp, arity must be 3 (size, comments, ops).");

  rc = str_to_int(sn_to_str(SYMNUM(size)), &domain_size);
  
  if (!rc || domain_size < 1)
    fatal_error("compile_interp, domain size out of range.");

  number_of_ops = listterm_length(operations);
  /*
  if (number_of_ops == 0)
    fatal_error("compile_interp, interpretation has no operations.");
  */

  /* Get the largest symnum, so we can get a table array big enough. */

  max = 0;
  for (i = 1; i <= number_of_ops; i++) {
    Term f = listterm_i(operations, i);
    if (ARITY(f) != 2 || VARIABLE(ARG(f,0)))
      fatal_error("compile_interp, bad operation.");
    symnum = SYMNUM(ARG(f,0));
    max = symnum > max ? symnum : max;
  }

  p = get_interp();
  p->t = copy_term(t);
  p->comments = copy_term(comments);
  p->size = domain_size;
  p->num_tables = max + 100 + 1;  /* allow 100 extra in case of new symbols */

  p->occurrences = malloc(domain_size * sizeof(int));
  p->blocks = malloc(domain_size * sizeof(int));
  p->profile = malloc(domain_size * sizeof(int *));
  for (i = 0; i < domain_size; i++) {
    p->occurrences[i] = 0;
    p->blocks[i] = -1;
    p->profile[i] = NULL;
  }

  p->tables  = malloc(p->num_tables * sizeof(int *));
  p->types   = malloc(p->num_tables * sizeof(int));
  p->arities = malloc(p->num_tables * sizeof(int));

  for (i = 0; i < p->num_tables; i++) {
    p->tables[i] = NULL;
    p->types[i] = UNDEFINED;
    p->arities[i] = -1;
  }

  for (i = 1; i <= number_of_ops; i++) {
    Term lst;
    Term f = listterm_i(operations, i);
    if (is_symbol(SYMNUM(f), "function", 2))
      function = TRUE;
    else if (is_symbol(SYMNUM(f), "relation", 2) || 
	     is_symbol(SYMNUM(f), "predicate", 2)) {
      function = FALSE;
    }
    
    else
      fatal_error("compile_interp, bad function/relation");

    symnum = SYMNUM(ARG(f,0));
    arity = ARITY(ARG(f,0));

    /* n = domain_size^arity */

    for (j = 0, n = 1; j < arity; j++)
      n = n * domain_size;

    lst = ARG(f,1);
    if (listterm_length(lst) != n)
      fatal_error("compile_interp, list of elements is wrong "
		  "length for arity/domain_size.");

    p->types[symnum] = (function ? FUNCTION : RELATION);
    p->arities[symnum] = arity;
    p->tables[symnum] = malloc(n * sizeof(int));
    table = p->tables[symnum];

    for (j = 0; j < n; j++, lst = ARG(lst,1)) {
      char *str = sn_to_str(SYMNUM(ARG(lst,0)));
      rc = str_to_int(str, &val);
      if (!rc) {
	if (allow_incomplete && str_ident(str, "-")) {
	  table[j] = -1;
	  p->incomplete = TRUE;
	}
	else
	  fatal_error("compile_interp, bad domain elemnt.");
      }
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

  rc = str_to_int(sn_to_str(SYMNUM(ARG(t,0))), &n);
  operations = ARG(t,2);
  number_of_ops = listterm_length(operations);

  for (i = 1; i <= number_of_ops; i++) {
    Term f = listterm_i(operations, i);  /* e.g., function(j(_,_), [0,1,1,0]) */
    if (ARITY(ARG(f,0)) == 2) {
      int j, k;
      Term lst = ARG(f,1);             /* e.g., [0,1,1,0] */
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
  free(p->blocks);
  free(p->types);
  free(p->arities);

  if (p->discriminator_counts)
    free(p->discriminator_counts);

  for (i = 0; i < p->size; i++)
    if (p->profile[i])
      free(p->profile[i]);
  free(p->profile);

  for (i = 0; i < p->num_tables; i++)
    if (p->tables[i] != NULL)
      free(p->tables[i]);

  free(p->tables);

  if (p->t != NULL)
    zap_term(p->t);
  if (p->comments != NULL)
    zap_term(p->comments);
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

  if (p->comments) {
    Term comment = p->comments;
    while (cons_term(comment)) {
      fprintf(fp, "%% ");
      fwrite_term(fp, ARG(comment,0));
      fprintf(fp, "\n");
      comment = ARG(comment,1);
    }
  }

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
 *   compute_args()
 *
 *************/

static
void compute_args(int *a, int arity, int n, int i)
{
  int x = i;
  int r;
  for (r = arity-1; r >= 0; r--) {
    a[r] = x % n;
    x = x - a[r];
    x = x / n;
  }
}  /* compute_args */

/*************
 *
 *   fprint_interp_xml()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in a form that might be useful as input to LaTeX.
*/

/* PUBLIC */
void fprint_interp_xml(FILE *fp, Interp p)
{
  int n = p->size;
  int i;

  fprintf(fp, "\n  <interp size=\"%d\"", n);

  if (p->comments && !nil_term(p->comments)) {
    Term comment = p->comments;
    while (cons_term(comment)) {
      Term pair  = ARG(comment,0);
      char *name  = sn_to_str(SYMNUM(ARG(pair,0)));
      char *value = sn_to_str(SYMNUM(ARG(pair,1)));
      fprintf(fp, " %s=\"%s\"", name, value);
      comment = ARG(comment,1);
    }
  }
  
  fprintf(fp, ">\n");

  for (i = 0; i < p->num_tables; i++) {  /* arity 0 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    char *type = p->types[i] == FUNCTION ? "function" : "relation";
    if (table != NULL && arity == 0) {
      fprintf(fp, "\n    <op0 type=\"%s\">\n", type);
      fprintf(fp, "      <sym><![CDATA[%s]]></sym>\n", sn_to_str(i));
      fprintf(fp, "      <v>%d</v>\n", table[0]);
      fprintf(fp, "    </op0>\n");
    }
  }
  
  for (i = 0; i < p->num_tables; i++) {  /* arity 1 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i); 
    char *type = p->types[i] == FUNCTION ? "function" : "relation";
    if (table != NULL && arity == 1) {
      int j;
      fprintf(fp, "\n    <op1 type=\"%s\">\n", type);
      fprintf(fp, "        <sym><![CDATA[%s]]></sym>\n", sn_to_str(i));
      fprintf(fp, "        <head>");
      for (j = 0; j < n; j++)
	fprintf(fp, "<i>%d</i>", j);
      fprintf(fp, "</head>\n");

      fprintf(fp, "        <row> ");
      for (j = 0; j < n; j++)
	fprintf(fp, "<v>%d</v>", table[j]);
      fprintf(fp, "</row>\n");
      fprintf(fp, "    </op1>\n");
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 2 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    char *type = p->types[i] == FUNCTION ? "function" : "relation";
    if (table != NULL && arity == 2) {
      int j, k;
      fprintf(fp, "\n    <op2 type=\"%s\">\n", type);
      fprintf(fp, "        <sym><![CDATA[%s]]></sym>\n", sn_to_str(i));
      fprintf(fp, "        <head>        ");
      for (j = 0; j < n; j++)
	fprintf(fp, "<i>%d</i>", j);
      fprintf(fp, "</head>\n");

      for (j = 0; j < n; j++) {
	fprintf(fp, "        <row><i>%d</i> ", j);
	for (k = 0; k < n; k++)
	  fprintf(fp, "<v>%d</v>", table[(n*j) + k]);
	fprintf(fp, "</row>\n");
      }
      fprintf(fp, "    </op2>\n");
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity > 2 */
    int *table = p->tables[i];
    int arity = sn_to_arity(i);
    char *type = p->types[i] == FUNCTION ? "function" : "relation";
    if (table != NULL && arity > 2) {
      int *a = malloc(arity * sizeof(int));
      int m = int_power(p->size, arity);
      int j;
      fprintf(fp, "\n    <opn type=\"%s\" arity=\"%d\">\n", type, arity);
      fprintf(fp, "      <sym><![CDATA[%s]]></sym>\n", sn_to_str(i));
      for (j = 0; j < m; j++) {
	int k;
	compute_args(a, arity, p->size, j);
	fprintf(fp, "      <tupval> <tup>");
	for (k = 0; k < arity; k++)
	  fprintf(fp, "<i>%d</i>", a[k]);
	fprintf(fp, "</tup>   <v>%d</v> </tupval>\n", table[j]);
      }
      free(a);
      fprintf(fp, "    </opn>\n");
    }
  }
  fprintf(fp, "  </interp>\n");
}  /* fprint_interp_xml */

/*************
 *
 *   fprint_interp_standard()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in standard form, with each operation on a separate line.
*/

/* PUBLIC */
void fprint_interp_standard(FILE *fp, Interp p)
{
  int i;

  fprintf(fp, "interpretation( %d, ", p->size);

  if (p->comments) {
    fwrite_term(fp, p->comments);
    fprintf(fp, ", [\n");
  }
  else
    fprintf(fp, "[], [\n");
  
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
      for (j = 0; j < n; j++) {
	if (table[j] == -1)
	  fprintf(fp, "-%s"           , j == n-1 ? "])" : ",");
	else
	  fprintf(fp, "%d%s", table[j], j == n-1 ? "])" : ",");
      }

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL)
	fprintf(fp, ",\n");
    }
  }
  fprintf(fp, "]).\n");
}  /* fprint_interp_standard */

/*************
 *
 *   fprint_interp_standard2()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in standard form, with each operation on a separate line,
except that binary operations are printed on multiple lines.
*/

/* PUBLIC */
void fprint_interp_standard2(FILE *fp, Interp p)
{
  int i;

  fprintf(fp, "interpretation( %d, ", p->size);

  if (p->comments) {
    fwrite_term(fp, p->comments);
    fprintf(fp, ", [\n");
  }
  else
    fprintf(fp, "[], [\n");
  
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
	if (table[j] == -1)
	  fprintf(fp, w ? " -%s" : "-%s",
		  j == n-1 ? "])" : ",");
	else
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
    }
  }
  fprintf(fp, "]).\n");
}  /* fprint_interp_standard2 */

/*************
 *
 *   portable_indent()
 *
 *************/

static void portable_indent(FILE *fp, int n)
{
  int i;
  fprintf(fp, "      ");
  for (i = 0; i < n; i++)
    fprintf(fp, "  ");
}  /* portable_indent */

/*************
 *
 *   portable_recurse()
 *
 *************/

static
void portable_recurse(FILE *fp,
		       int arity, int domain_size,
		       int *table, int *idx_ptr, int depth)
{
  if (arity == 0)
    fprintf(fp, "%2d", table[(*idx_ptr)++]);
  else {
    int i;
    portable_indent(fp, depth);
    fprintf(fp, "[%s", arity > 1 ? "\n" : "");
    for (i = 0; i < domain_size; i++) {
      portable_recurse(fp, arity-1, domain_size, table, idx_ptr, depth+1);
      if (i < domain_size-1)
	fprintf(fp, ",");
      fprintf(fp, "%s", arity > 1 ? "\n" : "");
    }
    if (arity > 1)
      portable_indent(fp, depth);
    fprintf(fp, "]");
  }
}  /* portable_recurse */

/*************
 *
 *   fprint_interp_portable()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in portable form.
*/

/* PUBLIC */
void fprint_interp_portable(FILE *fp, Interp p)
{
  int i;
  Term t;

  fprintf(fp, "  [%d,\n    [", p->size);

  for (t = p->comments; t && !nil_term(t) ; t = ARG(t,1)) {
    fprintf(fp, " \"");
    fprint_term(fp, ARG(t,0));
    fprintf(fp, "\"%s ", nil_term(ARG(t,1)) ? "" : ",");
  }

  fprintf(fp, "],\n");
  fprintf(fp, "    [\n");
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j;
      int arity = sn_to_arity(i);
      int idx = 0;
      /* BOOL w = (arity == 2 && p->size > 10); */
      fprintf(fp,"      [\"%s\", \"%s\", %d,\n",
	      p->types[i] == FUNCTION ? "function" : "relation",
	      sn_to_str(i), arity);

      if (arity == 0)
	portable_indent(fp, 1);

      portable_recurse(fp, arity, p->size, table, &idx, 1);

      fprintf(fp, "\n      ]");

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL)
	fprintf(fp, ",\n");
      else
	fprintf(fp, "\n    ]\n  ]");
    }
  }
}  /* fprint_interp_portable */

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
  fprint_interp_standard(stdout, p);
}  /* p_interp */

/*************
 *
 *   fprint_interp_cooked()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a compiled interpretation,
in cooked form, e.g., f(0,2)=1.
*/

/* PUBLIC */
void fprint_interp_cooked(FILE *fp, Interp p)
{
  int i;

  if (p->comments) {
    Term comment = p->comments;
    while (cons_term(comment)) {
      fprintf(fp, "%% ");
      fwrite_term(fp, ARG(comment,0));
      fprintf(fp, "\n");
      comment = ARG(comment,1);
    }
  }

  fprintf(fp, "\n%% Interpretation of size %d\n", p->size);

  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    BOOL function = (p->types[i] == FUNCTION);
    if (table != NULL) {
      int j, n;
      int arity = sn_to_arity(i);

      fprintf(fp, "\n");
      if (arity == 0) {
	if (table[0] == -1)
	  fprintf(fp, "%s = -.\n", sn_to_str(i));
	else
	  fprintf(fp, "%s = %d.\n", sn_to_str(i), table[0]);
      }
      else {
	int *a = malloc(arity * sizeof(int));
	n = int_power(p->size, arity);
	for (j = 0; j < n; j++) {
	  int k;
	  compute_args(a, arity, p->size, j);
	  if (function) {
	    fprintf(fp, "%s(", sn_to_str(i));
	    for (k = 0; k < arity; k++)
	      fprintf(fp, "%d%s", a[k], k == arity-1 ? "" : ",");
	    if (table[j] == -1)
	      fprintf(fp, ") = -.\n");
	    else
	      fprintf(fp, ") = %d.\n", table[j]);
	  }
	  else {
	    fprintf(fp, "%s %s(", table[j] ? " " : not_sym(), sn_to_str(i));
	    for (k = 0; k < arity; k++)
	      fprintf(fp, "%d%s", a[k], k == arity-1 ? "" : ",");
	    fprintf(fp, ").\n");
	  }
	}
      
	free(a);
      }
    }
  }
}  /* fprint_interp_cooked */

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

  if (p->comments) {
    Term comment = p->comments;
    while (cons_term(comment)) {
      fprintf(fp, "%% ");
      fwrite_term(fp, ARG(comment,0));
      fprintf(fp, "\n");
      comment = ARG(comment,1);
    }
  }

  fprintf(fp, "\n%% Interpretation of size %d\n", p->size);

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
	for (i = 0; i < n; i++) {
	  if (table[i] == -1)
	    fprintf(fp, " -");
	  else
	    fprintf(fp, "%2d", table[i]);
	}
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
	  for (j = 0; j < n; j++) {
	    if (table[I2(n,i,j)] == -1)
	      fprintf(fp, " -");
	    else
	      fprintf(fp, "%2d", table[I2(n,i,j)]);
	  }
	}
	fprintf(fp, "\n");
      }
      else {
	int m = int_power(n, arity);
	fprintf(fp, "[");
	for (i = 0; i < m; i++) {
	  if (table[i] == -1)
	    fprintf(fp, "-%s", i == m-1 ? "]\n" : ",");
	  else
	    fprintf(fp, "%d%s", table[i], i == m-1 ? "]\n" : ",");
	}
      }
    }
  }  /* for each function or relation */
}  /* fprint_interp_tabular */

/*************
 *
 *   fprint_interp_raw()
 *
 *************/

/* DOCUMENTATION
This routine pretty prints (to FILE *fp) an interpretation in raw form.
*/

/* PUBLIC */
void fprint_interp_raw(FILE *fp, Interp p)
{
  int f, i;

  if (p->comments) {
    Term comment = p->comments;
    while (cons_term(comment)) {
      fprintf(fp, "%% ");
      fwrite_term(fp, ARG(comment,0));
      fprintf(fp, "\n");
      comment = ARG(comment,1);
    }
  }

  fprintf(fp, "\n%% Interpretation of size %d\n", p->size);

  for (f = 0; f < p->num_tables; f++) {
    int *table = p->tables[f];
    if (table != NULL) {
      int n = p->size;
      int arity = sn_to_arity(f);
      int m = int_power(n, arity);
      BOOL function = (p->types[f] == FUNCTION);
      
      fprintf(fp, "\n%% %s %s / %d : \n\n", function ? "Function" : "Relation",
	      sn_to_str(f), arity);

      for (i = 0; i < m; i++) {
	if (table[i] == -1)
	  fprintf(fp, "  -");
	else
	  fprintf(fp, " %2d", table[i]);
	if (i % n == n-1)
	  fprintf(fp, "\n");
      }
      if (arity == 0)
	fprintf(fp, "\n");
    }
  }  /* for each function or relation */
}  /* fprint_interp_raw */

/*************
 *
 *   eval_term_ground()
 *
 *   This version is for clauses, where variables are represented as VARIABLEs.
 *   It works for terms and atoms.  All natural numbers are interpreted as
 *   domain values, and if any are out of range, a fatal error occurs.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
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
      for (i = ARITY(t)-1; i >= 0; i--) {
	int v = eval_term_ground(ARG(t,i), p, vals);
	j += v * mult;
	mult = mult * n;
      }
      return table[j];
    }
  }
}  /* eval_term_ground */

/*************
 *
 *   eval_literals_ground()
 *
 *   Given a ground clause and an interpretation,
 *   return "at least one literal is true in the interpretation".
 *
 *************/

static
BOOL eval_literals_ground(Literals lits, Interp p, int *vals)
{
  Literals lit;
  BOOL atom_val, true_literal;

  true_literal = FALSE;
  for (lit = lits; lit && !true_literal; lit = lit->next) {

    if (is_eq_symbol(SYMNUM(lit->atom)))
      atom_val = (eval_term_ground(ARG(lit->atom,0), p, vals) ==
		  eval_term_ground(ARG(lit->atom,1), p, vals));
    else
      atom_val = eval_term_ground(lit->atom, p, vals);

    true_literal = (lit->sign ? atom_val : !atom_val);
  }
  return true_literal;
}  /* eval_literals_ground */

/*************
 *
 *   all_recurse()
 *
 *************/

static
BOOL all_recurse(Literals lits, Interp p, int *vals, int nextvar, int nvars)
{
  if (nextvar == nvars)
    return eval_literals_ground(lits, p, vals);
  else if (vals[nextvar] >= 0)
    return all_recurse(lits, p, vals, nextvar+1, nvars);
  else {
    int i, rc;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      rc = all_recurse(lits, p, vals, nextvar+1, nvars);
      if (!rc)
	return FALSE;
    }
    vals[nextvar] = -1;
    return TRUE;
  }
}  /* all_recurse */

/*************
 *
 *   eval_literals()
 *
 *************/

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
in the clause (other than eq_sym(), which is always built in)
is absent from the interpetation.
*/

/* PUBLIC */
BOOL eval_literals(Literals lits, Interp p)
{
  int vals[MAX_VARS_EVAL];
  int nvars, i;
  BOOL rc;

  nvars = greatest_variable_in_clause(lits) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal_error("eval_literals: too many variables");

  for (i = 0; i < nvars; i++)
    vals[i] = -1;

  rc = all_recurse(lits, p, vals, 0, nvars);

  return rc;
}  /* eval_literals */

/*************
 *
 *   all_recurse2()
 *
 *************/

static
int all_recurse2(Literals lits, Interp p, int *vals, int nextvar, int nvars)
{
  if (nextvar == nvars) {
    return eval_literals_ground(lits, p, vals) ? 1 : 0;
  }
  else if (vals[nextvar] >= 0)
    return all_recurse2(lits, p, vals, nextvar+1, nvars);
  else {
    int i;
    int true_instances = 0;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      true_instances += all_recurse2(lits, p, vals, nextvar+1, nvars);
    }
    vals[nextvar] = -1;
    return true_instances;
  }
}  /* all_recurse2 */

/*************
 *
 *   eval_literals_true_instances()
 *
 *************/

/* DOCUMENTATION
This routine evaluates a clause in an interpretation.
The number of false instances is returned.f
The variables in the clause must be normal (0,1,2,...).
*/

/* PUBLIC */
int eval_literals_true_instances(Literals lits, Interp p)
{
  int vals[MAX_VARS_EVAL];
  int nvars, i, true_instances;

  nvars = greatest_variable_in_clause(lits) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal_error("eval_literals_true_instances: too many variables");

  for (i = 0; i < nvars; i++)
    vals[i] = -1;

  true_instances = all_recurse2(lits, p, vals, 0, nvars);

  return true_instances;
}  /* eval_literals_true_instances */

/*************
 *
 *   eval_literals_false_instances()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int eval_literals_false_instances(Literals lits, Interp p)
{
  int true_instances = eval_literals_true_instances(lits, p);
  int nvars = greatest_variable_in_clause(lits) + 1;
  return int_power(p->size, nvars) - true_instances;
}  /* eval_literals_false_instances */

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
      for (i = ARITY(t)-1; i >= 0; i--) {
	int v = eval_fterm_ground(ARG(t,i), p, vals);
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
      return (eval_fterm_ground(ARG(f->atom,0), p, vals) ==
	      eval_fterm_ground(ARG(f->atom,1), p, vals));
    else
      return eval_fterm_ground(f->atom, p, vals);
  }
  else if (f->type == ALL_FORM) {
    /* ok if true for every element of domain */
    int i;
    BOOL ok = TRUE;
    int sn = str_to_sn(f->qvar, 0);
    int saved_value = vals[sn];  /* in case in scope of this variable */
    for (i = 0; i < p->size && ok; i++) {
      vals[sn] = i;
      if (!eval_form(f->kids[0], p, vals))
	ok = FALSE;
    }
    vals[sn] = saved_value;
    return ok;
  }
  else if (f->type == EXISTS_FORM) {
    /* ok if true for any element of domain */
    int i;
    BOOL ok = FALSE;
    int sn = str_to_sn(f->qvar, 0);
    int saved_value = vals[sn];  /* in case in scope of this variable */
    for (i = 0; i < p->size && !ok; i++) {
      vals[sn] = i;
      if (eval_form(f->kids[0], p, vals))
	ok = TRUE;
    }
    vals[sn] = saved_value;
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
This routine evaluates a closed formula in an interpretation.
<P>
All natural numbers are interpreted as domain values, and if any
domain values are out of range, a fatal error occurs.
<P>
A fatal error occurs if any constant, function or predicate symbol
in the formula (other than eq_sym(), which is always built in)
is absent from the interpetation.
<p>
A fatal error occurs if the formula contains any terms of type
VARIABLE.  (Variables bound by quantifier are represented as terms
of type CONSTANT.)
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
 *   interp_remove_constants_recurse()
 *
 *************/

/* DOCUMENTATION
In a non-compiled interpretation, remove all constants.
*/

/* PUBLIC */
Term interp_remove_constants_recurse(Term ops)
{
  if (nil_term(ops))
    return ops;
  else {
    if (sn_to_arity(SYMNUM(ARG(ARG(ops,0),0))) == 0) {
      zap_term(ARG(ops,0));  /* deep */
      free_term(ops);        /* shallow */
      return interp_remove_constants_recurse(ARG(ops,1));
    }
    else {
      ARG(ops,1) = interp_remove_constants_recurse(ARG(ops,1));
      return ops;
    }
  }
}  /* interp_remove_constants_recurse */

/*************
 *
 *   interp_remove_constants()
 *
 *************/

/* DOCUMENTATION
In a non-compiled interpretation, remove all constants.
*/

/* PUBLIC */
void interp_remove_constants(Term t)
{
  ARG(t,2) = interp_remove_constants_recurse(ARG(t,2));
}  /* interp_remove_constants */

/*************
 *
 *   interp_remove_others_recurse()
 *
 *************/

/* DOCUMENTATION
In a non-compiled interpretation, remove all others.
*/

/* PUBLIC */
Term interp_remove_others_recurse(Term ops, Plist keepers)
{
  if (nil_term(ops))
    return ops;
  else {
    if (!string_member_plist(sn_to_str(SYMNUM(ARG(ARG(ops,0),0))), keepers)) {
      zap_term(ARG(ops,0));  /* deep */
      free_term(ops);        /* shallow */
      return interp_remove_others_recurse(ARG(ops,1), keepers);
    }
    else {
      ARG(ops,1) = interp_remove_others_recurse(ARG(ops,1), keepers);
      return ops;
    }
  }
}  /* interp_remove_others_recurse */

/*************
 *
 *   interp_remove_others()
 *
 *************/

/* DOCUMENTATION
In a non-compiled interpretation, remove all others.
*/

/* PUBLIC */
void interp_remove_others(Term t, Plist keepers)
{
  ARG(t,2) = interp_remove_others_recurse(ARG(t,2), keepers);
}  /* interp_remove_others */

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
  int i, j;
  Interp q = get_interp();

  q->t = copy_term(p->t);
  q->comments = copy_term(p->comments);
  q->size = p->size;
  q->incomplete = p->incomplete;
  q->num_tables = p->num_tables;

  /* discriminators */

  q->num_discriminators = p->num_discriminators;
  q->discriminator_counts = malloc(sizeof(int) * q->num_discriminators);
  for (i = 0; i < q->num_discriminators; i++)
    q->discriminator_counts = p->discriminator_counts;
  
  /* occurrences */

  q->occurrences = malloc(q->size * sizeof(int));
  for (i = 0; i < q->size; i++)
    q->occurrences[i] = p->occurrences[i];

  /* blocks */

  q->blocks = malloc(q->size * sizeof(int));
  for (i = 0; i < q->size; i++)
    q->blocks[i] = p->blocks[i];

  /* profile */

  q->num_profile_components = p->num_profile_components;
  q->profile = malloc(sizeof(int *) * q->size);
  for (i = 0; i < q->size; i++) {
    q->profile[i] = malloc(sizeof(int) * q->num_profile_components);
    for (j = 0; j < q->num_profile_components; j++)
      q->profile[i][j] = p->profile[i][j];
  }

  /* types, arities */

  q->types = malloc(q->num_tables * sizeof(int));
  q->arities = malloc(q->num_tables * sizeof(int));
  for (i = 0; i < q->num_tables; i++) {
    q->types[i] = p->types[i];
    q->arities[i] = p->arities[i];
  }

  /* tables */

  q->tables = malloc(q->num_tables * sizeof(int *));
  for (i = 0; i < q->num_tables; i++)
    q->tables[i] = NULL;

  for (i = 0; i < q->num_tables; i++)
    if (p->tables[i] != NULL) {
      int arity = p->arities[i];
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
      int arity = source->arities[f];
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
    for (i = 0; i < n; i++) {
      dest->occurrences[p[i]] = source->occurrences[i];
      copy_vec(source->profile[i],
	       dest->profile[p[i]],
	       source->num_profile_components);
    }
  }
  /* The term representation is no longer correct. */
  if (dest->t)
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
      int arity = a->arities[f];
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
 *   normal_interp()
 *
 *************/

/* DOCUMENTATION
This routine returns a normalized copy of an interpretation.
Normalized interpretations are used to speed up isomorphism checking.
<P>
Consider, in all of the function tables, the number of occurrences of each
element.  If we have a size-4 interpretation with a binary function,
a unary function, and a constant, the occurrence array might be
something like [7,6,4,4], meaning that there are 7 occurrences of
0, 6, occurrences of 1, 4 occurrences of 2, and 4 occurrences of 3.
If the occurrence array is nonincreasing, the interpretation is in
a normal form.  (Normal forms are not unique; otherwise,
isomorphism checking would be trivial.)
<P>
If 2 normal interpretations have different occurrence arrays,
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
Interp normal_interp(Interp a)
{
  int i;
  int *occ  = malloc(sizeof(int) * a->size);  /* remember to free this */
  int *perm = malloc(sizeof(int) * a->size);  /* remember to free this */
  int size = a->size;
  Interp can;

  /* Determine the permutation we'll use to normalize
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
}  /* normal_interp */

/*************
 *
 *   iso_interp_recurse()
 *
 *************/

static
BOOL iso_interp_recurse(int *p, int k, int n,
			Interp a, Interp b, BOOL normal)
{
  int i;
  if (k == n) {
    /* We have a permutation. */
    Iso_perms++;
    return ident_interp_perm(a, b, p);
  }
  else {
    /* Continue building permutations. */
    if (iso_interp_recurse(p, k+1, n, a, b, normal))
      return TRUE;
    for (i = k+1; i < n; i++) {
      /* If normal, and if i and k are in the same block,
	 don't swap them. */
      if (!normal || a->blocks[i] == a->blocks[k]) {
	ISWAP(p[k], p[i]);
	if (iso_interp_recurse(p, k+1, n, a, b, normal))
	  return TRUE;
	ISWAP(p[k], p[i]);
      }
    }
    return FALSE;
  }
}  /* iso_interp_recurse */

/*************
 *
 *   isomorphic_normal_interps()
 *
 *************/

/* DOCUMENTATION
Are interpretations A and B isomorphic?  We assume they are
compatible (same operations/arities).  If the flag normal
is set, it is assumed that both interps were produced by
normal_interp(); this allows some optimization.
*/

/* PUBLIC */
BOOL isomorphic_interps(Interp a, Interp b, BOOL normal)
{
  BOOL isomorphic;
  int *perm;

  if (a->size != b->size)
    return FALSE;

  /* If normal, make sure the interps have the same profiles. */

  if (normal) {
    if (!same_profiles(a, b))
      return FALSE;
  }

  if (!same_discriminator_counts(a, b))
    return FALSE;

  Iso_checks++;

  perm = trivial_permutation(a->size);  /* remember to free this */

  isomorphic = iso_interp_recurse(perm, 0, a->size, a, b, normal);
  free(perm);
  return isomorphic;
}  /* isomorphic_normal_interps */

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
 *   interp_comments()
 *
 *************/

/* DOCUMENTATION
Return the comments of an interpretation.   
*/

/* PUBLIC */
Term interp_comments(Interp a)
{
  return a->comments;
}  /* interp_comments */

/*************
 *
 *   interp_table()
 *
 *************/

/* DOCUMENTATION
Given a symbol and arity, return the corresponding table.
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
Return the number of isomorphism checks.  For normal checks,
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

/*************
 *
 *   evaluable_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_term(Term t, Interp p)
{
  if (VARIABLE(t))
    return TRUE;
  else {
    int domain_element;
    if (CONSTANT(t) && term_to_int(t, &domain_element))
      return domain_element >= 0 && domain_element < p->size;
    else if (SYMNUM(t) >= p->num_tables || p->tables[SYMNUM(t)] == NULL) {
      return FALSE;
    }
    else {
      int i;
      BOOL ok;
      for (i = 0, ok = TRUE; i < ARITY(t) && ok; i++)
	ok = evaluable_term(ARG(t,i), p);
      return ok;
    }
  }
}  /* evaluable_term */

/*************
 *
 *   evaluable_atom()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_atom(Term a, Interp p)
{
  if (is_eq_symbol(SYMNUM(a)))
    return evaluable_term(ARG(a,0), p) && evaluable_term(ARG(a,1), p);
  else {
    int b;
    if (CONSTANT(a) && term_to_int(a, &b))
      return b == 0 || b == 1;
    else if (SYMNUM(a) >= p->num_tables || p->tables[SYMNUM(a)] == NULL) {
      return FALSE;
    }
    else {
      int i;
      BOOL ok;
      for (i = 0, ok = TRUE; i < ARITY(a) && ok; i++)
	ok = evaluable_term(ARG(a,i), p);
      return ok;
    }
  }
}  /* evaluable_atom */

/*************
 *
 *   evaluable_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_literals(Literals lits, Interp p)
{
  if (lits == NULL)
    return TRUE;
  else
    return (evaluable_atom(lits->atom, p) &&
	    evaluable_literals(lits->next, p));
}  /* evaluable_literals */

/*************
 *
 *   evaluable_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_formula(Formula f, Interp p)
{
  if (f->type == ATOM_FORM)
    return evaluable_atom(f->atom, p);
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (!evaluable_formula(f->kids[i], p))
	return FALSE;
    return TRUE;
  }
}  /* evaluable_formula */

/*************
 *
 *   evaluable_topform()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL evaluable_topform(Topform tf, Interp p)
{
  if (tf->is_formula)
    return evaluable_formula(tf->formula, p);
  else
    return evaluable_literals(tf->literals, p);
}  /* evaluable_topform */

/*************
 *
 *   update_interp_with_constant()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void update_interp_with_constant(Interp p, Term constant, int val)
{
  int sn = SYMNUM(constant);

  printf("NOTE: sn=%d, num_tables=%d\n", sn, p->num_tables);

  if (sn >= p->num_tables)
    fatal_error("update_interp_with_constat, not enough tables");
  else if (p->tables[sn] != NULL)
    fatal_error("update_interp_with_constat, table not NULL");
  else {
    p->tables[sn] = malloc(sizeof(int));
    p->tables[sn][0] = val;
  }
}  /* update_interp_with_constant */

/*************
 *
 *   eval_topform()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL eval_topform(Topform tf, Interp p)
{
  if (tf->is_formula)
    return eval_formula(tf->formula, p);
  else
    return eval_literals(tf->literals, p);
}  /* eval_topform */

/*************
 *
 *   compare_interp()
 *
 *************/

/* DOCUMENTATION
Compare two compatible interpretations.
Return SAME_AS, GREATER_THAN, or LESS_THAN.
*/

/* PUBLIC */
Ordertype compare_interp(Interp a, Interp b)
{
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *at =   a->tables[f];
      int *bt =   b->tables[f];
      int n = int_power(a->size, a->arities[f]);
      int i;
      for (i = 0; i < n; i++) {
	if (at[i] < bt[i])
	  return LESS_THAN;
	else if (at[i] > bt[i])
	  return GREATER_THAN;
      }
    }
  }
  return TRUE;
}  /* compare_interp */

/*************
 *
 *   ident_interp()
 *
 *************/

/* DOCUMENTATION
Are interpretations A and  B identical?
It is assumed that A and B
are the same size and have the same symbols.
*/

/* PUBLIC */
BOOL ident_interp(Interp a, Interp b)
{
  return compare_interp(a,b) == SAME_AS;
}  /* ident_interp */

static
Ordertype compare_ints(int a, int b)
{
  if (a < b)
    return LESS_THAN;
  else if (a > b)
    return GREATER_THAN;
  else
    return SAME_AS;
}  /* compare_ints */

static
void invert_perm(int *a, int *b, int n)
{
  int i;
  for (i = 0; i < n; i++)
    b[a[i]] = i;
}  /* invert_perm */

static
void copy_perm(int *a, int *b, int n)
{
  int i;
  for (i = 0; i < n; i++)
    b[i] = a[i];
}  /* copy_perm */

/*************
 *
 *   compare_permed_interps()
 *
 *   Compare two permutations of an interpretation.
 *
 *   x  : perm1
 *   y  : perm2
 *   xx : inverse(perm1)
 *   yy : inverse(perm2)
 *   a  : interpretation
 *
 *************/

static
Ordertype compare_permed_interps(int *x, int *y, int *xx, int *yy, Interp a)
{
  int n = a->size;
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *t =   a->tables[f];
      int arity = a->arities[f];
      BOOL function = (a->types[f] == FUNCTION);
      if (arity == 0) {
	Ordertype result;
	if (function)
	  result = compare_ints(x[t[0]], y[t[0]]);
	else
	  result = SAME_AS;
	if (result != SAME_AS)
	  return result;
      }
      else if (arity == 1) {
	int i;
	for (i = 0; i < n; i++) {
	  Ordertype result;
	  if (function)
	    result = compare_ints(x[t[xx[i]]], y[t[yy[i]]]);
	  else
	    result = compare_ints(t[xx[i]], t[yy[i]]);
	  if (result != SAME_AS)
	    return result;
	}
      }
      else if (arity == 2) {
	int i, j;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++) {
	    Ordertype result;
	    if (function)
	      result = compare_ints(x[t[I2(n,xx[i],xx[j])]],
				    y[t[I2(n,yy[i],yy[j])]]);
	    else
	      result = compare_ints(t[I2(n,xx[i],xx[j])],
				    t[I2(n,yy[i],yy[j])]);
	    if (result != SAME_AS)
	      return result;
	  }
      }
      else if (arity == 3) {
	int i, j, k;
	for (i = 0; i < n; i++)
	  for (j = 0; j < n; j++)
	    for (k = 0; k < n; k++) {
	      Ordertype result;
	      if (function)
		result = compare_ints(x[t[I3(n,xx[i],xx[j],xx[k])]],
				      y[t[I3(n,yy[i],yy[j],yy[k])]]);
	      else
		result = compare_ints(t[I3(n,xx[i],xx[j],xx[k])],
				      t[I3(n,yy[i],yy[j],yy[k])]);
	      if (result != SAME_AS)
		return result;
	    }
      }
      else
	fatal_error("compare_permed_interps: arity > 3");
    }
  }
  return SAME_AS;
}  /* compare_permed_interps */

/*************
 *
 *   canon_recurse()
 *
 *************/

static
void canon_recurse(int k, int *perm, int *best, int *perm1, int *best1,
		   Interp a)
{
  /* 
     k: current position in working permutation
     perm:  working permutation
     best:  best permutation so far
     perm1: inverse of perm
     best1: inverse of best
     a: base interp
   */
  int n = a->size;
  if (k == n) {
    /* We have a permutation. */
    Iso_perms++;
    invert_perm(perm, perm1, a->size);
    if (compare_permed_interps(perm, best, perm1, best1, a) == LESS_THAN) {
      /* copy working permutation to best-so-far. */
      copy_perm(perm, best, a->size);
      copy_perm(perm1, best1, a->size);
    }
  }
  else {
    /* Continue building permutations. */
    int i;
    canon_recurse(k+1, perm, best, perm1, best1, a);
    for (i = k+1; i < n; i++) {
      /* If i and k are in different blocks, don't swap them. */
      if (a->blocks[i] == a->blocks[k]) {
	ISWAP(perm[k], perm[i]);
	canon_recurse(k+1, perm, best, perm1, best1, a);
	ISWAP(perm[k], perm[i]);
      }
    }
  }
}  /* canon_recurse */

/*************
 *
 *   canon_interp()
 *
 *************/

/* DOCUMENTATION
Return the (unique) canonical form of the interp.
The input interp (which is not changed) is assumed
to be in normal form.
*/

/* PUBLIC */
Interp canon_interp(Interp a)
{
  Interp canon;

  int *perm  = trivial_permutation(a->size);  /* remember to free this */
  int *best  = trivial_permutation(a->size);  /* remember to free this */
  int *perm1 = trivial_permutation(a->size);  /* remember to free this */
  int *best1 = trivial_permutation(a->size);  /* remember to free this */

  invert_perm(best, best1, a->size);  /* let best1 be the inverse of best */
  /* perm gets inverted when needed */

  canon_recurse(0, perm, best, perm1, best1, a);

  canon = permute_interp(a, best);  /* makes new copy */

  free(perm);
  free(best);
  free(perm1);
  free(best1);

  return canon;
}  /* canon_interp */

/*************
 *
 *   assign_discriminator_counts()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void assign_discriminator_counts(Interp a, Plist discriminators)
{
  int n = plist_count(discriminators);
  int *counts = malloc(sizeof(int) * n);
  int i;
  Plist p;
  for (p = discriminators, i = 0; p; p = p->next, i++) {
    Topform c = p->v;
    counts[i] = eval_literals_true_instances(c->literals, a);
  }
  a->discriminator_counts = counts;
  a->num_discriminators = n;
}  /* assign_discriminator_counts */

/*************
 *
 *   same_discriminator_counts()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL same_discriminator_counts(Interp a, Interp b)
{
  if (a->num_discriminators != b->num_discriminators)
    fatal_error("different number of discriminators");
  return (compare_vecs(a->discriminator_counts,
		       b->discriminator_counts,
		       a->num_discriminators) == SAME_AS);
}  /* same_discriminator_counts */

/*************
 *
 *   update_profile()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void update_profile(Topform c, Interp a, int *next)
     /* vecs[domain_element][profile_component] */
      
{
  int vals[MAX_VARS_EVAL];
  int nvars, v, i, true_instances;

  nvars = greatest_variable_in_clause(c->literals) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal_error("update_profile: too many variables");

  for (v = 0; v < nvars; v++)
    vals[v] = -1;

  for (v = 0; v < nvars; v++) {
    for (i = 0; i < a->size; i++) {
      vals[v] = i;
      true_instances = all_recurse2(c->literals, a, vals, 0, nvars);
      a->profile[i][*next] = true_instances;
    }
    vals[v] = -1;
    (*next)++;
  }
}  /* update_profile */

/*************
 *
 *   create_profile()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void create_profile(Interp a, Plist discriminators)
{
  int i, next;
  Plist p;

  a->num_profile_components = 1;  /* first is occurrences */

  for (p = discriminators; p; p = p->next) {
    Topform c = p->v;
    int nvars = greatest_variable_in_clause(c->literals) + 1;
    a->num_profile_components += nvars;
  }
  for (i = 0; i < a->size; i++) {
    a->profile[i] = malloc(sizeof(int) * a->num_profile_components);
    a->profile[i][0] = a->occurrences[i];
  }

  next = 1;
  for (p = discriminators; p; p = p->next)
    update_profile(p->v, a, &next);

  if (next != a->num_profile_components)
    fatal_error("create_profile, counts do not match");
}  /* create_profile */

/*************
 *
 *   p_interp_profile()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_interp_profile(Interp a, Plist discriminators)
{
  int i, j, k;
  char str[10];
  Plist p;
  printf("\n========================== PROFILE\n");
  fprint_interp_standard2(stdout, a);
  if (discriminators) {
    printf("\n     blocks:             ");
    for (k = 0; k < a->size; k++)
      printf(" %3c", 'A' + a->blocks[k]);
    printf("\n");
    printf("Permutations: %lu\n", perms_required(a));
    printf("occurrences:             ");
    for (k = 0; k < a->size; k++)
      printf(" %3d", a->profile[k][0]);
    printf("\n");

    i = 1;
    for (p = discriminators; p; p = p->next) {
      Topform c = p->v;
      int nvars = greatest_variable_in_clause(c->literals) + 1;
      fwrite_clause(stdout, c, CL_FORM_BARE);
      for (j = 0; j < nvars; j++, i++) {
	symbol_for_variable(str, j);
	printf("          %s:             ", str);
	for (k = 0; k < a->size; k++) {
	  printf(" %3d", a->profile[k][i]);
	}
	printf("\n");
      }
    }
  }
  else {
    for (i = 0; i < a->num_profile_components; i++) {
      for (k = 0; k < a->size; k++)
	printf(" %2d", a->profile[k][i]);
      printf("\n");
    }
  }
}  /* p_interp_profile */

/*************
 *
 *   normal3_interp()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Interp normal3_interp(Interp a, Plist discriminators)
{
  int i, b;
  int **prof = malloc(sizeof(int *) * a->size);  /* remember to free this */
  int *perm  = malloc(sizeof(int) * a->size);  /* remember to free this */
  int size = a->size;
  Interp norm;

  create_profile(a, discriminators);

  /* Determine the permutation we'll use to normalize
     the interpretation. */

  for (i = 0; i < size; i++)
    prof[i] = a->profile[i];

  for (i = 0; i < size; i++) {
    int *max = NULL;
    int index_of_max = -1;
    int j;
    for (j = 0; j < size; j++) {
      if (prof[j] != NULL) {
	if (max == NULL ||
	    compare_vecs(prof[j],
			 max, a->num_profile_components) == GREATER_THAN) {
	  index_of_max = j;
	  max = prof[j];
	}
      }
    }
    perm[index_of_max] = i;
    prof[index_of_max] = NULL;
  }

  free(prof);  /* This is now useless (all members are NULL). */

  /* Apply the permutation to the interpretation. */

  norm = permute_interp(a, perm);
  free(perm);

  /* Set up blocks of identical profile components.  Note that
     identical profile components are alreay adjacent.
     The blocks are specified by a vector of integers, e.g.,
     [1,1,1,1,2,3,3,4] says there are 4 blocks (domain size 8),
     with the first 4 identical, etc.
  */

  b = 0;  /* block counter */

  norm->blocks[0] = 0;
  for (i = 1; i < norm->size; i++) {
    if (compare_vecs(norm->profile[i-1], norm->profile[i],
		     norm->num_profile_components) != SAME_AS)
      b++;
    norm->blocks[i] = b;
  }
  return norm;
}  /* normal3_interp */

/*************
 *
 *   same_profiles()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL same_profiles(Interp a, Interp b)
{
  int i, j;
  for (i = 0; i < a->size; i++)
    for (j = 0; j < a->num_profile_components; j++)
      if (a->profile[i][j] != b->profile[i][j])
	return FALSE;
  return TRUE;
}  /* same_profiles */

/*************
 *
 *   perms_required()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
long unsigned perms_required(Interp a)
{
  int i, n;
  long unsigned p, r;
  p = 1;
  i = 0;
  while (i < a->size) {
    int c = a->blocks[i];
    n = 0;
    while (i < a->size && a->blocks[i] == c) {
      i++;
      n++;
    }
    r = factorial(n);
    if (r < 1 || r == ULONG_MAX)
      return 0;
    r = p * r;
    if (r < p || r == ULONG_MAX)
      return 0;
    p = r;
  }
  return p;
}  /* perms_required */

/*************
 *
 *   factorial()
 *
 *************/

/* DOCUMENTATION
If overflow, return 0.
*/

/* PUBLIC */
long unsigned factorial(int n)
{
  long unsigned f, x;
  int i;
  f = 1;
  for (i = 1; i <= n; i++) {
    x = f * i;
    if (x == ULONG_MAX || x < f)
      return 0;
    f = x;
  }
  return f;
}  /* factorial */
