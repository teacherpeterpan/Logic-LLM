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

#ifndef TP_CLAUSE_H
#define TP_CLAUSE_H

#include "literals.h"
#include "attrib.h"
#include "formula.h"
#include "maximal.h"

/* INTRODUCTION
A Topform can be used to store a formula or a clause.
The field is_formula says which it is.

<p>
In earlier versions of LADR, this data structure was called Clause.
When we decided to put non-clausal formulas in proofs, they
needed to have IDs, attributes, and justifications, so we elevated
the data structure to include non-clausal formulas and changed
the name to Topform (top formula).

<p>
In many cases, when we say "clause", we mean a list of Literals.
For example, most of the functions that tell the properties
of clauses (positive_clause, number_of_literals, etc.) take
a list of Literals, not a Topform.

<p>
If C had data structures with inheritance, this would
be a good place to use it.
*/

/* Public definitions */

typedef struct topform * Topform;

struct topform {

  /* for both clauses and formulas */

  int              id;
  struct clist_pos *containers;     /* Clists that contain the Topform */
  Attribute        attributes;
  struct just      *justification;
  double           weight;
  char             *compressed;     /* if nonNULL, a compressed form */
  Topform          matching_hint;   /* hint that matches clause, if any */

  /* for clauses only */

  Literals         literals;        /* NULL can mean the empty clause */

  /* for formulas only */

  Formula          formula;

  int   semantics;        /* evaluation in interpretations */

  /* The rest of the fields are flags.  These could be bits. */

  char   is_formula;      /* is this really a formula? */
  char   normal_vars;     /* variables have been renumbered */
  char   used;            /* used to infer a clause that was kept */
  char   official_id;     /* Topform is in the ID table */
  char   initial;         /* existed at the start of the search */
  char   neg_compressed;  /* negative and compressed */
  char   subsumer;        /* has this clause back subsumed anything? */

};

/* End of public definitions */

/* Public function prototypes from topform.c */

Topform get_topform(void);

void fprint_topform_mem(FILE *fp, int heading);

void p_topform_mem();

void zap_topform(Topform tf);

void fprint_clause(FILE *fp, Topform c);

void p_clause(Topform c);

Topform term_to_clause(Term t);

Topform term_to_topform(Term t, BOOL is_formula);

Term topform_to_term(Topform tf);

Term topform_to_term_without_attributes(Topform tf);

void clause_set_variables(Topform c, int max_vars);

void renumber_variables(Topform c, int max_vars);

void term_renumber_variables(Term t, int max_vars);

Plist renum_vars_map(Topform c);

void upward_clause_links(Topform c);

BOOL check_upward_clause_links(Topform c);

Topform copy_clause(Topform c);

Topform copy_clause_with_flags(Topform c);

Topform copy_clause_with_flag(Topform c, int flag);

void inherit_attributes(Topform par1, Context s1,
			Topform par2, Context s2,
			Topform child);

void gather_symbols_in_topform(Topform c, I2list *rsyms, I2list *fsyms);

void gather_symbols_in_topforms(Plist lst, I2list *rsyms, I2list *fsyms);

Ilist fsym_set_in_topforms(Plist lst);

Ilist rsym_set_in_topforms(Plist lst);

BOOL min_depth(Literals lit);

BOOL initial_clause(Topform c);

BOOL negative_clause_possibly_compressed(Topform c);

Term topform_properties(Topform c);

void append_label_attribute(Topform tf, char *s);

Ordertype cl_id_compare(Topform c1, Topform c2);

Ordertype cl_wt_id_compare(Topform c1, Topform c2);

#endif  /* conditional compilation of whole file */
