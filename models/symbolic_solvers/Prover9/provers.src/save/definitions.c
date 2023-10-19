#include "definitions.h"

/* Private definitions and types */

/*************
 *
 *   relation_in_clause() -- does the relation symbol occur in the clause?
 *
 *************/

static
BOOL relation_in_clause(int symnum, Clause c)
{
  Literal lit;
  for (lit = c->literals; lit; lit = lit->next)
    if (SYMNUM(lit->atom) == symnum)
      return TRUE;
  return FALSE;
}  /* relation_in_clause */

/*************
 *
 *   segregate_by_relation()
 *
 *************/

static
void segregate_by_relation(int symnum, Plist work, Clist with, Clist without)
{
  if (work != NULL) {
    Clause c = work->v;
    segregate_by_relation(symnum, work->next, with, without);
    if (relation_in_clause(symnum, c))
      clist_prepend(c, with);
    else
      clist_prepend(c, without);
    free_plist(work);
  }
}  /* segregate_by_relation */

/*************
 *
 *  resolve_lists_on_relation()
 *
 *  Given 2 lists of clauses, "defs" and "withs", consider the cross
 *  product and do binary resolution.
 *
 *************/

static
Plist resolve_lists_on_relation(int symnum,
				Clist defs,
				Clist withs)
{
  Plist resolvents = NULL;
  Clist_pos p1, p2;
  for (p1 = defs->first; p1; p1 = p1->next) {
    for (p2 = withs->first; p2; p2 = p2->next) {
      Clause def = p1->c;
      Clause with = p2->c;
      /* Use first literal of "def".  In "with" do at most one resolution
	 with that first literal of "def". */
      Literal dlit = def->literals;
      Literal wlit;
      BOOL ok;
      for (wlit = with->literals, ok = FALSE; wlit && !ok; wlit = wlit->next) {
	if (SYMNUM(dlit->atom) == symnum &&
	    SYMNUM(wlit->atom) == symnum &&
	    dlit->sign != wlit->sign) {
	  Clause res = resolve3(def, dlit, with, wlit);
	  if (res) {
	    assign_clause_id(res);
	    resolvents = plist_append(resolvents, res);
	    ok = TRUE;
	  }
	}
      }
    }
  }
  return resolvents;
}  /* resolve_lists_on_relation */

/*************
 *
 *   get_first_relations()
 *
 *************/

static
Ilist get_first_relations(Clist a)
{
  Ilist symnums = NULL;
  Clist_pos p;
  for (p = a->first; p; p = p->next) {
    Literal lit = ith_literal(p->c, 1);
    if (lit && !ilist_member(symnums, SYMNUM(lit->atom)))
      symnums = ilist_append(symnums, SYMNUM(lit->atom));
  }
  return symnums;
}  /* get_first_relations */

/*************
 *
 *   expand_relation()
 *
 *************/

static
void expand_relation(int symnum, Clist s, Clist defs, Clist disabled)
{
  Clist with    = clist_init("with");
  Plist work = NULL;

  /* move clauses in s to working list */

  while (s->first) {
    Clause c = s->first->c;
    clist_remove(c, s);
    work = plist_append(work, c);
  }

  /* expand with defs, putting fully expanded clauses back in s */

  segregate_by_relation(symnum, work, with, s);
  while (with->first) {
    work = resolve_lists_on_relation(symnum, defs, with);
    clist_move_clauses(with, disabled);
    segregate_by_relation(symnum, work, with, s);
  }
  clist_free(with);
}  /* expand_relation */

/*************
 *
 *   expand_definitions()
 *
 *************/

/* DOCUMENTATION
Given a list of clasues, expand with relational definitions.
That is, resolve away all of the defined literals.
The expanded clauses are appended to "clauses",
and the clauses containing defined literals (including
intermediate resolvents) are appended to "disabled".
*/

/* PUBLIC */
void expand_definitions(Clist clauses, Clist defs, Clist disabled)
{
  if (clauses->first) {
    Ilist symnums = get_first_relations(defs);
    Ilist p;
    printf("\n%% Before expanding %s:\n", clauses->name);
    fwrite_clause_clist(stdout, clauses, TRUE, TRUE);
    for (p = symnums; p; p = p->next) {
      int symnum = p->i;
      expand_relation(symnum, clauses, defs, disabled);
    }
    printf("\n%% After expanding %s:\n", clauses->name);
    fwrite_clause_clist(stdout, clauses, TRUE, TRUE);
  }
}  /* expand_definitions */

/*************
 *
 *   strip_uvars()
 *
 *************/

static
Formula strip_uvars(Formula f)
{
  if (f->type == ALL_FORM)
    return strip_uvars(f->kids[0]);
  else
    return f;
}  /* strip_uvars */

/*************
 *
 *   get_uvars()
 *
 *************/

static
Ilist get_uvars(Formula f)
{
  if (f->type == ALL_FORM)
    return ilist_prepend(get_uvars(f->kids[0]), str_to_sn(f->qvar, 0));
  else
    return NULL;
}  /* get_uvars */

/*************
 *
 *   check_definition()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL check_definition(Formula f)
{
  Formula iff = strip_uvars(f);
  if (iff->type != IFF_FORM || iff->kids[0]->type != ATOM_FORM)
    return FALSE;
  else {
    Ilist uvars = get_uvars(f);    /* list of char* */
    Term r = iff->kids[0]->atom;
    Ilist args = NULL;
    int i;
    for (i = 0; i < ARITY(r); i++) {
      Term arg = ARG(r,i);
      if (!CONSTANT(arg))
	return FALSE;
      else
	args = ilist_append(args, SYMNUM(arg));
    }
    /* Check that args and uvars are sets with the same members. */
    return (ilist_is_set(uvars) &&
	    ilist_is_set(args) &&
	    ilist_subset(uvars, args) &&
	    ilist_subset(args, uvars));
  }
}  /* check_definition */

/*************
 *
 *   check_definitions()
 *
 *************/

/* DOCUMENTATION
Check a list of definitions.  See check_definition.
*/

/* PUBLIC */
void check_definitions(Plist formulas)
{
  Plist p;
  BOOL ok = TRUE;
  for (p = formulas; p; p = p->next) {
    if (!check_definition(p->v)) {
      ok = FALSE;
      printf("bad definition: ");
      fwrite_formula(stdout, p->v);
    }
  }
  if (!ok)
    fatal_error("bad definition(s)");
}  /* check_definitions */

