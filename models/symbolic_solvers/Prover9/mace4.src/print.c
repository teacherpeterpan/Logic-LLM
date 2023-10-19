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

#include "msearch.h"

#include "../ladr/banner.h"

extern Symbol_data Symbols;

extern int Number_of_cells;
extern struct cell *Cells;

extern Term *Domain;
extern int Domain_size;

extern int Negation_flag;
extern int Eq_sn;

/* Statistics for entire run */

extern Clock Mace4_clock;

extern unsigned Total_models;

extern struct mace_stats Mstats;

/*************
 *
 *   f[01234]_val()
 *
 *************/

static int id2val(int id)
{
  return (Cells[id].value == NULL ? -1 : VARNUM(Cells[id].value));
}  /* id2val */

static
int f0_val(int base)
{
  int id = X0(base);
  return id2val(id);
}  /* f0_val */

static
int f1_val(int base, int i)
{
  int id = X1(base,i);
  return id2val(id);
}  /* f1_val */

static
int f2_val(int base, int i, int j)
{
  int id = X2(base,i,j);
  return id2val(id);
}  /* f2_val */

/*************
 *
 *   p_model()
 *
 *************/

void p_model(BOOL print_head)
{
  Symbol_data p;
  int n = Domain_size;

  if (print_head) {
    print_separator(stdout, "MODEL", TRUE);
    printf("\n%% Model %d at %.2f seconds.\n",
	   Total_models, user_seconds());
  }
  
  for (p = Symbols; p != NULL; p = p->next) {
    char *name = sn_to_str(p->sn);
    if (p->attribute != EQUALITY_SYMBOL) {
      /* This prints both relations and functions. */
      if (p->arity == 0) {
	int v = f0_val(p->base);
	if (v < 0)
	  printf("\n %s : -\n", name);
	else
	  printf("\n %s : %d\n", name, v);
      }
      else if (p->arity == 1) {
	char *s1 = n <= 10 ? "%2d" : "%3d";
	char *s2 = n <= 10 ? "--"  : "---";
	char *s3 = n <= 10 ? " -"  : "  -";
	int i;
	for (i = 0; i < n; i++) {
	  printf("\n %s :\n", name);
	  printf("       ");
	  for (i = 0; i < n; i++)
	    printf(s1, i);
	  printf("\n    ---");
	  for (i = 0; i < n; i++)
	    printf(s2);
	  printf("\n       ");
	  for (i = 0; i < n; i++) {
	    int v = f1_val(p->base, i);
	    if (v < 0)
	      printf(s3);
	    else
	      printf(s1, v);
	  }
	  printf("\n");	  
	}
      }
      else if (p->arity == 2) {
	char *s1 = n <= 10 ? "%2d" : "%3d";
	char *s2 = n <= 10 ? "--"  : "---";
	char *s3 = n <= 10 ? " -"  : "  -";
	int i, j;
	printf("\n %s :\n", name);
	printf("      |");
	for (i = 0; i < n; i++)
	  printf(s1, i);
	printf("\n    --+");
	for (i = 0; i < n; i++)
	  printf(s2);
	printf("\n");

	for (i = 0; i < n; i++) {
	  printf("%5d |", i);
	  for (j = 0; j < n; j++) {
	    int v = f2_val(p->base, i, j);
	    if (v < 0)
	      printf(s3);
	    else
	      printf(s1, v);
	  }
	  printf("\n");
	}
      }
      else {
	int n = int_power(Domain_size, p->arity);
	int i;
	Variable_style save_style = variable_style();
	set_variable_style(INTEGER_STYLE);
	for (i = 0; i < n; i++) {
	  int id = p->base + i;
	  fwrite_term(stdout, Cells[id].eterm);
	  if (Cells[id].value == NULL)
	    printf(" = -.\n");
	  else
	    printf(" = %d.\n", VARNUM(Cells[id].value));
	}
	set_variable_style(save_style);
      }
    }
  }

  if (print_head)
    print_separator(stdout, "end of model", TRUE);

}  /* p_model */

/*************
 *
 *   print_model_standard()
 *
 *************/

void print_model_standard(FILE *fp, BOOL print_head)
{
  int syms_printed;
  Symbol_data s;

  if (print_head)
    print_separator(fp, "MODEL", TRUE);

  fprintf(fp, "\ninterpretation( %d, [number=%d, seconds=%d], [\n",
	  Domain_size, Total_models, (int) user_seconds());

  syms_printed = 0;

  for (s = Symbols; s != NULL; s = s->next) {
    if (s->attribute != EQUALITY_SYMBOL) {
      int i, n;
      if (syms_printed > 0)
	fprintf(fp, ",\n");

      fprintf(fp, "\n        %s(%s%s",
	      s->type == FUNCTION ? "function" : "relation",
	      sn_to_str(s->sn),
	      s->arity == 0 ? "" : "(_");
      for (i = 1; i < s->arity; i++)
	fprintf(fp, ",_");
      fprintf(fp,"%s, [%s",
	      s->arity == 0 ? "" : ")",
	      s->arity >= 2 ? "\n\t\t\t  " : "");
      n = int_power(Domain_size, s->arity);
      for (i = 0; i < n; i++) {
	int id = s->base + i;
	if (Cells[id].value == NULL)
	  fprintf(fp, "-");
	else
	  fprintf(fp, "%2d", VARNUM(Cells[id].value));
	if (i < n-1)
	  fprintf(fp, ",%s", (i+1) % Domain_size == 0 ? "\n\t\t\t  " : "");
	else
	  fprintf(fp, " ])");
      }
      syms_printed++;
    }
  }

  fprintf(fp, "\n]).\n");

  if (print_head)
    print_separator(fp, "end of model", TRUE);

}  /* print_model_standard */

/*************
 *
 *   interp_term()
 *
 *   Construct a term representing the current interpretation, e.g.
 *
 *   interpretation( 3, [
 *         function(B, [2]),
 *         function(g(_), [1,0,1])]).
 *
 *************/

Term interp_term(void)
{
  Symbol_data s;

  Term symlist = get_nil_term();

  for (s = Symbols; s != NULL; s = s->next) {
    if (s->attribute != EQUALITY_SYMBOL) {
      int i, n;
      Term entry, symterm, tableterm;
      symterm = get_rigid_term_dangerously(s->sn, s->arity);
      for (i = 0; i < s->arity; i++)
	ARG(symterm,i) = get_variable_term(i);
      n = int_power(Domain_size, s->arity);
      tableterm = get_nil_term();
      for (i = n-1; i >= 0; i--) {
	int id = s->base + i;
	Term it;
	if (Cells[id].value == NULL)
	  fatal_error("interp_term, incomplete interpretation");
	it = nat_to_term(VARNUM(Cells[id].value));
	tableterm = listterm_cons(it, tableterm);
      }
      entry = build_binary_term(str_to_sn(s->type == FUNCTION ? "function" : "relation", 2),
				symterm, tableterm);
      symlist = listterm_cons(entry, symlist);
    }
  }
  return build_binary_term(str_to_sn("interpretation", 2),
			   nat_to_term(Domain_size),
			   symlist);
}  /* interp_term */

/*************
 *
 *   p_matom()
 *
 *************/

void p_matom(Term atom)
{
  if (atom == NULL)
    printf("(NULL)");
  else if (!NEGATED(atom))
    fwrite_term(stdout, atom);
  else if (EQ_TERM(atom)){
    fwrite_term(stdout, ARG(atom,0));
    printf(" != ");
    fwrite_term(stdout, ARG(atom,1));
  }
  else {
    printf("~(");
    fwrite_term(stdout, atom);
    printf(")");
  }
  printf(".\n");
}  /* p_matom */

/*************
 *
 *   p_mclause()
 *
 *************/

void p_mclause(Mclause c)
{
  int i;
  printf("numlits=%d, active=%ld, subsumed=%d: ",
	 c->numlits, c->u.active, c->subsumed);
  for (i = 0; i < c->numlits; i++) {
    Term atom = LIT(c,i);
    if (!NEGATED(atom))
      fwrite_term(stdout, atom);
    else {
      printf("~(");
      fwrite_term(stdout, atom);
      printf(")");
    }
    if (i < c->numlits-1)
      printf(" | ");
    else
      printf(".\n");
  }
}  /* p_mclause */

/*************
 *
 *   p_eterms()
 *
 *************/

static
int eterms_count(Term t)
{
  return (t == NULL ? 0 : 1 + eterms_count(t->u.vp));
}

void p_eterms(void)
{
  int i, j;
  printf("\n------- Cells --------\n");
  for (i = 0; i < Number_of_cells; i++) {
    int n = eterms_count(Cells[i].occurrences);
    if (n > 0) {
      fwrite_term(stdout, Cells[i].occurrences);
      printf(": %d occ, id=%d, val=", n, i);
      if (Cells[i].value == NULL)
	printf("NULL");
      else
	fwrite_term(stdout, Cells[i].value);
      printf(", pvals=");
      for (j = 0; j < id_to_domain_size(i); j++) {
	if (Cells[i].possible[j] == NULL)
	  printf(" -");
	else
	  printf("%2d", j);
      }
      printf("\n");
    }
  }
}  /* p_eterms */

/*************
 *
 *   p_stats()
 *
 *************/

void p_stats(void)
{
  print_separator(stdout, "STATISTICS", TRUE);

  printf("\nFor domain size %d.\n\n",Domain_size);

  printf("Current CPU time: %.2f seconds ", clock_seconds(Mace4_clock));
  printf("(total CPU time: %.2f seconds).\n",user_seconds());
  printf("Ground clauses: seen=%u, kept=%u.\n",
	 Mstats.ground_clauses_seen, Mstats.ground_clauses_kept);
  printf("Selections=%u, assignments=%u, propagations=%u, current_models=%u.\n",
	 Mstats.selections, Mstats.assignments, Mstats.propagations, Mstats.current_models);
  printf("Rewrite_terms=%u, rewrite_bools=%u, indexes=%u.\n",
	 Mstats.rewrite_terms, Mstats.rewrite_bools, Mstats.indexes);
  printf("Rules_from_neg_clauses=%u, cross_offs=%d.\n",
	 Mstats.rules_from_neg, Mstats.cross_offs);
#if 0
  printf("Negative propagation:\n");
  printf("                 attempts      agone      egone\n");
  printf("Neg_elim        %10u %10u %10u\n",
	 Mstats.neg_elim_attempts, Mstats.neg_elim_agone, Mstats.neg_elim_egone);
  printf("Neg_assign      %10u %10u %10u\n",
	 Mstats.neg_assign_attempts, Mstats.neg_assign_agone, Mstats.neg_assign_egone);
  printf("Neg_near_elim   %10u %10u %10u\n",
	 Mstats.neg_near_elim_attempts, Mstats.neg_near_elim_agone, Mstats.neg_near_elim_egone);
  printf("Neg_near_assign %10u %10u %10u\n",
	 Mstats.neg_near_assign_attempts, Mstats.neg_near_assign_agone, Mstats.neg_near_assign_egone);
#endif
  print_separator(stdout, "end of statistics", TRUE);
}  /* p_stats */

/*************
 *
 *   p_mem()
 *
 *************/

void p_mem(void)
{
  printf("\n------------- memory usage (for entire run) -------------------\n");

  printf("\nTotal malloced: %d megabytes\n", megs_malloced());

  fprint_strbuf_mem(stdout, 1);
  fprint_parse_mem(stdout, 0);
  fprint_glist_mem(stdout, 0);
  fprint_term_mem(stdout, 0);
  fprint_topform_mem(stdout, 0);
  fprint_clist_mem(stdout, 0);
  fprint_mclause_mem(stdout, 0);
  fprint_mstate_mem(stdout, 0);
  fprint_estack_mem(stdout, 0);
  memory_report(stdout);
}  /* p_mem */

/*************
 *
 *   reset_current_stats()
 *
 *************/

void reset_current_stats(void)
{
  Mstats.current_models = 0;
  Mstats.selections = 0;
  Mstats.assignments = 0;
  Mstats.propagations = 0;
  Mstats.cross_offs = 0;
  Mstats.rewrite_terms = 0;
  Mstats.rewrite_bools = 0;
  Mstats.indexes = 0;
  Mstats.ground_clauses_seen = 0;
  Mstats.ground_clauses_kept = 0;
  Mstats.rules_from_neg = 0;

  Mstats.neg_elim_attempts = 0;
  Mstats.neg_elim_agone = 0;
  Mstats.neg_elim_egone = 0;

  Mstats.neg_assign_attempts = 0;
  Mstats.neg_assign_agone = 0;
  Mstats.neg_assign_egone = 0;

  Mstats.neg_near_assign_attempts = 0;
  Mstats.neg_near_assign_agone = 0;
  Mstats.neg_near_assign_egone = 0;

  Mstats.neg_near_elim_attempts = 0;
  Mstats.neg_near_elim_agone = 0;
  Mstats.neg_near_elim_egone = 0;
}  /* reset_current_stats */

