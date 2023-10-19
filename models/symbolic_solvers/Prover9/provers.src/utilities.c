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

#include "utilities.h"

/* Private definitions and types */

/*************
 *
 *   print_memory_stats()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void print_memory_stats(FILE *fp)
{
  fprintf(fp, "\nMegs malloced by palloc(): %d.\n", megs_malloced());
  
  fprint_strbuf_mem(fp, TRUE);
  fprint_parse_mem(fp, FALSE);
  fprint_hash_mem(fp, FALSE);
  fprint_term_mem(fp, FALSE);
  fprint_attrib_mem(fp, FALSE);
  fprint_glist_mem(fp, FALSE);

  fprint_just_mem(fp, FALSE);
  fprint_formula_mem(fp, FALSE);
  fprint_topform_mem(fp, FALSE);
  fprint_clist_mem(fp, FALSE);

  fprint_unify_mem(fp, FALSE);
  fprint_btm_mem(fp, FALSE);
  fprint_btu_mem(fp, FALSE);

  fprint_fpa_mem(fp, FALSE);
  fprint_fpalist_mem(fp, FALSE);
  fprint_discrim_mem(fp, FALSE);
  fprint_discrimb_mem(fp, FALSE);
  fprint_discrimw_mem(fp, FALSE);
  fprint_flatterm_mem(fp, FALSE);
  fprint_mindex_mem(fp, FALSE);
  fprint_lindex_mem(fp, FALSE);
  fprint_clash_mem(fp, FALSE);
  fprint_di_tree_mem(fp, FALSE);
  fprint_avltree_mem(fp, FALSE);

  memory_report(fp);
}  // print_memory_stats

/*************
 *
 *   fsym_collect()
 *
 *************/

static
void fsym_collect(Ilist *table, Term t, int depth)
{
  if (VARIABLE(t))
    return;
  else {
    int i;
    table[SYMNUM(t)] = ilist_prepend(table[SYMNUM(t)], depth);
    for (i = 0; i < ARITY(t); i++)
      fsym_collect(table, ARG(t,i), depth+1);
  }
}  /* fsym_collect */

/*************
 *
 *   inverse_axiom()
 *
 *************/

static
BOOL inverse_axiom(Topform c, int *f2, int *f1, int *f0)
{
  if (number_of_literals(c->literals) != 1 || !pos_eq(c->literals))
    return FALSE;
  else {
    Term alpha = ARG(c->literals->atom,0);
    Term beta  = ARG(c->literals->atom,1);
    if (CONSTANT(alpha))
      { Term t = alpha; alpha = beta; beta = t; }
    if (!CONSTANT(beta) || ARITY(alpha) != 2)
      return FALSE;
    else {
      Term a0 = ARG(alpha,0);
      Term a1 = ARG(alpha,1);
      if (VARIABLE(a0))
	{ Term t = a0; a0 = a1; a1 = t; }
      if (!VARIABLE(a1) || ARITY(a0) != 1)
	return FALSE;
      else if (!term_ident(ARG(a0,0),a1))
	return FALSE;
      else {
	*f2 = SYMNUM(alpha);
	*f1 = SYMNUM(a0);
	*f0 = SYMNUM(beta);
	return TRUE;
      }
    }
  }
}  /* inverse_axiom */

/*************
 *
 *   fsym_report()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void fsym_report(Ilist fsyms, Plist clauses)
{
  Ilist *table = calloc(greatest_symnum()+1  , sizeof(Ilist));
  Plist p;

  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    Literals lit;
    int f0, f1, f2;
    for (lit = c->literals; lit; lit = lit->next)
      fsym_collect(table, lit->atom, 0);
    if (inverse_axiom(c, &f2, &f1, &f0))
      { printf("Inverse axiom: "); f_clause(c); }
  }

  {
    Ilist a;
    printf("Symbols:\n");
    for (a = fsyms; a; a = a->next) {
      Ilist b;
      int n = 0;
      printf("  Symbol %22s/%d: ", sn_to_str(a->i), sn_to_arity(a->i));
      for (b = table[a->i]; b; b = b->next) {
	printf(" %d", b->i);
	n += b->i;
      }
      printf("   (%d)\n", n);
    }
  }
  free(table);
}  /* fsym_report */

/*************
 *
 *   inverse_order()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL inverse_order(Clist clauses)
{
  Clist_pos p;
  Ilist binary = NULL;
  Ilist unary = NULL;
  BOOL change;
    
  for (p = clauses->first; p; p = p->next) {
    int f2, f1, f0;
    if (inverse_axiom(p->c, &f2, &f1, &f0)) {
      binary = ilist_append(binary, f2);
      unary = ilist_append(unary, f1);
    }
  }
  change = (unary != NULL || binary != NULL);
  lex_insert_after_initial_constants(unary);
  lex_insert_after_initial_constants(binary);
  zap_ilist(unary);
  zap_ilist(binary);
  return change;
}  /* inverse_order */

/*************
 *
 *   p_sym_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_sym_list(Ilist syms)
{
  Ilist p;
  printf("[");
  for (p = syms; p; p = p->next) {
    printf("%s/%d%s", sn_to_str(p->i), sn_to_arity(p->i), p->next ? "," : "");
  }
  printf("]");
}  /* p_sym_list */

/*************
 *
 *   symbol_order()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void symbol_order(Clist usable, Clist sos, Clist demods, BOOL echo)
{
  Ilist fsyms, rsyms;
  I2list fsyms_multiset, rsyms_multiset;
  Plist nonneg;

  Plist all = NULL;
  all = prepend_clist_to_plist(all, usable);
  all = prepend_clist_to_plist(all, sos);
  all = prepend_clist_to_plist(all, demods);

  fsyms = fsym_set_in_topforms(all);
  rsyms = rsym_set_in_topforms(all);

  // fsym_report(fsyms, all);

  nonneg = nonneg_clauses(all);
  zap_plist(all);

  rsyms_multiset = NULL;
  fsyms_multiset = NULL;
  gather_symbols_in_topforms(nonneg, &rsyms_multiset, &fsyms_multiset);

  lex_order(fsyms, rsyms, fsyms_multiset, rsyms_multiset,
	    lex_compare_arity_0213);

  if (echo && exists_preliminary_precedence(FUNCTION_SYMBOL))  {
    // print any symbols missing from the lex command
    Ilist p, missing_fsyms;
    missing_fsyms = not_in_preliminary_precedence(fsyms, FUNCTION_SYMBOL);
    // p_sym_list(fsyms); printf(" (fsyms)\n");
    // p_sym_list(missing_fsyms); printf(" (missing_fsyms)\n");
    if (missing_fsyms) {
      fprintf(stderr, "WARNING, function symbols not in function_order (lex) command:");
      printf("WARNING, function symbols not in function_order (lex) command:");
      for (p = missing_fsyms; p; p = p->next) {
	fprintf(stderr, " %s%s", sn_to_str(p->i), p->next ? "," : ".\n");
	printf(" %s%s", sn_to_str(p->i), p->next ? "," : ".\n");
      }
    }
    zap_ilist(missing_fsyms);
  }
  if (echo && exists_preliminary_precedence(PREDICATE_SYMBOL))  {
    // print any symbols missing from the lex command
    Ilist p, missing_rsyms;
    missing_rsyms = not_in_preliminary_precedence(rsyms, PREDICATE_SYMBOL);
    // p_sym_list(rsyms); printf(" (rsyms)\n");
    // p_sym_list(missing_rsyms); printf(" (missing_rsyms)\n");
    if (missing_rsyms) {
      fprintf(stderr, "WARNING, predicate symbols not in predicate_order command:");
      printf("WARNING, predicate symbols not in predicate_order command:");
      for (p = missing_rsyms; p; p = p->next) {
	fprintf(stderr, " %s%s", sn_to_str(p->i), p->next ? "," : ".\n");
	printf(" %s%s", sn_to_str(p->i), p->next ? "," : ".\n");
      }
    }
    zap_ilist(missing_rsyms);
  }

  init_features(fsyms, rsyms);  // feature-vector subsumption for nonunits

  zap_ilist(fsyms);
  zap_ilist(rsyms);
  zap_i2list(fsyms_multiset);
  zap_i2list(rsyms_multiset);
}  /* symbol_order */

/*************
 *
 *   unary_symbols()
 *
 *************/

/* DOCUMENTATION
Given an Ilist, return the sublist that represent unary symbols.
*/

/* PUBLIC */
Ilist unary_symbols(Ilist a)
{
  Ilist p;
  Ilist b = NULL;

  for (p = a; p; p = p->next) {
    if (sn_to_arity(p->i) == 1)
      b = ilist_append(b, p->i);
  }
  return b;
}  /* unary_symbols */

/*************
 *
 *   auto_kbo_weights()
 *
 *************/

/* DOCUMENTATION
If there is exactly one unary function symbol in usable+sos,
give it kb_weight=0 and highest precedence.
*/

/* PUBLIC */
void auto_kbo_weights(Clist usable, Clist sos)
{
  Plist clauses = NULL;
  Ilist fsyms, unaries;

  clauses = prepend_clist_to_plist(clauses, usable);  /* shallow */
  clauses = prepend_clist_to_plist(clauses, sos);     /* shallow */

  fsyms = fsym_set_in_topforms(clauses);
  unaries = unary_symbols(fsyms);

  if (ilist_count(unaries) == 1) {
    int symnum = unaries->i;
    if (!exists_preliminary_precedence(FUNCTION_SYMBOL) ||
	has_greatest_precedence(symnum)) {
      set_kb_weight(symnum, 0);
      if (!has_greatest_precedence(symnum))
	assign_greatest_precedence(symnum);
      printf("\n%% Assigning unary symbol %s kb_weight 0 and highest"
	     " precedence (%d).\n", sn_to_str(symnum), sn_to_lex_val(symnum));
    }
  }

  zap_ilist(unaries);
  zap_ilist(fsyms);
  zap_plist(clauses);  /* shallow */
}  /* auto_kbo_weights */

/*************
 *
 *   neg_pos_depth_diff()
 *
 *************/

static
int neg_pos_depth_diff(Topform c)
{
  Literals lit;
  int max_pos = 0;
  int max_neg = 0;
  for (lit = c->literals; lit; lit = lit->next) {
    int depth = term_depth(lit->atom);
    if (lit->sign)
      max_pos = IMAX(max_pos, depth);
    else
      max_neg = IMAX(max_neg, depth);
  }
  return max_neg - max_pos;
}  /* neg_pos_depth_diff */

/*************
 *
 *   neg_pos_wt_diff()
 *
 *************/

static
int neg_pos_wt_diff(Topform c)
{
  Literals lit;
  int max_pos = 0;
  int max_neg = 0;
  for (lit = c->literals; lit; lit = lit->next) {
    int wt = symbol_count(lit->atom);
    if (lit->sign)
      max_pos = IMAX(max_pos, wt);
    else
      max_neg = IMAX(max_neg, wt);
  }
  return max_neg - max_pos;
}  /* neg_pos_wt_diff */

/*************
 *
 *   neg_pos_depth_difference()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int neg_pos_depth_difference(Plist sos)
{
  Plist p;
  int neg = 0;
  int pos = 0;
  for (p = sos; p; p = p->next) {
    Topform c = p->v;
    if (mixed_clause(c->literals)) {
      Literals lit;
      int max_pos = 0;
      int max_neg = 0;
      for (lit = c->literals; lit; lit = lit->next) {
	int depth = term_depth(lit->atom);
	if (lit->sign)
	  max_pos = IMAX(max_pos, depth);
	else
	  max_neg = IMAX(max_neg, depth);
      }
      neg += max_neg;
      pos += max_pos;
      // printf("max_neg=%d, max_pos=%d, ", max_neg, max_pos); f_clause(c);
    }
  }
  // printf("neg=%d, pos=%d\n", neg, pos);
  return neg - pos;
}  /* neg_pos_depth_difference */

/*************
 *
 *   structure_of_clauses()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void structure_of_clauses(Clist clauses)
{
  int num_pos = 0;
  int pos_wt = 0;
  int pos_depth = 0;

  int neg_wt = 0;
  int neg_depth = 0;
  int num_neg = 0;

  int mix_wt = 0;
  int mix_depth = 0;
  int num_mix = 0;

  int mix_wt_diff = 0;
  int mix_depth_diff = 0;

  int num_nonhorn = 0;
  int nonhorn = 0;

  int num_nonunit = 0;
  int nonunit = 0;

  Clist_pos p;
  for (p = clauses->first; p; p = p->next) {
    Topform c = p->c;
    int wt = clause_symbol_count(c->literals);
    int depth = clause_depth(c->literals);
    if (positive_clause(c->literals)) {
      num_pos++;
      pos_wt += wt;
      pos_depth += depth;
    }
    else if (negative_clause(c->literals)) {
      num_neg++;
      neg_wt += wt;
      neg_depth += depth;
    }
    else {
      num_mix++;
      mix_wt += wt;
      mix_depth += depth;
      mix_wt_diff += neg_pos_wt_diff(c);
      mix_depth_diff += neg_pos_depth_diff(c);
    }

    if (!horn_clause(c->literals)) {
      num_nonhorn++;
      nonhorn += positive_literals(c->literals) - 1;
    }

    if (!unit_clause(c->literals)) {
      num_nonunit++;
      nonunit += number_of_literals(c->literals);
    }

  }
  {
    double pw = num_pos == 0 ? 0 : pos_wt / (double) num_pos;
    double pd = num_pos == 0 ? 0 : pos_depth / (double) num_pos;

    double nw = num_neg == 0 ? 0 : neg_wt / (double) num_neg;
    double nd = num_neg == 0 ? 0 : neg_depth / (double) num_neg;

    double mw = num_mix == 0 ? 0 : mix_wt / (double) num_mix;
    double md = num_mix == 0 ? 0 : mix_depth / (double) num_mix;

    double mwd = num_mix == 0 ? 0 : mix_wt_diff / (double) num_mix;
    double mdd = num_mix == 0 ? 0 : mix_depth_diff / (double) num_mix;

    double nh = num_nonhorn == 0 ? 0 : nonhorn / (double) num_nonhorn;
    double nu = num_nonunit == 0 ? 0 : nonunit / (double) num_nonunit;

    printf("\n%% Struc"
	   " (N %2d %2d %2d)"
	   " (WT %5.2f %5.2f %5.2f %5.2f)"
	   " (DP %5.2f %5.2f %5.2f %5.2f)"
	   " (NH %2d %5.2f)"
	   " (NU %2d %5.2f)"
	   "\n",
	   num_pos, num_neg, num_mix,
	   pw, nw, mw, mwd,
	   pd, nd, md, mdd,
	   num_nonhorn, nh, num_nonunit, nu);
  }
}  /* structure_of_clauses */

/*************
 *
 *   p_term_list()
 *
 *************/

static
void p_term_list(Plist terms)
{
  Plist p;
  printf("[");
  for (p = terms; p; p = p->next) {
    fwrite_term(stdout, p->v);
    printf("%s", p->next ? "," : "");
  }
  printf("]");
}  /* p_term_list */

/*************
 *
 *   plist_size_of_diff()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int plist_size_of_diff(Plist a, Plist b)
{
  if (a == NULL)
    return 0;
  else {
    int n = plist_size_of_diff(a->next, b);
    return (!plist_member(b, a->v) ? n+1 : n);
  }
}  /* plist_size_of_diff */

/*************
 *
 *   structure_of_variables()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void structure_of_variables(Clist clauses)
{
  int n = 0;
  int np = 0;
  int nn = 0;

  Clist_pos p;
  for (p = clauses->first; p; p = p->next) {
    Topform c = p->c;
    if (mixed_clause(c->literals)) {
      Plist pvars = NULL;
      Plist nvars = NULL;
      Literals lit;
      for (lit = c->literals; lit; lit = lit->next) {
	if (lit->sign)
	  pvars = set_of_vars(lit->atom, pvars);
	else
	  nvars = set_of_vars(lit->atom, nvars);
      }
      printf("\n");
      f_clause(c);

      printf(" nvars (%d): ", plist_size_of_diff(nvars, pvars));
      p_term_list(nvars); 
      printf(" pvars (%d): ", plist_size_of_diff(pvars, nvars));
      p_term_list(pvars); 
      n++;
      np += plist_count(pvars);
      nn += plist_count(nvars);
      zap_plist(nvars);
      zap_plist(pvars);
    }
  }
  printf("\nnn=%d, np=%d, n=%d\n", nn, np, n);
}  /* structure_of_variables */

/*************
 *
 *   clause_compare_m4()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype clause_compare_m4(Topform a, Topform b)
{
  /* pos < neg < mixed */
  BOOL a_pos = positive_clause(a->literals);
  BOOL b_pos = positive_clause(b->literals);
  BOOL a_neg = negative_clause(a->literals);
  BOOL b_neg = negative_clause(b->literals);
  
  if (a_pos && !b_pos)
    return LESS_THAN;
  else if (!a_pos && b_pos)
    return GREATER_THAN;
  else if (a_neg && !b_neg)
    return LESS_THAN;
  else if (!a_neg && b_neg)
    return GREATER_THAN;
  else {
    /* now both pos, both neg, or both mixed */
    /* fewer symbols < more symbols */
    int na = clause_symbol_count(a->literals);
    int nb = clause_symbol_count(b->literals);
    if (na > nb)
      return GREATER_THAN;
    else if (na < nb)
      return LESS_THAN;
    else {
      /* fewer literals < more literals */
      na = number_of_literals(a->literals);
      na = number_of_literals(b->literals);
      if (na > nb)
	return GREATER_THAN;
      else if (na < nb)
	return LESS_THAN;
      else {
	/* shallower < deeper */
	na = clause_depth(a->literals);
	na = clause_depth(b->literals);
	if (na > nb)
	  return GREATER_THAN;
	else if (na < nb)
	  return LESS_THAN;
	else 
	  return SAME_AS;  /* should we go further? */
      }
    }
  }
}  /* clause_compare_m4 */

/*************
 *
 *   bogo_ticks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int bogo_ticks(void)
{
  return mega_mem_calls();
}  /* bogo_ticks */

/*************
 *
 *   next_negative_clause_3()
 *
 *************/

static
Topform next_negative_clause_3(Clist_pos *ap, Clist_pos *bp, Clist_pos *cp)
{
  Clist_pos a = *ap;
  Clist_pos b = *bp;
  Clist_pos c = *cp;

  Clist_pos x;
  Topform next_neg;
  
  while (a && !negative_clause(a->c->literals)) a = a->next;
  while (b && !negative_clause(b->c->literals)) b = b->next;
  while (c && !negative_clause_possibly_compressed(c->c)) c = c->next;

  if (!a) {
    if (!b)
      x = c;  /* may be NULL */
    else if (!c)
      x = b;
    else
      x = (b->c->id < c->c->id ? b : c);
  }
  else if (!b) {
    if (!c)
      x = a;
    else
      x = (a->c->id < c->c->id ? a : c);
  }
  else if (!c)
    x = (a->c->id < b->c->id ? a : b);
  else if (a->c->id < b->c->id)
    x = (a->c->id < c->c->id ? a : c);
  else
    x = (b->c->id < c->c->id ? b : c);

  /* Advance position of the winner. */

  if (x == NULL)
    next_neg = NULL;
  else {
    next_neg = x->c;
    /* Advance position of the winner. */
    if (x == a)
      a = a->next;
    else if (x == b)
      b = b->next;
    else
      c = c->next;
  }

  /* Restore pointers for caller. */

  *ap = a;
  *bp = b;
  *cp = c;
  return next_neg;
}  /* next_negative_clause_3 */

/*************
 *
 *   first_negative_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform first_negative_clause(Plist proof)
{
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c = p->v;
    if (!c->is_formula && negative_clause(c->literals))
      return c;
  }
  return NULL;
}  /* first_negative_clause */

/*************
 *
 *   neg_clauses_and_descendants()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist neg_clauses_and_descendants(Plist proof,
				  Clist a_list, Clist b_list, Clist c_list)
{
  Topform first_neg = first_negative_clause(proof);
  Plist descendents = plist_prepend(NULL, first_neg);
  Plist p;

  /* Get all descendents of first_neg that appear in a, b, or c. */
  
  Clist_pos a = a_list->first;
  Clist_pos b = b_list->first;
  Clist_pos c = c_list->first;

  Topform next = next_negative_clause_3(&a, &b, &c);
  while (next) {
    Topform neg_parent = first_negative_parent(next);
    if (neg_parent && clause_plist_member(descendents, neg_parent, FALSE))
      descendents = insert_clause_into_plist(descendents, next, FALSE);
    next = next_negative_clause_3(&a, &b, &c);
  }

  /* The last 2 negative clauses might not be in a, b, or c,
     so make sure they get put into the list of descendants.
  */

  for (p = proof; p; p = p->next) {
    Topform c = p->v;
    if (!c->is_formula && negative_clause(c->literals))
      /* If already there, it will not be inserted. */
      descendents = insert_clause_into_plist(descendents, c, FALSE);
  }

  descendents = reverse_plist(descendents);  /* make it increasing */

#if 1
  {
    int n = 0;
    printf("\n%% Preparing to disable descendents of clause %d:",
	   first_neg->id);
    for (p = descendents; p; p = p->next) {
      printf(" %d", ((Topform) p->v)->id);
      if (++n % 20 == 0)
	printf("\n");
    }
    printf("\n");
  }
#endif

  return descendents;
}  /* neg_clauses_and_descendants */

/*************
 *
 *   neg_descendants()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist neg_descendants(Topform top_neg,
		      Clist a_list, Clist b_list, Clist c_list)
		      
{
  Plist descendants = plist_prepend(NULL, top_neg);
  Clist_pos a, b, c;
  Topform next;

  sort_clist_by_id(c_list);

  /* Get all descendants of top_neg that appear in a, b, or c. */
  
  a = a_list->first;
  b = b_list->first;
  c = c_list->first;

  next = next_negative_clause_3(&a, &b, &c);
  while (next) {
    Topform neg_parent = first_negative_parent(next);
    if (neg_parent && clause_plist_member(descendants, neg_parent, FALSE))
      descendants = insert_clause_into_plist(descendants, next, FALSE);
    next = next_negative_clause_3(&a, &b, &c);
  }
  descendants = reverse_plist(descendants);  /* make it increasing */
  return descendants;
}  /* neg_descendants */

/*************
 *
 *   check_constant_sharing()
 *
 *************/

/* DOCUMENTATION
Given a Plist of Clauses, output a warning message
for each pair of clauses that shares any constants.
*/

/* PUBLIC */
void check_constant_sharing(Plist clauses)
{
  Plist a, b;
  Ilist shared = NULL;
  for (a = clauses; a; a = a->next) {
    Topform aa = a->v;
    Ilist ca = constants_in_clause(aa->literals);
    for (b = a->next; b; b = b->next) {
      Topform bb = b->v;
      Ilist cb = constants_in_clause(bb->literals);
      Ilist sh2 = ilist_intersect(ca, cb);  /* preserves arguments */
      Ilist sh3 = ilist_cat(sh2, shared);      /* destroys arguments */
      shared = ilist_set(sh3);
      zap_ilist(sh3);
      zap_ilist(cb);
    }
    zap_ilist(ca);
  }
  if (shared) {
    Ilist p;
    bell(stderr);
    fprintf(stderr, "\nWARNING: denials share constants (see output).\n\n");
    printf("\nWARNING, because some of the denials share constants,\n"
	   "some of the denials or their descendents may be subsumed,\n"
	   "preventing the target number of proofs from being found.\n"
	   "The shared constants are: ");
    for (p = shared; p; p = p->next)
      printf(" %s%s", sn_to_str(p->i), p->next ? "," : ".\n");
    zap_ilist(shared);
  }
}  /* check_constant_sharing */

