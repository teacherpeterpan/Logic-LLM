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

#include "int_code.h"

/* Private definitions and types */

/*************
 *
 *   put_ilist_to_ibuf()
 *
 *************/

static
void put_ilist_to_ibuf(Ibuffer ibuf, Ilist p)
{
  /* size of Ilist, then members */
  ibuf_write(ibuf, ilist_count(p));
  for (; p; p = p->next)
    ibuf_write(ibuf, p->i);
}  /* put_ilist_to_ibuf */

/*************
 *
 *   get_ilist_from_ibuf()
 *
 *************/

static
Ilist get_ilist_from_ibuf(Ibuffer ibuf)
{
  /* size of Ilist, then members */
  int i;
  Ilist p = NULL;
  int n = ibuf_xread(ibuf);
  for (i = 0; i < n; i++)
    p = ilist_append(p, ibuf_xread(ibuf));
  return p;
}  /* get_ilist_from_ibuf */

/*************
 *
 *   put_i3list_to_ibuf()
 *
 *************/

static
void put_i3list_to_ibuf(Ibuffer ibuf, I3list p)
{
  /* size of I3list, then members */
  ibuf_write(ibuf, i3list_count(p));
  for (; p; p = p->next) {
    ibuf_write(ibuf, p->i);
    ibuf_write(ibuf, p->j);
    ibuf_write(ibuf, p->k);
  }
}  /* put_i3list_to_ibuf */

/*************
 *
 *   get_i3list_from_ibuf()
 *
 *************/

static
I3list get_i3list_from_ibuf(Ibuffer ibuf)
{
  /* size of I3list, then members */
  int i;
  I3list p = NULL;
  int n = ibuf_xread(ibuf);
  for (i = 0; i < n; i++) {
    int i1 = ibuf_xread(ibuf);
    int i2 = ibuf_xread(ibuf);
    int i3 = ibuf_xread(ibuf);
    p = i3list_append(p, i1, i2, i3);
  }
  return p;
}  /* get_i3list_from_ibuf */

/*************
 *
 *   put_term_to_ibuf()
 *
 *************/

static
void put_term_to_ibuf(Ibuffer ibuf, Term t)
{
  if (VARIABLE(t))
    ibuf_write(ibuf, -VARNUM(t));
  else {
    int i;
    ibuf_write(ibuf, SYMNUM(t));
    for (i = 0; i < ARITY(t); i++) {
      put_term_to_ibuf(ibuf, ARG(t,i));
    }
  }
}  /* put_term_to_ibuf */

/*************
 *
 *   get_term_from_ibuf()
 *
 *************/

static
Term get_term_from_ibuf(Ibuffer ibuf)
{
  int a = ibuf_xread(ibuf);
  if (a <= 0)
    return get_variable_term(-a);
  else {
    Term t;
    int i;
    int arity = sn_to_arity(a);
    if (arity == -1) {
      printf("bad symnum: %d\n", a);
      fatal_error("get_term_from_ibuf, symbol not in symbol table");
    }
    t = get_rigid_term_dangerously(a, arity);
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = get_term_from_ibuf(ibuf);
    return t;
  }
}  /* get_term_from_ibuf */

/*************
 *
 *   put_just_to_ibuf()
 *
 *************/

static
void put_just_to_ibuf(Ibuffer ibuf, Just j)
{
  /* size of Just, then members */
  ibuf_write(ibuf, just_count(j));

  for (; j; j = j->next) {
    ibuf_write(ibuf, j->type);
    switch (j->type) {
    case INPUT_JUST:
    case GOAL_JUST:
      /* nothing */
      break;
    case CLAUSIFY_JUST:
    case DENY_JUST:
    case COPY_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
    case NEW_SYMBOL_JUST:
      /* integer */
      ibuf_write(ibuf, j->u.id);
      break;
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
    case XXRES_JUST:
      /* list of integers */
      put_ilist_to_ibuf(ibuf, j->u.lst);
      break;
    case DEMOD_JUST:
      /* list of integer triples */
      put_i3list_to_ibuf(ibuf, j->u.demod);
      break;
    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      ibuf_write(ibuf, j->u.para->from_id);
      put_ilist_to_ibuf(ibuf, j->u.para->from_pos);
      ibuf_write(ibuf, j->u.para->into_id);
      put_ilist_to_ibuf(ibuf, j->u.para->into_pos);
      break;
    default: fatal_error("put_just_to_ibuf, unknown type");
    }
  }
}  /* put_just_to_ibuf */

/*************
 *
 *   get_just_from_ibuf()
 *
 *************/

static
Just get_just_from_ibuf(Ibuffer ibuf)
{
  /* size of Just, then members */
  Just j_collect = NULL;
  int i;
  int n = ibuf_xread(ibuf);
  for (i = 0; i < n; i++) {
    int type = ibuf_xread(ibuf);
    Just j = get_just();
    j->type = type;
    switch (type) {
    case INPUT_JUST:
    case GOAL_JUST:
      /* nothing */
      break;
    case CLAUSIFY_JUST:
    case DENY_JUST:
    case COPY_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
    case NEW_SYMBOL_JUST:
      /* integer */
      j->u.id = ibuf_xread(ibuf);
      break;
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
    case XXRES_JUST:
      /* list of integers */
      j->u.lst = get_ilist_from_ibuf(ibuf);
      break;
    case DEMOD_JUST:
      /* list of integer triples */
      j->u.demod = get_i3list_from_ibuf(ibuf);
      break;
    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      j->u.para = get_parajust();
      j->u.para->from_id = ibuf_xread(ibuf);
      j->u.para->from_pos = get_ilist_from_ibuf(ibuf);
      j->u.para->into_id =  ibuf_xread(ibuf);
      j->u.para->into_pos =  get_ilist_from_ibuf(ibuf);
      break;
    default:
      printf("unknown just type: %d\n", type);
      fatal_error("get_just_from_ibuf, unknown just");
    }
    j_collect = append_just(j_collect, j);
  }
  return j_collect;
}  /* get_just_from_ibuf */

/*************
 *
 *   put_clause_to_ibuf()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void put_clause_to_ibuf(Ibuffer ibuf, Topform c)
{
  /* id is_formula weight number-of-justs justs lits/atts [atomflags] */

  ibuf_write(ibuf, c->id);
  ibuf_write(ibuf, c->is_formula);
  ibuf_write(ibuf, c->weight);
  put_just_to_ibuf(ibuf, c->justification);

  /* literals and attributes */
  {
    Term t = topform_to_term(c);
    put_term_to_ibuf(ibuf, t);
    zap_term(t);
  }

  if (!c->is_formula)
    {
      /* flags on atoms (maximal, oriented, etc.) */
      Literals l;
      for (l = c->literals; l; l = l->next)
	ibuf_write(ibuf, l->atom->private_flags);
    }
}  /* put_clause_to_ibuf */

/*************
 *
 *   get_clause_from_ibuf()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform get_clause_from_ibuf(Ibuffer ibuf)
{
  /* id is_formula weight number-of-justs justs lits/atts [atomflags] */

  int id = ibuf_xread(ibuf);
  int is_formula = ibuf_xread(ibuf);
  int weight = ibuf_xread(ibuf);
  Just j = get_just_from_ibuf(ibuf);

  Term t = get_term_from_ibuf(ibuf);
  Topform c = term_to_topform(t, is_formula);
  zap_term(t);

  c->id = id;
  c->is_formula = is_formula;
  c->weight = weight;
  c->justification = j;

  if (!is_formula) {
    Literals l;
    for (l = c->literals; l; l = l->next)
      l->atom->private_flags = ibuf_xread(ibuf);
  }
  return c;
}  /* get_clause_from_ibuf */

/*************
 *
 *   check_ibuf_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void check_ibuf_clause(Topform c)
{
  Ibuffer ibuf = ibuf_init();
  put_clause_to_ibuf(ibuf, c);
  p_ibuf(ibuf);
  ibuf_rewind(ibuf);
  {
    Topform d = get_clause_from_ibuf(ibuf);
    fprint_clause(stdout, c);
    fprint_clause(stdout, d);
    if (!clause_ident(c->literals, d->literals))
      fatal_error("check_ibuf_clause, clasues not ident");
    zap_topform(d);
  }
  ibuf_free(ibuf);
}  /* check_ibuf_clause */

