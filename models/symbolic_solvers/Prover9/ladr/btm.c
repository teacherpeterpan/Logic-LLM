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

#include "btm.h"

/* Private definitions and types */

typedef struct ac_match_pos * Ac_match_pos;
typedef struct ac_match_free_vars_pos * Ac_match_free_vars_pos;

struct btm_state {

    Btm_state parent, next, prev, first_child, last_child;

    Term t1, t2;         /* terms being matched */
    Context c1;          /* context for variables of t1*/

    int varnum;          /* for unbinding when backtracking */
    Context cb;          /* for unbinding when backtracking */

    Unif_alternative alternative;  /* type of alternative (position) */

    /* for commutative unification */
    int flipped;
    Btm_state position_bt;      /* in sequence of alternatives */

    /* for AC matching */
    Ac_match_pos acm;  /* in sequence of AC matchers */
    int partial;               /* partial match for this pair */
    };

struct ac_match_pos {
    Term t1, t2;         /* t1 is pattern, t2 is subject */
    Context c1;          /* context for variables in t1  */
    int n1;              /* number of arguments in t1 */ 
    int n2;              /* size of set of set of args in t2 */
    Term args1[MAX_ACM_ARGS], args2[MAX_ACM_ARGS];  /* the arguments */
           /* position in sequence of matches for complex args of args2 */
    Btm_state bt1[MAX_ACM_ARGS];
           /* flags indicating which of args1 have been matched */
    int match1[MAX_ACM_ARGS];
           /* integer indicating how many of each of args2 have been matched */
    int match2[MAX_ACM_ARGS];
    int mults2[MAX_ACM_ARGS];  /* multiplicities for args2 */
           /* indicates which of args2 are matched by bound vars in args1 */
    int bound_matches[MAX_ACM_ARGS], bound_count;
    int last_a1_symbol;   /* position of last non-variable arg in args1 */
           /* list of backtrack positions for free variables of args1 */
    Ac_match_free_vars_pos free_first, free_last;
           /* # args of unmatched term---used for partial match */
    int partial_term_size;
    Ac_match_pos next;  /* for avail list only */
    };

struct ac_match_free_vars_pos {
    int varnum;                 /* the index of the free variable */
    int coef;                   /* # of occurrences of the var in args1 */
    int targets[MAX_ACM_ARGS];   /* terms in args2 that can go with variable */
    int n;                      /* number of tragets*/
    int combo[MAX_ACM_ARGS];     /* current subset of the targets */
    Ac_match_free_vars_pos prev, next;
    };

/* #define DEBUG */

#define POP       1
#define BACKTRACK 2
#define GO        3
#define SUCCESS   4
#define FAILURE   5

/* Bind a variable, record binding in a bt_node. */

#define BIND_BT(i, c1, t2, c2, bt) {  \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    bt->varnum = i; bt->cb = c1; }

/* The following declaration is due to mutual recursion with match_bt_guts. */

static int match_commute(Term t1, Context c1, Term t2, Btm_state bt);

/*
 * memory management
 */

#define PTRS_AC_MATCH_POS PTRS(sizeof(struct ac_match_pos))
static unsigned Ac_match_pos_gets, Ac_match_pos_frees;

#define PTRS_AC_MATCH_FREE_VARS_POS PTRS(sizeof(struct ac_match_free_vars_pos))
static unsigned Ac_match_free_vars_pos_gets, Ac_match_free_vars_pos_frees;

#define PTRS_BTM_STATE PTRS(sizeof(struct btm_state))
static unsigned Btm_state_gets, Btm_state_frees;

/*************
 *
 *   Ac_match_pos get_ac_match_pos()
 *
 *************/

static
Ac_match_pos get_ac_match_pos(void)
{
  Ac_match_pos p = get_cmem(PTRS_AC_MATCH_POS);
  Ac_match_pos_gets++;
  return(p);
}  /* get_ac_match_pos */

/*************
 *
 *    free_ac_match_pos()
 *
 *************/

static
void free_ac_match_pos(Ac_match_pos p)
{
  free_mem(p, PTRS_AC_MATCH_POS);
  Ac_match_pos_frees++;
}  /* free_ac_match_pos */

/*************
 *
 *   Ac_match_free_vars_pos get_ac_match_free_vars_pos()
 *
 *************/

static
Ac_match_free_vars_pos get_ac_match_free_vars_pos(void)
{
  Ac_match_free_vars_pos p = get_cmem(PTRS_AC_MATCH_FREE_VARS_POS);
  Ac_match_free_vars_pos_gets++;
  return(p);
}  /* get_ac_match_free_vars_pos */

/*************
 *
 *    free_ac_match_free_vars_pos()
 *
 *************/

static
void free_ac_match_free_vars_pos(Ac_match_free_vars_pos p)
{
  free_mem(p, PTRS_AC_MATCH_FREE_VARS_POS);
  Ac_match_free_vars_pos_frees++;
}  /* free_ac_match_free_vars_pos */

/*************
 *
 *   Btm_state get_btm_state()
 *
 *************/

static
Btm_state get_btm_state(void)
{
  Btm_state p = get_cmem(PTRS_BTM_STATE);
  p->varnum = -1;
  p->alternative = NO_ALT;
  Btm_state_gets++;
  return(p);
}  /* get_btm_state */

/*************
 *
 *    free_btm_state()
 *
 *************/

static
void free_btm_state(Btm_state p)
{
  free_mem(p, PTRS_BTM_STATE);
  Btm_state_frees++;
}  /* free_btm_state */

/*************
 *
 *   fprint_btm_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the btm package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_btm_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct ac_match_pos);
  fprintf(fp, "ac_match_pos (%4d)%11u%11u%11u%9.1f K\n",
          n, Ac_match_pos_gets, Ac_match_pos_frees,
          Ac_match_pos_gets - Ac_match_pos_frees,
          ((Ac_match_pos_gets - Ac_match_pos_frees) * n) / 1024.);

  n = sizeof(struct ac_match_free_vars_pos);
  fprintf(fp, "ac_match_free_vars_pos (%4d)\n                    %11u%11u%11u%9.1f K\n",
          n, Ac_match_free_vars_pos_gets, Ac_match_free_vars_pos_frees,
          Ac_match_free_vars_pos_gets - Ac_match_free_vars_pos_frees,
          ((Ac_match_free_vars_pos_gets - Ac_match_free_vars_pos_frees) * n) / 1024.);

  n = sizeof(struct btm_state);
  fprintf(fp, "btm_state (%4d)    %11u%11u%11u%9.1f K\n",
          n, Btm_state_gets, Btm_state_frees,
          Btm_state_gets - Btm_state_frees,
          ((Btm_state_gets - Btm_state_frees) * n) / 1024.);

}  /* fprint_btm_mem */

/*************
 *
 *   p_btm_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the btm package.
*/

/* PUBLIC */
void p_btm_mem()
{
  fprint_btm_mem(stdout, TRUE);
}  /* p_btm_mem */

/*
 *  end of memory management
 */
/*************
 *
 *    flatten_mult
 *
 *    Flatten an AC term into an array, collapsing multiple occurrences
 *    into one, filling in a parallel array with multiplicities.
 *    Also return a count of the total number of arguments.
 *
 *    The index (*ip) must be initialized by the calling routine.
 *
 *************/

static
void flatten_mult(Term t, Term *a, int *m, int *ip, int *totp,
		  int (*comp_proc) (void *, void *))
{
  Term t1;
  int sn, i;

  sn = SYMNUM(t);
  for (i = 0; i < ARITY(t); i++) {
    t1 = ARG(t,i);
    if (SYMNUM(t1) == sn)
      flatten_mult(t1, a, m, ip, totp, comp_proc);
    else {
      (*totp)++;
      if (*ip > 0 && (*comp_proc)(t1, a[(*ip)-1]) == SAME_AS)
	m[(*ip)-1]++;
      else {
	if (*ip >= MAX_ACM_ARGS) {
	  fprint_term(stdout, t);
	  fatal_error("flatten_mult, too many arguments.");
	}
	a[*ip] = t1;
	m[*ip] = 1;
	(*ip)++;
      }
    }
  }
}  /* flatten_mult */

/*************
 *
 *    macbv_rec -- match (identically) all args of an AC term.
 *
 *    Called by match_ac_bound_vars.
 *
 *************/

static
int macbv_rec(int ac_sn, Term t, Term *args2, int *mults2,
	      int *match2, int n2, int *bound_matches, int *bp)
{
  int i, available;
    
  if (!COMPLEX(t) || SYMNUM(t) != ac_sn) {
    for (i = 0; i < n2; i++) {
      available = mults2[i] - match2[i];
      if (available > 0 && term_ident(t, args2[i])) {
	match2[i]++;
	bound_matches[(*bp)++] = i;
	return(1);
      }
    }
    return(0);
  }
  else {
    if (!macbv_rec(ac_sn,ARG(t,0),args2,mults2,match2,n2,
		   bound_matches,bp))
      return(0);
    else
      return(macbv_rec(ac_sn,ARG(t,1),args2,mults2,match2,n2,
		       bound_matches,bp));
  }
}  /* macbv_rec */

/*************
 *
 *    match_ac_bound_vars -- match (identically) a set of bound variables.
 *
 *    For each bound variable of args1, find an identical match in args2.
 *    If bound to an AC term t, (with same AC symbol) find an identical
 *    match for each argument of t.  Record the positions of the matched
 *    terms in `bound_matches', so that they can be unmached on backtracking.
 *
 *************/

static
int match_ac_bound_vars(int ac_sn, Term *args1, int n1,
			Term *args2, int *mults2, int *match2,
			int n2, int begin,
			int *bound_matches, int *bp, Context c1)
{
  int i, ok, vn;
  Term t;

  for (i=begin, ok=1, *bp=0; i < n1 && ok; i++) {
    vn = VARNUM(args1[i]);
    t = c1->terms[vn];
    if (t)
      ok = macbv_rec(ac_sn,t,args2,mults2,match2,n2,bound_matches,bp);
  }
  if (!ok) {
    /* Subtract any matches that were made before failure. */
    for (i = 0; i < *bp; i++)
      match2[bound_matches[i]] -= 1;
    *bp = 0;  /* Not really necessary, but helpful for debugging. */
  }
  return(ok);
}  /* match_ac_bound_vars */

/*************
 *
 *    set_up_free_vars
 *
 *    Build a list of the set of free variables in args1.  Each node
 *    contains the number of occurrences (coef) of the variable.
 *    Sort the list---nonincreasing coef.
 *
 *    Variables are partitioned into `free' and `bound' according to
 *    their state after all nonvariable terms have been matched.
 *    A variable is called `bound' iff it occurs in a nonvariable term.
 *   
 *    Since the partition does not change during backtracking, this
 *    routine needs to be called only once, after all nonvariable
 *    terms have been matched for the first time.
 *
 *************/

static
void set_up_free_vars(Ac_match_pos ac, Context c1)
{
  Ac_match_free_vars_pos p1, p2;
  Term t;
  int i, temp;

  ac->free_first = NULL; ac->free_last = NULL;
  for (i = ac->last_a1_symbol+1; i < ac->n1; i++) {
    t = ac->args1[i];
    if (c1->terms[VARNUM(t)] == NULL) {
      /* We have a free variable. */
      for (p1=ac->free_first; p1 && p1->varnum!=VARNUM(t); p1=p1->next);
      if (p1 != NULL)
	(p1->coef)++;
      else {
	p1 = get_ac_match_free_vars_pos();
	p1->varnum = VARNUM(t);
	p1->coef = 1;
	p1->next = NULL;
	p1->prev = ac->free_last;
	if (ac->free_last)
	  ac->free_last->next = p1;
	else
	  ac->free_first = p1;
	ac->free_last = p1;
      }
    }
  }
  /* Now sort -- nonincreasing coefficients. */
  /* There won't be many, so use a quadratic sort. */
  p1 = ac->free_first;
  if (p1) {
    while (p1->next) {
      for (p2 = p1->next; p2; p2 = p2->next) {
	if (p1->coef < p2->coef) {
	  temp = p2->coef;
	  p2->coef = p1->coef;
	  p1->coef = temp;
	  temp = p2->varnum;
	  p2->varnum = p1->varnum;
	  p1->varnum = temp;
	}
      }
      p1 = p1->next;
    }
  }
}  /* set_up_free_vars */

/*************
 *
 *    unbind_free_var
 *
 *    This routine takes an `ac match free variable position' and
 *    unbinds the free variable.  If the variable is bound to a
 *    compound AC term that was created just for the binding,
 *    then the new parts of the term are deleted.
 *
 *************/

static
void unbind_free_var(Ac_match_free_vars_pos pos, Context c)
{
  int i, j;
  Term t, t1;

  /* Free the temporary substitution term, if necessary. */

  /* First count how many nodes have to be deleted. */
  for (i = j = 0; i < pos->n; i++)
    if (pos->combo[i])
      j++;
    
  t = c->terms[pos->varnum];
  for (i = 0; i < j-1; i++) {
    t1 = ARG(t,1);
    free_term(t);
    t = t1;
  }
    
  /* unbind variable */
  c->terms[pos->varnum] = NULL;
    
}  /* unbind_free_var */

/*************
 *
 *  free_var_match
 *
 *  Find the first or next match for a free variable.  If (match_all)
 *  then all remaining arguments of args2 must be matched.
 *  Otherwise, backtracking will produce matches in all combinations.
 *
 *************/

static
int free_var_match(Ac_match_free_vars_pos pos, Term *args2,
		   int *mults2, int *match2, int n2, Context c1,
		   int symbol, int match_all)
{
  Term t;
  int i, j, k, n, ok, go, avail;

  t = c1->terms[pos->varnum];

  if (!t) {
    /* It is not a continuation, so set up everything.
     *
     * Loop through args2, collecting targets, combinations of which
     * can be substituted for the current variable.
     * Example: current variable is 2x; terms available for
     * matching are 4a, 3b, 2c, 1d; targets are a,a,b,c.
     */
  
  n = 0;
    for (i = 0; i < n2; i++) {
      avail = mults2[i] - match2[i];
      if (match_all && (avail % pos->coef != 0))
	return 0; /* Fail, because there will be unmatched term(s) */
      j = avail / pos->coef;  /* integer division */
      for (k = 0; k < j; k++)
	pos->targets[n++] = i;
    }

    pos->n = n;
    if (n == 0)
      return 0;
    else {
      for (i = 0; i < n; i++)
	pos->combo[i] = 1;
    }
  }
  else {
    /* continutation */
    unbind_free_var(pos, c1);

    /* unmark args2 terms */
    for (i = 0; i < pos->n; i++)
      if (pos->combo[i]) {
	match2[pos->targets[i]] -= pos->coef;
      }

    if (match_all) {
      for (i = 0; i < pos->n; i++)
	pos->combo[i] = 0;
      return 0;
    }
    else {
      go = 1;
      while (go) {
	/* subtract 1 from  combo */
	for (i = (pos->n)-1; i >= 0 && pos->combo[i] == 0; i--)
	  pos->combo[i] = 1;
	if (i < 0)
	  return(0);
	else {
	  pos->combo[i] = 0;
	  /* Check redundancy condition. */
	  for (i = 0, ok = 1; i < (pos->n)-1 && ok; i++)
	    if (pos->targets[i] == pos->targets[i+1] &&
		pos->combo[i] < pos->combo[i+1])
	      ok = 0;
	  go = !ok;
	}
      }

      /* Now make sure that combo is not empty. */
      for (i = 0, ok = 0; i < pos->n && !ok; i++)
	ok = pos->combo[i];
      if (!ok)
	return 0;
    }
  }

  /* All is well---we have a match for the current variable. */
  /* Build a temporary substitution term, if necessary. */
  /* Note order in which it is built---this makes it AC canonical. */

  t = NULL;
  for (i = pos->n-1; i >= 0; i--) 
    if (pos->combo[i]) {
      if (t == NULL)
	t = args2[pos->targets[i]];
      else
	t = build_binary_term(symbol, args2[pos->targets[i]], t);
    }

  /* Bind variable. */
  c1->terms[pos->varnum] = t;
    
  /* Mark args2 terms matched to the current variable. */
  for (i = 0; i < pos->n; i++)
    if (pos->combo[i])
      match2[pos->targets[i]] += pos->coef;

  return 1;
}  /* free_var_match */

/*************
 *
 *    build_partial_term
 *
 *    When partial match has been found, this routine collects the
 *    unmatched arguments of args2 and builds and returns an AC term.
 *    The size of the new term is stored in the AC position so that
 *    it can easily be freed.
 *
 *************/

static
Term build_partial_term(Ac_match_pos ac)
{
  int i, j, k, n;
  Term t;

  t = NULL; k = 0;
  for (i = 0; i < ac->n2; i++) {
    n = ac->mults2[i] - ac->match2[i];
    for (j = 0; j < n; j++) {
      k++;
      if (!t)
	t = ac->args2[i];
      else
	t = build_binary_term(SYMNUM(ac->t1), ac->args2[i], t);
    }
  }
  ac->partial_term_size = k;
  return t;
}  /* build_partial_term */

/*************
 *
 *    clear_partial_term
 *
 *    Remove the partial term from the substitution and free the
 *    appropriate parts fo the partial term.
 *
 *************/

static
void clear_partial_term(Ac_match_pos ac)
{
  int i;
  Term t, t1;

  t = ac->c1->partial_term;
  ac->c1->partial_term = NULL;
    
  for (i = 0; i < ac->partial_term_size - 1; i++) {
    t1 = ARG(t,1);
    free_term(t);
    t = t1;
  }
  ac->partial_term_size = 0;
}  /* clear_partial_term */

#define GO_FUNCTORS   1
#define GO_BOUND_VARS 2
#define GO_FREE_VARS  3
#define SUCCESS       4
#define FAILURE       5

/*************
 *
 *    match_ac -- associative-commutative matching.
 *
 *    Get the first (bt->alternative == NO_ALT) or next AC matcher.
 *    I intend for this to be called from `match_bt_guts'.  
 *    It assumed that the root symbols of the input terms are AC. 
 *
 *    Call match_ac_cancel(ac) if you quit before getting all matchers. 
 *
 *    t1 -- pattern term
 *    c1 -- context (substitution table) for t1
 *    t2 -- subject term
 *    bt -- backtrack position
 *
 *************/

static
int match_ac(Term t1, Context c1, Term t2, Btm_state bt)
{
  int status, n1, n2, total2, i, ok, a1_pos, a2_pos;
  int free_var_forward;
  Term a1, a2;
  Ac_match_pos ac;
  Ac_match_free_vars_pos free_pos, p1, p2;
  Btm_state bt1 = NULL;

  a1_pos = a2_pos = free_var_forward = 0;  /* to quiet compiler */
  free_pos = NULL;  /* to quiet compiler */

  if (bt->alternative == NO_ALT) {  /* initialize, get first matcher */
    ac = get_ac_match_pos();
    bt->acm = ac;
    ac->t1 = t1; ac->t2 = t2; ac->c1 = c1;
    ac->free_first = NULL; ac->partial_term_size = 0;
    n1 = 0; n2 = 0; total2 = 0; 
    flatten(t1, ac->args1, &n1);
    flatten_mult(t2, ac->args2, ac->mults2, &n2, &total2,
		 (int (*)(void*,void*)) term_compare_ncv);
    if (n1 > total2)  /* fail if t1 has more arguments */
      status = FAILURE;
    else {
      /* Assume inputs are ac_canonical, so don't sort.       */
      /* Don't bother to eliminate common arguments, because  */
      /* It usually doesn't pay off.                          */
      ac->n1 = n1; ac->n2 = n2;
      for (i = 0; i < n1; i++)
	ac->match1[i] = -1;
      for (i = 0; i < n2; i++)
	ac->match2[i] = 0;
      for (i = 0; i < n1 && !VARIABLE(ac->args1[i]); i++);
      ac->last_a1_symbol = i-1;
      a1_pos = 0; a2_pos = 0; bt1 = NULL;
      status = GO_FUNCTORS;
    }
  }
  else {  /* continuation, get next matcher */
    ac = bt->acm;
    if (bt->partial) {
      printf("WARNING: partial match_ac on continuation.\n");
      if (c1->partial_term)
	clear_partial_term(ac);
    }
    n1 = ac->n1; n2 = ac->n2;
    if (n1 == 0 && n2 == 0)  /* vacuous success last time */
      status = FAILURE;
    else {
      free_pos = ac->free_last;
      free_var_forward = 0;
      status = GO_FREE_VARS;
    }
  }
    
  while (status != SUCCESS && status != FAILURE) {
    while (status == GO_FUNCTORS) {
      if (a1_pos > ac->last_a1_symbol)
	status = GO_BOUND_VARS;
      else if (a1_pos < 0)
	status = FAILURE;
      else {
	if (bt1) {
	  /* remove arrow */
	  ac->match1[a1_pos] = -1;
	  ac->bt1[a1_pos] = NULL;
	  ac->match2[a2_pos]--;
	  /* Try for another match with this pair. */
	  bt1 = match_bt_next(bt1);
	  if (!bt1)
	    a2_pos++;
	}

	if (!bt1) {
	  /* Look for a match for a1, starting with a2. */
	  a1 = ac->args1[a1_pos];
	  while (bt1 == NULL && a2_pos < ac->n2) {
	    a2 = ac->args2[a2_pos];
	    if (SYMNUM(a1) == SYMNUM(a2) &&
		ac->match2[a2_pos] < ac->mults2[a2_pos])
	      bt1 = match_bt_first(a1, c1, a2, 0);
	    if (bt1 == NULL)
	      a2_pos++;
	  }
	}

	if (bt1) {   /* We have a match: a1->a2. */
	  /* draw arrow */
	  ac->match1[a1_pos] = a2_pos;
	  ac->bt1[a1_pos] = bt1;
	  ac->match2[a2_pos]++;
	  a1_pos++; a2_pos = 0; bt1 = NULL;
	}
	else {  /* back up */
	  a1_pos--;
	  a2_pos = ac->match1[a1_pos];
	  bt1 = ac->bt1[a1_pos];
	}
      }
    }  /* while GO_FUNCTORS */
	
    if (status == GO_BOUND_VARS) {
      /* Try to macth (identically) bound variables. */
      ok = match_ac_bound_vars(SYMNUM(t1), ac->args1, n1,
			       ac->args2, ac->mults2, ac->match2, n2,
			       ac->last_a1_symbol+1, ac->bound_matches,
			       &(ac->bound_count), c1);
      if (ok) {
	free_pos = ac->free_first;
	free_var_forward = 1;
	status = GO_FREE_VARS;
      }
      else {  /* backup */
	a1_pos = ac->last_a1_symbol;
	if (a1_pos >= 0) {
	  a2_pos = ac->match1[a1_pos];
	  bt1 = ac->bt1[a1_pos];
	}
	status = GO_FUNCTORS;
      }
    }

    else if (status == GO_FREE_VARS) {
      if (ac->free_first == NULL) {
	set_up_free_vars(ac, c1);
	free_pos = ac->free_first;
      }
      while (free_pos) {
	if (free_var_match(free_pos, ac->args2, ac->mults2,
			   ac->match2, ac->n2, c1, SYMNUM(ac->t1),
			   !bt->partial && free_pos->next == NULL)) {
	  free_pos = free_pos->next;
	  free_var_forward = 1;
	}
	else {
	  free_pos = free_pos->prev;
	  free_var_forward = 0;
	}
      }
      if (free_var_forward) {
	/* Check for non-matched a2 terms. */
	for (i = 0, ok = 1; i < n2 && ok; i++)
	  ok = ac->mults2[i] == ac->match2[i];
	if (!ok) {
	  /*  Have at least 1 non-matched a2 term. */
	  if (bt->partial) {
	    c1->partial_term = build_partial_term(ac);
	    status = SUCCESS;
	  }
	  else
	    status = GO_FUNCTORS;  /* set up below */
	}
	else
	  status = SUCCESS;
      }
      else
	status = GO_FUNCTORS;

      if (status == GO_FUNCTORS) {
	/* Unmark bound variable matches. */
	for (i = 0; i < ac->bound_count; i++)
	  ac->match2[ac->bound_matches[i]] -= 1;
	a1_pos = ac->last_a1_symbol;
	if (a1_pos >= 0) {
	  a2_pos = ac->match1[a1_pos];
	  bt1 = ac->bt1[a1_pos];
	}
      }
    }  /* if GO_FREE_VARS */
  }  /* while !SUCCESS && !FAILURE */

  if (status == SUCCESS)
    bt->alternative = AC_ALT;
  else {
    /* free memory */
    p1 = ac->free_first;
    while (p1) {
      p2 = p1;
      p1 = p1->next;
      free_ac_match_free_vars_pos(p2);
    }
    free_ac_match_pos(ac);
    bt->alternative = NO_ALT;
  }
  return(status == SUCCESS);
}    /* match_ac */

/*************
 *
 *    match_ac_cancel
 *
 *    Free an AC match position.  This is to be used when you have obtained
 *    one or more AC matchers by calling match_ac, but you do not wish
 *    to backtrack to obtain additional AC matchers.  Do not call this
 *    routine if match_ac returned 0.
 *
 *************/

static
void match_ac_cancel(Ac_match_pos ac)
{
  Ac_match_free_vars_pos p1, p2;
  int i;
    
  for (i = 0; i <= ac->last_a1_symbol; i++)
    match_bt_cancel(ac->bt1[i]);
  p1 = ac->free_first;
  while (p1) {
    unbind_free_var(p1, ac->c1);
    p2 = p1;
    p1 = p1->next;
    free_ac_match_free_vars_pos(p2);
  }
  if (ac->partial_term_size > 0)
    clear_partial_term(ac);
  free_ac_match_pos(ac);
}  /* match_ac_cancel */

/*************
 *
 *    Btm_state match_bt_backup(bt)
 *
 *    Back up (freeing nodes) to the most recent node with an alternative.
 *
 *************/

static
Btm_state match_bt_backup(Btm_state bt1)
{
  Btm_state bt2, bt3;

  while (bt1 != NULL && bt1->alternative == NO_ALT) {

    if (bt1->cb) {  /* unbind variable */
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
    }
	
    if (bt1->prev) {
      bt1 = bt1->prev;
      while (bt1->last_child)
	bt1 = bt1->last_child;
    }
    else {
      bt2 = bt1;
      bt1 = bt1->parent;

      while (bt2) {
	bt3 = bt2;
	bt2 = bt2->next;
	free_btm_state(bt3);
      }

      if (bt1)
	bt1->first_child = bt1->last_child = NULL;
    }
  }
    
  return(bt1);
	
}  /* match_bt_backup */

/*************
 *
 *    match_bt_guts
 *
 *    Main loop for backtracking matching.
 *
 *************/

static
Btm_state match_bt_guts(Btm_state bt1)
{
  Term t1, t2;
  Context c1;
  int vn1, status;
  Btm_state bt2, bt3;

  status = GO;

  while (status == GO) {

    t1 = bt1->t1;
    t2 = bt1->t2;
    c1 = bt1->c1;

    if (bt1->alternative == COMM_ALT) {
      if (match_commute(t1, c1, t2, bt1))
	status = POP;
      else
	status = BACKTRACK;
    }
    else if (bt1->alternative == AC_ALT) {
      if (match_ac(t1, c1, t2, bt1))
	status = POP;
      else
	status = BACKTRACK;
    }
    else if (VARIABLE(t1)) {
      vn1 = VARNUM(t1);
      if (c1->terms[vn1]) {
	if (term_ident(c1->terms[vn1], t2))
	  status = POP;
	else
	  status = BACKTRACK;
      }
      else {
	BIND_BT(vn1, c1, t2, NULL, bt1)
	  status = POP;
      }
    }
	
    else if (VARIABLE(t2))
      status = BACKTRACK;

    else if (SYMNUM(t1) != SYMNUM(t2))
      status = BACKTRACK;
	
    else if (CONSTANT(t1))
      status = POP;
	
    else {  /* both COMPLEX with same symbol (and same arity) */
      int arity = ARITY(t1);

      if (arity == 2 && is_commutative(SYMNUM(t1))) {
	if (match_commute(t1, c1, t2, bt1))
	  status = POP;
	else
	  status = BACKTRACK;
      }
      else if (arity == 2 && is_assoc_comm(SYMNUM(t1))) {
	if (match_ac(t1, c1, t2, bt1))
	  status = POP;
	else
	  status = BACKTRACK;
      }
      else {
	int i;
	/* Set up children corresponding to args of <t1,t2>. */
	/* Order not important for correctness. */
	/* AC kids last for efficiency, but keep in order otherwise. */
	bt3 = NULL;
	for (i = 0; i < arity; i++) {

	  bt2 = get_btm_state();
	  bt2->t1 = ARG(t1,i);
	  bt2->t2 = ARG(t2,i);
	  bt2->c1 = c1;
	  bt2->parent = bt1;

	  if (is_assoc_comm(SYMNUM(bt2->t1))) {
	    /* insert at end */
	    bt2->prev = bt1->last_child;
	    if (bt1->last_child)
	      bt1->last_child->next = bt2;
	    else
	      bt1->first_child = bt2;
	    bt1->last_child = bt2;
	  }
	  else {
	    if (bt3) {
	      /* insert after bt3 */
	      bt2->next = bt3->next;
	      bt2->prev = bt3;
	      bt3->next = bt2;
	      if (bt2->next)
		bt2->next->prev = bt2;
	      else
		bt1->last_child = bt2;
	    }
	    else {
	      /* insert at beginning */
	      bt2->next = bt1->first_child;
	      if (bt2->next)
		bt2->next->prev = bt2;
	      else
		bt1->last_child = bt2;
	      bt1->first_child = bt2;
	    }
	    bt3 = bt2;
	  }
	}

	bt1 = bt1->first_child;
	status = GO;
      }
    }
	
    if (status == POP) {
      while (!bt1->next && bt1->parent)
	bt1 = bt1->parent;
      if (!bt1->next)
	status = SUCCESS;
      else {
	bt1 = bt1->next;
	status = GO;
      }
    }
    else if (status == BACKTRACK) {
      bt1 = match_bt_backup(bt1);
      if (bt1)
	status = GO;
      else
	status = FAILURE;
    }
  }
  return(bt1);
}  /* match_bt_guts */

/*************
 *
 *  match_commute()
 *
 *  There is nothing fancy here.  We simply try to unify both ways.
 *  We can get redundant matches if both ways match, for example,
 *  f(a,x) and f(a,a) match twice, both times with the same substitution.
 *
 *************/

static
int match_commute(Term t1, Context c1, Term t2, Btm_state bt)
{
  Btm_state bt1, bt2;

  if (bt->alternative == NO_ALT) {  /* first call */
    bt->alternative = COMM_ALT;
    bt->flipped = 0;

    /* Set up 2 subproblems, then match guts. */

    bt1 = get_btm_state();  bt2 = get_btm_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1;
    bt2->c1 = c1;
    bt1->t1 = ARG(t1,0); bt1->t2 = ARG(t2,0);
    bt2->t1 = ARG(t1,1); bt2->t2 = ARG(t2,1);

    bt->position_bt = match_bt_guts(bt1);
  }
  else  /* continuation */
    bt->position_bt = match_bt_next(bt->position_bt);

  if (!bt->position_bt && !bt->flipped) {

    /* Set up 2 subproblems, with t2 flipped, then match guts. */

    bt1 = get_btm_state();  bt2 = get_btm_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1;
    bt2->c1 = c1;
    bt1->t1=ARG(t1,0); bt1->t2=ARG(t2,1);
    bt2->t1=ARG(t1,1); bt2->t2=ARG(t2,0);

    bt->flipped = 1;
    bt->position_bt = match_bt_guts(bt1);
  }

  if (bt->position_bt)
    return(1);
  else {
    bt->alternative = NO_ALT;
    return(0);
  }
    
}  /* match_commute */

/*************
 *
 *    p_acm -- print an ac match position.
 *
 *************/

static
void p_acm(Ac_match_pos ac)
{
  int i;
  Ac_match_free_vars_pos p;
  
  printf("Ac_match_pos %p.\n", ac);
  printf("t1: "); p_term(ac->t1);
  printf("t2: "); p_term(ac->t2);
  for (i = 0; i < ac->n1; i++) {
    fprint_term(stdout, ac->args1[i]);
    printf(" %d ",ac->match1[i]);
  }
  printf("\n");
  for (i = 0; i < ac->n2; i++) {
    fprint_term(stdout, ac->args2[i]);
    printf(" <%d,%d> ",ac->mults2[i],ac->match2[i]);
  }
  printf("\n");

  printf("last_a1_symbol=%d.\n",ac->last_a1_symbol);
  printf("free vars list <symbol,coef>:\n");
  for (p = ac->free_first; p; p = p->next) {
    printf("<%d,%d>, ", p->varnum, p->coef);
    for (i = 0; i < p->n; i++) {
      fprint_term(stdout,ac->args2[p->targets[i]]);
      printf(":%d ",p->combo[i]);
    }
    printf("\n");
  }
  printf("\n");
}  /* p_acm */

/*************
 *
 *   p_btm_state()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) a Btm_state.  It is not pretty.
*/

/* PUBLIC */
void p_btm_state(Btm_state bt)
{
  printf("\nBtm node %p.\n", bt);
  printf("t1: "); p_term(bt->t1);
  printf("t2: "); p_term(bt->t2);
  printf("c1: "); p_context(bt->c1);
  printf("varnum: %d\n", bt->varnum);
  printf("cb: "); p_context(bt->c1);
  printf("alternative: %d\n", bt->alternative);
  printf("flipped: %d\n", bt->alternative);
  printf("position_bt: %p\n", bt->position_bt);
  printf("partial: %d\n", bt->partial);
  p_acm(bt->acm);
}  /* p_btm_state */

/*************
 *
 *    match_bt_first
 *
 *
 *************/

/* DOCUMENTATION
This is backtracking matching, to be used when there
can be more than one unifier.  This version handles (any number of)
commutative (C) and associative-commutative (AC) function symbols.
<P>
The flag `partial' says that if the top level is AC, then
not all arguments of t2 have to be matched.  The non-matched
args are put in c1->partial_term.  Partial matches are allowed
for the top level only.  This is useful for AC rewriting.
<P>
If any AC terms are in t1 or t2, then both t1 and t2 should be
in `ac_canonical()' form before the call, that is, AC terms are
right associated and sorted.  C terms are in t1 or t2
need not be c_canonical.  (Commutatvie matching is primitive,
and you can get duplicate unifiers.)
<P>
Get first matching substitution.  Return position for match_bt_next()
calls.  Here is an example.  Assume we have terms t1 and t2.
<PRE>
  {
    Context c1 = get_context();
    Btm_state bt = match_bt_first(t1, c1, t2, 0);
    while (bt != NULL) {
        Term t3 = apply(t1, c1);
        zap_term(t3);
        bt = match_bt_next(bt);
        }
    free_context(c1);
  }
</PRE>
If you quit before NULL is returned, call match_bt_cancel(bt)
to clear substitutions and free memory.
*/

/* PUBLIC */
Btm_state match_bt_first(Term t1, Context c1, Term t2, int partial)
{
  Btm_state bt;

  bt = get_btm_state();
  bt->t1 = t1; bt->t2 = t2; bt->c1 = c1;
  bt->partial = partial;
  return(match_bt_guts(bt));

}  /* match_bt */

/*************
 *
 *    match_bt_next -- see match_bt_first
 *
 *    Get next unifier.  Return position for subsequent calls.
 *
 *************/

/* DOCUMENTATION
This routine gets the next matching substitution.
See match_bt_first() for details.
*/

/* PUBLIC */
Btm_state match_bt_next(Btm_state bt1)
{
  /* Go to last node in tree, then back up to a node with an alternative. */

  while (bt1->next)
    bt1 = bt1->next;
  while (bt1->last_child)
    bt1 = bt1->last_child;

  bt1 = match_bt_backup(bt1);

  if (bt1)
    return(match_bt_guts(bt1));
  else
    return NULL;
}  /* match_bt_next */

/*************
 *
 *    match_bt_cancel
 *
 *    This routine should be called if the rest of a sequence of
 *    unifiers is not called for.  It clears substitutions and
 *    frees memory.
 *
 *************/

/* DOCUMENTATION
This routine clears any substitution and frees memory associated
with a backtrack matching state.  This should be called if you
get some, but not all, matching substitutions in backtrack matching.
See match_bt_first().
*/

/* PUBLIC */
void match_bt_cancel(Btm_state bt)
{
  Btm_state bt1, bt2;

  for (bt1 = bt; bt1 != NULL; ) {

    match_bt_cancel(bt1->first_child);
	
    if (bt1->alternative == COMM_ALT)
      /* match_bt_guts leaves us at the second child. */
      match_bt_cancel(bt1->position_bt->prev);
    else if (bt1->alternative == AC_ALT) {
      match_ac_cancel(bt1->acm);
    }
    else if (bt1->cb) {
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
    }
    bt2 = bt1;
    bt1 = bt1->next;
    free_btm_state(bt2);
  }
}  /* match_bt_cancel */

