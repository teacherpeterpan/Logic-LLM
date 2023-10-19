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
#include "../ladr/order.h"

extern int Number_of_cells;
extern struct cell *Cells;
extern struct cell **Ordered_cells;  /* Permutation of Cells. */

extern BOOL Skolems_last;
extern int First_skolem_cell;

/*************
 *
 *   sum_indexes(t)
 *
 *   Tssume t is an eterm, that is, nonvariable with variable arguments.
 *
 *************/

static
int sum_indexes(Term t)
{
  if (ARITY(t) == 0)
    return -1;
  else {
    int sum = 0;
    int i;
    for (i = 0; i < ARITY(t); i++)
      sum += VARNUM(ARG(t,i));
    return sum;
  }
}  /* sum_indexes */

/*************
 *
 *   compare_cells()
 *
 *   For example, if a < b < g < f, this gives the following order:
 *
 *   a.            g(2).           0 = 0.
 *   b.            f(0,2).         0 = 1.
 *   g(0).         f(2,0).         0 = 2.
 *   f(0,0).       f(1,2).         1 = 0.
 *   g(1).         f(2,1).         1 = 1.
 *   f(0,1).       f(2,2).         1 = 2.
 *   f(1,0).                       2 = 0.
 *   f(1,1).                       2 = 1.
 *                                 2 = 2.
 *
 *   Equality cells are greatest,
 *
 *   If (Skolems_last) Skolem cells are next greatest,
 *
 *   <maximum-index, mace_sn, sum-of-indexes>.
 */

static
Ordertype compare_cells(struct cell *a, struct cell *b)
{
  if (a->symbol->attribute == EQUALITY_SYMBOL &&
      b->symbol->attribute != EQUALITY_SYMBOL)       return GREATER_THAN;
                                                   
  else if (a->symbol->attribute != EQUALITY_SYMBOL &&
	   b->symbol->attribute == EQUALITY_SYMBOL)  return LESS_THAN;
                                                   
  else if (Skolems_last &&
	   a->symbol->attribute == SKOLEM_SYMBOL &&
	   b->symbol->attribute != SKOLEM_SYMBOL)    return GREATER_THAN;
                                                   
  else if (Skolems_last &&
	   a->symbol->attribute != SKOLEM_SYMBOL &&
	   b->symbol->attribute == SKOLEM_SYMBOL)    return LESS_THAN;
                                                   
  else if (a->max_index < b->max_index)              return LESS_THAN;
                                                       
  else if (a->max_index > b->max_index)              return GREATER_THAN;
                                                       
  else if (a->symbol->mace_sn < b->symbol->mace_sn)  return LESS_THAN;
                                                       
  else if (a->symbol->mace_sn > b->symbol->mace_sn)  return GREATER_THAN;

  else if (sum_indexes(a->eterm) <
           sum_indexes(b->eterm))                    return LESS_THAN;
    
  else if (sum_indexes(a->eterm) >
           sum_indexes(b->eterm))                    return GREATER_THAN;
    
  else

    return SAME_AS;  /* For now, let f(0,1) be the same as f(1,0), etc.  */
  
}  /* compare_cells */

/*************
 *
 *   order_cells()
 *
 *************/

void order_cells(BOOL verbose)
{
  int i;

  for (i = 0; i < Number_of_cells; i++)
    Ordered_cells[i] = Cells + i;

  merge_sort((void **) Ordered_cells, Number_of_cells,
             (Ordertype (*) (void*,void*)) compare_cells);

  if (Skolems_last) {
    for (i = 0; i < Number_of_cells; i++)
      if (Ordered_cells[i]->symbol->attribute == SKOLEM_SYMBOL)
	break;
    First_skolem_cell = i;  /* if none, set to Number_of_cells */
  }
  else
    First_skolem_cell = Number_of_cells;

  if (verbose) {
    /* print the Ordered_cells */
    printf("\n%% Cell selection order:\n\n");
    for (i = 0; i < Number_of_cells; i++) {
      Term t = Ordered_cells[i]->eterm;
      if (!is_eq_symbol(SYMNUM(t)))
	fwrite_term_nl(stdout, t);
    }
    fflush(stdout);
  }
}  /* order_cells */
