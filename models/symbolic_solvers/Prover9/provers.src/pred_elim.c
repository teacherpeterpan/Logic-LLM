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

#include "pred_elim.h"

/* Private definitions and types */

/*************
 *
 *   rsym_occurrences() - number of occurrences of a pred symbol in a clause
 *
 *************/

static
int rsym_occurrences(int symbol, Topform c)
{
  int n = 0;
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next)
    if (SYMNUM(lit->atom) == symbol)
      n++;
  return n;
}  /* rsym_occurrences */

/*************
 *
 *   arg_check()
 *
 *************/

static
BOOL arg_check(int symbol, Topform c)
{
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next) {
    if (!lit->sign &&
	SYMNUM(lit->atom) == symbol &&
	!args_distinct_vars(lit->atom))
      return FALSE;
  }
  return TRUE;
}  /* arg_check */

/*************
 *
 *   eliminable_relation()
 *
 *************/

static
BOOL eliminable_relation(int symbol, Plist clauses, BOOL equality)
{
  /* Eliminable if no clause has more than one occurrence.
     Additional constraint for equality: all negative occurrences
     have unique vars as args.
  */
  Plist p;
  for (p = clauses; p; p = p->next) {
    int n = rsym_occurrences(symbol, p->v);
    if (n > 1)
      return FALSE;
    else if (equality && !arg_check(symbol, p->v))
      return FALSE;
  }
  return TRUE;
}  /* eliminable_relation */

/*************
 *
 *   eliminable_relations()
 *
 *************/

static
Ilist eliminable_relations(Plist clauses, BOOL equality)
{
  Ilist rsyms = rsym_set_in_topforms(clauses);
  Ilist eliminable = NULL;
  Ilist p;
  for (p = rsyms; p; p = p->next) {
    if (!is_eq_symbol(p->i) &&          /* don't allow equalities */
	sn_to_arity(p->i) != 0 &&      /* don't allow prop atoms */
	eliminable_relation(p->i, clauses, equality))

      eliminable = ilist_append(eliminable, p->i); 
  }
  zap_ilist(rsyms);
  return eliminable;
}  /* eliminable_relations */

/*************
 *
 *   resolve_on_symbol()
 *
 *************/

static
Plist resolve_on_symbol(int symbol, Topform c1, Topform c2)
{
  Plist resolvents = NULL;
  Literals l1, l2;
  for (l1 = c1->literals; l1; l1 = l1->next)
    for (l2 = c2->literals; l2; l2 = l2->next) {
      if (l1->sign != l2->sign &&
	  SYMNUM(l1->atom) == symbol &&
	  SYMNUM(l2->atom) == symbol) {
	Topform res;
	if (c1->id == 0) {
	  assign_clause_id(c1);
	  fwrite_clause(stdout, c1, CL_FORM_STD);
	}
	if (c2->id == 0) {
	  assign_clause_id(c2);
	  fwrite_clause(stdout, c2, CL_FORM_STD);
	}
	res = resolve3(c1, l1, c2, l2, TRUE);
	if (res) {
	  if (tautology(res->literals))
	    delete_clause(res);
	  else {
	    resolvents = plist_append(resolvents, res);
	  }
	}
      }
    }
  return resolvents;
}  /* resolve_on_symbol */

/*************
 *
 *   gen_given()
 *
 *************/

static
Plist gen_given(int symbol, Topform given, Plist usable)
{
  Plist new = NULL;
  Plist p;
  for (p = usable; p; p = p->next) {
    Topform c = p->v;
    Plist resolvents = resolve_on_symbol(symbol, given, c);
    new = plist_cat(new, resolvents);
  }
  return new;
}  /* gen_given */

/*************
 *
 *   subsumed_by_member()
 *
 *************/

static
BOOL subsumed_by_member(Topform c, Plist p)
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
 *   incorporate_new_clauses()
 *
 *************/

static
Plist incorporate_new_clauses(Plist sos, Plist new, BOOL echo)
{
  Plist p;
  for (p = new; p; p = p->next) {
    if (!subsumed_by_member(p->v, sos)) {
      sos = plist_append(sos, p->v);
      printf("Derived: ");
      fwrite_clause(stdout, p->v, CL_FORM_STD);
    }
  }
  zap_plist(new);
  return sos;
}  /* incorporate_new_clauses */

/*************
 *
 *   elim_relation()
 *
 *************/

static
Plist elim_relation(int symbol, Plist sos, Clist disabled, BOOL echo)
{
  /* this does a naive given-clause loop */
  Plist usable = NULL;
  while (sos) {
    Plist new;
    Topform given = sos->v;
    sos = plist_pop(sos);
    usable = plist_append(usable, given);
#ifdef GEN_DEBUG
    printf("\ngiven: "); f_clause(given);
#endif
    new = gen_given(symbol, given, usable);
    sos = incorporate_new_clauses(sos, new, echo);
  }
  /* partition usable into clauses with and without symbol */
  {
    Plist without = NULL;
    Plist p;
    for (p = usable; p; p = p->next) {
      if (rsym_occurrences(symbol, p->v) > 0)
	clist_append(p->v, disabled);
      else
	without = plist_append(without, p->v);
    }
    zap_plist(usable);
    return without;
  }
}  /* elim_relation */

/*************
 *
 *   predicate_elimination()
 *
 *************/

/* DOCUMENTATION
Apply predicate elimination to a Clist of clauses.  Clauses that are
eliminated are moved to the disabled list, and new clauses are appended
to the list of clauses.  Any new clauses will have binary resolution
justifications.
<p>
The initial clauses should not have IDs.  All clauses that
are used for resolution are given IDs, but the clauses in
the end result do not have IDs.
*/

/* PUBLIC */
void predicate_elimination(Clist clauses, Clist disabled, BOOL echo)
{
  Plist clauses2 = prepend_clist_to_plist(NULL, clauses);
  BOOL equality = equality_in_clauses(clauses2);  /* eq => different method */
  Ilist syms = eliminable_relations(clauses2, equality);

  if (syms == NULL) {
    zap_plist(clauses2);
    if (echo)
      printf("\nNo predicates eliminated.\n");
  }
  else {
    clist_remove_all_clauses(clauses);
    while (syms) {
      /* use first symbol, discard rest, get new list */
      if (echo)
	  printf("\nEliminating %s/%d\n",
		 sn_to_str(syms->i),
		 sn_to_arity(syms->i));
	
      clauses2 = elim_relation(syms->i, clauses2, disabled, echo);
      zap_ilist(syms);
      syms = eliminable_relations(clauses2, equality);
    }
    clist_append_plist(clauses, clauses2);
    zap_plist(clauses2);
  }
}  /* predicate_elimination */

