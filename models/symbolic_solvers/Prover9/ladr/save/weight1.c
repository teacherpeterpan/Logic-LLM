#include "weight1.h"

/* Private definitions and types */

static int Number_of_weights;
static int *Weights;    /* This maps symbol numbers to weights. */
static int Variable_weight = 1;
static int Not_symnum;  /* Cache the negation symbol: it's needed often. */
static int Or_symnum;   /* Cache the disjunction symbol: it's needed often. */

/*************
 *
 *   set_weights_by_arity()
 *
 *************/

static
void set_weights_by_arity(int arity, int weight)
{
  int i;
  for (i = 0; i < Number_of_weights; i++) {
    /* It's OK if symbol i doesn't exist: sn_to_arity will return -1. */
    if (sn_to_arity(i) == arity)
      Weights[i] = weight;
  }
}  /* set_weights_by_arity */

/*************
 *
 *   init_weight1()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_weight1(Plist p, int constant_weight, int variable_weight)
{
  Plist q;
  int i;
  Number_of_weights = greatest_symnum() + 1;

  Weights = malloc(Number_of_weights * sizeof(int));  /* Allocate an array */
  for (i = 0; i < Number_of_weights; i++)
    Weights[i] = 1;  /* default weight of each symbol is 1 */

  for (q = p; q; q = q->next) {
    Term t = q->v;
    int w;
    if (!is_term(t, "weight", 2) || !term_to_int(ARG(t,1), &w))
      fatal_error("init_weight1, bad weight template");
    else {
      Weights[SYMNUM(ARG(t,0))] = w;
    }
  }
  Not_symnum = str_to_sn(NOT_SYM, 1); /* Cache the symnum for neg literals. */
  Or_symnum = str_to_sn(OR_SYM, 2);   /* Cache the symnum for OR. */

  Variable_weight = variable_weight;
  set_weights_by_arity(0, constant_weight);

}  /* init_weight1 */

/*************
 *
 *   term_weight1()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int term_weight1(Term t)
{
  if (VARIABLE(t))
    return Variable_weight;
  else {
    int w = Weights[SYMNUM(t)];
    int i;
    for (i = 0; i < ARITY(t); i++)
      w += term_weight1(ARG(t,i));
    return w;
  }
}  /* term_weight1 */

/*************
 *
 *   clause_weight1()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clause_weight1(Clause c)
{
  Literal l;
  int w = 0;
  for (l = c->literals; l; l = l->next) {
    if (l->sign == FALSE)
      w += Weights[Not_symnum];
    w += term_weight1(l->atom);
    if (l->next)
      w += Weights[Or_symnum];
  }
  return w;
}  /* clause_weight1 */

