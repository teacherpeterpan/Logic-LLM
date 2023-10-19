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

#include "ac_redun.h"

static Ilist  C_symbols = NULL;   /* C: commutative */
static Ilist A1_symbols = NULL;   /* A1: associative   (xy)z = x(yz) */
static Ilist A2_symbols = NULL;   /* A2: c-associative x(yz) = y(xz) */
static Ilist AC_symbols = NULL;   /* AC: All three */

/* Private definitions and types */

/*************
 *
 *   same_top()
 *
 *************/

/* DOCUMENTATION
Do the two terms have the same top symbol?
*/

/* PUBLIC */
BOOL same_top(Term t1, Term t2)
{
  if (VARIABLE(t1) || VARIABLE(t2))
    return term_ident(t1, t2);
  else
    return SYMNUM(t1) == SYMNUM(t2);
}  /* same_top */

/*************
 *
 *   commutativity()
 *
 *************/

/* DOCUMENTATION
If the atom is commutativity, return the symnum of the operation;
otherwise return 0 (which is never a symnum).
*/

/* PUBLIC */
int commutativity(Term atom)
{
  if (eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 && same_top(alpha, beta)) {
      Term a = ARG(alpha,0);
      Term b = ARG(alpha,1);
      Term c = ARG(beta,0);
      Term d = ARG(beta,1);
      if (VARIABLE(a) && VARIABLE(b) &&
	  VARNUM(a) != VARNUM(b) &&
	  term_ident(a,d) && term_ident(b,c))
	return SYMNUM(alpha);
    }
  }
  return 0;
}  /* commutativity */

/*************
 *
 *   associativity()
 *
 *************/

/* DOCUMENTATION
If the atom is associativity, f(f(x,y),z) = f(x,f(y,z)),
return the symnum of the operation;
otherwise return 0 (which is never a symnum).
*/

/* PUBLIC */
int associativity(Term atom)
{
  if (eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,0)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(ARG(alpha,0),0);
      Term b = ARG(ARG(alpha,0),1);
      Term c = ARG(alpha,1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  term_ident(a,d) && term_ident(b,e) && term_ident(c,f))
	return SYMNUM(alpha);
    }
  }
  return 0;
}  /* associativity */

/*************
 *
 *   c_associativity()
 *
 *************/

/* DOCUMENTATION
If the atom is c_associativity, f(x,f(y,z)) = f(y,f(x,z)),
return the symnum of the operation;
otherwise return 0 (which is never a symnum).
*/

/* PUBLIC */
int c_associativity(Term atom)
{
  if (eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,1)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(alpha,0);
      Term b = ARG(ARG(alpha,1),0);
      Term c = ARG(ARG(alpha,1),1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  term_ident(a,e) && term_ident(b,d) && term_ident(c,f))
	return SYMNUM(alpha);
    }
  }
  return 0;
}  /* c_associativity */

/*************
 *
 *   associativity3()
 *
 *************/

/* DOCUMENTATION
If the atom is any of the following

  (ab)c = a(bc)
  (ab)c = a(cb)
  (ab)c = b(ac)
  (ab)c = b(ca)

return the symnum of the operation;
otherwise return 0 (which is never a symnum).
*/

/* PUBLIC */
int associativity3(Term atom)
{
  if (eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,0)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(ARG(alpha,0),0);
      Term b = ARG(ARG(alpha,0),1);
      Term c = ARG(alpha,1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  ((term_ident(a,d) && term_ident(b,e) && term_ident(c,f)) |
	   (term_ident(a,d) && term_ident(b,f) && term_ident(c,e)) |
	   (term_ident(a,e) && term_ident(b,d) && term_ident(c,f)) |
	   (term_ident(a,f) && term_ident(b,d) && term_ident(c,e))
	   )
	  )
	return SYMNUM(alpha);
    }
  }
  return 0;
}  /* associativity3 */

/*************
 *
 *   associativity4()
 *
 *************/

/* DOCUMENTATION
If the atom is any of the following

  a(bc) = b(ac)
  a(bc) = b(ca)
  a(bc) = c(ab)
  a(bc) = c(ba)

return the symnum of the operation;
otherwise return 0 (which is never a symnum).
*/

/* PUBLIC */
int associativity4(Term atom)
{
  if (eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,1)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(alpha,0);
      Term b = ARG(ARG(alpha,1),0);
      Term c = ARG(ARG(alpha,1),1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  ((term_ident(d,b) && term_ident(e,a) && term_ident(f,c)) |
	   (term_ident(d,b) && term_ident(e,c) && term_ident(f,a)) |
	   (term_ident(d,c) && term_ident(e,a) && term_ident(f,b)) |
	   (term_ident(d,c) && term_ident(e,b) && term_ident(f,a))))
	return SYMNUM(alpha);
    }
  }
  return 0;
}  /* associativity4 */

/*************
 *
 *   can_compare()
 *
 *   Total lexicographic order on terms: var < constant < complex.
 *   Argument sn is the greatest symbol at the root.
 *
 *************/

static
Ordertype can_compare(Term a, Term b, int sn)
{
  if (VARIABLE(a) || VARIABLE(b))
    return term_compare_vcp(a, b);
  else if (SYMNUM(a) == sn && SYMNUM(b) == sn)
    return term_compare_vcp(a, b);
  else if (SYMNUM(a) == sn)
    return GREATER_THAN;
  else if (SYMNUM(b) == sn)
    return LESS_THAN;
  else
    return term_compare_vcp(a, b);
}  /* can_compare */

/*************
 *
 *   canon() -- yet another canonicalizer.
 *
 *************/

static
Term canon(Term t, int flag)
{
  if (term_flag(t, flag))
    return t;  /* already canonicalized */
  else if (VARIABLE(t))
    return t;
  else {
    int sn = SYMNUM(t);
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = canon(ARG(t,i), flag);

    if (ilist_member(AC_symbols, sn)) {
      Term a = ARG(t,0);
      Term b = ARG(t,1);
      if (same_top(t, b) && can_compare(a, ARG(b,0), sn) == GREATER_THAN) {
	Term b0 = ARG(b,0);
	ARG(t,0) = b0;
	ARG(b,0) = a;
	term_flag_clear(b, flag);
	ARG(t,1) = canon(b, flag);
	t = canon(t, flag);
      }
      else if (can_compare(a, b, sn) == GREATER_THAN) {
	ARG(t,0) = b;
	ARG(t,1) = a;
	t = canon(t, flag);
      }
    }
    else if (ilist_member(C_symbols, sn)) {
      Term a = ARG(t,0);
      Term b = ARG(t,1);
      if (can_compare(a, b, sn) == GREATER_THAN) {
	ARG(t,0) = b;
	ARG(t,1) = a;
      }
    }
    term_flag_set(t, flag);  /* mark as canonicalized */
    return t;
  }
}  /* canon */

/*************
 *
 *   cac_redundant_atom()
 *
 *************/

static
BOOL cac_redundant_atom(Term atom)
{
  if (!eq_term(atom))
    return FALSE;  /* must be equality atom */
  else if (VARIABLE(ARG(atom,0)) || VARIABLE(ARG(atom,1)))
    return FALSE;  /* neither side can be a variable */
  else if (SYMNUM(ARG(atom,0)) != SYMNUM(ARG(atom,1)))
    return FALSE;  /* function symbols must be the same */
  else if (symbol_count(ARG(atom,0)) != symbol_count(ARG(atom,1)))
    return FALSE;  /* symbol counts must be the same */
  else if (commutativity(atom))
    return FALSE;
  else if (associativity(atom))
    return FALSE;
  else if (c_associativity(atom))
    return FALSE;
  else {
    /* Copy terms, AC-canonicalize, check for identity. */

    int flag = claim_term_flag();  /* for marking canonicalized subterms */
    BOOL cac_redund;
    Term a = copy_term(ARG(atom, 0));
    Term b = copy_term(ARG(atom, 1));

    a = canon(a, flag);
    b = canon(b, flag);

    cac_redund = term_ident(a,b);
    zap_term(a);
    zap_term(b);
    release_term_flag(flag);
    return cac_redund;
  }
}  /* cac_redundant_atom */

/*************
 *
 *  cac_tautology()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL cac_tautology(Literals lits)
{
  if (C_symbols == NULL)
    return FALSE;
  else if (lits == NULL)
    return FALSE;
  else if (lits->sign && cac_redundant_atom(lits->atom))
    return TRUE;
  else
    return cac_tautology(lits->next);
}  /* cac_tautology */

/*************
 *
 *  cac_redundancy()
 *
 *************/

/* DOCUMENTATION
This routine checks if the clause is commutativity or 
"c-associativity" x(yz)=y(xz);
if so, it stores that information for later calls
to cac_redundancy().  If an operation is found to C or AC,
return TRUE; if found to be just A, return FALSE;
<P>
Otherwise,
if any positive literal is 
an instance of x=x (mod C and AC as previously noted),
it is rewritten to $T.
Then return FALSE.
<P>
Motivation: If we know that * is commutative, and we derive
an equation f(a,b*c)=f(a,c*b), we can delete that equation,
because commutativity can do anything that equation can do.
The same goes for AC operations.
*/

/* PUBLIC */
BOOL cac_redundancy(Topform c, BOOL print)
{
  if (pos_eq_unit(c->literals)) {
    Term atom = c->literals->atom;
    int sn = SYMNUM(ARG(atom, 0));
    BOOL new_assoc;

    if (!ilist_member(C_symbols, sn) && commutativity(atom)) {
      C_symbols = ilist_append(C_symbols, sn);
      if (ilist_member(A1_symbols, sn) &&
	  ilist_member(A2_symbols, sn)) {
	if (print)
	  printf("\n%% Operation %s associative-commutative; "
		 "CAC redundancy checks enabled.\n", sn_to_str(sn));
	AC_symbols = ilist_append(AC_symbols, sn);
      }
      else {
	if (print)
	  printf("\n%% Operation %s is commutative; "
		 "C redundancy checks enabled.\n", sn_to_str(sn));
      }
      return TRUE;  /* New C symbol, may also be A1 and/or A2. */
    }

    if (!ilist_member(A1_symbols, sn) && associativity(atom)) {
      A1_symbols = ilist_append(A1_symbols, sn);
      new_assoc = TRUE;
    }
    else if (!ilist_member(A2_symbols, sn) && c_associativity(atom)) {
      A2_symbols = ilist_append(A2_symbols, sn);
      new_assoc = TRUE;
    }
    else
      new_assoc = FALSE;

    if (new_assoc) {
      if (ilist_member(C_symbols, sn) &&
	  ilist_member(A1_symbols, sn) &&
	  ilist_member(A2_symbols, sn)) {
	if (print)
	  printf("\n%% Operation %s is associative-commutative; "
		 "CAC redundancy checks enabled.\n", sn_to_str(sn));
	AC_symbols = ilist_append(AC_symbols, sn);
	return TRUE;  /* C, A1, A2 (A1 or A2 is newly found) */
      }
      else
	return FALSE;  /* A1 or A2 newly found */
    }
  }  /* pos_eq_unit */

  if (cac_tautology(c->literals)) {
    zap_literals(c->literals);
    c->literals = get_literals();
    c->literals->sign = TRUE;
    c->literals->atom = get_rigid_term(true_sym(), 0);
    c->literals->atom->container = c;
  }
  return FALSE;

}  /* cac_redundancy */

