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

#include "mindex.h"

/* Private definitions and types */

struct mindex_pos {

  Mindex     index;
  Querytype  query_type;
  Term       query_term;
  Term       found_term;
  Context    query_subst;
  Context    found_subst;
  Trail      tr;
  Btu_state  btu_position;  /* backtrack unification */
  Btm_state  btm_position;  /* backtrack matching */
  BOOL       partial_match;

  /* FPA */
  Fpa_state  fpa_position;

  /* LINEAR */
  Plist   linear_position;
  
  /* DISCRIM_WILD */
  /* DISCRIM_BIND */
  Discrim_pos  discrim_position;

  Mindex_pos next;  /* for avail list */
};

/*
 * memory management
 */

#define PTRS_MINDEX PTRS(sizeof(struct mindex))
static unsigned Mindex_gets, Mindex_frees;

#define PTRS_MINDEX_POS PTRS(sizeof(struct mindex_pos))
static unsigned Mindex_pos_gets, Mindex_pos_frees;

/*************
 *
 *   Mindex get_mindex()
 *
 *************/

static
Mindex get_mindex(void)
{
  Mindex p = get_cmem(PTRS_MINDEX);
  p->index_type = -1;
  p->unif_type = -1;
  Mindex_gets++;
  return(p);
}  /* get_mindex */

/*************
 *
 *    free_mindex()
 *
 *************/

static
void free_mindex(Mindex p)
{
  free_mem(p, PTRS_MINDEX);
  Mindex_frees++;
}  /* free_mindex */

/*************
 *
 *   Mindex_pos get_mindex_pos()
 *
 *************/

static
Mindex_pos get_mindex_pos(void)
{
  Mindex_pos p = get_cmem(PTRS_MINDEX_POS);
  p->query_type = -1;
  Mindex_pos_gets++;
  return(p);
}  /* get_mindex_pos */

/*************
 *
 *    free_mindex_pos()
 *
 *************/

static
void free_mindex_pos(Mindex_pos p)
{
  free_mem(p, PTRS_MINDEX_POS);
  Mindex_pos_frees++;
}  /* free_mindex_pos */

/*************
 *
 *   fprint_mindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the mindex package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_mindex_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct mindex);
  fprintf(fp, "mindex (%4d)       %11u%11u%11u%9.1f K\n",
          n, Mindex_gets, Mindex_frees,
          Mindex_gets - Mindex_frees,
          ((Mindex_gets - Mindex_frees) * n) / 1024.);

  n = sizeof(struct mindex_pos);
  fprintf(fp, "mindex_pos (%4d)   %11u%11u%11u%9.1f K\n",
          n, Mindex_pos_gets, Mindex_pos_frees,
          Mindex_pos_gets - Mindex_pos_frees,
          ((Mindex_pos_gets - Mindex_pos_frees) * n) / 1024.);

}  /* fprint_mindex_mem */

/*************
 *
 *   p_mindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the mindex package.
*/

/* PUBLIC */
void p_mindex_mem()
{
  fprint_mindex_mem(stdout, TRUE);
}  /* p_mindex_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   mindex_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an (empty) Mindex, which is
used to retrieve unifiable terms.
<UL>
<LI><TT>index_type: {LINEAR, FPA, DISCRIM_WILD, DISCRIM_BIND}</TT>
<LI><TT>unif_type: {ORDINARY_UNIF, BACKTRACK_UNIF}</TT>
<LI><TT>fpa_depth: </TT>depth of FPA indexing
    (ignored for other index types).
</UL>
<I>Types of retrieval</I>.  LINEAR and FPA indexes support all types
of retrieval (FPA is slow for GENERALIZATION).
DISCRIM_WILD and DISCRIM_BIND indexes support GENERALIZATION retrieval only.
See mindex_retrieve_first().
<P>
<I>Associative-commutative (AC) and commutative (C) symbols</I>.
DISCRIM_BIND does <I>not</I> support
AC symbols.  All other combinations are okay.  If you have any
AC or C symbols, you must specify unif_type BACKTRACK_UNIF.
(BACKTRACK_UNIF is also okay with no AC or C symbols, but it
is a little bit slower than ORDINARY_UNIF.)
<P>
AC symbols must be declared (with set_assoc_comm()) before calling
mindex_update().
C symbols need not be declared before mindex_update(), but they
must be declared (with set_commutative()) before calling
mindex_retrieve_first().
*/

/* PUBLIC */
Mindex mindex_init(Mindextype mtype, Uniftype utype, int fpa_depth)
{
  Mindex mdx = get_mindex();

  mdx->index_type = mtype;
  mdx->unif_type = utype;

  switch(mtype) {
  case LINEAR:       mdx->linear_first = mdx->linear_last = NULL; break;
  case FPA:          mdx->fpa = fpa_init_index(fpa_depth); break;
  case DISCRIM_WILD: mdx->discrim_tree = discrim_init(); break;
  case DISCRIM_BIND: mdx->discrim_tree = discrim_init(); break;
  default:           free_mindex(mdx); mdx = NULL;
  }

  return mdx;
}  /* mindex_init */

/*************
 *
 *   mindex_empty()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if an Mindex is empty, that is, has no
terms.  It must exist (be non-NULL).
*/

/* PUBLIC */
BOOL mindex_empty(Mindex mdx)
{
  switch (mdx->index_type) {
  case FPA:
    return fpa_empty(mdx->fpa);
    break;
  case LINEAR:
    return mdx->linear_first == NULL;
    break;
  case DISCRIM_WILD:
  case DISCRIM_BIND:
    return discrim_empty(mdx->discrim_tree);
    break;
  }
  return FALSE;
}  /* mindex_empty */

/*************
 *
 *   mindex_free()
 *
 *************/

/* DOCUMENTATION
This routine frees an empty Mindex.  (If the Mindex is not empty,
nothing happens.)
*/

/* PUBLIC */
void mindex_free(Mindex mdx)
{
  if (!mindex_empty(mdx))
    fprintf(stderr, "WARNING, mindex_free called with nonempty mindex.\n");
  else {
    switch (mdx->index_type) {
    case FPA:
      zap_fpa_index(mdx->fpa);
      break;
    case LINEAR:
      break;
    case DISCRIM_WILD:
    case DISCRIM_BIND:
      discrim_dealloc(mdx->discrim_tree);
      break;
    }
    free_mindex(mdx);
  }
}  /* mindex_free */

/*************
 *
 *   mindex_destroy()
 *
 *************/

/* DOCUMENTATION
This frees all the memory associated with an Mindex.  Do not refer
to the Mindex after calling this routine.
*/

/* PUBLIC */
void mindex_destroy(Mindex mdx)
{
  if (!mindex_empty(mdx)) {
    fprintf(stdout, "\nWARNING: destroying nonempty mindex.\n\n");
    fprintf(stderr, "\nWARNING: destroying nonempty mindex.\n\n");
  }

  switch (mdx->index_type) {
  case FPA:
    zap_fpa_index(mdx->fpa);
    break;
  case LINEAR:
    zap_plist(mdx->linear_first);
    break;
  case DISCRIM_WILD:
  case DISCRIM_BIND:
    destroy_discrim_tree(mdx->discrim_tree);
    break;
  }
  free_mindex(mdx);
}  /* mindex_destroy */

/*************
 *
 *   linear_insert()
 *
 *************/

static
void linear_insert(Mindex mdx, Term t)
{
  Plist p = get_plist();
  p->v = t;
  p->next = NULL;
  if (mdx->linear_last != NULL)
    mdx->linear_last->next = p;
  else
    mdx->linear_first = p;
  mdx->linear_last = p;
}  /* linear_insert */

/*************
 *
 *   linear_delete()
 *
 *************/

static
void linear_delete(Mindex mdx, Term t)
{
  Plist curr, prev;
  prev = NULL;
  curr = mdx->linear_first;
  while (curr != NULL && curr->v != t) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL) {
    fprint_term(stderr, t);
    fprintf(stderr, "\n");
    fatal_error("mindex_delete (linear), term not found.");
  }
  else {
    if (prev != NULL)
      prev->next = curr->next;
    else
      mdx->linear_first = curr->next;
    if (curr == mdx->linear_last)
      mdx->linear_last = prev;
    free_plist(curr);
  }
}  /* linear_delete */

/*************
 *
 *   linear_update()
 *
 *************/

static
void linear_update(Mindex mdx, Term t, Indexop op)
{
  if (op == INSERT)
    linear_insert(mdx, t);
  else
    linear_delete(mdx, t);
}  /* linear_update */

/*************
 *
 *   mindex_update()
 *
 *************/

/* DOCUMENTATION
This routine inserts (op==INSERT) or deletes (op==DELETE)
a Term t into/from an Mindex mdx.
<P>
It is your responsibility to remember that t is in the index,
because we don't currently have a routine "mindex_member()".
*/

/* PUBLIC */
void mindex_update(Mindex mdx, Term t, Indexop op)
{
  if (mdx->index_type == FPA)
    fpa_update(t, mdx->fpa, op);
  else if (mdx->index_type == LINEAR)
    linear_update(mdx, t, op);
  else if (mdx->index_type == DISCRIM_WILD)
    discrim_wild_update(t, mdx->discrim_tree, t, op);
  else if (mdx->index_type == DISCRIM_BIND)
    discrim_bind_update(t, mdx->discrim_tree, t, op);
  else {
    fatal_error("ERROR, mindex_update: bad mindex type.");
  }
}  /* mindex_update */

/*************
 *
 *    mindex_retrieve_first
 *
 *************/

/* DOCUMENTATION
This routine finds and returns the first answer to a query (returning NULL
if there are no answers).
<UL>
<LI> Term t: the query term;
<LI> Mindex mdx: the Mindex;
<LI> int query_type: UNIFY, INSTANCE, GENERALIZATION, VARIANT, or IDENTICAL;
<LI> Context query_subst: subsitution for variables in query term t;
this can be NULL for GENERALIZATION, and IDENTICAL;
<LI> Context found_subst: subsitution for variables in the answer term;
this can be NULL for INSTANCE, VARIANT, and IDENTICAL;
<LI> BOOL partial_match: If TRUE, then for GENERALIZATION, with BACKTRACK_UNIF,  
when t has an AC symbol at the root, allow the retrieved term to match only
part of t, with the remainder of the retrieved term placed in
fond_subst->partial_term.  This is used for AC rewriting, for example, to
let x+x=x rewrite a+a+b.
<LI> Mindex_pos *ppos (output parameter): If an answer is returned,
this address is set to a pointer to a structure that holds the position
so you can get the rest of the answers.
</UL>
If you ask for a type of retrieval not supported by the Mindex mdx,
you will get no answers (and no error message).
<P>
Here is an example of how to retrieve all answers.  Assume we have
Term t and Mindex mdx.
<PRE>
{
  Mindex_pos pos;
  Term t2;
  Context cq = get_context();
  Context cf = get_context();
  int n = 0;
  
  t2 = mindex_retrieve_first(t, mdx, UNIFY, cq, cf, FALSE, &pos);
  while (t2 != NULL) {
    t2 = mindex_retrieve_next(pos);
    n++;
  }
  free_context(cq);
  free_context(cf);
  printf("there are %d mates\n", n);
}
</PRE>
*/

/* PUBLIC */
Term mindex_retrieve_first(Term t, Mindex mdx, Querytype qtype,
			   Context query_subst, Context found_subst,
			   BOOL partial_match,
			   Mindex_pos *ppos)
{
  Mindex_pos pos;

  if ((mdx->index_type == DISCRIM_WILD || mdx->index_type == DISCRIM_BIND) &&
      qtype != GENERALIZATION)
    return NULL;

  else {
    pos = get_mindex_pos();
    pos->index = mdx;
    pos->query_type = qtype;
    pos->query_term = t;
    pos->query_subst = query_subst;
    pos->found_term = NULL;
    pos->found_subst = found_subst;
    pos->tr = NULL;
    pos->btu_position = NULL;
    pos->btm_position = NULL;
    pos->partial_match = partial_match;

    if (mdx->index_type == FPA)
      pos->fpa_position = NULL;
    else if (mdx->index_type == DISCRIM_WILD)
      pos->discrim_position = NULL;
    else if (mdx->index_type == DISCRIM_BIND)
      pos->discrim_position = NULL;
    else if (mdx->index_type == LINEAR)
      pos->linear_position = mdx->linear_first;

    *ppos = pos;
    return mindex_retrieve_next(pos);
  }
}  /* mindex_retrieve_first */

/*************
 *
 *   next_candidate()
 *
 *************/

static
Term next_candidate(Mindex_pos pos)
{
  Term tf;

  if (pos->index->index_type == FPA) {
    if (pos->fpa_position == NULL) {
      tf = fpa_first_answer(pos->query_term,
			    pos->query_subst,
			    pos->query_type,
			    pos->index->fpa,
			    &(pos->fpa_position));
    }
    else
      tf = fpa_next_answer(pos->fpa_position);
  }
  else if (pos->index->index_type == DISCRIM_WILD) {
    if (pos->discrim_position == NULL)
      tf = discrim_wild_retrieve_first(pos->query_term,
				       pos->index->discrim_tree,
				       &(pos->discrim_position));
    else
      tf = discrim_wild_retrieve_next(pos->discrim_position);
  }

  else if (pos->index->index_type == DISCRIM_BIND) {
    if (pos->discrim_position == NULL)
      tf = discrim_bind_retrieve_first(pos->query_term,
				       pos->index->discrim_tree,
				       pos->found_subst,
				       &(pos->discrim_position));
    else
      tf = discrim_bind_retrieve_next(pos->discrim_position);
  }

  else if (pos->index->index_type == LINEAR) {
    if (pos->linear_position == NULL)
      tf = NULL;
    else {
      tf = pos->linear_position->v;
      pos->linear_position = pos->linear_position->next;
    }
  }

  else
    tf = NULL;
  return tf;
}  /* next_candidate */

/*************
 *
 *   retrieve_next_backtrack()
 *
 *************/

static
Term retrieve_next_backtrack(Mindex_pos pos)
{
  Term tq = pos->query_term;
  Term tf = pos->found_term;
  Context cq = pos->query_subst;
  Context cf = pos->found_subst;

  if (pos->query_type == UNIFY) {
    /* We already have a found_term from a previous call;
     * try for another unifier.
     */
    if (pos->btu_position != NULL) {
      pos->btu_position = unify_bt_next(pos->btu_position);
      if (pos->btu_position == NULL)
	tf = NULL;
    }
    if (pos->btu_position == NULL) {
      /* This is either the first call for the query, or there are
       * no more unifiers for the previous found_term.
       */
      tf = next_candidate(pos);
      while (tf != NULL && pos->btu_position == NULL) {
	pos->btu_position = unify_bt_first(tq, cq, tf, cf);
	if (pos->btu_position == NULL)
	  tf = next_candidate(pos);
      }
    }
  }  /* UNIFY */

  else if (pos->query_type == INSTANCE || pos->query_type == GENERALIZATION) {
    if (pos->btm_position != NULL) {
      pos->btm_position = match_bt_next(pos->btm_position);
      if (pos->btm_position == NULL)
	tf = NULL;
    }
    if (pos->btm_position == NULL) {
      tf = next_candidate(pos);
      while (tf != NULL && pos->btm_position == NULL) {
	if (pos->query_type == INSTANCE)
	  pos->btm_position = match_bt_first(tq, cq, tf, pos->partial_match);
	else
	  pos->btm_position = match_bt_first(tf, cf, tq, pos->partial_match);
	if (pos->btm_position == NULL)
	  tf = next_candidate(pos);
      }
    }
  }  /* INSTANCE || GENERALIZATION */

  else if (pos->query_type == VARIANT) {
    fatal_error("retrieve_next_backtrack, VARIANT not supported.");
  }  /* VARIANT */

  else if (pos->query_type == IDENTICAL) {
    fatal_error("retrieve_next_backtrack, IDENTICAL not supported.");
  }  /* IDENTICAL */

  if (tf == NULL) {
    free_mindex_pos(pos);
    return NULL;
  }
  else {
    pos->found_term = tf;
    return tf;
  }
}  /* retrieve_next_backtrack */

/*************
 *
 *    mindex_retrieve_next
 *
 *************/

/* DOCUMENTATION
This routine finds and returns the next answer to a query.
See mindex_retrieve_first() for an explanation.
*/

/* PUBLIC */
Term mindex_retrieve_next(Mindex_pos pos)
{
  if (pos->index->unif_type == BACKTRACK_UNIF)
    return retrieve_next_backtrack(pos);
  else {
    Term tq, tf;
    Context cq, cf;
    Trail tr;
    BOOL ok;

    tq = pos->query_term;
    cq = pos->query_subst;
    cf = pos->found_subst;

    undo_subst(pos->tr);  /* ok if NULL or not used */
    pos->tr = NULL;

    tf = next_candidate(pos);
    if (tf != NULL && pos->index->index_type == DISCRIM_BIND) 
      ok = TRUE;
    else
      ok = FALSE;
    while (tf && !ok) {
#if 0
      printf("potential mate, %d: ", tf->INDEX_ID); p_term(tf);
#endif	
      tr = NULL;
      switch (pos->query_type) {
      case UNIFY:
	ok = unify(tq, cq, tf, cf, &(pos->tr)); break;
      case GENERALIZATION:
	ok = match(tf, cf, tq, &(pos->tr)); break;
      case INSTANCE:
	ok = match(tq, cq, tf, &(pos->tr)); break;
      case VARIANT:
	ok = variant(tq, cq, tf, &(pos->tr)); break;
      case IDENTICAL:
	ok = term_ident(tq, tf); break;
      default:
	ok = FALSE; break;
      }
      if (!ok)
	tf = next_candidate(pos);
    }

    if (ok) {
#if 0
      printf("          MATE, %d: ", tf->INDEX_ID); p_term(tf);
#endif	
      return tf;
    }
    else {
      free_mindex_pos(pos);
      return NULL;
    }
  }
}  /* mindex_retrieve_next */

/*************
 *
 *    mindex_retrieve_cancel
 *
 *************/

/* DOCUMENTATION
This routine should be called if you get some, but not all,
answers to a query.  For example, if you need only one answer
you can do something like the following:
<PRE>
{
  Mindex_pos pos;
  Term t2;
  Context cf = get_context();
  
  t2 = mindex_retrieve_first(t, mdx, GENERALIZATION, (Context) NULL, cf, &pos);
  if (t2 != NULL) {
    printf("we found a mate!\n");
    mindex_retrieve_cancel(pos);
  }
  else
    printf("no answer\n");
  free_context(cf);
}
</PRE>
If you fail to  call this routine, then the memory associated
with an Mindex_pos will be forever lost, that is, you will have
a memory leak.
*/

/* PUBLIC */
void mindex_retrieve_cancel(Mindex_pos pos)
{
  /* Clean up the unification states.  The Mindex_pos doesn't know 
   * what kind of unification applies, so try them all.
   */

  if (pos->tr != NULL)
    undo_subst(pos->tr);
  else if (pos->btm_position != NULL)
    match_bt_cancel(pos->btm_position);
  else if (pos->btu_position != NULL)
    unify_bt_cancel(pos->btu_position);

  if (pos->index->index_type == FPA)
    fpa_cancel(pos->fpa_position);
  else if (pos->index->index_type == LINEAR)
    ;  /* do nothing */
  else if (pos->index->index_type == DISCRIM_WILD)
    discrim_wild_cancel(pos->discrim_position);
  else if (pos->index->index_type == DISCRIM_BIND)
    discrim_bind_cancel(pos->discrim_position);

  free_mindex_pos(pos);
}  /* mindex_retrieve_cancel */

/*************
 *
 *   fprint_linear_index()
 *
 *************/

static
void fprint_linear_index(FILE *fp, Plist first)
{
  Plist p;
  for (p = first; p != NULL; p = p->next) {
    Term t = p->v;
    fprintf(fp, "FPA_ID=%u: ", (unsigned) FPA_ID(t));
    fprint_term(fp, t);
    fprintf(fp, "\n");
  }
}  /* fprint_linear_index */

/*************
 *
 *   fprint_mindex()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) Mindex mdx.
*/

/* PUBLIC */
void fprint_mindex(FILE *fp, Mindex mdx)
{
  switch (mdx->index_type) {
  case LINEAR:
    fprintf(fp, "\nThis is an Mindex of type LINEAR.\n");
    fprint_linear_index(fp, mdx->linear_first);
    break;
  case FPA:
    fprintf(fp, "\nThis is an Mindex of type FPA.\n");
    fprint_fpa_index(fp, mdx->fpa);
    break;
  case DISCRIM_WILD:
    fprintf(fp, "\nThis is an Mindex of type DISCRIM_WILD.\n");
    fprint_discrim_wild_index(fp, mdx->discrim_tree);
    break;
  case DISCRIM_BIND:
    fprintf(fp, "\nThis is an Mindex of type DISCRIM_BIND.\n");
    fprint_discrim_bind_index(fp, mdx->discrim_tree);
    break;
  }
}  /* fprint_mindex */
