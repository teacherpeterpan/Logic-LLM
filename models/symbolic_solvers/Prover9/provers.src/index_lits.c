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

#include "index_lits.h"

/* Private definitions and types */

static Lindex  Unit_fpa_idx;          /* unit bsub, unit conflict */
static Lindex  Nonunit_fpa_idx;       /* back unit del */

static Lindex  Unit_discrim_idx;      /* unit fsub, unit del */
static Di_tree Nonunit_features_idx;  /* nonunit fsub, nonunit bsub */

/*************
 *
 *   init_lits_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_literals_index(void)
{
  Unit_fpa_idx     = lindex_init(FPA, ORDINARY_UNIF, 10,
				 FPA, ORDINARY_UNIF, 10);

  Nonunit_fpa_idx  = lindex_init(FPA, ORDINARY_UNIF, 10,
				 FPA, ORDINARY_UNIF, 10);

  Unit_discrim_idx = lindex_init(DISCRIM_BIND, ORDINARY_UNIF, 10,
				 DISCRIM_BIND, ORDINARY_UNIF, 10);

  Nonunit_features_idx = init_di_tree();
}  /* init_lits_index */

/*************
 *
 *   lits_destroy_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void destroy_literals_index(void)
{
  lindex_destroy(Unit_fpa_idx);       Unit_fpa_idx = NULL;
  lindex_destroy(Nonunit_fpa_idx);    Nonunit_fpa_idx = NULL;
  lindex_destroy(Unit_discrim_idx);   Unit_discrim_idx = NULL;
  zap_di_tree(Nonunit_features_idx,
	      feature_length());      Nonunit_features_idx = NULL;
}  /* lits_destroy_index */

/*************
 *
 *   index_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void index_literals(Topform c, Indexop op, Clock clock, BOOL no_fapl)
{
  BOOL unit = (number_of_literals(c->literals) == 1);
  clock_start(clock);
  if (!no_fapl || !positive_clause(c->literals))
    lindex_update(unit ? Unit_fpa_idx : Nonunit_fpa_idx, c, op);
  
  if (unit)
    lindex_update(Unit_discrim_idx, c, op);
  else {
    Ilist f = features(c->literals);
    if (op == INSERT)
      di_tree_insert(f, Nonunit_features_idx, c);
    else
      di_tree_delete(f, Nonunit_features_idx, c);
    zap_ilist(f);
  }
  clock_stop(clock);
}  /* index_literals */

/*************
 *
 *   index_denial()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void index_denial(Topform c, Indexop op, Clock clock)
{
  BOOL unit = (number_of_literals(c->literals) == 1);
  clock_start(clock);
  lindex_update(unit ? Unit_fpa_idx : Nonunit_fpa_idx, c, op);
  clock_stop(clock);
}  /* index_denial */

/*************
 *
 *   unit_conflict()
 *
 *************/

/* DOCUMENTATION
Look for conflicting units.  Send any that are found to empty_proc().
*/

/* PUBLIC */
void unit_conflict(Topform c, void (*empty_proc) (Topform))
{
  unit_conflict_by_index(c, Unit_fpa_idx, empty_proc);
}  /* unit_conflict */

/*************
 *
 *   unit_deletion()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void unit_deletion(Topform c)
{
  unit_delete(c, Unit_discrim_idx);
}  /* unit_deletion */

/*************
 *
 *   back_unit_deletable()
 *
 *************/

/* DOCUMENTATION
Return the list of clauses that can be back unit deleted by
the given clause.
*/

/* PUBLIC */
Plist back_unit_deletable(Topform c)
{
  return back_unit_del_by_index(c, Nonunit_fpa_idx);
}  /* back_unit_deletable */

/*************
 *
 *   forward_subsumption()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform forward_subsumption(Topform d)
{
  Topform subsumer = forward_subsume(d, Unit_discrim_idx);
  if (!subsumer)
    subsumer = forward_feature_subsume(d, Nonunit_features_idx);
  return subsumer;
}  /* forward_subsumption */

/*************
 *
 *   back_subsumption()
 *
 *************/

/* DOCUMENTATION
Return the list of clauses that can ar back subsumed by the given clause.
*/

/* PUBLIC */
Plist back_subsumption(Topform c)
{
  Plist p1 = back_subsume(c, Unit_fpa_idx);
#if 0
  Plist p2 = back_subsume(c, Nonunit_fpa_idx);
#else
  Plist p2 = back_feature_subsume(c, Nonunit_features_idx);
#endif

  Plist p3 = plist_cat(p1, p2);
  return p3;
}  /* back_subsumption */

/*************
 *
 *   lits_idx_report()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void lits_idx_report(void)
{
  printf("Pos unit lits index: ");
  p_fpa_density(Unit_fpa_idx->pos->fpa);
  printf("Neg unit lits index: ");
  p_fpa_density(Unit_fpa_idx->neg->fpa);
  printf("Pos nonunit lits index: ");
  p_fpa_density(Nonunit_fpa_idx->pos->fpa);
  printf("Neg nonunit lits index: ");
  p_fpa_density(Nonunit_fpa_idx->neg->fpa);
}  /* lits_idx_report */
