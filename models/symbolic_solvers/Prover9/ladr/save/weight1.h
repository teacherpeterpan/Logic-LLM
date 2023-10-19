#ifndef TP_WEIGHT1_H
#define TP_WEIGHT1_H

#include "clause.h"

/* INTRODUCTION
This is a simple weighting package.  Each symbol has
a weight (default 1).  The weight of a term is the
sum of the weights of the subterms plus the weight
of the root symbol.  (There are no multipliers.)
In this scheme, the negation symbol on literals is
treated like any other symbol.  The weight of a clause
is the sum of the weights of the literals.  (The OR
symbols in clauses don't count.)  Variables always have
weight 1.
<P>
You give init_weight_scheme a list of weight assignments,
for example,
<PRE>
weight(a, -5).
weight(g(x), 0).
weight(~x, -3).   % This is how to assign a weight to the negation symbol.
</PRE>
For non-constants, you have to include arguments so that the
arity is known; those arguments are ignored, and the convention is to use x.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from weight1.c */

void init_weight1(Plist p, int constant_weight, int variable_weight);

int term_weight1(Term t);

int clause_weight1(Clause c);

#endif  /* conditional compilation of whole file */
