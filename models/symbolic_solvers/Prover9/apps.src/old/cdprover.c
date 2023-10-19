#include "../ladr/top_input.h"
#include "../ladr/subsume.h"
#include "../ladr/clause2.h"

#define PROGRAM_NAME    "cdprover"
#define PROGRAM_VERSION "2005-A"
#include "../PROGRAM_DATE.h"

/*************
 *
 *   first_lightest()
 *
 *************/

static
Clause first_lightest(Plist p)
{
  if (p == NULL)
    return NULL;
  else {
    Clause a = p->v;
    Clause b = first_lightest(p->next);
    if (b == NULL)
      return a;
    else
      return (a->weight <= b->weight ? a : b);
  }
}  /* first_lightest */

/*************
 *
 *   extract_given_clause()
 *
 *************/

static
Clause extract_given_clause(Plist *lst)
{
  Clause c = first_lightest(*lst);
  *lst = plist_remove(*lst, c);
  return c;
}  /* extract_given_clause */

/*************
 *
 *   subsumed_by_member()
 *
 *************/

static
BOOL subsumed_by_member(Clause c, Plist p)
{
  if (p == NULL)
    return FALSE;
  else if (subsumes(p->v, c))
    return TRUE;
  else
    return subsumed_by_member(c, p->next);
}  /* subsumed_by_member */

/*************
 *
 *   conflicts()
 *
 *************/

static
Clause conflicts(Clause a, Clause b)
{
  if (!unit_clause(a) || !unit_clause(b))
    return NULL;
  else if (a->literals->sign == b->literals->sign)
    return NULL;
  else {
    Clause empty = NULL;
    Term a_atom = a->literals->atom;
    Term b_atom = b->literals->atom;
    Context ca = get_context();
    Context cb = get_context();
    Trail tr = NULL;
    if (unify(a_atom, ca, b_atom, cb, &tr)) {
      Ilist j = NULL;
      undo_subst(tr);
      empty = get_clause();

      j = ilist_append(j, a->id);
      j = ilist_append(j, 1);
      j = ilist_append(j, b->id);
      j = ilist_append(j, 1);
      empty->justification = resolve_just(j, BINARY_RES_JUST);
      upward_clause_links(empty);
      assign_clause_id(empty);
    }
    free_context(ca);
    free_context(cb);
    return empty;
  }
}  /* conflicts */

/*************
 *
 *   conflicts_with_member()
 *
 *************/

static
Clause conflicts_with_member(Clause c, Plist p)
{
  if (p == NULL)
    return NULL;
  else {
    Clause empty = conflicts(c, p->v);
    if (empty)
      return empty;
    else
      return conflicts_with_member(c, p->next);
  }
}  /* conflicts_with_member */

/*************
 *
 *   cd()
 *
 *************/

static
Clause cd(Clause maj, Clause min)
{
  if (!unit_clause(maj) || !unit_clause(min))
    return NULL;
  else if (!maj->literals->sign || !min->literals->sign)
    return NULL;
  else {
    Term a = ARG(maj->literals->atom,0);
    Term b = ARG(min->literals->atom,0);
    if (ARITY(a) != 2)
      return NULL;
    else {
      Clause resolvent = NULL;
      Term a0 = ARG(a,0);
      Term a1 = ARG(a,1);
      Context ca = get_context();
      Context cb = get_context();
      Trail tr = NULL;
      if (unify(a0, ca, b, cb, &tr)) {
	Term r = apply(a1, ca);
	Term r_atom = build_unary_term(SYMNUM(maj->literals->atom), r);
	Literal r_literal = get_literal();
	Ilist j = NULL;
	r_literal->sign = TRUE;
	r_literal->atom = r_atom;
	resolvent = get_clause();
	append_literal(resolvent, r_literal);
	
	j = ilist_append(j, maj->id);
	j = ilist_append(j, 1);
	j = ilist_append(j, min->id);
	j = ilist_append(j, 1);

	resolvent->justification = resolve_just(j, BINARY_RES_JUST);
	upward_clause_links(resolvent);
	renumber_variables(resolvent, MAX_VARS);

	undo_subst(tr);
      }
      free_context(ca);
      free_context(cb);
      return resolvent;
    }
  }
}  /* cd */

/*************
 *
 *   cd_given()
 *
 *************/

static
Plist cd_given(Clause given, Plist lst)
{
  Plist resolvents = NULL;
  Plist p;

  for (p = lst; p != NULL; p = p->next) {
    Clause other = p->v;

    Clause result = cd(given, other);
    if (result)
      resolvents = plist_append(resolvents, result);

    result = cd(other, given);
    if (result)
      resolvents = plist_append(resolvents, result);
  }
  return resolvents;
}  /* cd_given */

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  Plist sos, goals, usable, p;
  int given_count = 0;
  int generated_count = 0;
  int kept_count = 0;
  int just_att_id;

  init_standard_ladr();

  just_att_id = register_attribute("just", TERM_ATTRIBUTE);

  sos    = read_clause_list(stdin, stderr, TRUE);
  goals  = read_clause_list(stdin, stderr, TRUE);
  usable = NULL;

  fwrite_clause_list(stdout, sos, "sos", CL_FORM_BARE);
  fwrite_clause_list(stdout, goals, "goals", CL_FORM_BARE);

  for (p = sos; p; p = p->next) {
    Clause c = p->v;
    Term just = get_term_attribute(c->attributes, just_att_id, 1);
    c->weight = clause_symbol_count(c);
    if (just) {
      int i;
      for (i = 0; i < ARITY(just); i++) {
	Term arg = ARG(just,i);
	int val;
	if (term_to_int(arg, &val)) {
	  printf("justfication arg = %d\n", val);
	}
      }
    }
  }

  while (sos != NULL) {
    Plist resolvents, p;
    Clause given = extract_given_clause(&sos);  /* updates sos */
    given_count++;
    printf("given #%d (wt=%d): ", given_count, given->weight);
    fwrite_clause(stdout, given, CL_FORM_STD);
    usable = plist_append(usable, given);

    resolvents = cd_given(given, usable);
    for (p = resolvents; p; p = p->next) {
      Clause res = p->v;
      generated_count++;
      if (subsumed_by_member(res, usable) ||
	  subsumed_by_member(res, sos))
	delete_clause(res);
      else {
	Clause empty_clause;
	kept_count++;
	assign_clause_id(res);
	res->weight = clause_symbol_count(res);
    
	sos = plist_append(sos, res);
	printf("kept (wt=%d): ", res->weight);
	fwrite_clause(stdout, res, CL_FORM_BARE);

	empty_clause = conflicts_with_member(res, goals);
	if (empty_clause) {
	  Plist proof = get_clause_ancestors(empty_clause);
	  printf("\n-------- PROOF --------\n");
	  for (p = proof; p; p = p->next)
	    fwrite_clause(stdout, p->v, CL_FORM_BARE);
	  printf("\n-------- end of proof  -------\n");
	  zap_plist(proof);  /* shallow */
	  exit(0);
	}
      }
    }
  }
  printf("exiting %s\n", PROGRAM_NAME);
  exit(0);
}  /* main */
