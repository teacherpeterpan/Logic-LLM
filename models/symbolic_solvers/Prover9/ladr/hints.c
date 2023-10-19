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

#include "hints.h"

/* Private definitions and types */

static Lindex Hints_idx = NULL;       /* FPA index for hints */
static Clist Redundant_hints = NULL;  /* list of hints not indexed */
static Mindex Back_demod_idx;        /* to index hints for back demodulation */
static int Bsub_wt_attr;
static BOOL Back_demod_hints;
static BOOL Collect_labels;

/* pointer to procedure for demodulating hints (when back demod hints) */

static void (*Demod_proc) (Topform, int, int, BOOL, BOOL);

/* stats */

static int Hint_id_count = 0;
static int Active_hints_count = 0;
static int Redundant_hints_count = 0;

/*************
 *
 *   init_hints()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_hints(Uniftype utype,
		int bsub_wt_attr,
		BOOL collect_labels,
		BOOL back_demod_hints,
		void (*demod_proc) (Topform, int, int, BOOL, BOOL))
{
  Bsub_wt_attr = bsub_wt_attr;
  Collect_labels = collect_labels;
  Back_demod_hints = back_demod_hints;
  Demod_proc = demod_proc;
  Hints_idx = lindex_init(FPA, utype, 10, FPA, utype, 10);
  if (Back_demod_hints)
    Back_demod_idx = mindex_init(FPA, utype, 10);
  Redundant_hints = clist_init("redundant_hints");
}  /* init_hints */

/*************
 *
 *   done_with_hints()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void done_with_hints(void)
{
  if (!lindex_empty(Hints_idx) ||
      !clist_empty(Redundant_hints))
    printf("ERROR: Hints index not empty!\n");
  lindex_destroy(Hints_idx);
  if (Back_demod_hints)
    mindex_destroy(Back_demod_idx);
  Hints_idx = NULL;
  clist_free(Redundant_hints);
  Redundant_hints = NULL;
}  /* done_with_hints */

/*************
 *
 *   redundant_hints()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int redundant_hints(void)
{
  return clist_length(Redundant_hints);
}  /* redundant_hints */

/*************
 *
 *   find_equivalent_hint()
 *
 *************/

static
Topform find_equivalent_hint(Topform c, Lindex idx)
{
  Topform equiv_hint = NULL;
  Plist subsumees = back_subsume(c, idx);
  Plist p;
  for (p = subsumees; p && equiv_hint == NULL; p = p->next) {
    if (subsumes(p->v, c))
      equiv_hint = p->v;
  }
  zap_plist(subsumees);
  return equiv_hint;
}  /* find_equivalent_hint */

/*************
 *
 *   find_matching_hint()
 *
 *   Return the first equivalent hint;  if none, return the last
 *   subsumed hint.
 *
 *   "First" and "last" refer to the order returned by the index,
 *   which is not necessarily the order in which the hints were
 *   inserted into the index.  In fact, it is likely that the
 *   clauses are returned in the reverse order.
 *
 *************/

static
Topform find_matching_hint(Topform c, Lindex idx)
{
  Topform hint = NULL;
  Plist subsumees = back_subsume(c, idx);
  Plist p;
  BOOL equivalent = FALSE;
  for (p = subsumees; p && !equivalent; p = p->next) {
    /* printf("subsumee: "); f_clause(p->v); */
    hint = p->v;
    if (subsumes(p->v, c))
      equivalent = TRUE;
  }
  zap_plist(subsumees);
  return hint;
}  /* find_matching_hint */

/*************
 *
 *   index_hint()
 *
 *************/

/* DOCUMENTATION
Index a clause C as a hint (make sure to call init_hints first).
If the clause is equivalent to a previously indexed hint H, any
labels on C are copied to H, and C is not indexed.
*/

/* PUBLIC */
void index_hint(Topform c)
{
  Topform h = find_equivalent_hint(c, Hints_idx);
  c->weight = 0;  /* this is used in hints degradation to count matches */
  if (h != NULL) {
    /* copy any bsub_hint_wt attrs from rundundant hint to the indexed hint */
    h->attributes = copy_int_attribute(c->attributes, h->attributes,
				       Bsub_wt_attr);
    if (Collect_labels) {
      /* copy any labels from rundundant hint to the indexed hint */
      h->attributes = copy_string_attribute(c->attributes, h->attributes,
					    label_att());
    }
    clist_append(c, Redundant_hints);
    Redundant_hints_count++;
    /*
    printf("redundant hint: "); f_clause(c);
    printf("      original: "); f_clause(h);
    */
  }
  else {
    Active_hints_count++;
    Hint_id_count++;
    c->id = Hint_id_count;  /* need IDs so that back_subsume() will work */
    lindex_update(Hints_idx, c, INSERT);
    if (Back_demod_hints)
      index_clause_back_demod(c, Back_demod_idx, INSERT);
  }
}  /* index_hint */

/*************
 *
 *   unindex_hint()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void unindex_hint(Topform c)
{
  if (clist_member(c, Redundant_hints)) {
    clist_remove(c, Redundant_hints);
    Redundant_hints_count--;
  }
  else {
    lindex_update(Hints_idx, c, DELETE);
    if (Back_demod_hints)
      index_clause_back_demod(c, Back_demod_idx, DELETE);
    Active_hints_count--;
  }
}  /* unindex_hint */

/*************
 *
 *   adjust_weight_with_hints()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void adjust_weight_with_hints(Topform c,
			      BOOL degrade,
			      BOOL breadth_first_hints)
{
  Topform hint = find_matching_hint(c, Hints_idx);

  if (hint == NULL &&
      unit_clause(c->literals) &&
      eq_term(c->literals->atom) &&
      !oriented_eq(c->literals->atom)) {

    /* Try to find a hint that matches the flipped equality. */

    Term save_atom = c->literals->atom;
    c->literals->atom = top_flip(save_atom);
    hint = find_matching_hint(c, Hints_idx);
    zap_top_flip(c->literals->atom);
    c->literals->atom = save_atom;
    if (hint != NULL)
      c->attributes = set_string_attribute(c->attributes, label_att(),
					   "flip_matches_hint");
  }

  if (hint != NULL) {

    int bsub_wt = get_int_attribute(hint->attributes, Bsub_wt_attr, 1);

    if (bsub_wt != INT_MAX)
      c->weight = bsub_wt;
    else if (breadth_first_hints)
      c->weight = 0;

    /* If the hint has label attributes, copy them to the clause. */
    
    {
      int i = 0;
      char *s = get_string_attribute(hint->attributes, label_att(), ++i);
      while (s) {
	if (!string_attribute_member(c->attributes, label_att(), s))
	  c->attributes = set_string_attribute(c->attributes, label_att(), s);
	s = get_string_attribute(hint->attributes, label_att(), ++i);
      }
    }

    /* Veroff's hint degradation strategy. */

    if (degrade) {
      /* for now, add 1000 for each previous match */
      int i;
      for (i = 0; i < hint->weight; i++) 
	c->weight = c->weight + 1000;
    }
    c->matching_hint = hint;
    /* If/when c is eventually kept, the hint will have its weight
       field incremented in case hint degradation is being used. */
  }
}  /* adjust_weight_with_hints */

/*************
 *
 *   keep_hint_matcher()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void keep_hint_matcher(Topform c)
{
  Topform hint = c->matching_hint;
  hint->weight++;
}  /* keep_hint_matcher */

/*************
 *
 *   back_demod_hints()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void back_demod_hints(Topform demod, int type, BOOL lex_order_vars)
{
  if (Back_demod_hints) {
    Plist rewritables = back_demod_indexed(demod, type, Back_demod_idx,
					   lex_order_vars);
    Plist p;
    for (p = rewritables; p; p = p->next) {
      Topform hint = p->v;
      /* printf("\nBEFORE: "); f_clause(hint); */
      unindex_hint(hint);
      (*Demod_proc)(hint, 1000, 1000, FALSE, lex_order_vars);

      orient_equalities(hint, TRUE);
      simplify_literals2(hint);
      merge_literals(hint);
      renumber_variables(hint, MAX_VARS);

      /* printf("AFTER : "); f_clause(hint); */
      index_hint(hint);
      hint->weight = 0;  /* reset count of number of matches */
    }
  }
}  /* back_demod_hints */
