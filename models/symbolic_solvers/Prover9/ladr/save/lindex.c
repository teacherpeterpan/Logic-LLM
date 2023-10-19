#include "lindex.h"

/* Private definitions and types */

/*
 * memory management
 */

static unsigned Lindex_gets, Lindex_frees;

#define BYTES_LINDEX sizeof(struct lindex)
#define PTRS_LINDEX BYTES_LINDEX%BPP == 0 ? BYTES_LINDEX/BPP : BYTES_LINDEX/BPP + 1

/*************
 *
 *   Lindex get_lindex()
 *
 *************/

static
Lindex get_lindex(void)
{
  Lindex p = get_mem(PTRS_LINDEX);
  Lindex_gets++;
  return(p);
}  /* get_lindex */

/*************
 *
 *    free_lindex()
 *
 *************/

static
void free_lindex(Lindex p)
{
  free_mem(p, PTRS_LINDEX);
  Lindex_frees++;
}  /* free_lindex */

/*************
 *
 *   fprint_lindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the lindex package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_lindex_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_LINDEX;
  fprintf(fp, "lindex (%4d)       %11u%11u%11u%9.1f K\n",
          n, Lindex_gets, Lindex_frees,
          Lindex_gets - Lindex_frees,
          ((Lindex_gets - Lindex_frees) * n) / 1024.);

}  /* fprint_lindex_mem */

/*************
 *
 *   p_lindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the lindex package.
*/

/* PUBLIC */
void p_lindex_mem()
{
  fprint_lindex_mem(stdout, TRUE);
}  /* p_lindex_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   lindex_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns a literal index (Lindex), which is
a pair of Mindexes, one for positive literals, and one for negative literals.
The first three parameters are for the positive literal Mindex,
and the second three are for the negative.
<P>
<UL>
<LI><TT>Mindextype: {LINEAR, FPA, DISCRIM, DISCRIM_BIND}</TT>
<LI><TT>Uniftype:   {ORDINARY_UNIF, BACKTRACK_UNIF}</TT>
<LI><TT>fpa_depth: </TT>depth of FPA indexing
    (ignored for other index types).
</UL>
See the routine mindex_init() for further information.
*/

/* PUBLIC */
Lindex lindex_init(Mindextype pos_mtype, Uniftype pos_utype, int pos_fpa_depth,
		   Mindextype neg_mtype, Uniftype neg_utype, int neg_fpa_depth)
{
  Lindex ldx = get_lindex();

  ldx->pos = mindex_init(pos_mtype, pos_utype, pos_fpa_depth);
  ldx->neg = mindex_init(neg_mtype, neg_utype, neg_fpa_depth);

  return ldx;
}  /* lindex_init */

/*************
 *
 *   lindex_destroy()
 *
 *************/

/* DOCUMENTATION
This frees all the memory associated with a Lindex.  Do not refer
to the Lindex after calling this routine.
*/

/* PUBLIC */
void lindex_destroy(Lindex ldx)
{
  mindex_destroy(ldx->pos);
  mindex_destroy(ldx->neg);
  free_lindex(ldx);
}  /* lindex_destroy */

/*************
 *
 *   lindex_update()
 *
 *************/

/* DOCUMENTATION
This routine indexes (or unindexes) all literals of a clause.
*/

/* PUBLIC */
void lindex_update(Lindex ldx, Clause c, Indexop op)
{
  Literal lit;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    if (lit->sign)
      mindex_update(ldx->pos, lit->atom, op);
    else
      mindex_update(ldx->neg, lit->atom, op);
  }
}  /* lindex_update */

/*************
 *
 *   lindex_update_maximal()
 *
 *************/

/* DOCUMENTATION
Positive clauses: index maximal literals.
<P>
Nonpositive clauses: index negative literals.
*/

/* PUBLIC */
void lindex_update_maximal(Lindex ldx, Clause c, Indexop op)
{
  BOOL positive = positive_clause(c);
  Literal lit;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    if (positive) {
      if (maximal_literal_check(lit))
	mindex_update(ldx->pos, lit->atom, op);
    }
    else {
      if (lit->sign == FALSE)
	mindex_update(ldx->neg, lit->atom, op);
    }
  }
}  /* lindex_update_maximal */

/*************
 *
 *   lindex_update_first()
 *
 *************/

/* DOCUMENTATION
This routine indexes (or unindexes) the first literal of a clause.
*/

/* PUBLIC */
void lindex_update_first(Lindex ldx, Clause c, Indexop op)
{
  Literal lit = c->literals;
  if (lit) {
    if (lit->sign)
      mindex_update(ldx->pos, lit->atom, op);
    else
      mindex_update(ldx->neg, lit->atom, op);
  }
}  /* lindex_update_first */

/*************
 *
 *   lindex_empty()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if an Lindex is empty, that is, has no
atoms.  It must exist (be non-NULL).
*/

/* PUBLIC */
BOOL lindex_empty(Lindex idx)
{
  return mindex_empty(idx->pos) && mindex_empty(idx->neg);
}  /* lindex_empty */

/*************
 *
 *   lindex_backtrack()
 *
 *************/

/* DOCUMENTATION
This Boolean function checks if either of the Mindex components (pos, neg)
uses backtrack unification.
*/

/* PUBLIC */
BOOL lindex_backtrack(Lindex idx)
{
  return (idx->pos->unif_type == BACKTRACK_UNIF ||
	  idx->neg->unif_type == BACKTRACK_UNIF);
}  /* lindex_backtrack */
