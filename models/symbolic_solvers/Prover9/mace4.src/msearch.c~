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
#include "../ladr/banner.h"

// #define DEBUG

/*****************************************************************************/
/* Variables -- most are used (extern) by other source files */

/* Options and Statistics */

Mace_options Opt;
struct mace_stats Mstats;

/* List of symbols and associated data */

Symbol_data Symbols;

/* This maps OPS symbol IDs to MACE symbol IDs, which start with 0. */

static int Sn_map_size;
Symbol_data *Sn_to_mace_sn;

/* Cell table is indexed by eterm IDs. */

int Number_of_cells;
struct cell *Cells;           /* the table of cells (dynamically allocated) */
struct cell **Ordered_cells;  /* (pointers to) permutation of Cells */
int First_skolem_cell;

/* Misc Variables*/

int Domain_size;
Term *Domain;    /* array of terms representing (shared) domain elements  */
BOOL Skolems_last;

Plist Ground_clauses;  /* Mclauses (see ground.h) */

int Relation_flag;  /* term flag */
int Negation_flag;  /* term flag */

int Eq_sn;
int Or_sn;
int Not_sn;

static int Max_domain_element_in_input;  /* For Least Number Heuristic */

static Plist Models;  /* in case we collect models as terms */

Clock Mace4_clock;

/* stats for entire run */

unsigned Total_models;

static double Start_seconds;
static double Start_domain_seconds;
static int Start_megs;

/* end of variables */
/*****************************************************************************/

/* search return codes */

enum {
  SEARCH_GO_MODELS,           /* continue: model(s) found on current path */
  SEARCH_GO_NO_MODELS,        /* continue: no models found on current path */
  SEARCH_MAX_MODELS,          /* stop */
  SEARCH_MAX_MEGS,            /* stop */
  SEARCH_MAX_TOTAL_SECONDS,   /* stop */
  SEARCH_MAX_DOMAIN_SECONDS,  /* stop */
  SEARCH_DOMAIN_OUT_OF_RANGE  /* stop */
};

/* Ground terms.  MACE4 operates on ground clauses, which are
   represented by the structure mclause.  Ground terms (and
   atoms) are represented with ordinary LADR terms.  There are
   a few tricks:

   (1) We use upward pointers from terms to superterms
   and from atoms to clauses.
 
   (2) We need to mark atoms with a termflag, so that we know
   when to stop when following the upward pointers.  Also,
   a termflag is used to indicate that an atom is negated.

   (3) Domain elements are represented by variables (sorry,
   but it is very convenient to do so).  Also, there is only
   one actual copy of each domain element (structure sharing
   of domain elements).  Global array *Domain contains them.

   IDs.  If all of the arguments of a term (including atoms) are
   domain elements, that term is called an eterm.  For example,
   f(3,4), a, P(0), and Q.

   Each eterm has a unique ID which is used as an index into
   the cell table, for example, when a new eterm is obtained by
   evaluating a subterm to a domain element, we have to quickly
   check if this new eterm can be evaluated.  We do this by
   calculating its ID and looking up in Cells[ID].value.
   And when we have a new assignment, say f(3,4)=2, we find the
   list of occurrences of f(3,4) by looking in Cells[ID].occurrences.
*/

/*************
 *
 *   init_mace_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_mace_options(Mace_options opt)
{
  opt->domain_size       = init_parm("domain_size",      0,       0, INT_MAX);
  opt->start_size        = init_parm("start_size",       2,       2, INT_MAX);
  opt->end_size          = init_parm("end_size",        -1,      -1, INT_MAX);
  opt->iterate_up_to     = init_parm("iterate_up_to",   -1,      -1, INT_MAX);
  opt->max_models        = init_parm("max_models",       1,      -1, INT_MAX);
  opt->max_seconds       = init_parm("max_seconds",     -1,      -1, INT_MAX);
  opt->max_seconds_per   = init_parm("max_seconds_per", -1,      -1, INT_MAX);
  opt->selection_order   = init_parm("selection_order",  2,       0, 2);
  opt->selection_measure = init_parm("selection_measure",4,       0, 4);
  opt->increment         = init_parm("increment",        1,       1, INT_MAX);
  opt->max_megs          = init_parm("max_megs",       200,      -1, INT_MAX);
  opt->report_stderr     = init_parm("report_stderr",   -1,      -1, INT_MAX);
         
  opt->print_models           = init_flag("print_models",           TRUE);
  opt->print_models_tabular   = init_flag("print_models_tabular",   FALSE);
  opt->lnh                    = init_flag("lnh",                    TRUE);
  opt->trace                  = init_flag("trace",                  FALSE);
  opt->negprop                = init_flag("negprop",                TRUE);
  opt->neg_assign             = init_flag("neg_assign",             TRUE);
  opt->neg_assign_near        = init_flag("neg_assign_near",        TRUE);
  opt->neg_elim               = init_flag("neg_elim",               TRUE);
  opt->neg_elim_near          = init_flag("neg_elim_near",          TRUE);
  opt->verbose                = init_flag("verbose",                FALSE);
  opt->integer_ring           = init_flag("integer_ring",           FALSE);
  opt->order_domain           = init_flag("order_domain",           FALSE);
  opt->arithmetic             = init_flag("arithmetic",             FALSE);
  opt->iterate_primes         = init_flag("iterate_primes",         FALSE);
  opt->iterate_nonprimes      = init_flag("iterate_nonprimes",      FALSE);
  opt->skolems_last           = init_flag("skolems_last",           FALSE);
  opt->return_models          = init_flag("return_models",          FALSE);

  opt->iterate = init_stringparm("iterate", 5,
				 "all",
				 "evens",
				 "odds",
				 "primes",
				 "nonprimes");

  /* dependencies */

  flag_flag_dependency(opt->print_models_tabular,TRUE,opt->print_models,FALSE);
  flag_flag_dependency(opt->print_models,TRUE,opt->print_models_tabular,FALSE);

  flag_flag_dependency(opt->iterate_primes, TRUE,opt->iterate_nonprimes,FALSE);
  flag_flag_dependency(opt->iterate_nonprimes, TRUE,opt->iterate_primes,FALSE);

  parm_parm_dependency(opt->domain_size, opt->start_size, 1, TRUE);
  parm_parm_dependency(opt->domain_size, opt->end_size, 1, TRUE);

  parm_parm_dependency(opt->iterate_up_to, opt->end_size, 1, TRUE);

  flag_stringparm_dependency(opt->iterate_primes,TRUE,opt->iterate,"primes");
  flag_stringparm_dependency(opt->iterate_nonprimes,TRUE,opt->iterate,"nonprimes");

  flag_flag_dependency(opt->integer_ring, TRUE, opt->lnh, FALSE);
  flag_flag_dependency(opt->order_domain, TRUE, opt->lnh, FALSE);
  flag_flag_dependency(opt->arithmetic, TRUE, opt->lnh, FALSE);

  flag_parm_dependency(opt->arithmetic,TRUE,opt->selection_order,0);

}  /* init_mace_options */

/*************
 *
 *   exit_string()
 *
 *************/

static
char *exit_string(int code)
{
  char *message;
  switch (code) {
  case MAX_MODELS_EXIT:   message = "max_models";    break;
  case ALL_MODELS_EXIT:   message = "all_models";    break;
  case EXHAUSTED_EXIT:    message = "exhausted";     break;
  case MAX_MEGS_YES_EXIT: message = "max_megs_yes";  break;
  case MAX_MEGS_NO_EXIT:  message = "max_megs_no" ;  break;
  case MAX_SEC_YES_EXIT:  message = "max_sec_yes";   break;
  case MAX_SEC_NO_EXIT:   message = "max_sec_no";    break;
  case MACE_SIGINT_EXIT:  message = "mace_sigint";   break;
  case MACE_SIGSEGV_EXIT: message = "mace_sigsegv";  break;
  default: message = "???";
  }
  return message;
}  /* exit_string */

/*************
 *
 *   mace4_exit()
 *
 *************/

void mace4_exit(int exit_code)
{
  if (Opt && flag(Opt->verbose))
    p_mem();

  if (Opt && parm(Opt->report_stderr) > 0)
    fprintf(stderr, "Domain_size=%d. Models=%d. User_CPU=%.2f.\n",
	    Domain_size, Total_models, user_seconds());

  printf("\nUser_CPU=%.2f, System_CPU=%.2f, Wall_clock=%d.\n",
	 user_seconds(), system_seconds(), wallclock());

  if (Total_models == 0)
    printf("\nExiting with failure.\n");
  else
    printf("\nExiting with %d model%s.\n",
	    Total_models, Total_models == 1 ? "" : "s");
  
  fprintf(stderr, "\n------ process %d exit (%s) ------\n",
	  my_process_id(), exit_string(exit_code));
  printf("\nProcess %d exit (%s) %s",
	  my_process_id(), exit_string(exit_code), get_date());
  
  printf("The process finished %s", get_date());
  exit(exit_code);
}  /* mace4_exit */

/*************
 *
 *   initialize_for_search()
 *
 *   This is the initialization that has to be done only once
 *   for a given set of clauses.  It is independent of the
 *   domain size.
 *
 *************/

static
void initialize_for_search(Plist clauses)
{
  int max, i;
  Symbol_data s;

  Mace4_clock = clock_init("Mace4");

  /* In ground clauses, VARIABLEs represent domain elements,
     so from here on, print variables as integers. */

  // set_variable_style(INTEGER_STYLE);

  /* These flags are for ground clause (mclause) literals. */

  Relation_flag = claim_term_flag();
  Negation_flag = claim_term_flag();

  /* Cache some symbol numbers. */

  Eq_sn  = str_to_sn(eq_sym(), 2);
  Or_sn  = str_to_sn(or_sym(), 2);
  Not_sn = str_to_sn(not_sym(), 1);

  /* Set up Symbols list. */

  init_built_in_symbols();  /* =/2 (and maybe others) */

  /* Maybe initialize for arithmetic. */

  if (flag(Opt->arithmetic))
    init_arithmetic();

  Skolems_last = flag(Opt->skolems_last);

  /* Collect data for each symbol. */

  Max_domain_element_in_input = -1;
  i = collect_mace4_syms(clauses, flag(Opt->arithmetic));
  Max_domain_element_in_input = MAX(Max_domain_element_in_input, i);

  if (Max_domain_element_in_input == -1)
    printf("\n%% There are no natural numbers in the input.\n");
  else
    printf("\n%% The largest natural number in the input is %d.\n",
	   Max_domain_element_in_input);

  /* Set up map from ordinary symnums to mace symnums. */

  max = 0;
  i = 0;

  for (s = Symbols; s != NULL; s = s->next) {
    s->mace_sn = i++;
    /* printf("mace symbol: %s/%d\n", sn_to_str(s->sn), sn_to_arity(s->sn)); */
    max = (s->sn > max ? s->sn : max);
  }

  Sn_map_size = max+1;

  Sn_to_mace_sn = malloc(Sn_map_size * sizeof(void *));

  for (i = 0; i < Sn_map_size; i++)
    Sn_to_mace_sn[i] = NULL;

  for (s = Symbols; s != NULL; s = s->next) {
    Sn_to_mace_sn[s->sn] = s;
  }
}  /* initialize_for_search */

/*************
 *
 *   init_for_domain_size()
 *
 *   Given the list of (general) clauses, set up the various data
 *   structures that will be needed for a given domain size.
 *
 *************/

static
void init_for_domain_size(void)
{
  int i, j, nextbase, id;
  Symbol_data s;

  /* Give each symbol its "base" value, which is used to index cells.  */

  nextbase = 0;
  for (s = Symbols; s != NULL; s = s->next) {
    s->base = nextbase;
    nextbase += int_power(Domain_size, s->arity);
  }

  /* Set up the array of domain terms.  All ground terms refer to these. */

  Domain = malloc(Domain_size * sizeof(void *));
  for (i = 0; i < Domain_size; i++)
    Domain[i] = get_variable_term(i);
  
  /* Set up the table of cells. */

  Number_of_cells = nextbase;
  Cells           = malloc(Number_of_cells * sizeof(struct cell));
  Ordered_cells   = malloc(Number_of_cells * sizeof(void *));

  for (id = 0; id < Number_of_cells; id++) {
    struct cell *c = Cells + id;
    int n;
    c->id = id;
    c->occurrences = NULL;
    c->value = NULL;
    c->symbol = find_symbol_node(id);
    c->eterm = decode_eterm_id(id);
    c->max_index = max_index(id, c->symbol);
    n = id_to_domain_size(id);
    c->possible = malloc(n * sizeof(void *));
    for (j = 0; j < n; j++)
      c->possible[j] = Domain[j];  /* really just a flag */
  }

  order_cells(flag(Opt->verbose));
  
  if (flag(Opt->negprop))
    init_negprop_index();
} /* init_for_domain_size */

/*************
 *
 *   built_in_assignments()
 *
 *************/

static
void built_in_assignments(void)
{
  Symbol_data s;
  for (s = Symbols; s != NULL; s = s->next) {
    if (s->attribute == EQUALITY_SYMBOL) {
      int i, j;
      for (i = 0; i < Domain_size; i++)
	for (j = 0; j < Domain_size; j++)
          Cells[X2(s->base,i,j)].value = (Domain[i==j ? 1 : 0]);
    }
  }
}  /* built_in_assignments */

/*************
 *
 *   special_assignments()
 *
 *************/

static
void special_assignments(void)
{
  if (flag(Opt->integer_ring)) {
    /* Fix [+,-,*] as the ring of integers mod domain_size. */
    /* If any of those operations doesn't exist, then ignore it.*/
    Symbol_data s;
    for (s = Symbols; s != NULL; s = s->next) {
      int i, j;
      if (is_symbol(s->sn, "+", 2)) {
	for (i = 0; i < Domain_size; i++)
	  for (j = 0; j < Domain_size; j++)
	    Cells[X2(s->base,i,j)].value = Domain[(i + j) % Domain_size];
      }
      else if (is_symbol(s->sn, "*", 2)) {
	for (i = 0; i < Domain_size; i++)
	  for (j = 0; j < Domain_size; j++)
	    Cells[X2(s->base,i,j)].value = Domain[(i * j) % Domain_size];
      }
      else if (is_symbol(s->sn, "-", 1)) {
	for (i = 0; i < Domain_size; i++)
	  Cells[X1(s->base,i)].value = Domain[(Domain_size - i) % Domain_size];
      }
      else if (is_symbol(s->sn, "--", 2)) {
	for (i = 0; i < Domain_size; i++)
	  for (j = 0; j < Domain_size; j++)
	    Cells[X2(s->base,i,j)].value = Domain[((i + Domain_size) - j) %
						  Domain_size];
      }
    }
  }
  if (flag(Opt->order_domain)) {
    Symbol_data s;
    for (s = Symbols; s != NULL; s = s->next) {
      int i, j;
      if (is_symbol(s->sn, "<", 2)) {
	for (i = 0; i < Domain_size; i++)
	  for (j = 0; j < Domain_size; j++)
	    Cells[X2(s->base,i,j)].value = (Domain[i<j ? 1 : 0]);
      }
      if (is_symbol(s->sn, "<=", 2)) {
	for (i = 0; i < Domain_size; i++)
	  for (j = 0; j < Domain_size; j++)
	    Cells[X2(s->base,i,j)].value = (Domain[i<=j ? 1 : 0]);
      }
    }
  }
}  /* special_assignments */

/*************
 *
 *   check_that_ground_clauses_are_true()
 *
 *************/

static
BOOL check_that_ground_clauses_are_true(void)
{
  Plist g;
  BOOL ok = TRUE;
  for (g = Ground_clauses; g != NULL; g = g->next) {
    Mclause c = g->v;
    if (!c->subsumed) {
      fprintf(stderr, "ERROR, model reported, but clause not true!\n");
      fprintf(stdout, "ERROR, model reported, but clause not true! ");
      p_mclause(c);
      ok = FALSE;
    }
  }
  return ok;
}  /* check_that_ground_clauses_are_true */

/*************
 *
 *   possible_model()
 *
 *************/

static
int possible_model(void)
{
  if (flag(Opt->arithmetic)) {
    if (!check_with_arithmetic(Ground_clauses))
      return SEARCH_GO_NO_MODELS;
  }
  else if (!check_that_ground_clauses_are_true())
    fatal_error("possible_model, bad model found");

  {
    static int next_message = 1;
    Total_models++;
    Mstats.current_models++;

    if (flag(Opt->return_models)) {
      Term modelterm = interp_term();
      Interp model = compile_interp(modelterm, FALSE);
      zap_term(modelterm);
      Models = plist_append(Models, model);
    }

    if (flag(Opt->print_models))
      print_model_standard(stdout, TRUE);
    else if (flag(Opt->print_models_tabular))
      p_model(FALSE);
    else if (next_message == Total_models) {
      printf("\nModel %d has been found.\n", Total_models);
      next_message *= 10;
    }
    fflush(stdout);
    if (parm(Opt->max_models) != -1 && Total_models >= parm(Opt->max_models))
      return SEARCH_MAX_MODELS;
    else
      return SEARCH_GO_MODELS;
  }
}  /* possible_model */

/*************
 *
 *   mace_megs()
 *
 *************/

static
int mace_megs(void)
{
  return (megs_malloced() - Start_megs) + (estack_bytes() / (1024*1024));
}  /* mace_megs */

/*************
 *
 *   check_time_memory()
 *
 *************/

static
int check_time_memory(void)
{
  static int Next_report;

  double seconds = user_seconds();
  int max_seconds = parm(Opt->max_seconds);
  int max_seconds_per = parm(Opt->max_seconds_per);
  int max_megs = parm(Opt->max_megs);
  int report = parm(Opt->report_stderr);

  if (max_seconds != -1 && seconds - Start_seconds > max_seconds)
    return SEARCH_MAX_TOTAL_SECONDS;
  else if (max_seconds_per != -1 &&
	   seconds - Start_domain_seconds > parm(Opt->max_seconds_per))
    return SEARCH_MAX_DOMAIN_SECONDS;
  else if (max_megs != -1 && mace_megs() > parm(Opt->max_megs))
    return SEARCH_MAX_MEGS;
  else {
    if (report > 0) {
      if (Next_report == 0)
	Next_report = parm(Opt->report_stderr);
      if (seconds >= Next_report) {
	fprintf(stderr, "Domain_size=%d. Models=%d. User_CPU=%.2f.\n", Domain_size, Total_models, seconds);
	fflush(stderr);
	while (seconds >= Next_report)
	  Next_report += report;
      }      
    }
    return SEARCH_GO_NO_MODELS;
  }
}  /* check_time_memory */

/*************
 *
 *   mace4_skolem_check()
 *
 *************/

static
BOOL mace4_skolem_check(int id)
{
  /* Should we keep going w.r.t. the Skolem restriction? */
  if (!flag(Opt->skolems_last))
    return TRUE;
  else if (Cells[id].symbol->attribute == SKOLEM_SYMBOL) {
    printf("pruning\n");
    return FALSE;
  }
  else
    return TRUE;
}  /* mace4_skolem_check */

/*************
 *
 *   p_possible_values()
 *
 *************/

#if 0
static
void p_possible_values(void)
{
  int i;
  for (i = 0; i < Number_of_cells; i++) {
    if (Cells[i].symbol->attribute == ORDINARY_SYMBOL) {
      int j;
      printf("Cell %d: ", i);
      for (j = 0; j < id_to_domain_size(i); j++) {
	if (Cells[i].possible[j] != NULL)
	  printf(" %d", j);
      }
      printf("\n");
    }
  }
}  /* p_possible_values */
#endif

/*************
 *
 *   search()
 *
 *   Max_constrained is the maximum constrained domain element
 *   (or -1 is none is constrained).  Greater domain elements
 *   can all be considered symmetric.  An element can become
 *   constrained in two ways:  (1) it is an index of some selected
 *   cell, or (2) it is the value assigned to some selected cell.
 *   (Propagation does not constrain elements.  This might need
 *   careful justification.)
 *
 *   To apply the least number heuristic, we consider values
 *   0 ... MIN(max_constrained+1, Domain_size-1).
 *
 *   To make this effective, we should keep max_constrained as low as
 *   possible by selecting cells with maximum index <= max_constrained.
 *
 *   return:
 *     SEARCH_GO_MODELS
 *     SEARCH_GO_NO_MODELS
 *     SEARCH_MAX_MODELS
 *     SEARCH_MAX_MEGS
 *     SEARCH_MAX_TOTAL_SECONDS
 *     SEARCH_MAX_DOMAIN_SECONDS
 *
 *************/

static
int search(int max_constrained, int depth)
{
  int id;
  BOOL go;
  int rc = check_time_memory();
  if (rc != SEARCH_GO_NO_MODELS)
    return rc;
  else {
    id = select_cell(max_constrained);
    if (id == -1) {
      rc = possible_model();
      return rc;
    }
    else {
      int i, last;  /* we'll do 0 .. last */
      int x = Cells[id].max_index;
      max_constrained = MAX(max_constrained, x);
      Mstats.selections++;

      if (flag(Opt->trace)) {
	printf("select: ");
	p_model(FALSE);
	/* p_possible_values(); */
      }
	
      if (Cells[id].symbol->type == RELATION)
	last = 1;
      else if (flag(Opt->lnh))
	last = MIN(max_constrained+1, Domain_size-1);
      else
	last = Domain_size-1;

      for (i = 0, go = TRUE; i <= last && go; i++) {
	Estack stk;
	Mstats.assignments++;

	if (flag(Opt->trace)) {
	  printf("assign: ");
	  fwrite_term(stdout, Cells[id].eterm);
	  printf("=%d (%d) depth=%d\n", i, last, depth);
	}
	
	stk = assign_and_propagate(id, Domain[i]);

	if (stk != NULL) {
	  /* no contradiction found during propagation, so we recurse */
	  rc = search(MAX(max_constrained, i), depth+1);
	  /* undo assign_and_propagate changes */
	  restore_from_stack(stk);
	  if (rc == SEARCH_GO_MODELS)
	    go = mace4_skolem_check(id);
	  else
	    go = (rc == SEARCH_GO_NO_MODELS);
	}
      }
      return rc;
    }
  }
}  /* search */

/*************
 *
 *   mace4n() -- look for a model of a specific size
 *
 *************/

static
int mace4n(Plist clauses, int order)
{
  Plist p, g;
  int i, rc;
  Mstate initial_state = get_mstate();

  Variable_style save_style = variable_style();
  set_variable_style(INTEGER_STYLE);

  if (Max_domain_element_in_input >= order) {
    if (flag(Opt->arithmetic)) {
      if (!ok_for_arithmetic(clauses, order))
	return SEARCH_DOMAIN_OUT_OF_RANGE;
    }
    else
      return SEARCH_DOMAIN_OUT_OF_RANGE;
  }

  Domain_size = order;

  init_for_domain_size();

  built_in_assignments();  /* Fill out equality table (and maybe others). */

  special_assignments();  /* assignments determined by options */

  /* Instantiate clauses over the domain.  This also 
     (1) makes any domain element constants into real domain elements,
     (2) applies OR, NOT, and EQ simplification, and
     (3) does unit propagation (which pushes events onto initial_state->stack).
     Do the units first, then the 2-clauses, then the rest. */

  for (p = clauses; initial_state->ok && p != NULL; p = p->next)
    if (number_of_literals(p->v) < 2)
      generate_ground_clauses(p->v, initial_state);

  for (p = clauses; initial_state->ok && p != NULL; p = p->next)
    if (number_of_literals(p->v) == 2)
      generate_ground_clauses(p->v, initial_state);

  for (p = clauses; initial_state->ok && p != NULL; p = p->next)
    if (number_of_literals(p->v) > 2)
      generate_ground_clauses(p->v, initial_state);

  /* The preceding calls push propagation events onto initial_state->stack.
     We won't have to undo those initial events during the search,
     but we can undo them after the search.
  */

  if (flag(Opt->verbose)) {
    printf("\nInitial partial model:\n");
    p_model(FALSE);
    fflush(stdout);
  }

  /* Here we go! */

  if (initial_state->ok)
    rc = search(Max_domain_element_in_input, 0);
  else
    rc = SEARCH_GO_NO_MODELS;  /* contradiction in initial state */

  /* Free all of the memory associated with the current domain size. */

  restore_from_stack(initial_state->stack);
  free_mstate(initial_state);

  if (flag(Opt->negprop))
    free_negprop_index();

  free(Ordered_cells);
  Ordered_cells = NULL;

  for (i = 0; i < Number_of_cells; i++) {
    zap_mterm(Cells[i].eterm);
    free(Cells[i].possible);
  }
  free(Cells);
  Cells = NULL;

  for (i = 0; i < Domain_size; i++)
    zap_term(Domain[i]);
  free(Domain);
  Domain = NULL;

  for (g = Ground_clauses; g != NULL; g = g->next)
    zap_mclause(g->v);
  zap_plist(Ground_clauses);
  Ground_clauses = NULL;

  set_variable_style(save_style);
  return rc;
}  /* mace4n */

/*************
 *
 *   iterate_ok()
 *
 *************/

static
BOOL iterate_ok(int n, char *class)
{
  if (str_ident(class, "all"))
    return TRUE;
  else if (str_ident(class, "evens"))
    return n % 2 == 0;
  else if (str_ident(class, "odds"))
    return n % 2 == 1;
  else if (str_ident(class, "primes"))
    return prime(n);
  else if (str_ident(class, "nonprimes"))
    return !prime(n);
  else {
    fatal_error("iterate_ok, unknown class");
    return FALSE;   /* to please compiler */
  }
}  /* iterate_ok */

/*************
 *
 *   next_domain_size()
 *
 *************/

static
int next_domain_size(n)
{
  int top = (parm(Opt->end_size) == -1 ? INT_MAX : parm(Opt->end_size));
      
  if (n == 0)
    n = parm(Opt->start_size);  /* first call */
  else
    n += parm(Opt->increment);

  while (!iterate_ok(n, stringparm1(Opt->iterate)))
    n += parm(Opt->increment);

  return (n > top ? -1 : n);
}  /* next_domain_size */

/*************
 *
 *   mace4()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Mace_results mace4(Plist clauses, Mace_options opt)
{
  int n, rc;
  Mace_results results = malloc(sizeof(struct mace_results));

  disable_max_megs();  /* mace4 does its own max_megs check */
  Start_seconds = user_seconds();
  Start_megs = megs_malloced();

  Opt = opt;  /* put options into a global variable */
  initialize_for_search(clauses);

  n = next_domain_size(0);  /* returns -1 if we're done */
  rc = SEARCH_GO_NO_MODELS;

  while (n >= 2 && (rc == SEARCH_GO_NO_MODELS || rc == SEARCH_GO_MODELS)) {
    char str[20];
    sprintf(str, "DOMAIN SIZE %d", n);
    print_separator(stdout, str, TRUE);
    fflush(stdout);
    fprintf(stderr,"\n=== Mace4 starting on domain size %d. ===\n",n);

    Start_domain_seconds = user_seconds();
    clock_start(Mace4_clock);
    rc = mace4n(clauses, n);
    if (rc == SEARCH_MAX_DOMAIN_SECONDS) {
      printf("\n====== Domain size %d terminated by max_seconds_per. ======\n",n);
      rc = SEARCH_GO_NO_MODELS;
    }
    else if (rc == SEARCH_DOMAIN_OUT_OF_RANGE) {
      printf("\n====== Domain size %d skipped because domain elememt too big. ======\n",n);
      rc = SEARCH_GO_NO_MODELS;
    }
    clock_stop(Mace4_clock);
    p_stats();
    reset_current_stats();
    clock_reset(Mace4_clock);
    n = next_domain_size(n);  /* returns -1 if we're done */
  }

  /* free memory used for all domain sizes */
  free_estack_memory();
  free(Sn_to_mace_sn);
  Sn_to_mace_sn = NULL;

  results->success = Total_models != 0;
  results->models = Models;  /* NULL if no models or not collecting models */
  results->user_seconds = user_seconds() - Start_seconds;

  if (rc == SEARCH_MAX_MODELS)
    results->return_code = MAX_MODELS_EXIT;
  else if (rc == SEARCH_GO_MODELS || rc == SEARCH_GO_NO_MODELS)
    results->return_code = Total_models==0 ? EXHAUSTED_EXIT : ALL_MODELS_EXIT;
  else if (rc == SEARCH_MAX_TOTAL_SECONDS)
    results->return_code = Total_models==0 ? MAX_SEC_NO_EXIT : MAX_SEC_YES_EXIT;
  else if (rc == SEARCH_MAX_MEGS)
    results->return_code = Total_models==0 ? MAX_MEGS_NO_EXIT : MAX_MEGS_YES_EXIT;
  else
    fatal_error("mace4: unknown return code");

  enable_max_megs();
  return results;
}  /* mace4 */
