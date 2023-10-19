#include "lits_index.h"

/* Private definitions and types */

/*
  Index and retrieve literals for back subsumption, unit conflict,
  and back unit deletion.

  Maintain separate indexes for unit and nonunit clauses.
*/

static Lindex Unit_idx;
static Lindex Nonunit_idx;

/*************
 *
 *   init_lits_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_lits_index(Mindextype mtype,
		     Uniftype utype,
		     int fpa_depth)
{
  Unit_idx    = lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
  Nonunit_idx = lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
}  /* init_lits_index */

/*************
 *
 *   lits_destroy_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void lits_destroy_index(void)
{
  lindex_destroy(Unit_idx);
  lindex_destroy(Nonunit_idx);
  Unit_idx = NULL;
  Nonunit_idx = NULL;
}  /* lits_destroy_index */

/*************
 *
 *   index_literals_old()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void index_literals_old(Clause c, Indexop op, Clock clock)
{
  if (!flag(NO_FAPL) || !positive_clause(c)) {
    /* If c is a nonunit, all literals will be indexed, because
       a subsumer has to match only a subset of c. */
    BOOL unit = (number_of_literals(c) == 1);
    clock_start(clock);
    lindex_update(unit ? Unit_idx : Nonunit_idx, c, op);
    clock_stop(clock);
  }
}  /* index_literals_old */

/*************
 *
 *   back_subsumption()
 *
 *************/

/* DOCUMENTATION
Return the list of clauses that can ar back subsumed by the given clause.
*/

/* PUBLIC */
Plist back_subsumption_old(Clause c)
{
  Plist p1 = back_subsume(c, Unit_idx);
  Plist p2 = back_subsume(c, Nonunit_idx);
  Plist p3 = plist_cat(p1, p2);
  return p3;
}  /* back_subsumption */

/*************
 *
 *   unit_conflict()
 *
 *************/

/* DOCUMENTATION
Look for a set (up to max) of conflicting units.  Return a list
of those found.
*/

/* PUBLIC */
Plist unit_conflict_old(Clause c, int max)
{
  return unit_conflict_by_index(c, max, Unit_idx);
}  /* unit_conflict */

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
Plist back_unit_deletable_old(Clause c)
{
  return back_unit_del_by_index(c, Nonunit_idx);
}  /* back_unit_deletable */

