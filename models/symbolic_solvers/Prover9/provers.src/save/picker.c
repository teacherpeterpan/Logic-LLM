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

#include "picker.h"
#include "../ladr/avltree.h"

/* Private definitions and types */

/*
  A *picker* is a way of selecting the given clause.  Examples
  are an "age picker", which always picks the oldest clause, and
  a "weight picker", which always picks the lightest clause.

  The overall selection strategy is determined by a ratio among
  the various pickers.

  Given a clause to be inserted into SOS, the pickers decide whether
  the want to index the clause.

  A clause can be indexed by more than one picker; for example, in the
  basic pick_given_ratio strategy, which is an age:weight ratio,
  two pickers are used, and each clause is indexed by both pickers.

  Pickers may also partition the SOS clauses: for example, in the
  semantic selection strategy, each clause goes either to the
  "true" picker or the "false" picker.

  Or, a picker may simply index a subset of the SOS clauses; for example,
  a "hints" picker indexes only the clauses that match hints.

  The complicated part of this code has to do with the parameter
  sos_limit, which limits the size of the SOS.  We don't simply
  accept clauses until we reach the limit, and then, when a good
  clause comes along, delete a bad one and keep the good one.  This
  causes too much keeping and deleting (indexing/unindexing is
  expensive).  Instead, when SOS is half full, we start being
  selective, and as it fills up, we become more selective.  When it
  is full, we are very selective.  See the routine sos_keep() below.
  When the SOS is full, and we decide to keep something new, we
  remove the "worst" sos clause.  See sos_displace() below.  These
  two routines have to agree on what is "good" and "bad".
*/

struct picker {
  Avl_node idx;          /* index of clauses (binary search (AVL) tree) */
  Ordertype (*compare) (void *, void *);  /* function for ordering idx */
  BOOL (*recognizer) (Topform); /* func to recognize clauses for this picker */
  int ratio_part;        /* how often this picker is used for selection */
  char *name;            /* "age", "true_semantics", etc. */
  char *code;            /* "A", "T", etc. */
  BOOL use_for_sos_limit;   /* use for sos_limit operation */
                         /* Statistics: */
  int number_deleted;    /*   deleted by sos_keep(), using this picker */
  int low_delete;        /*   lightest deleted by this picker */
  int number_displaced;  /*   displaced by sos_displace(), using this picker */
  int low_displace;      /*   lightest displaced by this picker */
};

typedef struct picker * Picker;

enum { HINTS_PICKER, AGE_PICKER, WEIGHT_PICKER,
       FALSE_PICKER, TRUE_PICKER, RANDOM_PICKER };  /* number must match following */

#define NUM_PICKERS 6   /* make sure this matches the preceding line */

static struct picker Pickers[NUM_PICKERS];

static int Current_picker, Next_i;  /* state for ratio */

/*************
 *
 *   any_topform()
 *
 *************/

static
BOOL any_topform(Topform c)
{
  return TRUE;
}  /* any_topform */

/*************
 *
 *   hint_matcher()
 *
 *************/

static
BOOL hint_matcher(Topform c)
{
  return c->matches_hint;
}  /* hint_matcher */

/*************
 *
 *   true_semantics()
 *
 *************/

static
BOOL true_semantics(Topform c)
{
  return c->semantics;
}  /* true_semantics */

/*************
 *
 *   false_semantics()
 *
 *************/

static
BOOL false_semantics(Topform c)
{
  return !c->semantics;
}  /* false_semantics */

/*************
 *
 *   update_picker_ratios()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void update_picker_ratios(Prover_options opt)
{
  Pickers[HINTS_PICKER].ratio_part = parm(opt->hints_part);
  Pickers[AGE_PICKER].ratio_part = parm(opt->age_part);
  Pickers[WEIGHT_PICKER].ratio_part = parm(opt->weight_part);
  Pickers[FALSE_PICKER].ratio_part = parm(opt->false_part);
  Pickers[TRUE_PICKER].ratio_part = parm(opt->true_part);
  Pickers[RANDOM_PICKER].ratio_part = parm(opt->random_part);
  /* reset ratio state */
  Current_picker = 0;
  Next_i = 0;
}  /* update_picker_ratios */

/*************
 *
 *   init_pickers()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_pickers(Prover_options opt)
{
  Picker p;
  int i;
  for (i = 0; i < NUM_PICKERS; i++) {
    p = &(Pickers[i]);
    p->idx = NULL;
    p->number_deleted = 0;
    p->number_displaced = 0;
    p->low_delete = INT_MAX;
    p->low_displace = INT_MAX;
  }

  p = &(Pickers[HINTS_PICKER]);
  p->code = "H"; p->name = "Hints";
  p->compare = (Ordertype (*) (void *, void *)) cl_wt_id_compare;
  p->recognizer = hint_matcher;  /* function */
  p->use_for_sos_limit = TRUE;

  p = &(Pickers[AGE_PICKER]);
  p->code = "A"; p->name = "Age";
  p->compare = (Ordertype (*) (void *, void *)) cl_id_compare;
  p->recognizer = any_topform;  /* function */
  p->use_for_sos_limit = FALSE;

  p = &(Pickers[WEIGHT_PICKER]);
  p->code = "W"; p->name = "Weight";
  p->compare = (Ordertype (*) (void *, void *)) cl_wt_id_compare;
  p->recognizer = any_topform;  /* function */
  p->use_for_sos_limit = TRUE;

  p = &(Pickers[FALSE_PICKER]);
  p->code = "F"; p->name = "False_semantics";
  p->compare = (Ordertype (*) (void *, void *)) cl_wt_id_compare;
  p->recognizer = false_semantics;  /* function */
  p->use_for_sos_limit = TRUE;

  p = &(Pickers[TRUE_PICKER]);
  p->code = "T"; p->name = "True_semantics";
  p->compare = (Ordertype (*) (void *, void *)) cl_wt_id_compare;
  p->recognizer = true_semantics;  /* function */
  p->use_for_sos_limit = TRUE;

  p = &(Pickers[RANDOM_PICKER]);
  p->code = "R"; p->name = "Random";
  p->compare = (Ordertype (*) (void *, void *)) cl_id_compare;
  p->recognizer = any_topform;  /* function */
  p->use_for_sos_limit = FALSE;

  update_picker_ratios(opt);

  if (parm(opt->random_seed) >= 0)
    srand(parm(opt->random_seed));
  else {
    unsigned t = absolute_wallclock() + my_process_id();
    printf("\nGenerated seed random number generation: %u\n", t);
    srand(t);
  }
}  /* init_pickers */

/*************
 *
 *   picker_empty()
 *
 *************/

static
BOOL picker_empty(int i)
{
  return Pickers[i].idx == NULL;
}  /* picker_empty */

/*************
 *
 *   next_from_ratio()
 *
 *************/

static
int next_from_ratio()
{
  /* Get the next picker, which is determined by the ratio.
     This uses two static variables to maintain the state.
     Current_picker:
     Next_i: counts up to the ratio for the Current_picker.
   */
  int start = Current_picker;  /* to prevent a loop when nothing avaliable */

  while (picker_empty(Current_picker) ||
	 Next_i >= Pickers[Current_picker].ratio_part) {
    Next_i = 0;
    Current_picker = (Current_picker + 1) % NUM_PICKERS;
    if (Current_picker == start)
      break;  /* we're back to the start */
  }

  if (picker_empty(Current_picker))
    return -1;  /* nothing available */
  else {
    Next_i++;  /* for next call */
    return Current_picker;
  }
}  /* next_from_ratio */

/*************
 *
 *   pickers_for_clause()
 *
 *   Return a Plist identifying the pickers for a clause.
 *   Exclude ones with ratio_part = 0, because those pickers are not in use.
 *   If (c == NULL), get all pickers, except those with ratio_part=0
 *
 *************/

static
Plist pickers_for_clause(Topform c)
{
  Plist pickers = NULL;
  int i;
  for (i = 0; i < NUM_PICKERS; i++) {
    Picker p = &(Pickers[i]);
    if (p->ratio_part != 0 && (!c || (*(p->recognizer))(c)))
      pickers = plist_append(pickers, p);
  }
  return pickers;
}  /* pickers_for_clause */

/*************
 *
 *   get_age_picker()
 *
 *************/

static
Picker get_age_picker(void)
{
  Picker p = &(Pickers[AGE_PICKER]);
  return (p->ratio_part == 0 ? NULL : p);
}  /* get_age_picker */

/*************
 *
 *   get_sos_limit_pickers()
 *
 *************/

static
Plist get_sos_limit_pickers(Plist pickers)
{
  if (pickers == NULL)
    return NULL;
  else {
    Plist rest = get_sos_limit_pickers(pickers->next);
    Picker p = pickers->v;
    if (p->use_for_sos_limit) {
      pickers->next = rest;
      return(pickers);
    }
    else {
      free_plist(pickers);
      return rest;
    }
  }
}  /* get_sos_limit_pickers */

/*************
 *
 *   largest_picker()
 *
 *************/

static
Picker largest_picker(Plist pickers)
{
  Picker largest_picker = NULL;
  int size_of_largest_picker = -1;
  Plist a;
  for (a = pickers; a; a = a->next) {
    Picker p = a->v;
    int size = avl_size(p->idx);
    if (size > size_of_largest_picker) {
      largest_picker = p;
      size_of_largest_picker = size;
    }
  }
  return largest_picker;
}  /* largest_picker */

/*************
 *
 *   update_pickers()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void update_pickers(Topform c, BOOL insert)
{
  Plist pickers = pickers_for_clause(c);
  Plist a;
  for (a = pickers; a; a = a->next) {
    Picker p = a->v;
    if (insert)
      p->idx = avl_insert(p->idx, c, p->compare);
    else
      p->idx = avl_delete(p->idx, c, p->compare);
  }
  zap_plist(pickers);
}  /* update_pickers */

/*************
 *
 *   insert_into_sos1()
 *
 *************/

/* DOCUMENTATION
This routine appends a clause to the sos list and updates
the (private) index for extracting sos clauses.
*/

/* PUBLIC */
void insert_into_sos1(Topform c, Clist sos)
{
  update_pickers(c, TRUE);
  clist_append(c, sos);
}  /* insert_into_sos1 */

/*************
 *
 *   remove_from_sos1()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from the sos list and updates
the index for extracting the lightest and heaviest clauses.
*/

/* PUBLIC */
void remove_from_sos1(Topform c, Clist sos)
{
  update_pickers(c, FALSE);
  clist_remove(c, sos);
}  /* remove_from_sos1 */

/*************
 *
 *   get_given_clause1()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform get_given_clause1(Clist sos, int num_given,
			 Prover_options opt, char **type)
{
  if (clist_empty(sos))
    return NULL;
  else {
    Topform giv;

    if (flag(opt->input_sos_first) && initial_clause(sos->first->c)) {
      giv = sos->first->c;
      *type = "I";
    }
    else {
      int i = next_from_ratio();
      if (i < 0)
	return NULL;  /* something's probably wrong */
      else {
	Picker p = &(Pickers[i]);
	if (str_ident(p->name, "Random")) {
	  int n = avl_size(p->idx);
	  int i = (rand() % n) + 1;
	  giv = avl_nth_item(p->idx, i);
	}
	else
	  giv = avl_smallest(p->idx);
	*type = p->code;
      }
    }
    remove_from_sos1(giv, sos);
    return giv;
  }
}  /* get_given_clause1 */

/***************
 *
 *   sos_keep1()
 *
 **************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL sos_keep1(Topform c, Clist sos, Prover_options opt)
{
  Plist pickers = pickers_for_clause(c);
  if (pickers == NULL)
    return FALSE;  /* no picker will accept the clause */
  else {
    int limit = parm(opt->sos_limit);
    /* printf("sos_keep, sos=%d, c=%d:", clist_length(sos), c->weight); */
    if (limit == -1) {
      zap_plist(pickers);
      return TRUE;  /* some picker wants the clause, and there is no limit */
    }
    else {
      double fullness_of_sos = clist_length(sos) / (double) limit;
      double x = .5;    /* when sos is at (x * limit), start being selective */
      if (fullness_of_sos < x) {
	zap_plist(pickers);
	return TRUE;  /* there is a limit, but we don't yet consider it */
      }
      else {
	Picker p;
	pickers = get_sos_limit_pickers(pickers);
	p = largest_picker(pickers);
	zap_plist(pickers);
	if (p == NULL) {
	  return FALSE;  /* all pickers complete, and we're getting full */
	}
	else {
	  /* keep if wt < than clause at ref_point */
	  double ref_point = x + (1 - fullness_of_sos);
	  /* fullness_of_sos >= x,
	     x <= ref_point <= 1.
	         if sos is full: ref_point=x
		 if sos is at x: ref_point=1
	   */
	  int n = avl_size(p->idx) * ref_point;
	  Topform d = avl_nth_item(p->idx, n);
	  if (d == NULL) {
	    return TRUE;  /* the picker is empty */
	  }
	  else {
	    if (c->weight >= d->weight) {
	      if (c->weight < p->low_delete) {
		printf("\nLow Water (keep, %s): wt=%d\n", p->name, c->weight);
		fflush(stdout);
		p->low_delete = c->weight;
	      }
	      p->number_deleted++;
	      return FALSE;
	    }
	    else {
	      /* keep the clause (smaller than clause at reference point) */
	      return TRUE;
	    }
	  }
	}
      }
    }
  }
}  /* sos_keep1 */

/*************
 *
 *   largest_sos_limit_picker()
 *
 *************/

static
Picker largest_sos_limit_picker(Topform c)
{
  Plist pickers = get_sos_limit_pickers(pickers_for_clause(c));
  Picker p = largest_picker(pickers);
  zap_plist(pickers);
  return p;
}  /* largest_sos_limit_picker */

/*************
 *
 *   worst_clause()
 *
 *************/

static
Topform worst_clause(Picker p)
{
  /* If AGE_PICKER is not being used, simply return the last clause in P
     that does not match a hint.
     Otherwise, let A be the age (ID) of the median clause in AGE_PICKER;
     then return the last clause in P with ID > A that does not match a hint.
     (We don't delete old heavy clauses because they might be picked by
     AGE_PICKER.)
  */
  if (p == NULL || p->idx == NULL)
    return NULL;
  else {
    int median_id, n;
    Topform c;
    if (picker_empty(AGE_PICKER))
      median_id = 0;
    else {
      c = avl_item_at_position(Pickers[AGE_PICKER].idx, 0.5);
      median_id = c->id;
    }
    
    /* Start with largest, count down until ID > median_age && not hint matcher */
    n = avl_size(p->idx);
    c = avl_nth_item(p->idx, n);
    while (c && (c->id <= median_id || hint_matcher(c))) {
      c = avl_nth_item(p->idx, --n);
    }
    return c;
  }
}  /* worst_clause */

/*************
 *
 *   sos_displace1() - delete the worst sos clause
 *
 *************/

/* DOCUMENTATION
Disable the "worst" clause.  Try to make it a clause
similar to c (in the same picker).
*/

/* PUBLIC */
void sos_displace1(Topform c,
		  void (*disable_proc) (Topform))
{
  Picker p = largest_sos_limit_picker(c);
  Topform worst = worst_clause(p);
  if (worst == NULL) {
    /* Ignore the clause c, and just get the worst clause. */
    p = largest_sos_limit_picker(NULL);
    worst = worst_clause(p);
  }
  if (worst == NULL) {
    p = get_age_picker();
    if (p)
      worst = avl_largest(p->idx);
  }
  if (worst == NULL) {
    picker_report();
    fatal_error("sos_displace1, cannot find worst clause\n");
  }
  else {
    if (worst->weight < p->low_displace) {
      p->low_displace = worst->weight;
      printf("\nLow Water (displace, %s): id=%d, wt=%d\n",
	     p->name, worst->id, worst->weight);
      fflush(stdout);
    }
    disable_proc(worst);
    p->number_displaced++;
  }
}  /* sos_displace1 */

/*************
 *
 *   zap_picker_indexes()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_picker_indexes(void)
{
  /* free all Picker indexes */
  int i;
  for (i = 0; i < NUM_PICKERS; i++) {
    avl_zap(Pickers[i].idx);
    Pickers[i].idx = NULL;
  }
  /* reset ratio state */
  Current_picker = 0;
  Next_i = 0;
}  /* zap_picker_indexes */

/*************
 *
 *   picker_report()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void picker_report(void)
{
  int i;
  print_separator(stdout, "PICKER REPORT", TRUE);
  printf("%16s  %6s  %10s  %8s  %6s  %11s  %6s\n",
	 "Picker","Size","Median_wt","Deleted","(Low)","Displaced","(Low)");
  for (i = 0; i < NUM_PICKERS; i++) {
    Picker p = &(Pickers[i]);
    Topform c = avl_item_at_position(p->idx, 0.5);
    printf("%16s  %6d  %10d  %8d  %6d  %11d  %6d\n",
	   p->name, avl_size(p->idx), c ? c->weight : -1,
	   p->number_deleted,
	   p->low_delete == INT_MAX ? -1 : p->low_delete,
	   p->number_displaced,
	   p->low_displace == INT_MAX ? -1 : p->low_displace);
  }
  print_separator(stdout, "end of picker report", FALSE);
  fflush(stdout);
}  /* picker_report */

