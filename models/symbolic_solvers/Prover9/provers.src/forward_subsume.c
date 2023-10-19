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

#include "forward_subsume.h"

/* Private definitions and types */

// #define FEATURES

#ifdef FEATURES
static Di_tree Nonunit_index;
static Lindex Unit_index;
#else
#define NUM_INDEXES 10  /* Must be >= 2 */
static Lindex Idx[NUM_INDEXES];
#endif

/*************
 *
 *   init_fsub_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_fsub_index(Mindextype mtype,
		     Uniftype utype,
		     int fpa_depth)
{
#ifdef FEATURES
  Nonunit_index = init_di_tree();
  Unit_index = lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
#else
  int i;
  if (NUM_INDEXES < 2)
    fatal_error("init_fsub_index: NUM_INDEXES < 2");

  for (i = 0; i < NUM_INDEXES; i++)
    Idx[i] = lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
#endif
}  /* init_fsub_index */

/*************
 *
 *   fsub_destroy_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void fsub_destroy_index(void)
{
#ifdef FEATURES
  lindex_destroy(Unit_index);
  /* Nonunit_index??? */
#else
  int i;
  for (i = 0; i < NUM_INDEXES; i++) {
    lindex_destroy(Idx[i]);
    Idx[i] = NULL;
  }
#endif
}  /* fsub_destroy_index */

/*************
 *
 *   index_fsub()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void index_fsub(Topform c, Indexop op, Clock clock)
{
#ifdef FEATURES
  if (number_of_literals(c) == 1)
    lindex_update_first(Unit_index, c, op);
  else {
    Ilist f = features(c);
    if (op == INSERT)
      di_tree_insert(f, Nonunit_index, c);
    else
      di_tree_delete(f, Nonunit_index, c);
    zap_ilist(f);
  }
#else
  int n = number_of_literals(c->literals);
  if (n >= NUM_INDEXES)
    n = NUM_INDEXES-1;
  /* Index only the first literal of c, because any subsumee
     will have to match that first literal.
   */
  clock_start(clock);
  lindex_update_first(Idx[n], c, op);
  clock_stop(clock);
#endif
}  /* index_fsub */

/*************
 *
 *   forward_subsumption()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform forward_subsumption_old(Topform d)
{
#ifdef FEATURES
  Topform subsumer = forward_subsume(d, Unit_index);
  if (!subsumer)
    subsumer = forward_nonunit_subsume(d, Nonunit_index);
  return subsumer;
#else
  int nc = number_of_literals(d->literals);  /* Don't let a longer clause subsume c. */
  int i;
  for (i = 1; i < NUM_INDEXES && i <= nc; i++) {
    Topform subsumer = forward_subsume(d, Idx[i]);
    if (subsumer != NULL)
      return subsumer;
  }
  return NULL;
#endif
}  /* forward_subsumption */

/*************
 *
 *   unit_deletion()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void unit_deletion_old(Topform c)
{
#ifdef FEATURES
  unit_delete(c, Unit_index);
#else
  unit_delete(c, Idx[1]);
#endif
}  /* unit_deletion */
