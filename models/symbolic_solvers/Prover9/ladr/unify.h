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

#ifndef TP_UNIFY_H
#define TP_UNIFY_H

#include "listterm.h"
#include "termflag.h"

/* INTRODUCTION
This package handles ordinary <GL>unification</GL> and <GL>matching</GL>
of terms.
The methods are probably different from what you learned in your
logic course, so pay close attention.
<P>
In situations where you consider instantiating the variables of a term
t, you will have an associated Context c.  The Context
keeps track of the terms that are substituted for the variables---
think of it as a substitution table indexed by the variable number.
<P>
Contexts allow you to separate variables of two terms without
creating a renamed copy of one of the terms.  Say we have
two terms with different Contexts: (t1,c1) and (t2,c2).
If t1 and t2 share a variable, say v3, occurrences in
t1 are a different variable from occurrences in t2, because
the contexts are different.
Think of the those variables as (v3,c1) and (v3,c2).
In fact, when an instantiation is recorded in a Context,
the variable in question is set to a pair, say (t4,c4) rather
than just t4, because we have to know how to interpret the
variables of t4.
<P>
There are situations where the terms being unified really do
share variables, for example when factoring the literals of
a clause; in those cases, you simply use the same context
for both terms.
<P>
When you call unify(), match(), or any other routine that
tries to make terms identical by instantiation, the terms you
give are not changed---all of the action happens in their contexts
(and in the trail, see below).
So you do not have to copy terms before calling the routine.
<P>
When a unify or match routine succeeds, the Contexts are updated
to reflect the common instance of the terms.  Also, a Trail is
returned.  A Trail is a linked list of (variable,context)
pairs telling exactly which variables were instantiated during
the operation.  Its purpose is to quickly restore the Contexts to 
their previous states.
<P>
You must explicitly allocate and free Contexts.
To save time, we don't initialize the arrays each
time a Context is re-allocated, and we don't check that Contexts
are clear when they are freed.  Therefore, <I>you must make
sure that a Context is clear before freeing it</I>.
See undo_subst().  Also, if you forget to clear the Context with
undo_subst(), you will have a memory leak, because the Trail will be
lost.
(If you suspect that you have a bug which causes a non-empty
context to be freed, you can enable a run-time check in free_context()
and recompile unify.c.)
<P>
When you wish to find out what a context does to a term t, you can
call apply(), which builds a new copy of the term with all of
the instantiations of the context applied to the term t.
But I wrote above that (v3,c1) is a different variable 
from (v3,c2)---what does apply do with uninstantiated variables?
Each context has a unique multiplier (a small natural number);
When apply() gets to an uninstantiated variable v, it returns
a variable with index (multiplier*MAX_VARS)+VARNUM(v).
Of course, this can give you VARNUMs > MAX_VARS, so you may
have to rename variables of the result before calling a unification
routine on the result.
<P>
Unification and matching can be used incrementally.  For example,
you can all unify() with a context which has entries from a
previous call to unify().  Hyperresolution can be implemented by
backtracking through the negative literals of a nucleus
and the satellites that unify with a given literal of the nucleus,
constructing and undoing partial substitutions along the way.
Another example is subsumption.
Checking whether one clause subsumes another can be done by
incremental matching, backtracking through the literals of the
potential subsumer, trying to map them to the literals of the
other clause.
<P>
Associative-commutative unification and matching, and commutative
unification and matching, use different unification code, because they
have to deal with multiple unifiers for a pair of terms.  (These other
kinds of unification and matching may use the Context data
type defined here.)
*/

/* Public definitions */

/* Dereference a variable. */

#define DEREFERENCE(t, c) { int i; \
    while (c!=NULL && VARIABLE(t) && c->terms[i=VARNUM(t)]) \
    { t = c->terms[i]; c = c->contexts[i]; } } 

/* A Context records a substitution of terms for variables. */

typedef struct context * Context;

struct context {
  Term    terms[MAX_VARS];    /* terms substituted for variables */
  Context contexts[MAX_VARS]; /* Contexts corresponding to terms */
  int     multiplier;         /* for getting separate vars in apply */
  Term    partial_term;       /* for AC matching */
};

typedef struct trail * Trail;

/* The following type is for backtrack unification and matching. */

typedef enum { NO_ALT = 0,
	       AC_ALT,
	       COMM_ALT
             } Unif_alternative;

/* End of public definitions */

/* Public function prototypes from unify.c */

Context get_context(void);

void free_context(Context p);

void fprint_unify_mem(FILE *fp, BOOL heading);

void p_unify_mem();

BOOL unify(Term t1, Context c1,
	   Term t2, Context c2, Trail *trp);

BOOL variant(Term t1, Context c1,
	    Term t2, Trail *trp);

BOOL occur_check(int vn, Context vc, Term t, Context c);

BOOL match(Term t1, Context c1, Term t2, Trail *trp);

Term apply(Term t, Context c);

Term apply_substitute(Term t, Term beta, Context c_from,
		      Term into_term, Context c_into);

Term apply_substitute2(Term t, Term beta, Context c_from,
		       Ilist into_pos, Context c_into);

Term apply_demod(Term t, Context c, int flag);

void undo_subst(Trail tr);

void undo_subst_2(Trail tr, Trail sub_tr);

void fprint_context(FILE *fp, Context c);

void p_context(Context c);

void fprint_trail(FILE *fp, Trail t);

void p_trail(Trail t);

BOOL match_weight(Term t1, Context c1, Term t2, Trail *trp, int var_sn);

Ilist vars_in_trail(Trail tr);

Plist context_to_pairs(Ilist varnums, Context c);

BOOL empty_substitution(Context s);

BOOL variable_substitution(Context s);

BOOL subst_changes_term(Term t, Context c);

#endif  /* conditional compilation of whole file */
