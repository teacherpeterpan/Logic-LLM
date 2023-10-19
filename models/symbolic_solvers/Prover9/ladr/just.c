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

#include "just.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_JUST PTRS(sizeof(struct just))
static unsigned Just_gets, Just_frees;

#define PTRS_PARAJUST PTRS(sizeof(struct parajust))
static unsigned Parajust_gets, Parajust_frees;

#define PTRS_INSTANCEJUST PTRS(sizeof(struct instancejust))
static unsigned Instancejust_gets, Instancejust_frees;

#define PTRS_IVYJUST PTRS(sizeof(struct ivyjust))
static unsigned Ivyjust_gets, Ivyjust_frees;

/*************
 *
 *   Just get_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Just get_just(void)
{
  Just p = get_cmem(PTRS_JUST);
  Just_gets++;
  return(p);
}  /* get_just */

/*************
 *
 *    free_just()
 *
 *************/

static
void free_just(Just p)
{
  free_mem(p, PTRS_JUST);
  Just_frees++;
}  /* free_just */

/*************
 *
 *   Parajust get_parajust()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Parajust get_parajust(void)
{
  Parajust p = get_cmem(PTRS_PARAJUST);
  Parajust_gets++;
  return(p);
}  /* get_parajust */

/*************
 *
 *    free_parajust()
 *
 *************/

static
void free_parajust(Parajust p)
{
  free_mem(p, PTRS_PARAJUST);
  Parajust_frees++;
}  /* free_parajust */

/*************
 *
 *   Instancejust get_instancejust()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Instancejust get_instancejust(void)
{
  Instancejust p = get_cmem(PTRS_INSTANCEJUST);
  Instancejust_gets++;
  return(p);
}  /* get_instancejust */

/*************
 *
 *    free_instancejust()
 *
 *************/

static
void free_instancejust(Instancejust p)
{
  free_mem(p, PTRS_INSTANCEJUST);
  Instancejust_frees++;
}  /* free_instancejust */

/*************
 *
 *   Ivyjust get_ivyjust()
 *
 *************/

static
Ivyjust get_ivyjust(void)
{
  Ivyjust p = get_mem(PTRS_IVYJUST);
  Ivyjust_gets++;
  return(p);
}  /* get_ivyjust */

/*************
 *
 *    free_ivyjust()
 *
 *************/

static
void free_ivyjust(Ivyjust p)
{
  free_mem(p, PTRS_IVYJUST);
  Ivyjust_frees++;
}  /* free_ivyjust */

/*************
 *
 *   fprint_just_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the just package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_just_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct just);
  fprintf(fp, "just (%4d)         %11u%11u%11u%9.1f K\n",
          n, Just_gets, Just_frees,
          Just_gets - Just_frees,
          ((Just_gets - Just_frees) * n) / 1024.);

  n = sizeof(struct parajust);
  fprintf(fp, "parajust (%4d)     %11u%11u%11u%9.1f K\n",
          n, Parajust_gets, Parajust_frees,
          Parajust_gets - Parajust_frees,
          ((Parajust_gets - Parajust_frees) * n) / 1024.);

  n = sizeof(struct instancejust);
  fprintf(fp, "instancejust (%4d) %11u%11u%11u%9.1f K\n",
          n, Instancejust_gets, Instancejust_frees,
          Instancejust_gets - Instancejust_frees,
          ((Instancejust_gets - Instancejust_frees) * n) / 1024.);

  n = sizeof(struct ivyjust);
  fprintf(fp, "ivyjust (%4d)      %11u%11u%11u%9.1f K\n",
          n, Ivyjust_gets, Ivyjust_frees,
          Ivyjust_gets - Ivyjust_frees,
          ((Ivyjust_gets - Ivyjust_frees) * n) / 1024.);

}  /* fprint_just_mem */

/*************
 *
 *   p_just_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the just package.
*/

/* PUBLIC */
void p_just_mem()
{
  fprint_just_mem(stdout, TRUE);
}  /* p_just_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   ivy_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Just ivy_just(Just_type type,
	      int parent1, Ilist pos1,
	      int parent2, Ilist pos2,
	      Plist pairs)
{
  Just j = get_just();
  j->type = IVY_JUST;
  j->u.ivy = get_ivyjust();
  j->u.ivy->type = type;
  j->u.ivy->parent1 = parent1;
  j->u.ivy->parent2 = parent2;
  j->u.ivy->pos1 = pos1;
  j->u.ivy->pos2 = pos2;
  j->u.ivy->pairs = pairs;
  return j;
}  /* ivy_just */

/*************
 *
 *   input_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for input.
*/

/* PUBLIC */
Just input_just(void)
{
  /* (INPUT_JUST) */
  Just j = get_just();
  j->type = INPUT_JUST;
  return j;
}  /* input_just */

/*************
 *
 *   goal_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for goal.
*/

/* PUBLIC */
Just goal_just(void)
{
  /* (GOAL_JUST) */
  Just j = get_just();
  j->type = GOAL_JUST;
  return j;
}  /* goal_just */

/*************
 *
 *   deny_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for deny.
*/

/* PUBLIC */
Just deny_just(Topform tf)
{
  /* (DENY_JUST) */
  Just j = get_just();
  j->type = DENY_JUST;
  j->u.id = tf->id;
  return j;
}  /* deny_just */

/*************
 *
 *   clausify_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for clausify.
*/

/* PUBLIC */
Just clausify_just(Topform tf)
{
  /* (CLAUSIFY_JUST) */
  Just j = get_just();
  j->type = CLAUSIFY_JUST;
  j->u.id = tf->id;
  return j;
}  /* clausify_just */

/*************
 *
 *   expand_def_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for expand_def.
*/

/* PUBLIC */
Just expand_def_just(Topform tf, Topform def)
{
  /* (expand_def_JUST) */
  Just j = get_just();
  j->type = EXPAND_DEF_JUST;
  j->u.lst = ilist_append(ilist_append(NULL, tf->id), def->id);
  return j;
}  /* expand_def_just */

/*************
 *
 *   copy_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for copy.
*/

/* PUBLIC */
Just copy_just(Topform c)
{
  /* (COPY_JUST parent_id) */
  Just j = get_just();
  j->type = COPY_JUST;
  j->u.id = c->id;
  return j;
}  /* copy_just */

/*************
 *
 *   propositional_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for propositional.
*/

/* PUBLIC */
Just propositional_just(Topform c)
{
  /* (PROPOSITIONAL_JUST parent_id) */
  Just j = get_just();
  j->type = PROPOSITIONAL_JUST;
  j->u.id = c->id;
  return j;
}  /* propositional_just */

/*************
 *
 *   new_symbol_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for new_symbol inference.
*/

/* PUBLIC */
Just new_symbol_just(Topform c)
{
  /* (NEW_SYMBOL_JUST parent_id) */
  Just j = get_just();
  j->type = NEW_SYMBOL_JUST;
  j->u.id = c->id;
  return j;
}  /* new_symbol_just */

/*************
 *
 *   back_demod_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for back_demod.
*/

/* PUBLIC */
Just back_demod_just(Topform c)
{
  /* (BACK_DEMOD_JUST parent_id) */
  Just j = get_just();
  j->type = BACK_DEMOD_JUST;
  j->u.id = c->id;
  return j;
}  /* back_demod_just */

/*************
 *
 *   back_unit_deletion_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for back_unit_deletion.
*/

/* PUBLIC */
Just back_unit_deletion_just(Topform c)
{
  /* (BACK_UNIT_DEL_JUST parent_id) */
  Just j = get_just();
  j->type = BACK_UNIT_DEL_JUST;
  j->u.id = c->id;
  return j;
}  /* back_unit_deletion_just */

/*************
 *
 *   binary_res_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for binary resolution.
(Binary res justifications may also be constructed in resolve(), along
with hyper and UR.)
*/

/* PUBLIC */
Just binary_res_just(Topform c1, int n1, Topform c2, int n2)
{
  /* (BINARY_RES_JUST (id1 lit1 id2 lit2) */
  Just j = get_just();
  j->type = BINARY_RES_JUST;
  j->u.lst = ilist_append(
                  ilist_append(
                       ilist_append(
                            ilist_append(NULL,c1->id),n1),c2->id),n2);

  return j;
}  /* binary_res_just */

/*************
 *
 *   binary_res_just_by_id()
 *
 *************/

/* DOCUMENTATION
Similar to binary_res_just, except that IDs are given instead of clauses.
*/

/* PUBLIC */
Just binary_res_just_by_id(int c1, int n1, int c2, int n2)
{
  /* (BINARY_RES_JUST (id1 lit1 id2 lit2) */
  Just j = get_just();
  j->type = BINARY_RES_JUST;
  j->u.lst = ilist_append(
                  ilist_append(
                       ilist_append(
                            ilist_append(NULL,c1),n1),c2),n2);

  return j;
}  /* binary_res_just_by_id */

/*************
 *
 *   factor_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a factorization.
*/

/* PUBLIC */
Just factor_just(Topform c, int lit1, int lit2)
{
  /* (FACTOR_JUST (clause_id lit1 lit2)) */
  Just j = get_just();
  j->type = FACTOR_JUST;
  j->u.lst = ilist_append(ilist_append(ilist_append(NULL,c->id),lit1),lit2);
  return j;
}  /* factor_just */

/*************
 *
 *   xxres_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for resolution with x=x.
*/

/* PUBLIC */
Just xxres_just(Topform c, int lit)
{
  /* (XXRES_JUST (clause_id lit)) */
  Just j = get_just();
  j->type = XXRES_JUST;
  j->u.lst = ilist_append(ilist_append(NULL,c->id),lit);
  return j;
}  /* xxres_just */

/*************
 *
 *   resolve_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for resolution rules.
This handles binary, hyper, ur, and maybe others.
*/

/* PUBLIC */
Just resolve_just(Ilist g, Just_type type)
{
  Just j = get_just();
  j->type = type;
  j->u.lst = g;
  return j;
}  /* resolve_just */

/*************
 *
 *   demod_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for a demodulation.
*/

/* PUBLIC */
Just demod_just(I3list steps)
{
  Just j = get_just();
  j->type = DEMOD_JUST;
  j->u.demod = steps;
  return j;
}  /* demod_just */

/*************
 *
 *   para_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a paramodulation
inference.  The position vectors are not copied.
*/

/* PUBLIC */
Just para_just(Just_type rule,
	       Topform from, Ilist from_vec,
	       Topform into, Ilist into_vec)
{
  Just j = get_just();
  j->type = rule;
  j->u.para = get_parajust();

  j->u.para->from_id = from->id;
  j->u.para->into_id = into->id;
  j->u.para->from_pos = from_vec;
  j->u.para->into_pos = into_vec;

  return j;
}  /* para_just */

/*************
 *
 *   instance_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for an instance
inference.  The list of pairs is not copied.
*/

/* PUBLIC */
Just instance_just(Topform parent, Plist pairs)
{
  Just j = get_just();
  j->type = INSTANCE_JUST;
  j->u.instance = get_instancejust();

  j->u.instance->parent_id = parent->id;
  j->u.instance->pairs = pairs;
  
  return j;
}  /* instance_just */

/*************
 *
 *   para_just_rev_copy()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a paramodulation
inference.  The position vectors are copied and reversed.
*/

/* PUBLIC */
Just para_just_rev_copy(Just_type rule,
			Topform from, Ilist from_vec,
			Topform into, Ilist into_vec)
{
  return para_just(rule,
		   from, reverse_ilist(copy_ilist(from_vec)),
		   into, reverse_ilist(copy_ilist(into_vec)));
}  /* para_just_rev_copy */

/*************
 *
 *   unit_del_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a factorization.
*/

/* PUBLIC */
Just unit_del_just(Topform deleter, int literal_num)
{
  /* UNIT_DEL (literal-num clause-id) */
  Just j = get_just();
  j->type = UNIT_DEL_JUST;
  j->u.lst = ilist_append(ilist_append(NULL, literal_num), deleter->id);
  return j;
}  /* cd_just */

/*************
 *
 *   flip_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for equality flipping.
*/

/* PUBLIC */
Just flip_just(int n)
{
  Just j = get_just();
  j->type = FLIP_JUST;
  j->u.id = n;
  return j;
}  /* flip_just */

/*************
 *
 *   xx_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for the XX rule,
which removes literals that are instances of x!=x.
*/

/* PUBLIC */
Just xx_just(int n)
{
  Just j = get_just();
  j->type = XX_JUST;
  j->u.id = n;
  return j;
}  /* xx_just */

/*************
 *
 *   merge_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for the merging
a literal.  The n-th literal has been removed because it is
identical to another literal.
*/

/* PUBLIC */
Just merge_just(int n)
{
  Just j = get_just();
  j->type = MERGE_JUST;
  j->u.id = n;
  return j;
}  /* merge_just */

/*************
 *
 *   eval_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification for an eval rewrite.
The argument is the number of rewrites.
*/

/* PUBLIC */
Just eval_just(int n)
{
  Just j = get_just();
  j->type = EVAL_JUST;
  j->u.id = n;
  return j;
}  /* eval_just */

/*************
 *
 *   append_just()
 *
 *************/

/* DOCUMENTATION
This appends two justifications.  No copying occurs.
*/

/* PUBLIC */
Just append_just(Just j1, Just j2)
{
  if (j1 == NULL)
    return j2;
  else {
    j1->next = append_just(j1->next, j2);
    return j1;
  }
}  /* append_just */

/*************
 *
 *   copy_justification()
 *
 *************/

/* DOCUMENTATION
Copy a justification.
*/

/* PUBLIC */
Just copy_justification(Just j)
{
  if (j == NULL)
    return NULL;
  else {
    Just j2 = get_just();
    j2->type = j->type;
    j2->next = copy_justification(j->next);
    switch (j->type) {
    case INPUT_JUST:
    case GOAL_JUST:
      break;
    case DENY_JUST:
    case CLAUSIFY_JUST:
    case COPY_JUST:
    case PROPOSITIONAL_JUST:
    case NEW_SYMBOL_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
    case EVAL_JUST:
      j2->u.id = j->u.id;
      break;
    case EXPAND_DEF_JUST:
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
    case XXRES_JUST:
      j2->u.lst = copy_ilist(j->u.lst);
      break;
    case DEMOD_JUST:
      j2->u.demod = copy_i3list(j->u.demod);
      break;
    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      j2->u.para = get_parajust();
      j2->u.para->from_id = j->u.para->from_id;
      j2->u.para->into_id = j->u.para->into_id;
      j2->u.para->from_pos = copy_ilist(j->u.para->from_pos);
      j2->u.para->into_pos = copy_ilist(j->u.para->into_pos);
      break;
    case INSTANCE_JUST:
      j2->u.instance = get_instancejust();
      j2->u.instance->parent_id = j->u.instance->parent_id;
      j2->u.instance->pairs = copy_plist_of_terms(j->u.instance->pairs);
      break;
    case IVY_JUST:
      j2->u.ivy = get_ivyjust();
      j2->u.ivy->type = j->u.ivy->type;
      j2->u.ivy->parent1 = j->u.ivy->parent1;
      j2->u.ivy->parent2 = j->u.ivy->parent2;
      j2->u.ivy->pos1 = copy_ilist(j->u.ivy->pos1);
      j2->u.ivy->pos2 = copy_ilist(j->u.ivy->pos2);
      j2->u.ivy->pairs = copy_plist_of_terms(j->u.ivy->pairs);
      break;
    default: fatal_error("copy_justification: unknown type");
    }
    return j2;
  }
}  /* copy_justification */

/*************
 *
 *   jstring() - strings for printing justifications
 *
 *************/

/* DOCUMENTATION
What is the string, e.g., "resolve" associated with a justification node?
*/

/* PUBLIC */
char *jstring(Just j)
{
  switch (j->type) {

    /* primary justifications */

  case INPUT_JUST:         return "assumption";
  case GOAL_JUST:          return "goal";
  case DENY_JUST:          return "deny";
  case CLAUSIFY_JUST:      return "clausify";
  case COPY_JUST:          return "copy";
  case PROPOSITIONAL_JUST: return "propositional";
  case NEW_SYMBOL_JUST:    return "new_symbol";
  case BACK_DEMOD_JUST:    return "back_rewrite";
  case BACK_UNIT_DEL_JUST: return "back_unit_del";
  case EXPAND_DEF_JUST:    return "expand_def";
  case BINARY_RES_JUST:    return "resolve";
  case HYPER_RES_JUST:     return "hyper";
  case UR_RES_JUST:        return "ur";
  case FACTOR_JUST:        return "factor";
  case XXRES_JUST:         return "xx_res";
  case PARA_JUST:          return "para";
  case PARA_FX_JUST:       return "para_fx";
  case PARA_IX_JUST:       return "para_ix";
  case PARA_FX_IX_JUST:    return "para_fx_ix";
  case INSTANCE_JUST:      return "instantiate";
  case IVY_JUST:           return "ivy";

    /* secondary justifications */

  case FLIP_JUST:          return "flip";
  case XX_JUST:            return "xx";
  case MERGE_JUST:         return "merge";
  case EVAL_JUST:          return "eval";
  case DEMOD_JUST:         return "rewrite";
  case UNIT_DEL_JUST:      return "unit_del";
  case UNKNOWN_JUST:       return "unknown";
  }
  return "unknown";
}  /* jstring */

/*************
 *
 *   jstring_to_jtype() - strings for printing justifications
 *
 *************/

static
int jstring_to_jtype(char *s)
{
  if (str_ident(s, "assumption"))
    return INPUT_JUST;
  else if (str_ident(s, "goal"))
    return GOAL_JUST;
  else if (str_ident(s, "deny"))
    return DENY_JUST;
  else if (str_ident(s, "clausify"))
    return CLAUSIFY_JUST;
  else if (str_ident(s, "copy"))
    return COPY_JUST;
  else if (str_ident(s, "propositional"))
    return PROPOSITIONAL_JUST;
  else if (str_ident(s, "new_symbol"))
    return NEW_SYMBOL_JUST;
  else if (str_ident(s, "back_rewrite"))
    return BACK_DEMOD_JUST;
  else if (str_ident(s, "back_unit_del"))
    return BACK_UNIT_DEL_JUST;
  else if (str_ident(s, "expand_def"))
    return EXPAND_DEF_JUST;
  else if (str_ident(s, "resolve"))
    return BINARY_RES_JUST;
  else if (str_ident(s, "hyper"))
    return HYPER_RES_JUST;
  else if (str_ident(s, "ur"))
    return UR_RES_JUST;
  else if (str_ident(s, "factor"))
    return FACTOR_JUST;
  else if (str_ident(s, "xx_res"))
    return XXRES_JUST;
  else if (str_ident(s, "para"))
    return PARA_JUST;
  else if (str_ident(s, "para_fx"))
    return PARA_FX_JUST;
  else if (str_ident(s, "para_ix"))
    return PARA_IX_JUST;
  else if (str_ident(s, "instantiate"))
    return INSTANCE_JUST;
  else if (str_ident(s, "para_fx_ix"))
    return PARA_FX_IX_JUST;
  else if (str_ident(s, "flip"))
    return FLIP_JUST;
  else if (str_ident(s, "xx"))
    return XX_JUST;
  else if (str_ident(s, "merge"))
    return MERGE_JUST;
  else if (str_ident(s, "eval"))
    return EVAL_JUST;
  else if (str_ident(s, "rewrite"))
    return DEMOD_JUST;
  else if (str_ident(s, "unit_del"))
    return UNIT_DEL_JUST;
  else if (str_ident(s, "ivy"))
    return IVY_JUST;
  else
    return UNKNOWN_JUST;
}  /* jstring_to_jtype */

/*************
 *
 *   itoc()
 *
 *************/

static
char itoc(int i)
{
  if (i <= 0)
    return '?';
  else if (i <= 26)
    return 'a' + i - 1;
  else if (i <= 52)
    return 'A' + i - 27;
  else
    return '?';
}  /* itoc */

/*************
 *
 *   ctoi()
 *
 *************/

static
int ctoi(char c)
{
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 1;
  else if (c >= 'A' && c <= 'Z')
    return c - 'A' + 27;
  else
    return INT_MIN;
}  /* ctoi */

/*************
 *
 *   jmap1()
 *
 *************/

/* DOCUMENTATION
A jmap maps ints to pairs of ints.  This returns the first.
If i is not in the map, i is returned.
 */

/* PUBLIC */
int jmap1(I3list map, int i)
{
  int id = assoc2a(map, i);
  return (id == INT_MIN ? i : id);
}  /* jmap1 */

/*************
 *
 *   jmap2()
 *
 *************/

/* DOCUMENTATION
A jmap maps ints to pairs of ints.  This returns a string
representation of the second.  If i is not in the map, or
if the int value of is INT_MIN, "" is returned.

Starting with 0, the strings are "A" - "Z", "A26", "A27", ... .

The argument *a must point to available space for the result.
The result is returned.
 */

/* PUBLIC */
char *jmap2(I3list map, int i, char *a)
{
  int n = assoc2b(map, i);
  if (n == INT_MIN)
    a[0] = '\0';
  else if (n >= 0 && n <= 25) {   /* "A" -- "Z" */
    a[0] = 'A' + n;
    a[1] = '\0';
  }
  else {               /* "A26", ... */
    a[0] = 'A';
    sprintf(a+1, "%d", n);
  }
  return a;
}  /* jmap2 */

/*************
 *
 *   sb_append_id()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sb_append_id(String_buf sb, int id, I3list map)
{
  char s[21];
  sb_append_int(sb, jmap1(map, id));
  sb_append(sb, jmap2(map, id, s));
}  /* sb_append_id */

/*************
 *
 *   sb_write_res_just() -- (1 a 2 b c 3 d e 4 f)
 *
 *   Assume input is well-formed, that is, length is 3n+1 for n>1.
 *
 *************/

static
void sb_write_res_just(String_buf sb, Just g, I3list map)
{
  Ilist q;
  Ilist p = g->u.lst;

  sb_append(sb, jstring(g));
  sb_append(sb, "(");
  sb_append_id(sb, p->i, map);

  for (q = p->next; q != NULL; q = q->next->next->next) {
    int nuc_lit = q->i;
    int sat_id  = q->next->i;
    int sat_lit = q->next->next->i;
    sb_append(sb, ",");
    sb_append_char(sb, itoc(nuc_lit));
    if (sat_id == 0)
      sb_append(sb, ",xx");
    else {
      sb_append(sb, ",");
      sb_append_id(sb, sat_id, map);
      sb_append(sb, ",");
      if (sat_lit > 0)
	sb_append_char(sb, itoc(sat_lit));
      else {
	sb_append_char(sb, itoc(-sat_lit));
	sb_append(sb, "(flip)");
      }
    }
  }
  sb_append(sb, ")");
}  /* sb_write_res_just */

/*************
 *
 *   sb_write_position() - like this (a,2,1,3)
 *
 *************/

static
void sb_write_position(String_buf sb, Ilist p)
{
  Ilist q;
  sb_append(sb, "(");
  sb_append_char(sb, itoc(p->i));
  for (q = p->next; q != NULL; q = q->next) {
    sb_append(sb, ",");
    sb_append_int(sb, q->i);
  }
  sb_append(sb, ")");
}  /* sb_write_position */

/*************
 *
 *   sb_write_ids() - separated by space
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sb_write_ids(String_buf sb, Ilist p, I3list map)
{
  Ilist q;
  for (q = p; q; q = q->next) {
    sb_append_id(sb, q->i, map);
    if (q->next)
      sb_append(sb, " ");
  }
}  /* sb_write_ids */

/*************
 *
 *   sb_write_just()
 *
 *************/

/* DOCUMENTATION
This routine writes (to a String_buf) a clause justification.
No whitespace is printed before or after.
*/

/* PUBLIC */
void sb_write_just(String_buf sb, Just just, I3list map)
{
  Just g = just;
  sb_append(sb, "[");
  while (g != NULL) {
    Just_type rule = g->type;
    if (rule == INPUT_JUST || rule == GOAL_JUST)
      sb_append(sb, jstring(g));
    else if (rule==BINARY_RES_JUST ||
	     rule==HYPER_RES_JUST ||
	     rule==UR_RES_JUST) {
      sb_write_res_just(sb, g, map);
    }
    else if (rule == DEMOD_JUST) {
      I3list p;
      sb_append(sb, jstring(g));
      sb_append(sb, "([");
      for (p = g->u.demod; p; p = p->next) {
	sb_append_int(sb, p->i);
	if (p->j > 0) {
	  sb_append(sb, "(");
	  sb_append_int(sb, p->j);
	  if (p->k == 2)
	    sb_append(sb, ",R");
	  sb_append(sb, ")");
	}

	sb_append(sb, p->next ? "," : "");
      }
      sb_append(sb, "])");
    }
    else if (rule == UNIT_DEL_JUST) {
      Ilist p = g->u.lst;
      int n = p->i;
      int id = p->next->i;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      if (n < 0) {
	sb_append_char(sb, itoc(-n));
	sb_append(sb, "(flip),");
      }
      else {
	sb_append_char(sb, itoc(n));
	sb_append(sb, ",");
      }
      sb_append_id(sb, id, map);
      sb_append(sb, ")");
    }
    else if (rule == FACTOR_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->i));
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->next->i));
      sb_append(sb, ")");
    }
    else if (rule == XXRES_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->i));
      sb_append(sb, ")");
    }
    else if (rule == EXPAND_DEF_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_id(sb, p->next->i, map);
      sb_append(sb, ")");
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == BACK_UNIT_DEL_JUST ||
	     rule == NEW_SYMBOL_JUST ||
	     rule == COPY_JUST ||
	     rule == DENY_JUST ||
	     rule == CLAUSIFY_JUST ||
	     rule == PROPOSITIONAL_JUST) {
      int id = g->u.id;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, id, map);
      sb_append(sb, ")");
    }
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == MERGE_JUST) {
      int id = g->u.id;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_char(sb, itoc(id));
      sb_append(sb, ")");
    }
    else if (rule == EVAL_JUST) {
      int id = g->u.id;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_int(sb, id);
      sb_append(sb, ")");
    }
    else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	     rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
      Parajust p = g->u.para;

      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->from_id, map);
      sb_write_position(sb, p->from_pos);

      sb_append(sb, ",");
      sb_append_id(sb, p->into_id, map);
      sb_write_position(sb, p->into_pos);

      sb_append(sb, ")");
    }
    else if (rule == INSTANCE_JUST) {
      Plist p;

      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_int(sb, g->u.instance->parent_id);
      sb_append(sb, ",[");

      for (p = g->u.instance->pairs; p; p = p->next) {
	sb_write_term(sb, p->v);
	if (p->next)
	  sb_append(sb, ",");
      }
      sb_append(sb, "])");
    }
    else if (rule == IVY_JUST) {
      sb_append(sb, jstring(g));
    }
    else {
      printf("\nunknown rule: %d\n", rule);
      fatal_error("sb_write_just: unknown rule");
    }
    g = g->next;
    if (g)
      sb_append(sb, ",");
  }
  sb_append(sb, "].");
}  /* sb_write_just */

/*************
 *
 *   sb_xml_write_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sb_xml_write_just(String_buf sb, Just just, I3list map)
{
  Just g;

  /* Put the standard form into an attribute. */

  String_buf sb_standard = get_string_buf();
  sb_write_just(sb_standard, just, map);
  sb_append(sb, "    <justification jstring=\""); 
  sb_cat(sb, sb_standard);  /* this frees sb_standard */
  sb_append(sb, "\">\n");

  /* Put an abbreviated form (rule, parents) into an XML elements. */

  for (g = just; g; g = g->next) {

    Ilist parents = get_parents(g, FALSE);  /* for this node only */

    if (g == just)
      sb_append(sb, "      <j1 rule=\"");
    else
      sb_append(sb, "      <j2 rule=\"");
    sb_append(sb, jstring(g));
    sb_append(sb, "\"");

    if (parents) {
      sb_append(sb, " parents=\"");
      sb_write_ids(sb, parents, map);
      zap_ilist(parents);
      sb_append(sb, "\"");
    }

    sb_append(sb, "/>\n");  /* close the <j1 or <j2 */
  }
  sb_append(sb, "    </justification>\n");
}  /* sb_xml_write_just */

/*************
 *
 *   p_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_just(Just j)
{
  String_buf sb = get_string_buf();
  sb_write_just(sb, j, NULL);
  sb_append(sb, "\n");
  fprint_sb(stdout, sb);
  zap_string_buf(sb);
}  /* p_just */

/*************
 *
 *   zap_parajust()
 *
 *************/

static
void zap_parajust(Parajust p)
{
  zap_ilist(p->from_pos);
  zap_ilist(p->into_pos);
  free_parajust(p);
}  /* zap_parajust */

/*************
 *
 *   zap_instancejust()
 *
 *************/

static
void zap_instancejust(Instancejust p)
{
  zap_plist_of_terms(p->pairs);
  free_instancejust(p);
}  /* zap_instancejust */

/*************
 *
 *   zap_ivyjust()
 *
 *************/

static
void zap_ivyjust(Ivyjust p)
{
  zap_ilist(p->pos1);
  zap_ilist(p->pos2);
  zap_plist_of_terms(p->pairs);
  free_ivyjust(p);
}  /* zap_ivyjust */

/*************
 *
 *   zap_just()
 *
 *************/

/* DOCUMENTATION
This routine frees a justification list, including any sublists.
*/

/* PUBLIC */
void zap_just(Just just)
{
  if (just != NULL) {
    zap_just(just->next);
    
    switch (just->type) {
    case INPUT_JUST:
    case GOAL_JUST:
    case DENY_JUST:
    case CLAUSIFY_JUST:
    case COPY_JUST:
    case PROPOSITIONAL_JUST:
    case NEW_SYMBOL_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
    case EVAL_JUST:
      break;  /* nothing to do */
    case EXPAND_DEF_JUST:
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
    case XXRES_JUST:
      zap_ilist(just->u.lst); break;
    case DEMOD_JUST:
      zap_i3list(just->u.demod); break;
    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      zap_parajust(just->u.para); break;
    case INSTANCE_JUST:
      zap_instancejust(just->u.instance); break;
    case IVY_JUST:
      zap_ivyjust(just->u.ivy); break;
    default: fatal_error("zap_just: unknown type");
    }
    free_just(just);
  }
}  /* zap_just */

/*************
 *
 *   get_parents()
 *
 *************/

/* DOCUMENTATION
This routine returns an Ilist of parent IDs.
If (all), get parents from the whole justification; otherwise
get parents from the first node only.
*/

/* PUBLIC */
Ilist get_parents(Just just, BOOL all)
{
  Ilist parents = NULL;
  Just g = just;

  while (g) {
    Just_type rule = g->type;
    if (rule==BINARY_RES_JUST || rule==HYPER_RES_JUST || rule==UR_RES_JUST) {
      /* [rule (nucid nuclit sat1id sat1lit nuclit2 sat2id sat2lit ...)] */
      Ilist p = g->u.lst;
      int nuc_id = p->i;
      parents = ilist_prepend(parents, nuc_id);
      p = p->next;
      while (p != NULL) {
	int sat_id = p->next->i;
	if (sat_id == 0)
	  ; /* resolution with x=x */
	else
	  parents = ilist_prepend(parents, sat_id);
	p = p->next->next->next;
      }
    }
    else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	     rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
      Parajust p   = g->u.para;
      parents = ilist_prepend(parents, p->from_id);
      parents = ilist_prepend(parents, p->into_id);
    }
    else if (rule == INSTANCE_JUST) {
      parents = ilist_prepend(parents, g->u.instance->parent_id);
    }
    else if (rule == EXPAND_DEF_JUST) {
      parents = ilist_prepend(parents, g->u.lst->i);
      parents = ilist_prepend(parents, g->u.lst->next->i);
    }
    else if (rule == FACTOR_JUST || rule == XXRES_JUST) {
      int parent_id = g->u.lst->i;
      parents = ilist_prepend(parents, parent_id);
    }
    else if (rule == UNIT_DEL_JUST) {
      int parent_id = g->u.lst->next->i;
      parents = ilist_prepend(parents, parent_id);
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == COPY_JUST ||
	     rule == DENY_JUST ||
	     rule == CLAUSIFY_JUST ||
	     rule == PROPOSITIONAL_JUST ||
	     rule == NEW_SYMBOL_JUST ||
	     rule == BACK_UNIT_DEL_JUST) {
      int parent_id = g->u.id;
      parents = ilist_prepend(parents, parent_id);
    }
    else if (rule == DEMOD_JUST) {
      I3list p;
      /* list of triples: (i=ID, j=position, k=direction) */
      for (p = g->u.demod; p; p = p->next)
	parents = ilist_prepend(parents, p->i);
    }
    else if (rule == IVY_JUST) {
      if (g->u.ivy->type == FLIP_JUST ||
	  g->u.ivy->type == BINARY_RES_JUST ||
	  g->u.ivy->type == PARA_JUST ||
	  g->u.ivy->type == INSTANCE_JUST ||
	  g->u.ivy->type == PROPOSITIONAL_JUST)
	parents = ilist_prepend(parents, g->u.ivy->parent1);
      if (g->u.ivy->type == BINARY_RES_JUST ||
	  g->u.ivy->type == PARA_JUST)
	parents = ilist_prepend(parents, g->u.ivy->parent2);
    }
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == MERGE_JUST ||
	     rule == EVAL_JUST ||
	     rule == GOAL_JUST ||
	     rule == INPUT_JUST)
      ;  /* do nothing */
    else
      fatal_error("get_parents, unknown rule.");

    g = (all ? g->next : NULL);
  }
  return reverse_ilist(parents);
}  /* get_parents */

/*************
 *
 *   first_negative_parent()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform first_negative_parent(Topform c)
{
  Ilist parents = get_parents(c->justification, TRUE);
  Ilist p;
  Topform first_neg = NULL;
  for (p = parents; p && first_neg == NULL; p = p->next) {
    Topform c = find_clause_by_id(p->i);
    if (negative_clause_possibly_compressed(c))
      first_neg = c;
  }
  zap_ilist(p);
  return first_neg;
}  /* first_negative_parent */

/*************
 *
 *   get_clanc()
 *
 *************/

static
Plist get_clanc(int id, Plist anc)
{
  Topform c = find_clause_by_id(id);

  if (c == NULL) {
    printf("get_clanc, clause with id=%d not found.\n", id);
    fatal_error("get_clanc, clause not found.");
  }
  
  if (!plist_member(anc, c)) {
    Ilist parents, p;
    anc = insert_clause_into_plist(anc, c, TRUE);
    parents = get_parents(c->justification, TRUE);

    for (p = parents; p; p = p->next) {
      anc = get_clanc(p->i, anc);
    }
    zap_ilist(parents);
  }
  return anc;
}  /* get_clanc */

/*************
 *
 *   get_clause_ancestors()
 *
 *************/

/* DOCUMENTATION
This routine returns the Plist of clauses that are ancestors of Topform c,
including clause c.  The result is sorted (increasing) by ID.
If any of the ancestors are compressed, they are uncompressed
(in place) and left uncompressed.
*/

/* PUBLIC */
Plist get_clause_ancestors(Topform c)
{
  Plist ancestors = get_clanc(c->id, NULL);
  return ancestors;
}  /* get_clause_ancestors */

/*************
 *
 *   proof_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int proof_length(Plist proof)
{
  return plist_count(proof);
}  /* proof_length */

/*************
 *
 *   map_id()
 *
 *************/

static
int map_id(I2list a, int arg)
{
  int val = assoc(a, arg);
  return val == INT_MIN ? arg : val;
}  /* map_id */

/*************
 *
 *   map_just()
 *
 *************/

/* DOCUMENTATION
Update the clause IDs in a justification according to the map.
*/

/* PUBLIC */
void map_just(Just just, I2list map)
{
  Just j;

  for (j = just; j; j = j->next) {
    Just_type rule = j->type;
    if (rule==BINARY_RES_JUST || rule==HYPER_RES_JUST || rule==UR_RES_JUST) {
      /* [rule (nucid n sat1id n n sat2id n ...)] */
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);  /* nucleus */
      p = p->next;
      while (p != NULL) {
	int sat_id = p->next->i;
	if (sat_id == 0)
	  ;  /* resolution with x=x */
	else
	  p->next->i = map_id(map, p->next->i);  /* satellite */
	p = p->next->next->next;
      }
    }
    else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	     rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
      Parajust p   = j->u.para;
      p->from_id = map_id(map, p->from_id);
      p->into_id = map_id(map, p->into_id);
    }
    else if (rule == INSTANCE_JUST) {
      Instancejust p   = j->u.instance;
      p->parent_id = map_id(map, p->parent_id);
    }
    else if (rule == EXPAND_DEF_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == FACTOR_JUST || rule == XXRES_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
    }
    else if (rule == UNIT_DEL_JUST) {
      Ilist p = j->u.lst;
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == COPY_JUST ||
	     rule == DENY_JUST ||
	     rule == CLAUSIFY_JUST ||
	     rule == PROPOSITIONAL_JUST ||
	     rule == NEW_SYMBOL_JUST ||
	     rule == BACK_UNIT_DEL_JUST) {
      j->u.id = map_id(map, j->u.id);
    }
    else if (rule == DEMOD_JUST) {
      I3list p;
      /* list of triples: <ID, position, direction> */
      for (p = j->u.demod; p; p = p->next)
	p->i = map_id(map, p->i);
    }
    else if (rule == IVY_JUST) {
      if (j->u.ivy->type == FLIP_JUST ||
	  j->u.ivy->type == BINARY_RES_JUST ||
	  j->u.ivy->type == PARA_JUST ||
	  j->u.ivy->type == INSTANCE_JUST ||
	  j->u.ivy->type == PROPOSITIONAL_JUST)
	j->u.ivy->parent1 = map_id(map, j->u.ivy->parent1);
      if (j->u.ivy->type == BINARY_RES_JUST ||
	  j->u.ivy->type == PARA_JUST)
	j->u.ivy->parent2 = map_id(map, j->u.ivy->parent2);
    }
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == MERGE_JUST ||
	     rule == EVAL_JUST ||
	     rule == GOAL_JUST ||
	     rule == INPUT_JUST)
      ;  /* do nothing */
    else
      fatal_error("get_clanc, unknown rule.");
  }
}  /* map_just */

/*************
 *
 *   just_count()
 *
 *************/

/* DOCUMENTATION
Return the number of justification elements in a justtification.
*/

/* PUBLIC */
int just_count(Just j)
{
  if (j == 0)
    return 0;
  else
    return 1 + just_count(j->next);
}  /* just_count */
/*************
 *
 *   mark_parents_as_used()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_parents_as_used(Topform c)
{
  Ilist parents = get_parents(c->justification, TRUE);
  Ilist p;
  for (p = parents; p; p = p->next) {
    Topform parent = find_clause_by_id(p->i);
    parent->used = TRUE;
  }
  zap_ilist(parents);
}  /* mark_parents_as_used */

/*************
 *
 *   cl_level()
 *
 *************/

static
I2list cl_level(Topform c, I2list others)
{
  int level = assoc(others, c->id);
  if (level != INT_MIN)
    return others;
  else {
    Ilist parents = get_parents(c->justification, TRUE);
    Ilist p;
    int max = -1;
    for (p = parents; p; p = p->next) {
      Topform parent = find_clause_by_id(p->i);
      int parent_level;
      others = cl_level(parent, others);
      parent_level = assoc(others, parent->id);
      max = IMAX(max, parent_level);
      
    }
    others = alist_insert(others, c->id, max+1);
    return others;
  }
}  /* cl_level */

/*************
 *
 *   clause_level()
 *
 *************/

/* DOCUMENTATION
Return the level of a clause.  Input clauses have level=0, and
a derived clause has level 1 more than the max of the levels of its parents.
*/

/* PUBLIC */
int clause_level(Topform c)
{
  I2list ancestors = cl_level(c, NULL);
  int level = assoc(ancestors, c->id);
  zap_i2list(ancestors);
  return level;
}  /* clause_level */

/*************
 *
 *   lit_string_to_int()
 *
 *************/

static
int lit_string_to_int(char *s)
{
  int i;
  if (str_to_int(s, &i))
    return i;
  else if (strlen(s) > 1)
    return INT_MIN;
  else
    return ctoi(s[0]);
}  /* lit_string_to_int */

/*************
 *
 *   args_to_ilist()
 *
 *************/

static
Ilist args_to_ilist(Term t)
{
  Ilist p = NULL;
  int i;
  for (i = 0; i < ARITY(t); i++) {
    Term a = ARG(t,i);
    char *s = sn_to_str(SYMNUM(a));
    int x = lit_string_to_int(s);
    if (x > 0) {
      if (ARITY(a) == 1 && is_constant(ARG(a,0), "flip"))
	p = ilist_append(p, -x);
      else
	p = ilist_append(p, x);
    }
    else if (str_ident(s, "xx"))
      p = ilist_append(ilist_append(p, 0), 0);
    else
      fatal_error("args_to_ilist, bad arg");
  }
  return p;
}  /* args_to_ilist */

/*************
 *
 *   term_to_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Just term_to_just(Term lst)
{
  if (nil_term(lst))
    return NULL;
  else {
    Term t = ARG(lst,0);
    Just j = get_just();
    j->next = term_to_just(ARG(lst,1));  /* do the tail */
    
    j->type = jstring_to_jtype(sn_to_str(SYMNUM(t)));

    switch (j->type) {

      /* primary and secondary are mixed */

    case INPUT_JUST:
    case GOAL_JUST:
      return j;

    case COPY_JUST:
    case DENY_JUST:
    case CLAUSIFY_JUST:
    case PROPOSITIONAL_JUST:
    case NEW_SYMBOL_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
      {
	int id;
	if (term_to_int(ARG(t,0), &id))
	  j->u.id = id;
	else
	  fatal_error("term_to_just, bad just id");
	return j;
      }

    case FLIP_JUST:   /* secondary */
    case XX_JUST:     /* secondary */
    case EVAL_JUST:   /* secondary */
    case MERGE_JUST:  /* secondary */
      {
	j->u.id = lit_string_to_int(sn_to_str(SYMNUM(ARG(t,0))));
	return j;
      }

    case DEMOD_JUST:      /* secondary, rewrite([id(pos,side), ... ]) */
      {
	I3list p = NULL;
	Term lst = ARG(t,0);
	if (!proper_listterm(lst))
	  fatal_error("term_to_just: rewrites must be proper list");
	while(cons_term(lst)) {
	  Term a = ARG(lst,0);
	  int i, j;
	  int k = 0;
	  char *s = sn_to_str(SYMNUM(a));
	  if (ARITY(a) == 2 &&
	      str_to_int(s,&i) &
	      term_to_int(ARG(a,0),&j)) {
	    if (is_constant(ARG(a,1), "L"))
	      k = 1;
	    else if (is_constant(ARG(a,1), "R"))
	      k = 2;
	    else
	      fatal_error("term_to_just: bad justification term (demod 1)");
	    p = i3list_append(p, i, j, k);
	  }
	  else if (ARITY(a) == 1 &&
	      str_to_int(s,&i) &
	      term_to_int(ARG(a,0),&j)) {
	    p = i3list_append(p, i, j, 1);
	  }
	  else if (ARITY(a) == 0 &&
		   str_to_int(s,&i)) {
	    p = i3list_append(p, i, 0, 1);
	  }
	  else
	    fatal_error("term_to_just: bad justification term (demod 2)");
	  lst = ARG(lst,1);
	}
	j->u.demod = p;
	return j;
      }

    case EXPAND_DEF_JUST:
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case FACTOR_JUST:
    case XXRES_JUST:
    case UNIT_DEL_JUST:   /* secondary */
      j->u.lst = args_to_ilist(t);
      return j;

    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      {
	int id;
	Term from = ARG(t,0);
	Term into = ARG(t,1);
	Parajust p = get_parajust();
	j->u.para = p;

	if (str_to_int(sn_to_str(SYMNUM(from)), &id))
	  p->from_id = id;
	else
	  fatal_error("term_to_just, bad from_id");

	p->from_pos = args_to_ilist(from);

	if (str_to_int(sn_to_str(SYMNUM(into)), &id))
	  p->into_id = id;
	else
	  fatal_error("term_to_just, bad into_id");

	p->into_pos = args_to_ilist(into);

	return j;
      }
    case INSTANCE_JUST:
      {
	int id;
	Term parent = ARG(t,0);
	Term pairs = ARG(t,1);
	Instancejust ij = get_instancejust();
	j->u.instance = ij;
	if (str_to_int(sn_to_str(SYMNUM(parent)), &id))
	  ij->parent_id = id;
	else
	  fatal_error("term_to_just, bad parent_id");

	ij->pairs = NULL;
	while (cons_term(pairs)) {
	  ij->pairs = plist_append(ij->pairs, copy_term(ARG(pairs,0)));
	  pairs = ARG(pairs,1);
	}
	return j;
      }
    
    case IVY_JUST:
      fatal_error("term_to_just, IVY_JUST not handled");
      return NULL;

    case UNKNOWN_JUST:
    default:
      printf("unknown: %d, %s\n", j->type, jstring(j));
      fatal_error("term_to_just, unknown justification");
      return NULL;
    }
  }
}  /* term_to_just */

/*************
 *
 *   primary_just_type()
 *
 *************/

/* DOCUMENTATION
Does a clause have justtification "input"?
*/

/* PUBLIC */
BOOL primary_just_type(Topform c, Just_type t)
{
  return c->justification && c->justification->type == t;
}  /* primary_just_type */

/*************
 *
 *   has_input_just()
 *
 *************/

/* DOCUMENTATION
Does a clause have justtification "input"?
*/

/* PUBLIC */
BOOL has_input_just(Topform c)
{
  return primary_just_type(c, INPUT_JUST);
}  /* has_input_just */

/*************
 *
 *   has_copy_just()
 *
 *************/

/* DOCUMENTATION
Does a clause have justification "copy"?
*/

/* PUBLIC */
BOOL has_copy_just(Topform c)
{
  return primary_just_type(c, COPY_JUST);
}  /* has_copy_just */

/*************
 *
 *   has_copy_flip_just()
 *
 *************/

/* DOCUMENTATION
Does a clause have justification "copy, flip", and nothing else?
*/

/* PUBLIC */
BOOL has_copy_flip_just(Topform c)
{
  return (c->justification &&
	  c->justification->type == COPY_JUST &&
	  c->justification->next &&
	  c->justification->next->type == FLIP_JUST &&
	  c->justification->next->next == NULL);
}  /* has_copy_flip_just */

/* ************************************************************************
   BV(2007-aug-20):  new functions to support tagged proofs (prooftrans)
   ***********************************************************************/

/*************
 *
 *   sb_tagged_write_res_just() -- (1 a 2 b c 3 d e 4 f)
 *
 *   Assume input is well-formed, that is, length is 3n+1 for n>1.
 *
 *************/

static
void sb_tagged_write_res_just(String_buf sb, Just g, I3list map)
{
  Ilist q;
  Ilist p = g->u.lst;

#if 1
   /* BV(2007-jul-10) */
  sb_append(sb, jstring(g));
  sb_append(sb, "\np ");
  sb_append_id(sb, p->i, map);
  for (q = p->next; q != NULL; q = q->next->next->next) {
    int sat_id  = q->next->i;
    sb_append(sb, "\np ");
    if (sat_id == 0)
      sb_append(sb, ",xx");
    else
      sb_append_id(sb, sat_id, map);
    }
  return;
#endif

}  /* sb_tagged_write_res_just */

/*************
 *
 *   sb_tagged_write_just()
 *
 *************/

/* DOCUMENTATION
This routine writes (to a String_buf) a clause justification.
No whitespace is printed before or after.
*/

/* PUBLIC */
void sb_tagged_write_just(String_buf sb, Just just, I3list map)
{
  Just g = just;
  /* sb_append(sb, "["); */
  while (g != NULL) {
    Just_type rule = g->type;
    sb_append(sb, "i ");
    if (rule == INPUT_JUST || rule == GOAL_JUST)
      sb_append(sb, jstring(g));
    else if (rule==BINARY_RES_JUST ||
	     rule==HYPER_RES_JUST ||
	     rule==UR_RES_JUST) {
      sb_tagged_write_res_just(sb, g, map);
    }
    else if (rule == DEMOD_JUST) {
      I3list p;
      sb_append(sb, jstring(g));
      for (p = g->u.demod; p; p = p->next) {
        sb_append(sb, "\np ");
	sb_append_int(sb, p->i);
      }
    }
    else if (rule == UNIT_DEL_JUST) {
      Ilist p = g->u.lst;
      int n = p->i;
      int id = p->next->i;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      if (n < 0) {
	sb_append_char(sb, itoc(-n));
	sb_append(sb, "(flip),");
      }
      else {
	sb_append_char(sb, itoc(n));
	sb_append(sb, ",");
      }
      sb_append_id(sb, id, map);
      sb_append(sb, ")");
    }
    else if (rule == FACTOR_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->i));
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->next->i));
      sb_append(sb, ")");
    }
    else if (rule == XXRES_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_char(sb, itoc(p->next->i));
      sb_append(sb, ")");
    }
    else if (rule == EXPAND_DEF_JUST) {
      Ilist p = g->u.lst;
      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_id(sb, p->i, map);
      sb_append(sb, ",");
      sb_append_id(sb, p->next->i, map);
      sb_append(sb, ")");
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == BACK_UNIT_DEL_JUST ||
	     rule == NEW_SYMBOL_JUST ||
	     rule == COPY_JUST ||
	     rule == DENY_JUST ||
	     rule == CLAUSIFY_JUST ||
	     rule == PROPOSITIONAL_JUST) {
      int id = g->u.id;
      sb_append(sb, jstring(g));
      sb_append(sb, "\np ");
      sb_append_id(sb, id, map);
    }
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == EVAL_JUST ||
	     rule == MERGE_JUST) {
      /* int id = g->u.id; */

#if 1
      /* BV(2007-jul-10) */
      sb_append(sb, "(flip)");
      /* break; */
#endif

    }
    else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	     rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
      Parajust p = g->u.para;

#if 1
      /* BV(2007-jul-10) */
      sb_append(sb, "para");
      sb_append(sb, "\np ");
      sb_append_id(sb, p->from_id, map);
      sb_append(sb, "\np ");
      sb_append_id(sb, p->into_id, map);
      /* break; */
#endif

    }
    else if (rule == INSTANCE_JUST) {
      Plist p;

      sb_append(sb, jstring(g));
      sb_append(sb, "(");
      sb_append_int(sb, g->u.instance->parent_id);
      sb_append(sb, ",[");

      for (p = g->u.instance->pairs; p; p = p->next) {
	sb_write_term(sb, p->v);
	if (p->next)
	  sb_append(sb, ",");
      }
      sb_append(sb, "])");
    }
    else if (rule == IVY_JUST) {
      sb_append(sb, jstring(g));
    }
    else {
      printf("\nunknown rule: %d\n", rule);
      fatal_error("sb_write_just: unknown rule");
    }
    g = g->next;
    /* if (g) */
    /*   sb_append(sb, ","); */
    sb_append(sb, "\n");
  }
  /* sb_append(sb, "]."); */
}  /* sb_tagged_write_just */

