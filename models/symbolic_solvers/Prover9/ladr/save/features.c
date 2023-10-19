#include "features.h"

/* Private definitions and types */

static Ilist Feature_symbols;  /* list of featured symbols (symnums)*/

/* The following are work arrays, indexed by symnum, used for calculating
   the features of a clause.  They are allocated by init_features() and
   left in place throughout the process.
*/

static int *Pos_occurrences;
static int *Neg_occurrences;
static int *Pos_maxdepth;
static int *Neg_maxdepth;

/*************
 *
 *   init_features()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_features(Ilist fsyms, Ilist rsyms)
{
  Feature_symbols = ilist_cat(ilist_copy(fsyms), ilist_copy(rsyms));
  Pos_occurrences = calloc(greatest_symnum() + 1, sizeof(int));
  Neg_occurrences = calloc(greatest_symnum() + 1, sizeof(int));
  Pos_maxdepth    = calloc(greatest_symnum() + 1, sizeof(int));
  Neg_maxdepth    = calloc(greatest_symnum() + 1, sizeof(int));
}  /* init_features */

/*************
 *
 *   fill_in_arrays()
 *
 *************/

static
void fill_in_arrays(Term t, BOOL sign, int depth)
{
  if (!VARIABLE(t)) {
    int i;
    if (sign) {
      Pos_occurrences[SYMNUM(t)]++;
      Pos_maxdepth[SYMNUM(t)] = IMAX(depth, Pos_maxdepth[SYMNUM(t)]);
    }
    else {
      Neg_occurrences[SYMNUM(t)]++;
      Neg_maxdepth[SYMNUM(t)] = IMAX(depth, Neg_maxdepth[SYMNUM(t)]);
    }
    for (i = 0; i < ARITY(t); i++)
      fill_in_arrays(ARG(t,i), sign, depth+1);
  }
}  /* fill_in_arrays */

/*************
 *
 *   features()
 *
 *************/

/* DOCUMENTATION
For each symbol in the list of symbols given to init_features(),
count positive occurrences, negative occurrences, max depth in
a positive literal, and max_depth in a negative literal.
Return the Ilist of results.
*/

/* PUBLIC */
Ilist features(Clause c)
{
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next) {
    fill_in_arrays(lit->atom, lit->sign, 0);
  }
  {
    /* Build it backwards, then reverse it. */
    Ilist f = NULL;
    Ilist p;
    for (p = Feature_symbols; p; p = p->next) {
// #define FINTS  /* use an int for each feature */
#ifdef FINTS
      f = ilist_prepend(f, Pos_occurrences[p->i]);
      f = ilist_prepend(f, Neg_occurrences[p->i]);
      f = ilist_prepend(f, Pos_maxdepth[p->i]);
      f = ilist_prepend(f, Neg_maxdepth[p->i]);
#else  /* put 4 features into each int */
      int i1 = IMIN(8,Pos_occurrences[p->i]);
      int i2 = IMIN(8,Neg_occurrences[p->i]);
      int i3 = IMIN(8,Pos_maxdepth[p->i]);
      int i4 = IMIN(8,Neg_maxdepth[p->i]);

      unsigned char u1 = (1 << i1) - 1;
      unsigned char u2 = (1 << i2) - 1;
      unsigned char u3 = (1 << i3) - 1;
      unsigned char u4 = (1 << i4) - 1;

      int i = 0;
      i = (i << 8) | u1;
      i = (i << 8) | u2;
      i = (i << 8) | u3;
      i = (i << 8) | u4;

      f = ilist_prepend(f, i);
#endif
      Pos_occurrences[p->i] = 0;
      Neg_occurrences[p->i] = 0;
      Pos_maxdepth[p->i] = 0;
      Neg_maxdepth[p->i] = 0;
    }
    f = reverse_ilist(f);
#if 0
    printf("Features for clause "); f_clause(c);
    p_features(f);
#endif
    return f;
  }
}  /* features */

/*************
 *
 *   feature_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int feature_length(void)
{
#ifdef FINTS
  return ilist_count(Feature_symbols) * 4;
#else
  return ilist_count(Feature_symbols);
#endif
}  /* feature_length */

/*************
 *
 *   features_less_or_equal()
 *
 *************/

/* DOCUMENTATION
Return TRUE if Ilists c and d are thr same length and
each member of c is <= the corresponding member of d.
*/

/* PUBLIC */
BOOL features_less_or_equal(Ilist c, Ilist d)
{
#ifdef FINTS
  while (c && d && c->i <= d->i) {
#else
  while (c && d && ((c->i | d->i) == d->i)) {
#endif
    c = c->next;
    d = d->next;
  }
  return !c && !d;
}  /* features_less_or_equal */

/*************
 *
 *   p_features()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_features(Ilist f)
{
  Ilist p;
  for (p = Feature_symbols; p; p = p->next) {
#ifdef FINTS
    printf("  symbol %s: ", sn_to_str(p->i));
    printf("pos_occ=%d, neg_occ=%d, pos_max=%d, neg_max=%d\n",
	   f->i, f->next->i, f->next->next->i, f->next->next->next->i);
    f = f->next->next->next->next;
#else
    int i = f->i;
    int i1, i2, i3, i4;
    i4 = i & 0xFF;  i = i >> 8;
    i3 = i & 0xFF;  i = i >> 8;
    i2 = i & 0xFF;  i = i >> 8;
    i1 = i & 0xFF;
    
    printf("  symbol %s: ", sn_to_str(p->i));
    printf("pos_occ=%d, neg_occ=%d, pos_max=%d, neg_max=%d.     %x\n",
	   i1,i2,i3,i4,f->i);
    f = f->next;
#endif
  }
}  /* p_features */

