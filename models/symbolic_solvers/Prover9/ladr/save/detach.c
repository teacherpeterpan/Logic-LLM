#include "detach.h"

/* Private definitions and types */

/*************
 *
 *   detachable()
 *
 *************/

static
BOOL detachable(Clause c)
{
  return
    unit_clause(c) &&
    positive_clause(c) &&
    ARITY(c->literals->atom) == 1 &&
    ARITY(ARG(c->literals->atom,0)) == 2;
}  /* detachable */

/*************
 *
 *   detach()
 *
 *************/

static
Clause detach(Clause c1, Clause c2)
{
  /* Assume each is positive unit in unary relation with binary arg.
   * Condensed detach with major premise c1 and minor premise c2.
   */
  Term major = ARG(c1->literals->atom,0);
  Term alpha = ARG(major,0);
  Term beta  = ARG(major,1);
  Term minor = ARG(c2->literals->atom,0);
  Context s1 = get_context();
  Context s2 = get_context();
  Trail tr = NULL;
  Clause result;

  if (unify(alpha, s1, minor, s2, &tr)) {
    Term atom = get_rigid_term_like(c2->literals->atom);
    Literal lit = new_literal(1, atom);

    ARG(atom,0) = apply(beta, s1);
    result = get_clause();
    append_literal(result, lit);
    result->justification = cd_just(c1, c2);
    inherit_attributes(c1, s1, c2, s2, result);
    undo_subst(tr);
    upward_clause_links(result);
  }
  else
    result = NULL;

  free_context(s1);
  free_context(s2);
  return result;
}  /* detach */

/*************
 *
 *   cond_detach()
 *
 *************/

/* DOCUMENTATION
Attempt a condensed detachment.
*/

/* PUBLIC */
void cond_detach(Clause major, Clause minor, void (*proc_proc) (Clause))
{
  /* First make sure that both are positive units of the form p(i(a,b)),
   * for any p, i, a, b.
   */

  if (detachable(major) && detachable(minor)) {
    Clause cd = detach(major, minor);
    if (cd != NULL)
      (*proc_proc)(cd);
  }
}  /* cond_detach */

/*************
 *
 *   cond_detach_2()
 *
 *************/

/* DOCUMENTATION
Attempt two condensed detachments, with each clause as major premise.
*/

/* PUBLIC */
void cond_detach_2(Clause c1, Clause c2, void (*proc_proc) (Clause))
{
  cond_detach(c1, c2, proc_proc);
  if (c1 != c2)
    cond_detach(c2, c1, proc_proc);
}  /* cond_detach_2 */
