#include "just.h"

/* Private definitions and types */

/*
 * memory management
 */

static unsigned Just_gets, Just_frees;
static unsigned Parajust_gets, Parajust_frees;

#define BYTES_JUST sizeof(struct just)
#define PTRS_JUST BYTES_JUST%BPP == 0 ? BYTES_JUST/BPP : BYTES_JUST/BPP + 1

#define BYTES_PARAJUST sizeof(struct parajust)
#define PTRS_PARAJUST BYTES_PARAJUST%BPP == 0 ? BYTES_PARAJUST/BPP : BYTES_PARAJUST/BPP + 1

/*************
 *
 *   Just get_just()
 *
 *************/

static
Just get_just(void)
{
  Just p = get_mem(PTRS_JUST);
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

static
Parajust get_parajust(void)
{
  Parajust p = get_mem(PTRS_PARAJUST);
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

  n = BYTES_JUST;
  fprintf(fp, "just (%4d)         %11u%11u%11u%9.1f K\n",
          n, Just_gets, Just_frees,
          Just_gets - Just_frees,
          ((Just_gets - Just_frees) * n) / 1024.);

  n = BYTES_PARAJUST;
  fprintf(fp, "parajust (%4d)     %11u%11u%11u%9.1f K\n",
          n, Parajust_gets, Parajust_frees,
          Parajust_gets - Parajust_frees,
          ((Parajust_gets - Parajust_frees) * n) / 1024.);

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
 *   clausify_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for clausify.
*/

/* PUBLIC */
Just clausify_just(void)
{
  /* (CLAUSIFY_JUST) */
  Just j = get_just();
  j->type = CLAUSIFY_JUST;
  return j;
}  /* clausify_just */

/*************
 *
 *   copy_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for copy.
*/

/* PUBLIC */
Just copy_just(Clause c)
{
  /* (COPY_JUST parent_id) */
  Just j = get_just();
  j->type = COPY_JUST;
  j->u.id = c->id;
  return j;
}  /* copy_just */

/*************
 *
 *   back_demod_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for back_demod.
*/

/* PUBLIC */
Just back_demod_just(Clause c)
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
Just back_unit_deletion_just(Clause c)
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
Just binary_res_just(Clause c1, int n1, Clause c2, int n2)
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
 *   cd_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a factorization.
*/

/* PUBLIC */
Just cd_just(Clause major, Clause minor)
{
  /* (CD_JUST (major_id minor_id)) */
  Just j = get_just();
  j->type = CD_JUST;
  j->u.lst = ilist_append(ilist_append(NULL, major->id), minor->id);
  return j;
}  /* cd_just */

/*************
 *
 *   factor_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a factorization.
*/

/* PUBLIC */
Just factor_just(Clause c, int lit1, int lit2)
{
  /* (FACTOR_JUST (clause_id lit1 lit2)) */
  Just j = get_just();
  j->type = FACTOR_JUST;
  j->u.lst = ilist_append(ilist_append(ilist_append(NULL,c->id),lit1),lit2);
  return j;
}  /* factor_just */

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
 *   para_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a paramodulation
inference.  The position vectors are not copied.
*/

/* PUBLIC */
Just para_just(Just_type rule,
	       Clause from, Ilist from_vec,
	       Clause into, Ilist into_vec)
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
 *   para_just_rev_copy()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a paramodulation
inference.  The position vectors are copied and reversed.
*/

/* PUBLIC */
Just para_just_rev_copy(Just_type rule,
			Clause from, Ilist from_vec,
			Clause into, Ilist into_vec)
{
  return para_just(rule,
		   from, reverse_ilist(copy_ilist(from_vec)),
		   into, reverse_ilist(copy_ilist(into_vec)));
}  /* para_just_rev_copy */

/*************
 *
 *   demod_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for demodulation.
The given ilist is not copied.
*/

/* PUBLIC */
Just demod_just(Ilist g)
{
  Just j = get_just();
  j->type = DEMOD_JUST;
  j->u.lst = g;
  return j;
}  /* demod_just */

/*************
 *
 *   unit_del_just()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a justification list for a factorization.
*/

/* PUBLIC */
Just unit_del_just(Clause deleter, int literal_num)
{
  /* UNIT_DEL (clause-id literal-num) */
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
    case CLAUSIFY_JUST:
    case COPY_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
      j2->u.id = j->u.id;
      break;
    case CD_JUST:
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case DEMOD_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
      j2->u.lst = copy_ilist(j->u.lst);
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
    default: fatal_error("copy_justification: unknown type");
    }
    return j2;
  }
}  /* copy_justification */

/*************
 *
 *   fix_input_just()
 *
 *************/

/* DOCUMENTATION
If the clause's justification is simply a copy, replace the
justification with the justification of the parent.  This
is intended for input clauses for which the pre_processing
does nothing.  The purpose is simply to prevent duplicate
steps when printing proofs.
*/

/* PUBLIC */
void fix_input_just(Clause c)
{
  Just j = c->justification;
  if (j && j->type == COPY_JUST && j->next == NULL) {
    Clause parent = find_clause_by_id(j->u.id);
    c->justification = copy_justification(parent->justification);
    zap_just(j);
  }
}  /* fix_input_just */

/*************
 *
 *   jstring() - strings for printing justifications
 *
 *************/

static
char *jstring(Just j)
{
  switch (j->type) {

  // primary justifications

  case INPUT_JUST:         return "input";
  case CLAUSIFY_JUST:      return "clausify";
  case COPY_JUST:          return "copy";
  case BACK_DEMOD_JUST:    return "back_demod";
  case BACK_UNIT_DEL_JUST: return "back_unit_del";
  case BINARY_RES_JUST:    return "resolve";
  case HYPER_RES_JUST:     return "hyper";
  case UR_RES_JUST:        return "ur";
  case CD_JUST:            return "cd";
  case FACTOR_JUST:        return "factor";
  case PARA_JUST:          return "para";
  case PARA_FX_JUST:       return "para_fx";
  case PARA_IX_JUST:       return "para_ix";
  case PARA_FX_IX_JUST:    return "para_fx_ix";

  // secondary justifications need a space

  case FLIP_JUST:          return " flip";
  case XX_JUST:            return " xx";
  case MERGE_JUST:         return " merge";
  case DEMOD_JUST:         return " demod";
  case UNIT_DEL_JUST:      return " unit_del";
  }
  return "unknown";
}  /* jstring */

/*************
 *
 *   itoc()
 *
 *************/

static
char itoc(int i)
{
  return 'a' + i - 1;
}  /* itoc */

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
int jmap1(Ilist map, int i)
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
char *jmap2(Ilist map, int i, char *a)
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
 *   fprint_res_just() -- (1 a 2 b c 3 d e 4 f)
 *
 *   Assume input is well-formed, that is, length is 3n+1 for n>1.
 *
 *************/

static
void fprint_res_just(FILE *fp, Ilist p, Ilist map)
{
  char a[10];
  Ilist q;
  fprintf(fp, "(%d%s", jmap1(map, p->i), jmap2(map, p->i, a));
  for (q = p->next; q != NULL; q = q->next->next->next) {
    fprintf(fp, " %c", itoc(q->i));
    fprintf(fp, " %d%s", jmap1(map, q->next->i), jmap2(map, q->next->i, a));
    fprintf(fp, " %c", itoc(q->next->next->i));
  }
  fprintf(fp, ")");
  fflush(fp);
}  /* fprint_res_just */

/*************
 *
 *   fprint_position()
 *
 *************/

static
void fprint_position(FILE *fp, Ilist p)
{
  Ilist q;
  fprintf(fp, "(%c", itoc(p->i));
  for (q = p->next; q != NULL; q = q->next) {
    fprintf(fp, " %d", q->i);
  }
  fprintf(fp, ")");
  fflush(fp);
}  /* fprint_position */

/*************
 *
 *   fprint_just()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a clause justification.
No whitespace is printed before or after.
*/

/* PUBLIC */
void fprint_just(FILE *fp, Just just, Ilist map)
{
  char a[10];
  Just g = just;
  fprintf(fp, "[");
  while (g != NULL) {
    Just_type rule = g->type;
    if (rule == INPUT_JUST ||
	     rule == CLAUSIFY_JUST)
      fprintf(fp, "%s", jstring(g));
    else if (rule==BINARY_RES_JUST ||
	     rule==HYPER_RES_JUST ||
	     rule==UR_RES_JUST) {
      fprintf(fp, "%s ", jstring(g));
      fprint_res_just(fp, g->u.lst, map);
    }
    else if (rule == DEMOD_JUST ||
	     rule == CD_JUST) {
      fprintf(fp, "%s ", jstring(g));
      fprint_ilist(fp, g->u.lst);
    }
    else if (rule == UNIT_DEL_JUST) {
      Ilist p = g->u.lst;
      fprintf(fp, "%s (%c %d%s)", jstring(g), itoc(p->i),
	      jmap1(map, p->next->i),
	      jmap2(map, p->next->i, a));
    }
    else if (rule == FACTOR_JUST) {
      Ilist p = g->u.lst;
      fprintf(fp, "%s (%d%s %c %c)", jstring(g),
	      jmap1(map, p->i), jmap2(map, p->i, a),
	      itoc(p->next->i), itoc(p->next->next->i));
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == BACK_UNIT_DEL_JUST ||
	     rule == COPY_JUST)
      fprintf(fp, "%s %d%s", jstring(g),
	      jmap1(map, g->u.id),
	      jmap2(map, g->u.id, a));
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == MERGE_JUST)
      fprintf(fp, "%s %c", jstring(g), itoc(g->u.id));
    else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	     rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
      Parajust p = g->u.para;
      fprintf(fp, "%s (%d%s ", jstring(g),
	      jmap1(map, p->from_id), jmap2(map, p->from_id, a));
      fprint_position(fp, p->from_pos);
      fprintf(fp, " %d%s ",
	      jmap1(map, p->into_id), jmap2(map, p->into_id, a));
      fprint_position(fp, p->into_pos);
      fprintf(fp, ")");
    }
    else {
      printf("\nunknown rule: %d\n", rule);
      fatal_error("fprint_just: unknown rule");
    }
    g = g->next;
  }
  fprintf(fp, "]");
}  /* fprint_just */

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
  fprint_just(stdout, j, NULL);
  printf("\n");
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
    case CLAUSIFY_JUST:
    case COPY_JUST:
    case BACK_DEMOD_JUST:
    case BACK_UNIT_DEL_JUST:
    case FLIP_JUST:
    case XX_JUST:
    case MERGE_JUST:
      break;  /* nothing to do */
    case CD_JUST:
    case BINARY_RES_JUST:
    case HYPER_RES_JUST:
    case UR_RES_JUST:
    case DEMOD_JUST:
    case UNIT_DEL_JUST:
    case FACTOR_JUST:
      zap_ilist(just->u.lst); break;
    case PARA_JUST:
    case PARA_FX_JUST:
    case PARA_IX_JUST:
    case PARA_FX_IX_JUST:
      zap_parajust(just->u.para); break;
    default: fatal_error("zap_just: unknown type");
    }
    free_just(just);
  }
}  /* zap_just */

/*************
 *
 *   get_clanc()
 *
 *************/

static
Plist get_clanc(int id, Plist anc)
{
  Clause c = find_clause_by_id(id);
  if (c == NULL) {
    printf("get_clanc, clause with id=%d not found.\n", id);
    fatal_error("get_clanc, clause not found.");
  }

  if (!plist_member(anc, c)) {
    Just g = c->justification;
    anc = insert_clause_into_plist(anc, c, TRUE);

    while (g != NULL) {
      Just_type rule = g->type;
      if (rule==BINARY_RES_JUST || rule==HYPER_RES_JUST || rule==UR_RES_JUST) {
	/* [rule (nucid nuclit sat1id sat1lit nuclit2 sat2id sat2lit ...)] */
	Ilist p = g->u.lst;
	int nuc_id = p->i;
	anc = get_clanc(nuc_id, anc);
	p = p->next;
	while (p != NULL) {
	  int sat_id = p->next->i;
	  anc = get_clanc(sat_id, anc);
	  p = p->next->next->next;
	}
      }
      else if (rule == PARA_JUST || rule == PARA_FX_JUST ||
	       rule == PARA_IX_JUST || rule == PARA_FX_IX_JUST) {
	Parajust p   = g->u.para;
	anc = get_clanc(p->from_id, anc);
	anc = get_clanc(p->into_id, anc);
      }
      else if (rule == FACTOR_JUST) {
	int parent_id = g->u.lst->i;
	anc = get_clanc(parent_id, anc);
      }
      else if (rule == UNIT_DEL_JUST) {
	int parent_id = g->u.lst->next->i;
	anc = get_clanc(parent_id, anc);
      }
      else if (rule == CD_JUST) {
	Ilist p = g->u.lst;
	int major_id = p->i;
	int minor_id = p->next->i;
	anc = get_clanc(major_id, anc);
	anc = get_clanc(minor_id, anc);
      }
      else if (rule == BACK_DEMOD_JUST ||
	       rule == COPY_JUST ||
	       rule == BACK_UNIT_DEL_JUST) {
	int parent_id = g->u.id;
	anc = get_clanc(parent_id, anc);
      }
      else if (rule == DEMOD_JUST) {
	Ilist p;
	for (p = g->u.lst; p; p = p->next)
	  anc = get_clanc(p->i, anc);
      }
      else if (rule == FLIP_JUST ||
	       rule == XX_JUST ||
	       rule == MERGE_JUST ||
	       rule == INPUT_JUST ||
	       rule == CLAUSIFY_JUST)
	;  /* do nothing */
      else
	fatal_error("get_clanc, unknown rule.");
      g = g->next;
    }
  }
  return anc;
}  /* get_clanc */

/*************
 *
 *   get_clause_ancestors()
 *
 *************/

/* DOCUMENTATION
This routine returns the Plist of clauses that are ancestors of Clause c,
including clause c.  The result is sorted (increasing) by ID.
*/

/* PUBLIC */
Plist get_clause_ancestors(Clause c)
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
  Plist p;
  int n = 0;
  for (p = proof; p; p = p->next) {
    Clause c = p->v;
    if (c->justification != NULL)
      n++;
  }
  return n;
}  /* proof_length */

/*************
 *
 *   map_id()
 *
 *************/

static
int map_id(Ilist a, int arg)
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
void map_just(Just just, Ilist map)
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
    else if (rule == FACTOR_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
    }
    else if (rule == UNIT_DEL_JUST) {
      Ilist p = j->u.lst;
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == CD_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == BACK_DEMOD_JUST ||
	     rule == COPY_JUST ||
	     rule == BACK_UNIT_DEL_JUST) {
      j->u.id = map_id(map, j->u.id);
    }
    else if (rule == DEMOD_JUST) {
      Ilist p;
      for (p = j->u.lst; p; p = p->next)
	p->i = map_id(map, p->i);
    }
    else if (rule == FLIP_JUST ||
	     rule == XX_JUST ||
	     rule == MERGE_JUST ||
	     rule == INPUT_JUST ||
	     rule == CLAUSIFY_JUST)
      ;  /* do nothing */
    else
      fatal_error("get_clanc, unknown rule.");
  }
}  /* map_just */

