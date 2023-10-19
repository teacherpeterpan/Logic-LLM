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
extern int Domain_size;

extern struct cell *Cells;

/*************
 *
 *    Symbol_data get_symbol_data()
 *
 *************/

Symbol_data get_symbol_data(void)
{
  Symbol_data p = tp_alloc(sizeof(struct symbol_data));

  p->sn = 0;
  p->mace_sn = 0;
  p->arity = 0;
  p->type = 0;
  p->base = 0;
  p->attribute = ORDINARY_SYMBOL;
  p->next = NULL;
  return(p);
}  /* get_symbol_data */

/*************
 *
 *    Symbol_data find_symbol_data()
 *
 *************/

Symbol_data find_symbol_data(int sn)
{
  Symbol_data p = Symbols;
  while (p != NULL && p->sn != sn)
    p = p->next;
  return p;
}  /* find_symbol_data */

/*************
 *
 *   init_built_in_symbols()
 *
 *************/

void init_built_in_symbols(void)
{
  Symbol_data s;

  s = get_symbol_data();
  s->sn = str_to_sn(eq_sym(), 2);
  s->arity = 2;
  s->type = RELATION;
  s->attribute = EQUALITY_SYMBOL;
  s->next = Symbols;
  Symbols = s;
}  /* init_built_in_symbols */

/*************
 *
 *   insert_mace4_sym()
 *
 *************/

static
Symbol_data insert_mace4_sym(Symbol_data syms, int sn, int type)
{
  if (syms == NULL || sn_to_lex_val(sn) < sn_to_lex_val(syms->sn)) {
    Symbol_data new = get_symbol_data();
    new->sn = sn;
    new->arity = sn_to_arity(sn);
    new->type = type;
    if (is_skolem(sn))
      new->attribute = SKOLEM_SYMBOL;
    new->next = syms;
    return new;
  }
  else {
    syms->next = insert_mace4_sym(syms->next, sn, type);
    return syms;
  }
}  /* insert_mace4_sym */

/*************
 *
 *   collect_mace4_syms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int collect_mace4_syms(Plist clauses, BOOL arithmetic)
{
  Ilist fsyms, rsyms, p;
  int max_domain = -1;

  fsyms = fsym_set_in_topforms(clauses);
  rsyms = rsym_set_in_topforms(clauses);

  lex_order(fsyms, rsyms, NULL, NULL, lex_compare_arity_0123);

  for (p = fsyms; p; p = p->next) {
    int sn = p->i;
    int dom = natural_string(sn_to_str(sn));
    if (dom >= 0)
      max_domain = MAX(dom, max_domain);
    else if (arithmetic && arith_op_sn(sn))
      ;  /* don't insert */
    else
      Symbols = insert_mace4_sym(Symbols, sn, FUNCTION); 
  }
  
  for (p = rsyms; p; p = p->next) {
    int sn = p->i;
    int dom = natural_string(sn_to_str(sn));
    if (dom >= 0) {
      printf("\nThe bad symbol is: %s\n", sn_to_str(sn));
      fatal_error("collect_mace4_syms, relation symbol is domain element");
    }
    else if (is_eq_symbol(sn) || (arithmetic && arith_rel_sn(sn)))
      ;  /* don't insert */
    else
      Symbols = insert_mace4_sym(Symbols, sn, RELATION); 
  }

  zap_ilist(fsyms);
  zap_ilist(rsyms);
  return max_domain;
}  /* collect_mace4_syms */

/*************
 *
 *   p_symbol_data()
 *
 *************/

void p_symbol_data(Symbol_data s)
{
  if (s != NULL)
    printf("%s/%d, sn=%d, mace_sn=%d, base=%d, type=%d, attribute=%d.\n",
	   sn_to_str(s->sn), s->arity,
	   s->sn, s->mace_sn, s->base, s->type, s->attribute);
}  /* p_symbol_data */

/*************
 *
 *   find_symbol_node()
 *
 *************/

Symbol_data find_symbol_node(int id)
{
  /* Assume bases are increasing and that the id is in range.
     Return node with the largest base <= the id. */
  Symbol_data prev = NULL;
  Symbol_data curr = Symbols;
  while (curr != NULL && curr->base <= id) {
    prev = curr;
    curr = curr->next;
  }
  return prev;
}  /* find_symbol_node */

/*************
 *
 *   id_to_domain_size()
 *
 *************/

int id_to_domain_size(int id)
{
  /* Assume the id is in range. */
  Symbol_data s = Cells[id].symbol;
  return (s->type == RELATION ? 2 : Domain_size);
}  /* id_to_domain_size */

/*************
 *
 *   max_index()
 *
 *   If the cell given by ID represents f(i,j,k,...), return the 
 *   maximum of (i,j,k,...).  If it is a constant, return -1.
 *
 *************/

int max_index(int id, Symbol_data s)
{
  int max = -1;
  int n = Domain_size;
  int x = id - s->base;
  int i;
  for (i = s->arity - 1; i >= 0; i--) {
    int p = int_power(n, i);
    int e = x / p;
    max = MAX(e,max);
    x = x % p;
  }
  return max;
}  /* max_index */

