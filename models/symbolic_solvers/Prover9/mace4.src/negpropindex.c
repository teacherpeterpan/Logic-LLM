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

#include "msearch.h"

extern Symbol_data Symbols;
extern Symbol_data *Sn_to_mace_sn;
extern int Domain_size;
extern int Relation_flag;
extern int Negation_flag;
extern int Eq_sn;

void ****Index;

/* The index is a simple discrimination tree in which each
   node is simply an array of pointers to the children.

   0:                                 Index                        void ****
   1: sign             negative                positive            void ***
   2: symbol      sym1    sym2    sym3     sym1    sym2    sym3    void **
   3: value      v1 v2   v1 v2   v1 v2    v1 v2   v1 v2   v1 v2    void *
   4:     domain element of arg 0 plus the special value "eterm"
   5:     domain element of arg 1 plus the special value "eterm"
   etc.

   After level 3, the processing is recursive, and the * on the void
   get replenished.  That's what makes the typing kludgy, but the
   compiler accepts it.

   The term space is not very big, so we construct a full tree at the start.
*/

/*************
 *
 *   init_recurse()
 *
 *************/

static
void *init_recurse(int n)
{
  if (n == 0)
    return NULL;
  else {
    void **p;
    int i;
    p = calloc(Domain_size+1, sizeof(void *));
    for (i = 0; i < Domain_size+1; i++)
      p[i] = init_recurse(n-1);  /* assign a void** to a void* */
    return p;
  }
}  /* init_recurse */

/*************
 *
 *   init_negprop_index()
 *
 *************/

void init_negprop_index(void)
{
  int sign, sym, val;
  int num_syms = 0;
  Symbol_data p;

  for(p = Symbols; p != NULL; p = p->next)
    num_syms++;

  Index = calloc(2, sizeof(void *));

  for (sign = 0; sign < 2; sign++) {
    Index[sign] = calloc(num_syms, sizeof(void *));

    for (sym = 0, p = Symbols; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
	  !(p->type == RELATION && sign == 0)) {

	int range_size = (p->type == FUNCTION ? Domain_size : 2);
	Index[sign][sym] = calloc(range_size, sizeof(void *));

	for (val = 0; val < range_size; val++) {
	  Index[sign][sym][val] = init_recurse(p->arity);
	}
      }
    }
  }
}  /* init_negprop_index */

/*************
 *
 *   free_recurse()
 *
 *************/

static
void free_recurse(int n, void **p)
{
  if (n == 0)
    return;
  else {
    int i;
    for (i = 0; i < Domain_size+1; i++)
      free_recurse(n-1, p[i]);
    free(p);
  }
}  /* free_recurse */

/*************
 *
 *   free_negprop_index()
 *
 *************/

void free_negprop_index(void)
{
  int sign, sym, val;
  int num_syms = 0;
  Symbol_data p;

  for(p = Symbols; p != NULL; p = p->next)
    num_syms++;

  for (sign = 0; sign < 2; sign++) {

    for (sym = 0, p = Symbols; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
	  !(p->type == RELATION && sign == 0)) {

	int range_size = (p->type == FUNCTION ? Domain_size : 2);

	for (val = 0; val < range_size; val++) {
	  free_recurse(p->arity, Index[sign][sym][val]);
	}
	free(Index[sign][sym]);
      }
    }
    free(Index[sign]);
  }
  free(Index);
  Index = NULL;
}  /* free_negprop_index */

/*************
 *
 *   p_recurse()
 *
 *************/

static
void p_recurse(void **p, int x, int n, int depth)
{
  int j;
  for (j = 0; j < depth; j++)
    printf("    ");
  printf("[%d] %p", x, p);
  if (n == 0) {
    if (p != NULL) {
      Term t;
      for (t = (Term) p; t != NULL; t = t->u.vp) {
	printf(" : ");
	fwrite_term(stdout, (Term) t);
      }
    }
    printf("\n");
  }
  else {
    int i;
    printf("\n");
    for (i = 0; i < Domain_size+1; i++) {
      p_recurse(p[i], i, n-1, depth+1);
    }
  }
}  /* p_recurse */

/*************
 *
 *   p_negprop_index()
 *
 *************/

void p_negprop_index(void)
{
  int sign, sym, val;
  int num_syms = 0;
  Symbol_data p;

  for(p = Symbols; p != NULL; p = p->next)
    num_syms++;

  for (sign = 0; sign < 2; sign++) {
    for (sym = 0, p = Symbols; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
	  !(p->type == RELATION && sign == 0)) {

	int range_size = (p->type == FUNCTION ? Domain_size : 2);
	for (val = 0; val < range_size; val++) {
	  printf("%s %s val=%d\n",
		 sign == 0 ? "~" : "+",
		 sn_to_str(p->sn),
		 val);
	  p_recurse(Index[sign][sym][val], -1, p->arity, 1);
	}
      }
    }
  }
}  /* p_negprop_index */

/*************
 *
 *   insert_recurse()
 *
 *************/

static
void insert_recurse(void **p, Term atom, Term t, int n, Mstate state)
{
  Term arg = ARG(t,n);
  int i;
  /* If the argument is not a domain element, then use the
     subtree "Domain_size", which is 1 more than the maximum
     domain element.  Note that many terms correspond to each leaf.
  */
  i = (VARIABLE(arg) ? VARNUM(arg) : Domain_size);
  if (ARITY(t) == n+1) {
    /* We are at a leaf.  Insert the atom into the list.
       If this is an eterm, then the list will never have more
       than one member.
    */
    if (atom->u.vp != NULL)
      fatal_error("insert_recurse: atom link in use");
    state->stack = update_and_push((void **) &(atom->u.vp),p[i],state->stack);
    state->stack = update_and_push((void **) &(p[i]), atom, state->stack);
  }
  else
    insert_recurse(p[i], atom, t, n+1, state);
}  /* insert_recurse */

/*************
 *
 *   insert_negprop_eq()
 *
 *   Insert an equality atom into the index.  The parameters
 *   alpha and val are used so that we don't have to figure
 *   out (again) the orientation.
 *
 *************/

void insert_negprop_eq(Term atom, Term alpha, int val, Mstate state)
{
  int sign = (NEGATED(atom) ? 0 : 1);
  int sym = Sn_to_mace_sn[SYMNUM(alpha)]->mace_sn;

#if 0
  printf("insert_negprop_eq: ");
  p_matom(atom);
#endif

  if (ARITY(alpha) == 0)
    /* This could be handled if we need it.  The assignments are
       atom->u.p = Index[sign][sym][val];
       Index[sign][sym][val] = atom;
     */
    fatal_error("insert_negprop_eq, arity 0");
  else
    insert_recurse(Index[sign][sym][val], atom, alpha, 0, state);
}  /* insert_negprop_eq */

/*************
 *
 *   insert_negprop_noneq()
 *
 *   Insert a nonequality atom into the index.
 *
 *************/

void insert_negprop_noneq(Term atom, Mstate state)
{
  int val = (NEGATED(atom) ? 0 : 1);
  int sym = Sn_to_mace_sn[SYMNUM(atom)]->mace_sn;

#if 0
  printf("insert_negprop_noneq: ");
  p_matom(atom);
#endif

  if (ARITY(atom) == 0)
    /* This could be handled if we need it.  The assignments are
       atom->u.p = Index[1][sym][val];
       Index[1][sym][val] = atom;
     */
    fatal_error("insert_negprop_noneq, arity 0");
  else
    insert_recurse(Index[1][sym][val], atom, atom, 0, state);
}  /* insert_negprop_noneq */

/*************
 *
 *   nterm()
 *
 *   Check if a term is "nearly evaluable".  That is, exactly
 *   one argument is not a domain element, and that argument
 *   is an eterm.  For example, f(2,g(3)) is an nterm.
 *
 *   If so, set "position" to the index of the eterm argument
 *   and "id" to the ID of the eterm argument.
 *
 *************/

BOOL nterm(Term t, int *ppos, int *pid)
{
  if (t == NULL || VARIABLE(t) || arith_rel_term(t) || arith_op_term(t))
    return FALSE;
  else {
    int i;
    int pos = -1;
    for (i = 0; i < ARITY(t); i++) {
      if (!VARIABLE(ARG(t,i))) {
	if (pos != -1)
	  return FALSE;
	else if (eterm(ARG(t,i), pid))
	  pos = i;
	else
	  return FALSE;
      }
    }
    if (pos == -1)
      return FALSE;
    else {
      *ppos = pos;
      return TRUE;
    }
  }
}  /* nterm */

/*************
 *
 *   negprop_find_near()
 *
 *************/

static Term negprop_find_near_recurse(void **p, Term query, int pos, int n)
{
  int i = (pos == 0 ? Domain_size : VARNUM(ARG(query, n)));
  if (ARITY(query) == n+1)
    return (Term) p[i];
  else
    return negprop_find_near_recurse(p[i], query, pos-1, n+1);
}

Term negprop_find_near(int sign, int sym, int val, Term query, int pos)
{
  return negprop_find_near_recurse(Index[sign][sym][val], query, pos, 0);
}  /* negprop_find_near */

