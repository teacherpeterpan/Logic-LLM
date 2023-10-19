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

#define PROGRAM_NAME     "Mace4"
#include "../VERSION_DATE.h"

/*********************************************** LADR includes */

#include "../ladr/header.h"
#include "../ladr/top_input.h" /* includes most of what we need */
#include "../ladr/clausify.h"
#include "../ladr/clock.h"
#include "../ladr/nonport.h"
#include "../ladr/interp.h"

/*********************************************** local includes */

#include "estack.h"
#include "arithmetic.h"
#include "mstate.h"
#include "syms.h"
#include "ground.h"
#include "propagate.h"

/*********************************************** macro definitions */

#define MAX(a,b)  ((a) > (b) ? a : b)
#define MIN(a,b)  ((a) < (b) ? a : b)

typedef struct mace_options * Mace_options;

struct mace_options {

  /* This structure holds the option IDs, not the option values! */

  /* flags */

  int print_models,
    print_models_tabular,
    lnh,
    trace,
    negprop,
    neg_assign,
    neg_assign_near,
    neg_elim,
    neg_elim_near,
    verbose,
    integer_ring,
    order_domain,
    arithmetic,
    iterate_primes,
    iterate_nonprimes,
    skolems_last,

    return_models;  /* special case */

  /* parms */
  
  int domain_size,
    start_size,
    end_size,
    iterate_up_to,
    increment,
    max_models,
    selection_order,
    selection_measure,
    max_seconds,
    max_seconds_per,
    max_megs,
    report_stderr;

  /* stringparms */

  int iterate;
};

struct mace_stats {
  /* stats for the current domain size */
  unsigned
    current_models,
    selections,
    assignments,
    propagations,
    cross_offs,
    rewrite_terms,
    rewrite_bools,
    indexes,
    ground_clauses_seen,
    ground_clauses_kept,
    rules_from_neg,

    neg_elim_attempts,
    neg_elim_agone,
    neg_elim_egone,

    neg_assign_attempts,
    neg_assign_agone,
    neg_assign_egone,

    neg_near_assign_attempts,
    neg_near_assign_agone,
    neg_near_assign_egone,

    neg_near_elim_attempts,
    neg_near_elim_agone,
    neg_near_elim_egone;
};

/* Mace results */

typedef struct mace_results * Mace_results;

struct mace_results {
  BOOL success;
  Plist models;
  double user_seconds;
  int return_code;
};

/* Exit codes */

enum {
  MAX_MODELS_EXIT   = 0,
  /* FATAL_EXIT     = 1,   declared elsewhere, and not needed here*/
  EXHAUSTED_EXIT    = 2,
  ALL_MODELS_EXIT   = 3,
  MAX_SEC_YES_EXIT  = 4,
  MAX_SEC_NO_EXIT   = 5,
  MAX_MEGS_YES_EXIT = 6,
  MAX_MEGS_NO_EXIT  = 7,

  MACE_SIGINT_EXIT  = 101,
  MACE_SIGSEGV_EXIT = 102
};


/*********************************************** structure declarations */

/* A cell is member of a function or relation table.  For example,
   if we have a group operation and the domain is size 6, then
   the term f(2,3) represents one cell of the group multiplication
   table.  We will dynamically allocate a (1-dimensional) array
   of cells (to hold all tables) at the beginning of the run after
   we know how many we'll need.  The array will be indexed by IDs
   that are constructed from the terms representing the cells.
   (Each symbol has a "base" value, and the cell ID is an offset
   from the base.)  We have, in effect, multidimensional arrays
   within our main array of cells, we calculate the IDs from
   the symbol base and the term arguments.  Anyway, given a term
   like f(2,3), in which all of the arguments are domain members,
   we can quickly get to the corresponding cell.
*/

struct cell {
  int id;
  Term eterm;          /* the term representation, e.g., f(2,3) */
  Term value;          /* current value of cell (domain element or NULL) */
  Term occurrences;    /* current occurrences of the term */
  Term *possible;      /* current set of possible values */
  int max_index;       /* maximum index for this cell */
  Symbol_data symbol;  /* data on the function or relation symbol */
};

/* These macros take you to the correct cell in the Cells table.
   That is, given the base for the symbol and the indexes, get the ID.
*/

#define X0(b)         (b)
#define X1(b,i)       (b) + (i)
#define X2(b,i,j)     (b) + (i)*Domain_size + (j)
#define X3(b,i,j,k)   (b) + (i)*Domain_size*Domain_size + (j)*Domain_size + (k)
#define X4(b,i,j,k,l) (b) + (i)*Domain_size*Domain_size*Domain_size + (j)*Domain_size*Domain_size + (k)*Domain_size + (l)

/*********************************************** function prototypes */

/* from msearch.c */

void init_mace_options(Mace_options opt);
Mace_results mace4(Plist clauses, Mace_options opt);
void mace4_exit(int exit_code);

/* from util.c */

void random_permutation(int *a, int n);
int int_power(int n, int exp);
BOOL prime(int n);

/* from print.c */

void p_model(BOOL print_head);
void print_model_standard(FILE *fp, BOOL print_head);
void p_matom(Term atom);
void p_mclause(Mclause c);
void p_eterms(void);
void p_stats(void);
void p_mem(void);
void reset_current_stats(void);
Term interp_term(void);

/* from select.c */

int select_cell(int max_constrained);

/* from negpropindex.c */

void init_negprop_index(void);
void free_negprop_index(void);
void p_negprop_index(void);
void insert_negprop_index(Term atom);
void negprop_n_index(Term atom);
void insert_negprop_eq(Term atom, Term alpha, int val, Mstate state);
void insert_negprop_noneq(Term atom, Mstate state);
BOOL nterm(Term t, int *ppos, int *pid);
Term negprop_find_near(int sign, int sn, int val, Term query, int pos);

/* from negprop.c */

void propagate_negative(int type, int id, Term alpha, Term beta, int pos,
			Mstate state);

/* from ordercells.c */

void order_cells(BOOL verbose);

/* from commandline.c */

BOOL member_args(int argc, char **argv, char *str);
void usage_message(FILE *fp, Mace_options opt);
void process_command_line_args(int argc, char **argv, Mace_options opt);
			       
