#include "clash.h"

/* Private definitions and types */

/*
 * memory management
 */

static unsigned Clash_gets, Clash_frees;

#define BYTES_CLASH sizeof(struct clash)
#define PTRS_CLASH BYTES_CLASH%BPP == 0 ? BYTES_CLASH/BPP : BYTES_CLASH/BPP + 1

/*************
 *
 *   Clash get_clash()
 *
 *************/

static
Clash get_clash(void)
{
  Clash p = get_mem(PTRS_CLASH);
  Clash_gets++;
  return(p);
}  /* get_clash */

/*************
 *
 *    free_clash()
 *
 *************/

static
void free_clash(Clash p)
{
  free_mem(p, PTRS_CLASH);
  Clash_frees++;
}  /* free_clash */

/*************
 *
 *   fprint_clash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clash package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_clash_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_CLASH;
  fprintf(fp, "clash (%4d)        %11u%11u%11u%9.1f K\n",
          n, Clash_gets, Clash_frees,
          Clash_gets - Clash_frees,
          ((Clash_gets - Clash_frees) * n) / 1024.);

}  /* fprint_clash_mem */

/*************
 *
 *   p_clash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clash package.
*/

/* PUBLIC */
void p_clash_mem()
{
  fprint_clash_mem(stdout, TRUE);
}  /* p_clash_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   append_clash()
 *
 *************/

/* DOCUMENTATION
This routine simply allocates a new clash node, links it in after the
given node, and returns the new node.
*/

/* PUBLIC */
Clash append_clash(Clash p)
{
  Clash q = get_clash();
  if (p != NULL)
    p->next = q;
  return q;
}  /* append_clash */

/*************
 *
 *   zap_clash()
 *
 *************/

/* DOCUMENTATION
Free a clash list.  Contexts in clashable nodes (which are assumed to exist
and be empty) are freed as well.
*/

/* PUBLIC */
void  zap_clash(Clash p)
{
  while (p != NULL) {
    Clash q = p;
    p = p->next;
    if (q->clashable && !q->clashed)
      free_context(q->sat_subst);
    free_clash(q);
  }
}  /* zap_clash */

/*************
 *
 *   atom_to_literal()
 *
 *************/

/* DOCUMENTATION
This routine takes an atom and returns its parent literal.
*/

/* PUBLIC */
Literal atom_to_literal(Term atom)
{
  Clause c = atom->container;
  Literal lit = (c == NULL ? NULL : c->literals);
  while (lit != NULL && lit->atom != atom)
    lit = lit->next;
  return lit;
}  /* atom_to_literal */

/*************
 *
 *   apply_lit()
 *
 *************/

/* DOCUMENTATION
This routine applies a substitution to a literal and returns the
instance.  The given literal is not changed.
*/

/* PUBLIC */
Literal apply_lit(Literal lit, Context c)
{
  return new_literal(lit->sign, apply(lit->atom, c));
}  /* apply_lit */

/*************
 *
 *   lit_position()
 *
 *************/

static
int lit_position(Clause c, Literal lit)
{
  int i = 1;
  Literal l = c->literals;
  while (l != NULL && l != lit) {
    i++;
    l = l->next;
  }
  if (l == lit)
    return i;
  else
    return -1;
}  /* lit_position */

/*************
 *
 *   resolve()
 *
 *************/

static
Clause resolve(Clash first, Just_type rule)
{
  Clause r = get_clause();
  Clause nuc =  first->nuc_lit->atom->container;
  Ilist j = ilist_append(NULL, nuc->id);
  Clash p;
  int n;

  /* First, include literals in the nucleus. */
  for (p = first; p != NULL; p = p->next, n++) {
    if (!p->clashed)
      append_literal(r, apply_lit(p->nuc_lit, p->nuc_subst));
  }

  r->attributes = cat_att(r->attributes,
			  inheritable_att_instances(nuc->attributes,
						    first->nuc_subst));

  /* Next, include literals in the satellites. */

  n = 1;  /* n-th nucleus literal, starting with 1 */
  for (p = first; p != NULL; p = p->next, n++) {
    if (p->clashed) {
      Literal lit;
      Clause sat = p->sat_lit->atom->container;
      j = ilist_append(j, n);
      j = ilist_append(j, sat->id);
      j = ilist_append(j, lit_position(sat, p->sat_lit));
      for (lit = sat->literals; lit != NULL; lit = lit->next) {
	if (lit != p->sat_lit)
	  append_literal(r, apply_lit(lit,  p->sat_subst));
      }
      r->attributes = cat_att(r->attributes,
			      inheritable_att_instances(sat->attributes,
							p->sat_subst));
    }
  }
  r->justification = resolve_just(j, rule);
  upward_clause_links(r);
  return r;
}  /* resolve */

/*************
 *
 *   clash_recurse()
 *
 *************/

static
void clash_recurse(Clash first,
		   Clash p,
		   BOOL (*sat_test) (Literal),
		   Just_type rule,
		   void (*proc_proc) (Clause))
{
  if (p == NULL) {
    Clause resolvent = resolve(first, rule);
    (*proc_proc)(resolvent);
  }
  else if (!p->clashable | p->clashed)
    clash_recurse(first, p->next, sat_test, rule, proc_proc);
  else {
    Term fnd_atom;
    fnd_atom = mindex_retrieve_first(p->nuc_lit->atom, p->mate_index, UNIFY,
				     p->nuc_subst, p->sat_subst,
				     FALSE, &(p->mate_pos));
    while (fnd_atom != NULL) {
      Literal slit = atom_to_literal(fnd_atom);
      if ((*sat_test)(slit)) {
	p->sat_lit = slit;
	p->clashed = TRUE;
	clash_recurse(first, p->next, sat_test, rule, proc_proc);
	p->clashed = FALSE;
      }
      fnd_atom = mindex_retrieve_next(p->mate_pos);
    }
  }
}  /* clash_recurse */

/*************
 *
 *   clash()
 *
 *************/

/* DOCUMENTATION
This routine takes a complete clash list and computes the
resolvents.
<UL>
<LI>clash -- a complete clash list corresponding to the nucleus.
<LI>sat_test -- a Boolean function on clauses which identifies
potential satellites (e.g., positive clauses for hyperresolution).
<LI>rule -- the name of the inference rule for the justification list
(see just.h).
<LI>proc_proc -- procedure for processing resolvents.
</UL>
*/

/* PUBLIC */
void clash(Clash c,
	   BOOL (*sat_test) (Literal),
	   Just_type rule,
	   void (*proc_proc) (Clause))
{
  clash_recurse(c, c, sat_test, rule, proc_proc);
}  /* clash */

