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

#include "../ladr/top_input.h"

#define PROGRAM_NAME    "olfilter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a stream of meet/join/complement/0/1/sheffer\n"
"equations (from stdin) and writes (to stdout) those that are\n"
"ortholattice (OL) identities.  Bruns's procedure is used.  An optional\n"
"argument 'fast' says to read and write the clauses in fastparse form\n"
"(e.g., =mxxx.).  The base terms can be either constants or variables.\n\n"
"Example OL identities (f is Sheffer stroke).\n\n"
"  ordinary:  'c(x ^ 0) = f(x,c(x v x)).'\n"
"  fastparse: '=cmx0fxcmxx.' (exactly one per line, without spaces)\n\n"
"Another optional argument 'x' says to output the equations that are\n"
"*not* OL identities.\n\n"
;

/*
 *  Take a stream of equations, and for each, decide if it is an
 *  ortholattice identity.  Use the procedure outlined in
 *
 *      Bruns, Gunter.  Free ortholattices.
 *      Canad. J. Math. 28 (1976), no. 5, 977--985.
 *
 *  I'm not sure this is correct, because there are few things
 *  about the paper I don't understand.
 */

/* Cache the important symbol numbers to avoid symbol table lookups. */

int Meet_sym;
int Join_sym;
int Comp_sym;
int Zero_sym;
int One_sym;
int Sheffer_sym;

#define MEET_TERM(t)        (SYMNUM(t) == Meet_sym)
#define JOIN_TERM(t)        (SYMNUM(t) == Join_sym)
#define COMPLEMENT_TERM(t)  (SYMNUM(t) == Comp_sym)
#define ZERO_TERM(t)        (SYMNUM(t) == Zero_sym)
#define ONE_TERM(t)         (SYMNUM(t) == One_sym)
#define SHEFFER_TERM(t)     (SYMNUM(t) == Sheffer_sym)

/*************
 *
 *   complement() - complement a term.
 *
 *   This is a destructive operation.  That is, if you call it as
 *   a = complement(b), then you should never again refer to b.
 *   So a good way to call it is b = complement(b).
 *
 *************/

static Term complement(Term t)
{
  return build_unary_term(Comp_sym, t);
}  /* complement */

/*************
 *
 *   neg_norm_form(t) - destructively transform t.
 *
 *   Negation normal form (NNF).
 *
 *   Apply the following rules as much as possible (sound for OL).
 *     c(x ^ y) -> c(x) v c(y)
 *     c(x v y) -> c(x) ^ c(y)
 *     c(c(x)) -> x
 *     c(0) -> 1
 *     c(1) -> 0
 *
 *   The Bruns paper doesn't say anything about this, but parts
 *   of it seem to assume that all complements are applied to
 *   simple terms.  So we'll use this to make it so.
 *
 *************/

static Term neg_norm_form(Term t)
{
  if (VARIABLE(t) || (CONSTANT(t)))
    return t;

  else if (JOIN_TERM(t) || MEET_TERM(t)) {
    ARG(t,0) = neg_norm_form(ARG(t,0));
    ARG(t,1) = neg_norm_form(ARG(t,1));
    return t;
  }

  else if (COMPLEMENT_TERM(t)) {
    Term s = ARG(t,0);

    if (ZERO_TERM(s) || ONE_TERM(s)) {
      zap_term(t);
      return get_rigid_term_dangerously(ZERO_TERM(s) ? One_sym : Zero_sym, 0);
    }
    else if (VARIABLE(s) || CONSTANT(s))
      return t;
    else if (COMPLEMENT_TERM(s)) {
      Term a = ARG(s,0);
      free_term(t);
      free_term(s);
      return neg_norm_form(a);
    }
    else if (MEET_TERM(s) || JOIN_TERM(s)) {
      int dual_sym = MEET_TERM(s) ? Join_sym : Meet_sym;
      Term a0 = ARG(s,0);
      Term a1 = ARG(s,1);
      free_term(t);
      free_term(s);
      return build_binary_term(dual_sym,
			       neg_norm_form(build_unary_term(Comp_sym, a0)),
			       neg_norm_form(build_unary_term(Comp_sym, a1)));
    }
    else {
      fatal_error("neg_norm_form: bad term");
      return NULL;
    }
  }
  else {
    fatal_error("neg_norm_form: bad term");
    return NULL;
  }
}  /* neg_norm_form */

/*************
 *
 *   simplify_01(t) - destructively transform t.
 *
 *   Get rid of 0 and 1 by the ordinary rules 
 *   (unless, of course, the top is 0 or 1).
 *
 *************/

static Term simplify_01(Term t)
{
  if (VARIABLE(t) || (CONSTANT(t)))
    return t;

  else if (COMPLEMENT_TERM(t)) {
    Term s0;
    ARG(t, 0) = simplify_01(ARG(t,0));
    s0 = ARG(t, 0);
    if (ONE_TERM(s0)) {
      zap_term(t);
      return get_rigid_term_dangerously(Zero_sym, 0);
    }
    else if (ZERO_TERM(s0)) {
      zap_term(t);
      return get_rigid_term_dangerously(One_sym, 0);
    }
    else
      return t;
  }

  else if (JOIN_TERM(t) || MEET_TERM(t)) {
    Term s0, s1;
    ARG(t, 0) = simplify_01(ARG(t,0));
    ARG(t, 1) = simplify_01(ARG(t,1));
    s0 = ARG(t, 0);
    s1 = ARG(t, 1);

    if (MEET_TERM(t) && (ZERO_TERM(s0) || ZERO_TERM(s1))) {
      zap_term(t);
      return get_rigid_term_dangerously(Zero_sym, 0);
    }
    else if (JOIN_TERM(t) && (ONE_TERM(s0) || ONE_TERM(s1))) {
      zap_term(t);
      return get_rigid_term_dangerously(One_sym, 0);
    }
    else if ((JOIN_TERM(t) && ZERO_TERM(s0)) ||
	     (MEET_TERM(t) && ONE_TERM(s0))) {
      free_term(t);  /* frees top node only */
      zap_term(s0);  /* frees entire term */
      return s1;
    }
    else if ((JOIN_TERM(t) && ZERO_TERM(s1)) ||
	     (MEET_TERM(t) && ONE_TERM(s1))) {
      free_term(t);
      zap_term(s1);
      return s0;
    }
    else
      return t;
  }
  else
    return t;
}  /* simplify_01 */

/*************
 *
 *   ol_leq()
 *
 *************/

/* DOCUMENTATION
Given OL terms S and T, which have already been preprocessed
by the beta() operation, this routine checks if S <= T.
It is assumed that S and T are in terms of operations
\{meet,join,complement,0,1\}.
<P>
<P>
This is an extension of Whitman's procedure for lattice theory, and
it should work also as a decision procedure for LT {meet,join} terms.
<P>
Solutions to subproblems are not cached, so the behavior of
this implementation can be exponential.
*/

BOOL ol_leq(Term s, Term t)
{
  BOOL result;

  if (VARIABLE(s) && (VARIABLE(t)))
    result = (VARNUM(s) == VARNUM(t));

  else if (ZERO_TERM(s))
    result = TRUE;
  else if (ONE_TERM(t))
    result = TRUE;
#if 0
  else if (ZERO_TERM(t))
    result = FALSE;
  else if (ONE_TERM(s))
    result = FALSE;
#endif

  else if (CONSTANT(s) && (CONSTANT(t)))
    result = (SYMNUM(s) == SYMNUM(t));

  else if (JOIN_TERM(s))
    result = (ol_leq(ARG(s,0), t) &&
	      ol_leq(ARG(s,1), t));

  else if (MEET_TERM(t))
    result = (ol_leq(s, ARG(t,0)) &&
	      ol_leq(s, ARG(t,1)));
					 
  else if (MEET_TERM(s) && JOIN_TERM(t))
    result = (ol_leq(s, ARG(t,0)) ||
	      ol_leq(s, ARG(t,1)) ||
	      ol_leq(ARG(s,0), t) ||
	      ol_leq(ARG(s,1), t));

  else if (JOIN_TERM(t))
    result = (ol_leq(s, ARG(t,0)) ||
	      ol_leq(s, ARG(t,1)));

  else if (MEET_TERM(s))
    result = (ol_leq(ARG(s,0), t) ||
	      ol_leq(ARG(s,1), t));

  else if (COMPLEMENT_TERM(s) && COMPLEMENT_TERM(t))
    result = term_ident(ARG(s,0), ARG(t,0));

  else
    result = FALSE;

#if 0
  printf("ol_leq %d: ", result); fwrite_term(stdout, s);
  printf("   ---   "); fwrite_term_nl(stdout, t);
#endif

  return result;
}  /* ol_leq */

/*************
 *
 *   reduced_join(t)
 *
 *************/

static BOOL reduced_join(Term a, Term t)
{
  Term ca = neg_norm_form(complement(copy_term(a)));
  BOOL ok = !ol_leq(ca, t);
  zap_term(ca);
  return ok;
}  /* reduced_join */

/*************
 *
 *   reduced_meet(t)
 *
 *************/

static BOOL reduced_meet(Term a, Term t)
{
  Term ca = neg_norm_form(complement(copy_term(a)));
  BOOL ok = !ol_leq(t, ca);
  zap_term(ca);
  return ok;
}  /* reduced_meet */

/*************
 *
 *   reduced(t)
 *
 *   As in the Bruns paper, page 979.
 *
 *************/

static BOOL reduced(Term t)
{
  BOOL result = TRUE;

  if (VARIABLE(t) || (CONSTANT(t)))
    result = TRUE;
  else if (COMPLEMENT_TERM(t)) {
    if (VARIABLE(ARG(t,0)) || (CONSTANT(ARG(t,0))))
      result = TRUE;
    else
      fatal_error("reduced gets complemented complex term");
  }
  else if (JOIN_TERM(t))
    result = (reduced(ARG(t,0)) &&
	      reduced(ARG(t,1)) &&
	      reduced_join(ARG(t,0), t) &&
	      reduced_join(ARG(t,1), t));
  else if (MEET_TERM(t))
    result = (reduced(ARG(t,0)) &&
	      reduced(ARG(t,1)) &&
	      reduced_meet(ARG(t,0), t) &&
	      reduced_meet(ARG(t,1), t));
  else
    fatal_error("reduced gets unrecognized term");

#if 0
  printf("reduced=%d:   ", result); fwrite_term_nl(stdout, t);
#endif
  return result;
}  /* reduced */

/*************
 *
 *   beta(t) -- destructively transform t.
 *
 *   As in the Bruns paper, page 980.
 *
 *************/

static Term beta(Term t)
{
  if (JOIN_TERM(t)) {
    Term a0, a1;
    ARG(t,0) = simplify_01(beta(ARG(t,0)));
    ARG(t,1) = simplify_01(beta(ARG(t,1)));
    a0 = ARG(t,0);
    a1 = ARG(t,1);

    if (reduced(t) || ZERO_TERM(a0) || ZERO_TERM(a1))
      return t;
    else {
      zap_term(t);
      return get_rigid_term_dangerously(One_sym, 0);
    }
  }
    
  else if (MEET_TERM(t)) {
    Term a0, a1;
    ARG(t,0) = simplify_01(beta(ARG(t,0)));
    ARG(t,1) = simplify_01(beta(ARG(t,1)));
    a0 = ARG(t,0);
    a1 = ARG(t,1);

    if (reduced(t) || ONE_TERM(a0) || ONE_TERM(a1))
      return t;
    else {
      zap_term(t);
      return get_rigid_term_dangerously(Zero_sym, 0);
    }
  }

  else
    return t;
}  /* beta */

/*************
 *
 *   ol_identity()
 *
 *   Given an equality, check if it is an ortholattice (OL) identity.
 *
 *************/

BOOL ol_identity(Term equality)
{
  if (equality == NULL || !is_symbol(SYMNUM(equality), "=", 2))
    return FALSE;
  else {
    Term b0 = beta(simplify_01(neg_norm_form(copy_term(ARG(equality,0)))));
    Term b1 = beta(simplify_01(neg_norm_form(copy_term(ARG(equality,1)))));
    BOOL ok1 = ol_leq(b0, b1);
    BOOL ok2 = ol_leq(b1, b0); 
    BOOL ok = ok1 && ok2;
#if 0
    printf("-----------------\n");
    printf("    "); fwrite_term_nl(stdout, equality);
    printf("b0: "); fwrite_term_nl(stdout, b0);
    printf("b1: "); fwrite_term_nl(stdout, b1);
    printf("b0-le=%d, b1-le=%d\n", ok1, ok2);
#endif
    zap_term(b0);
    zap_term(b1);
    return ok;
  }
}  /* ol_identity */

/*************
 *
 *   expand_defs() - expand definitions.
 *
 *   This is not destructive.  It creates an entirely new copy.
 *
 *************/

static
Term expand_defs(Term t)
{
  if (SHEFFER_TERM(t)) {
    Term a0 = expand_defs(ARG(t,0));
    Term a1 = expand_defs(ARG(t,1));
    return build_binary_term(Join_sym,
			     build_unary_term(Comp_sym, a0),
			     build_unary_term(Comp_sym, a1));
  }
  else if VARIABLE(t)
    return copy_term(t);
  else {
    int i;
    Term s = get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(s,i) = expand_defs(ARG(t,i));
    return s;
  }
}  /* expand_defs */

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  Term t;
  unsigned long int checked = 0;
  unsigned long int passed = 0;
  BOOL fast_parse;
  BOOL output_non_identities;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  fast_parse = string_member("fast", argv, argc);
  output_non_identities = string_member("x", argv, argc);

  /* Assume stdin contains equality units.

     Note that if we're not using fastparse, we use read_term
     which does not "set_variables"; that is,
     the terms that you expect to be variables are still constants.
     That's okay, because the ol identity checker doesn't care whether
     the "base" terms are constants, variables, or mixed.
  */

  if (fast_parse) {
    /* Declare the symbols for fastparse. */
    fast_set_defaults();
    /* Cache symbol IDs. */
    Meet_sym = str_to_sn("m", 2);
    Join_sym = str_to_sn("j", 2);
    Comp_sym = str_to_sn("c", 1);
    Zero_sym = str_to_sn("0", 0);
    One_sym = str_to_sn("1", 0);
    Sheffer_sym = str_to_sn("f", 2);
  }
  else {
    init_standard_ladr();
    /* Cache symbol IDs. */
    Meet_sym = str_to_sn("^", 2);
    Join_sym = str_to_sn("v", 2);
    Comp_sym = str_to_sn("c", 1);
    Zero_sym = str_to_sn("0", 0);
    One_sym = str_to_sn("1", 0);
    Sheffer_sym = str_to_sn("f", 2);
  }

  /* Read the first equation. */
  t = term_reader(fast_parse);

  while (t != NULL) {
    Term expanded = expand_defs(t);
    BOOL ident = ol_identity(expanded);
    checked++;
    if ((!output_non_identities && ident) ||
	(output_non_identities && !ident)) {
      passed++;
      term_writer(t, fast_parse);
    }
    zap_term(t);
    zap_term(expanded);
    t = term_reader(fast_parse);
  }

  printf("%% olfilter%s: checked %lu, passed %lu, user %.2f, system %.2f.\n",
	 output_non_identities ? " x" : "",
	 checked, passed,
	 user_seconds(),
	 system_seconds());

#if 0
  p_term_mem();
#endif
  exit(0);
}  /* main */
