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

#include "msearch.h"

extern Mace_options Opt;

extern Symbol_data Symbols;

extern int Number_of_cells;
extern struct cell *Cells;
extern struct cell **Ordered_cells;  /* Permutation of Cells. */
extern int First_skolem_cell;  /* =Number_of_cells if !Skolems_last */

extern int Domain_size;
extern Term *Domain;

extern struct mace_stats Mstats;

/* Here are the meanings of the parameters that control selection. */

#define SELECT_LINEAR             0  /* selection orders */
#define SELECT_CONCENTRIC         1
#define SELECT_CONCENTRIC_BAND    2

#define NO_MEASURE                0  /* selection measures */
#define MOST_OCCURRENCES          1
#define MOST_PROPAGATIONS         2
#define MOST_CONTRADICTIONS       3
#define MOST_CROSSED              4

/*************
 *
 *   num_contradictions() - for a given ID, try all assignments and see
 *   how many give contradiction by propagation.
 *
 *   Leave the Propaagtions statistic as it was.
 *
 *   We're not interested in a number less than max_so_far, and
 *   if we determine that we'll never get that many, we return -1.
 *
 *************/

static
int num_contradictions(int id, int max_so_far)
{
  int n = 0;
  int save_prop_count = Mstats.propagations;
  int ds = id_to_domain_size(id);
  int i, to_try;
  for (i = 0; i < ds; i++) {
    Estack stk = assign_and_propagate(id, Domain[i]);
    if (stk == NULL)
      n++;
    else
      restore_from_stack(stk);

    to_try = (ds - i) - 1;
    if (n + to_try <= max_so_far) {
      Mstats.propagations = save_prop_count;
      return -1;  /* can't beat max_so_far */
    }
  }
  Mstats.propagations = save_prop_count;
  return n;
}  /* num_contradictions */

/*************
 *
 *   num_propagations() - for a given ID, try all assignments
 *   and return the total number of propagations.
 *
 *   Leave the Propaagtions statistic as it was.
 *
 *************/

static
int num_propagations(int id)
{
  int n = 0;
  int i;
  int save_prop_count = Mstats.propagations;
  int ds = id_to_domain_size(id);
  Mstats.propagations = 0;
  for (i = 0; i < ds; i++) {
    Estack stk = assign_and_propagate(id, Domain[i]);
    restore_from_stack(stk);
    n += Mstats.propagations;
    Mstats.propagations = 0;
  }
  Mstats.propagations = save_prop_count;
  return n;
}  /* num_propagations */

/*************
 *
 *   num_crossed() - number of values crossed off for a given ID.
 *
 *************/

static
int num_crossed(int id)
{
  Term *p = Cells[id].possible;
  int n = 0;
  int i;
  int ds = id_to_domain_size(id);
  for (i = 0; i < ds; i++)
    if (p[i] == NULL)
      n++;
  return n;
}  /* num_crossed */

/*************
 *
 *   num_occurrences() - number of (active) occurrences of an eterm
 *
 *************/

static
int num_occurrences(int id)
{
  Term t;
  int n = 0;
  for (t = Cells[id].occurrences; t != NULL; t = t->u.vp)
    n++;
  return n;
}  /* num_occurrences */

/*************
 *
 *   selection_measure()
 *
 *************/

static
void selection_measure(int id, int *max, int *max_id)
{
  int n = 0;
  switch (parm(Opt->selection_measure)) {
  case MOST_OCCURRENCES:    n = num_occurrences(id);          break;
  case MOST_PROPAGATIONS:   n = num_propagations(id);         break;
  case MOST_CONTRADICTIONS: n = num_contradictions(id, *max); break;
  case MOST_CROSSED:        n = num_crossed(id);              break;
  default: fatal_error("selection_measure: bad selection measure");
  }
  if (n > *max) {
    *max = n;
    *max_id = id;
  }
}  /* selection_measure */

/*************
 *
 *   select_linear()
 *
 *************/

int select_linear(int min_id, int max_id)
{
  if (parm(Opt->selection_measure) == NO_MEASURE) {
    /* Return the first open cell. */
    int i = min_id;
    while (i <= max_id && Cells[i].value != NULL)
      i++;
    return (i > max_id ? -1 : i);
  }
  else {
    int id_of_max = -1;
    int max = -1;
    int i;
    for (i = min_id; i <= max_id; i++) {
      if (Cells[i].value == NULL) {
	selection_measure(i, &max, &id_of_max);
      }
    }
    return id_of_max;    
  }
}  /* select_linear */

/*************
 *
 *   select_concentric()
 *
 *   This assumes that Ordered_cells is ordered FIRST by max_index.
 *
 *   If the first open cell has max_index n, return the best open
 *   cell with max index n.
 *
 *************/

int select_concentric(int min_id, int max_id)
{
  /* Find the first open cell. */
  int i = min_id;
  while (i <= max_id && Ordered_cells[i]->value != NULL)
    i++;
  if (i > max_id)
    return -1;
  else {
    /* Find the best cell with the same max_index as the first open cell. */
    int n = Ordered_cells[i]->max_index;
    int max_val = -1;
    int id_of_max = -1;

    while (i <= max_id && Ordered_cells[i]->max_index <= n) {
      if (Ordered_cells[i]->value == NULL)
	selection_measure(Ordered_cells[i]->id, &max_val, &id_of_max);
      i++;
    }
    return id_of_max;
  }
}  /* select_concentric */

/*************
 *
 *   select_concentric_band()
 *
 *************/

int select_concentric_band(min_id, max_id, max_constrained)
{
  int max = -1;
  int id_of_max = -1;
  int i = min_id;
  
  while (i <= max_id &&
	 Ordered_cells[i]->max_index <= max_constrained) {
    if (Ordered_cells[i]->value == NULL)
      selection_measure(Ordered_cells[i]->id, &max, &id_of_max);
    i++;
  }
  if (id_of_max >= 0)
    return id_of_max;
  else
    /* There is nothing in the band, so revert to select_concentric.
       This is a bit redundant, because it will scan (again) the full cells.
    */
    return select_concentric(min_id, max_id);
}  /* select_concentric_band */

/*************
 *
 *   select_cell()
 *
 *************/

int select_cell(int max_constrained)
{
  int id = -1;
  switch (parm(Opt->selection_order)) {
  case SELECT_LINEAR: id = select_linear(0, First_skolem_cell-1); break;
  case SELECT_CONCENTRIC: id = select_concentric(0, First_skolem_cell-1); break;
  case SELECT_CONCENTRIC_BAND: id = select_concentric_band(0, First_skolem_cell-1, max_constrained); break;
  default: fatal_error("bad selection order");
  }
  
  if (id >= 0)
    return id;

  switch (parm(Opt->selection_order)) {
  case SELECT_LINEAR: id = select_linear(First_skolem_cell, Number_of_cells-1); break;
  case SELECT_CONCENTRIC: id = select_concentric(First_skolem_cell, Number_of_cells-1); break;
  case SELECT_CONCENTRIC_BAND: id = select_concentric_band(First_skolem_cell, Number_of_cells-1, max_constrained); break;
  default: fatal_error("bad selection order");
  }

  return id;
}  /* select_cell */
