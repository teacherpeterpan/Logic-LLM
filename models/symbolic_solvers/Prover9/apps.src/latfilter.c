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

#define PROGRAM_NAME    "latfilter"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a stream of meet/join equations (from stdin)\n"
"and writes (to stdout) those that are lattice identities.\n"
"Whitman's procedure is used.  An optional argument 'fast' says\n"
"to read and write the clauses in fastparse form (e.g., =mxxx.).\n"
"The base terms can be either constants or variables.\n\n"
"Example LT identities.\n\n"
"  ordinary:   'x ^ (x v y) = x.'\n"
"  fastparse:  '=mxjxyx.' (exactly one per line, without spaces)\n\n"
"Another optional argument 'x' says to output the equations that are\n"
"*not* OL identities.\n\n";

/* We cache the important symbol numbers to avoid symbol table lookups. */

int Meet_sym;  /* ^ */
int Join_sym;  /* v */
int M_sym;     /* m */
int J_sym;     /* j */

/*************
 *
 *   meet_term()
 *
 *************/

BOOL meet_term(Term t)
{
  return (SYMNUM(t) == Meet_sym || SYMNUM(t) == M_sym);
}  /* meet_term */

/*************
 *
 *   join_term()
 *
 *************/

BOOL join_term(Term t)
{
  return (SYMNUM(t) == Join_sym || SYMNUM(t) == J_sym);
}  /* join_term */

/*************
 *
 *   lattice_leq()
 *
 *************/

/* DOCUMENTATION
Given lattice terms S and T, this routine checks if S <= T.
It is assumed that S and T are in terms of binary operations
meet ("^", "meet", or "m") and join ("v", "join", or "j").
<P>
Whitman's algorithm (from the 1930s) is used, as described in
"Free Lattices", by Freese, Jezek, and Nation, AMS Mathematical
Surveys and Monographs, Vol. 42 (1991).
<P>
Solutions to subproblems are not cached, so the behavior of
this implementation can be exponential.
*/

BOOL lattice_leq(Term s, Term t)
{
  if (VARIABLE(s) && (VARIABLE(t)))        /* (1) */
    return (VARNUM(s) == VARNUM(t));	 
					 
  else if (CONSTANT(s) && (CONSTANT(t)))   /* (1) */
    return (SYMNUM(s) == SYMNUM(t));	   
					 
  else if (join_term(s))                   /* (2) */
    return (lattice_leq(ARG(s,0), t) &&	 
	    lattice_leq(ARG(s,1), t));	 
					 
  else if (meet_term(t))                   /* (3) */
    return (lattice_leq(s, ARG(t,0)) &&	 
	    lattice_leq(s, ARG(t,1)));	 
					 
  else if ((VARIABLE(s) || CONSTANT(s)) && join_term(t))    /* (4) */
    return (lattice_leq(s, ARG(t,0)) ||	 
	    lattice_leq(s, ARG(t,1)));	 
					 
  else if (meet_term(s) && (VARIABLE(t) || CONSTANT(t)))    /* (5) */
    return (lattice_leq(ARG(s,0), t) ||	 
	    lattice_leq(ARG(s,1), t));	 
					 
  else if (meet_term(s) && join_term(t))   /* (6) */
    return (lattice_leq(s, ARG(t,0)) ||
	    lattice_leq(s, ARG(t,1)) ||
	    lattice_leq(ARG(s,0), t) ||
	    lattice_leq(ARG(s,1), t));

  else
    return FALSE;

}  /* lattice_leq */

/*************
 *
 *   lattice_identity()
 *
 *************/

int lattice_identity(Term atom)
{
  return (atom != NULL &&
	  is_symbol(SYMNUM(atom), "=", 2) &&
	  lattice_leq(ARG(atom,0), ARG(atom,1)) &&
	  lattice_leq(ARG(atom,1), ARG(atom,0)));
}  /* lattice_identity */

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

  /* Assume stdin contains equality units.  For each, if it is a
     lattice identity, print it to stdout.
     
     Note that if we're not using fastparse, we use read_term
     which does not "set_variables"; that is, 
     the terms that you expect to be variables are still constants.
     That's okay, because the lattice identity checker doesn't care.
  */

  if (fast_parse) {
    /* Declare the symbols for fastparse. */
    fast_set_defaults();
  }
  else {
    init_standard_ladr();
  }

  /* Cache the symbol numbers for decision procedure lattice_leq. */

  M_sym = str_to_sn("m", 2);
  J_sym = str_to_sn("j", 2);
  Meet_sym = str_to_sn("^", 2);  /* not used for fastparse */
  Join_sym = str_to_sn("v", 2);  /* not used for fastparse */

  t = term_reader(fast_parse);

  while (t != NULL) {
    BOOL ident = lattice_identity(t);
    checked++;
    if ((!output_non_identities && ident) ||
	(output_non_identities && !ident)) {
      passed++;
      term_writer(t, fast_parse);
      fflush(stdout);
    }
    zap_term(t);
    t = term_reader(fast_parse);
  }

  printf("%% latfilter%s: checked %lu, passed %lu, user %.2f, system %.2f.\n",
	 output_non_identities ? " x" : "",
	 checked, passed,
	 user_seconds(),
	 system_seconds());

  exit(0);
}  /* main */
