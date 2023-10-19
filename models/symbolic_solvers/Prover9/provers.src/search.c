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

#include "search.h"

// system includes

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <float.h>
#include <math.h>
#include <setjmp.h>  /* Yikes! */

// Private definitions and types

static jmp_buf Jump_env;                 // for setjmp/longjmp

static Prover_options Opt;               // Prover9 options
static struct prover_attributes Att;     // Prover9 accepted attributes
static struct prover_stats Stats;        // Prover9 statistics
static struct prover_clocks Clocks;      // Prover9 clocks

// The following is a global structure for this file.

static struct {

  // basic clause lists

  Clist sos;
  Clist usable;
  Clist demods;
  Clist hints;

  // other lists

  Plist actions;
  Plist weights;
  Plist kbo_weights;
  Plist interps;
  Plist given_selection;
  Plist keep_rules;
  Plist delete_rules;

  // auxiliary clause lists

  Clist limbo;
  Clist disabled;
  Plist empties;

  // indexing

  Lindex clashable_idx;  // literal index for resolution rules
  BOOL use_clash_idx;    // GET RID OF THIS VARIABLE!!

  // basic properties of usable+sos

  BOOL horn, unit, equality, number_of_clauses, number_of_neg_clauses;

  // other stuff

  Plist desc_to_be_disabled;   // Descendents of these to be disabled
  Plist cac_clauses;           // Clauses that trigger back CAC check

  BOOL searching;      // set to TRUE when first given is selected
  BOOL initialized;    // has this structure been initialized?
  double start_time;   // when was it initialized? 
  int start_ticks;     // quasi-clock that times the same for all machines

  int return_code;     // result of search
} Glob;

// How many statics are to be output?

/*************
 *
 *    init_prover_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Prover_options init_prover_options(void)
{
  Prover_options p = calloc(1, sizeof(struct prover_options));
  // FLAGS:
  //   internal name                    external name            default

  // The following are now in ../ladr/std_options.c.
  // ?? = init_flag("prolog_style_variables", FALSE);
  // ?? = init_flag("clocks",                 FALSE);

  p->binary_resolution      = init_flag("binary_resolution",      FALSE);
  p->neg_binary_resolution  = init_flag("neg_binary_resolution",  FALSE);
  p->hyper_resolution       = init_flag("hyper_resolution",       FALSE);
  p->pos_hyper_resolution   = init_flag("pos_hyper_resolution",   FALSE);
  p->neg_hyper_resolution   = init_flag("neg_hyper_resolution",   FALSE);
  p->ur_resolution          = init_flag("ur_resolution",          FALSE);
  p->pos_ur_resolution      = init_flag("pos_ur_resolution",      FALSE);
  p->neg_ur_resolution      = init_flag("neg_ur_resolution",      FALSE);
  p->paramodulation         = init_flag("paramodulation",         FALSE);
  p->eval_rewrite           = init_flag("eval_rewrite",           FALSE);

  p->ordered_res            = init_flag("ordered_res",            TRUE);
  p->check_res_instances    = init_flag("check_res_instances",    FALSE);
  p->ordered_para           = init_flag("ordered_para",           TRUE);
  p->check_para_instances   = init_flag("check_para_instances",   FALSE);
  p->para_units_only        = init_flag("para_units_only",        FALSE);
  p->para_from_vars         = init_flag("para_from_vars",         TRUE);
  p->para_into_vars         = init_flag("para_into_vars",         FALSE);
  p->para_from_small        = init_flag("para_from_small",        FALSE);
  p->basic_paramodulation   = init_flag("basic_paramodulation",   FALSE);
  p->initial_nuclei         = init_flag("initial_nuclei",         FALSE);

  p->process_initial_sos    = init_flag("process_initial_sos",     TRUE);
  p->back_demod             = init_flag("back_demod",              TRUE);
  p->lex_dep_demod          = init_flag("lex_dep_demod",           TRUE);
  p->lex_dep_demod_sane     = init_flag("lex_dep_demod_sane",      TRUE);
  p->safe_unit_conflict     = init_flag("safe_unit_conflict",     FALSE);
  p->reuse_denials          = init_flag("reuse_denials",          FALSE);
  p->back_subsume           = init_flag("back_subsume",            TRUE);
  p->unit_deletion          = init_flag("unit_deletion",          FALSE);
  p->factor                 = init_flag("factor",                 FALSE);
  p->cac_redundancy         = init_flag("cac_redundancy",          TRUE);
  p->degrade_hints          = init_flag("degrade_hints",           TRUE);
  p->limit_hint_matchers    = init_flag("limit_hint_matchers",    FALSE);
  p->back_demod_hints       = init_flag("back_demod_hints",        TRUE);
  p->collect_hint_labels    = init_flag("collect_hint_labels",    FALSE);
  p->dont_flip_input        = init_flag("dont_flip_input",        FALSE);

  p->echo_input             = init_flag("echo_input",              TRUE);
  p->bell                   = init_flag("bell",                    TRUE);
  p->quiet                  = init_flag("quiet",                  FALSE);
  p->print_initial_clauses  = init_flag("print_initial_clauses",   TRUE);
  p->print_given            = init_flag("print_given",             TRUE);
  p->print_gen              = init_flag("print_gen",              FALSE);
  p->print_kept             = init_flag("print_kept",             FALSE);
  p->print_labeled          = init_flag("print_labeled",          FALSE);
  p->print_proofs           = init_flag("print_proofs",            TRUE);
  p->default_output         = init_flag("default_output",          TRUE);
  p->print_clause_properties= init_flag("print_clause_properties",FALSE);

  p->expand_relational_defs = init_flag("expand_relational_defs", FALSE);
  p->predicate_elim         = init_flag("predicate_elim",          TRUE);
  p->inverse_order          = init_flag("inverse_order",           TRUE);
  p->sort_initial_sos       = init_flag("sort_initial_sos",       FALSE);
  p->restrict_denials       = init_flag("restrict_denials",       FALSE);

  p->input_sos_first        = init_flag("input_sos_first",         TRUE);
  p->breadth_first          = init_flag("breadth_first",          FALSE);
  p->lightest_first         = init_flag("lightest_first",         FALSE);
  p->random_given           = init_flag("random_given",           FALSE);
  p->breadth_first_hints    = init_flag("breadth_first_hints",    FALSE);
  p->default_parts          = init_flag("default_parts",           TRUE);

  p->automatic              = init_flag("auto",                    TRUE);
  p->auto_setup             = init_flag("auto_setup",              TRUE);
  p->auto_limits            = init_flag("auto_limits",             TRUE);
  p->auto_denials           = init_flag("auto_denials",            TRUE);
  p->auto_inference         = init_flag("auto_inference",          TRUE);
  p->auto_process           = init_flag("auto_process",            TRUE);
  p->auto2                  = init_flag("auto2",                  FALSE);
  p->raw                    = init_flag("raw",                    FALSE);
  p->production             = init_flag("production",             FALSE);

  p->lex_order_vars         = init_flag("lex_order_vars",         FALSE);

  // PARMS:
  //  internal name               external name      default    min      max

  p->max_given =        init_parm("max_given",            -1,     -1,INT_MAX);
  p->max_kept =         init_parm("max_kept",             -1,     -1,INT_MAX);
  p->max_proofs =       init_parm("max_proofs",            1,     -1,INT_MAX);
  p->max_megs =         init_parm("max_megs",            500,     -1,INT_MAX);
  p->max_seconds =      init_parm("max_seconds",          -1,     -1,INT_MAX);
  p->max_minutes =      init_parm("max_minutes",          -1,     -1,INT_MAX);
  p->max_hours =        init_parm("max_hours",          -1,     -1,INT_MAX);
  p->max_days =         init_parm("max_days",          -1,     -1,INT_MAX);

  p->new_constants =    init_parm("new_constants",         0,     -1,INT_MAX);
  p->para_lit_limit =   init_parm("para_lit_limit",       -1,     -1,INT_MAX);
  p->ur_nucleus_limit = init_parm("ur_nucleus_limit",     -1,     -1,INT_MAX);

  p->fold_denial_max =  init_parm("fold_denial_max",       0,     -1,INT_MAX);

  p->pick_given_ratio  = init_parm("pick_given_ratio",    -1,     -1,INT_MAX);
  p->hints_part        = init_parm("hints_part",     INT_MAX,      0,INT_MAX);
  p->age_part          = init_parm("age_part",             1,      0,INT_MAX);
  p->weight_part       = init_parm("weight_part",          0,      0,INT_MAX);
  p->false_part        = init_parm("false_part",           4,      0,INT_MAX);
  p->true_part         = init_parm("true_part",            4,      0,INT_MAX);
  p->random_part       = init_parm("random_part",          0,      0,INT_MAX);
  p->random_seed       = init_parm("random_seed",          0,     -1,INT_MAX);
  p->eval_limit        = init_parm("eval_limit",        1024,     -1,INT_MAX);
  p->eval_var_limit    = init_parm("eval_var_limit",      -1,     -1,INT_MAX);

  p->max_depth =        init_parm("max_depth",            -1,     -1,INT_MAX);
  p->lex_dep_demod_lim =init_parm("lex_dep_demod_lim",    11,     -1,INT_MAX);
  p->max_literals =     init_parm("max_literals",         -1,     -1,INT_MAX);
  p->max_vars =         init_parm("max_vars",             -1,     -1,INT_MAX);
  p->demod_step_limit = init_parm("demod_step_limit",   1000,     -1,INT_MAX);
  p->demod_increase_limit = init_parm("demod_increase_limit",1000,-1,INT_MAX);
  p->backsub_check    = init_parm("backsub_check",       500,     -1,INT_MAX);

  p->variable_weight =  init_floatparm("variable_weight",       1.0,-DBL_LARGE,DBL_LARGE);
  p->constant_weight =  init_floatparm("constant_weight",       1.0,-DBL_LARGE,DBL_LARGE);
  p->not_weight =       init_floatparm("not_weight",            0.0,-DBL_LARGE,DBL_LARGE);
  p->or_weight =        init_floatparm("or_weight",             0.0,-DBL_LARGE,DBL_LARGE);
  p->sk_constant_weight=init_floatparm("sk_constant_weight",    1.0,-DBL_LARGE,DBL_LARGE);
  p->prop_atom_weight = init_floatparm("prop_atom_weight",      1.0,-DBL_LARGE,DBL_LARGE);
  p->nest_penalty =     init_floatparm("nest_penalty",          0.0,     0.0,DBL_LARGE);
  p->depth_penalty =    init_floatparm("depth_penalty",         0.0,-DBL_LARGE,DBL_LARGE);
  p->var_penalty =      init_floatparm("var_penalty",           0.0,-DBL_LARGE,DBL_LARGE);
  p->default_weight =   init_floatparm("default_weight",  DBL_LARGE,-DBL_LARGE,DBL_LARGE);
  p->complexity =       init_floatparm("complexity",            0.0,-DBL_LARGE,DBL_LARGE);

  p->sos_limit =        init_parm("sos_limit",         20000,     -1,INT_MAX);
  p->sos_keep_factor =  init_parm("sos_keep_factor",       3,      2,10);
  p->min_sos_limit =    init_parm("min_sos_limit",         0,      0,INT_MAX);
  p->lrs_interval =     init_parm("lrs_interval",         50,      1,INT_MAX);
  p->lrs_ticks =        init_parm("lrs_ticks",            -1,     -1,INT_MAX);

  p->report =           init_parm("report",               -1,     -1,INT_MAX);
  p->report_stderr =    init_parm("report_stderr",        -1,     -1,INT_MAX);

  // FLOATPARMS:
  //  internal name      external name           default    min      max )

  p->max_weight =   init_floatparm("max_weight",  100.0, -DBL_LARGE, DBL_LARGE);

  // STRINGPARMS:
  // (internal-name, external-name, number-of-strings, str1, str2, ... )
  // str1 is always the default

  p->order = init_stringparm("order", 3,
			     "lpo",
			     "rpo",
			     "kbo");

  p->eq_defs = init_stringparm("eq_defs", 3,
			       "unfold",
			       "fold",
			       "pass");

  p->literal_selection = init_stringparm("literal_selection", 3,
					 "max_negative",
					 "all_negative",
					 "none");

  p->stats = init_stringparm("stats", 4,
			     "lots",
			     "some",
			     "all",
			     "none");

  p->multiple_interps = init_stringparm("multiple_interps", 2,
					"false_in_all",
					"false_in_some");

  // Flag and parm Dependencies.  These cause other flags and parms
  // to be changed.  The changes happen immediately and can be undone
  // by later settings in the input.
  // DEPENDENCIES ARE NOT APPLIED TO DEFAULT SETTINGS!!!

  parm_parm_dependency(p->max_minutes, p->max_seconds,         60, TRUE);
  parm_parm_dependency(p->max_hours,   p->max_seconds,       3600, TRUE);
  parm_parm_dependency(p->max_days,    p->max_seconds,      86400, TRUE);

  flag_parm_dependency(p->para_units_only,    TRUE,  p->para_lit_limit,      1);
  flag_parm_dep_default(p->para_units_only,   FALSE, p->para_lit_limit);

  flag_flag_dependency(p->hyper_resolution, TRUE,  p->pos_hyper_resolution, TRUE);
  flag_flag_dependency(p->hyper_resolution, FALSE, p->pos_hyper_resolution, FALSE);

  flag_flag_dependency(p->ur_resolution, TRUE,  p->pos_ur_resolution, TRUE);
  flag_flag_dependency(p->ur_resolution, TRUE,  p->neg_ur_resolution, TRUE);
  flag_flag_dependency(p->ur_resolution, FALSE,  p->pos_ur_resolution, FALSE);
  flag_flag_dependency(p->ur_resolution, FALSE,  p->neg_ur_resolution, FALSE);

  flag_parm_dependency(p->lex_dep_demod, FALSE, p->lex_dep_demod_lim, 0);
  flag_parm_dependency(p->lex_dep_demod,  TRUE, p->lex_dep_demod_lim, 11);

  /***********************/

  parm_parm_dependency(p->pick_given_ratio, p->age_part,          1, FALSE);
  parm_parm_dependency(p->pick_given_ratio, p->weight_part,       1,  TRUE);
  parm_parm_dependency(p->pick_given_ratio, p->false_part,        0, FALSE);
  parm_parm_dependency(p->pick_given_ratio, p->true_part,         0, FALSE);
  parm_parm_dependency(p->pick_given_ratio, p->random_part,       0, FALSE);

  flag_parm_dependency(p->lightest_first,    TRUE,  p->weight_part,     1);
  flag_parm_dependency(p->lightest_first,    TRUE,  p->age_part,        0);
  flag_parm_dependency(p->lightest_first,    TRUE,  p->false_part,      0);
  flag_parm_dependency(p->lightest_first,    TRUE,  p->true_part,       0);
  flag_parm_dependency(p->lightest_first,    TRUE,  p->random_part,     0);
  flag_flag_dependency(p->lightest_first,   FALSE,  p->default_parts, TRUE);

  flag_parm_dependency(p->random_given,    TRUE,  p->weight_part,     0);
  flag_parm_dependency(p->random_given,    TRUE,  p->age_part,        0);
  flag_parm_dependency(p->random_given,    TRUE,  p->false_part,      0);
  flag_parm_dependency(p->random_given,    TRUE,  p->true_part,       0);
  flag_parm_dependency(p->random_given,    TRUE,  p->random_part,     1);
  flag_flag_dependency(p->random_given,   FALSE,  p->default_parts, TRUE);

  flag_parm_dependency(p->breadth_first,    TRUE,  p->age_part,        1);
  flag_parm_dependency(p->breadth_first,    TRUE,  p->weight_part,     0);
  flag_parm_dependency(p->breadth_first,    TRUE,  p->false_part,      0);
  flag_parm_dependency(p->breadth_first,    TRUE,  p->true_part,       0);
  flag_parm_dependency(p->breadth_first,    TRUE,  p->random_part,     0);
  flag_flag_dependency(p->breadth_first,    FALSE, p->default_parts, TRUE);

  /* flag_parm_dependency(p->default_parts, TRUE,  p->hints_part, INT_MAX); */
  flag_parm_dependency(p->default_parts,    TRUE,  p->age_part,          1);
  flag_parm_dependency(p->default_parts,    TRUE,  p->weight_part,       0);
  flag_parm_dependency(p->default_parts,    TRUE,  p->false_part,        4);
  flag_parm_dependency(p->default_parts,    TRUE,  p->true_part,         4);
  flag_parm_dependency(p->default_parts,    TRUE,  p->random_part,       0);

  /* flag_parm_dependency(p->default_parts,    FALSE,  p->hints_part,  0); */
  flag_parm_dependency(p->default_parts,    FALSE,  p->age_part,         0);
  flag_parm_dependency(p->default_parts,    FALSE,  p->weight_part,      0);
  flag_parm_dependency(p->default_parts,    FALSE,  p->false_part,       0);
  flag_parm_dependency(p->default_parts,    FALSE,  p->true_part,        0);
  flag_parm_dependency(p->default_parts,    FALSE,  p->random_part,      0);

  /***********************/

  flag_flag_dependency(p->default_output, TRUE, p->quiet,               FALSE);
  flag_flag_dependency(p->default_output, TRUE, p->echo_input,           TRUE);
  flag_flag_dependency(p->default_output, TRUE, p->print_initial_clauses,TRUE);
  flag_flag_dependency(p->default_output, TRUE, p->print_given,          TRUE);
  flag_flag_dependency(p->default_output, TRUE, p->print_proofs,         TRUE);
  flag_stringparm_dependency(p->default_output, TRUE, p->stats,        "lots");

  flag_flag_dependency(p->default_output, TRUE, p->print_kept,          FALSE);
  flag_flag_dependency(p->default_output, TRUE, p->print_gen,           FALSE);

  // auto_setup

  flag_flag_dependency(p->auto_setup,  TRUE, p->predicate_elim,    TRUE);
  flag_stringparm_dependency(p->auto_setup, TRUE, p->eq_defs,    "unfold");

  flag_flag_dependency(p->auto_setup,  FALSE, p->predicate_elim,    FALSE);
  flag_stringparm_dependency(p->auto_setup, FALSE, p->eq_defs,   "pass");

  // auto_limits

  flag_floatparm_dependency(p->auto_limits,  TRUE, p->max_weight,    100.0);
  flag_parm_dependency(p->auto_limits,       TRUE, p->sos_limit,     20000);

  flag_floatparm_dependency(p->auto_limits, FALSE, p->max_weight, DBL_LARGE);
  flag_parm_dependency(p->auto_limits,      FALSE, p->sos_limit,         -1);

  // automatic

  flag_flag_dependency(p->automatic,       TRUE, p->auto_inference,     TRUE);
  flag_flag_dependency(p->automatic,       TRUE, p->auto_setup,         TRUE);
  flag_flag_dependency(p->automatic,       TRUE, p->auto_limits,        TRUE);
  flag_flag_dependency(p->automatic,       TRUE, p->auto_denials,       TRUE);
  flag_flag_dependency(p->automatic,       TRUE, p->auto_process,       TRUE);

  flag_flag_dependency(p->automatic,       FALSE, p->auto_inference,    FALSE);
  flag_flag_dependency(p->automatic,       FALSE, p->auto_setup,        FALSE);
  flag_flag_dependency(p->automatic,       FALSE, p->auto_limits,       FALSE);
  flag_flag_dependency(p->automatic,       FALSE, p->auto_denials,      FALSE);
  flag_flag_dependency(p->automatic,       FALSE, p->auto_process,      FALSE);

  // auto2  (also triggered by -x on the command line)

  flag_flag_dependency(p->auto2, TRUE, p->automatic,                 TRUE);
  flag_parm_dependency(p->auto2, TRUE, p->new_constants,            1);
  flag_parm_dependency(p->auto2, TRUE, p->fold_denial_max,          3);
  flag_floatparm_dependency(p->auto2, TRUE, p->max_weight,      200.0);
  flag_parm_dependency(p->auto2, TRUE, p->nest_penalty,             1);
  flag_parm_dependency(p->auto2, TRUE, p->sk_constant_weight,       0);
  flag_parm_dependency(p->auto2, TRUE, p->prop_atom_weight,         5);
  flag_flag_dependency(p->auto2, TRUE, p->sort_initial_sos,       TRUE);
  flag_parm_dependency(p->auto2, TRUE, p->sos_limit,                -1);
  flag_parm_dependency(p->auto2, TRUE, p->lrs_ticks,              3000);
  flag_parm_dependency(p->auto2, TRUE, p->max_megs,                400);
  flag_stringparm_dependency(p->auto2, TRUE, p->stats,          "some");
  flag_flag_dependency(p->auto2, TRUE, p->echo_input,            FALSE);
  flag_flag_dependency(p->auto2, TRUE, p->quiet,                  TRUE);
  flag_flag_dependency(p->auto2, TRUE, p->print_initial_clauses, FALSE);
  flag_flag_dependency(p->auto2, TRUE, p->print_given,           FALSE);

  flag_flag_dep_default(p->auto2, FALSE, p->automatic);
  flag_parm_dep_default(p->auto2, FALSE, p->new_constants);
  flag_parm_dep_default(p->auto2, FALSE, p->fold_denial_max);
  flag_floatparm_dep_default(p->auto2, FALSE, p->max_weight);
  flag_parm_dep_default(p->auto2, FALSE, p->nest_penalty);
  flag_parm_dep_default(p->auto2, FALSE, p->sk_constant_weight);
  flag_parm_dep_default(p->auto2, FALSE, p->prop_atom_weight);
  flag_flag_dep_default(p->auto2, FALSE, p->sort_initial_sos);
  flag_parm_dep_default(p->auto2, FALSE, p->sos_limit);
  flag_parm_dep_default(p->auto2, FALSE, p->lrs_ticks);
  flag_parm_dep_default(p->auto2, FALSE, p->max_megs);
  flag_stringparm_dep_default(p->auto2, FALSE, p->stats);
  flag_flag_dep_default(p->auto2, FALSE, p->echo_input);
  flag_flag_dep_default(p->auto2, FALSE, p->quiet);
  flag_flag_dep_default(p->auto2, FALSE, p->print_initial_clauses);
  flag_flag_dep_default(p->auto2, FALSE, p->print_given);

  // raw

  flag_flag_dependency(p->raw, TRUE, p->automatic,           FALSE);
  flag_flag_dependency(p->raw, TRUE, p->ordered_res,         FALSE);
  flag_flag_dependency(p->raw, TRUE, p->ordered_para,        FALSE);
  flag_flag_dependency(p->raw, TRUE, p->para_into_vars,      TRUE);
  flag_flag_dependency(p->raw, TRUE, p->para_from_small,     TRUE);
  flag_flag_dependency(p->raw, TRUE, p->ordered_para,        FALSE);
  flag_flag_dependency(p->raw, TRUE, p->back_demod,          FALSE);
  flag_flag_dependency(p->raw, TRUE, p->cac_redundancy,      FALSE);
  flag_parm_dependency(p->raw, TRUE, p->backsub_check,     INT_MAX);
  flag_flag_dependency(p->raw, TRUE, p->lightest_first,       TRUE);
  flag_stringparm_dependency(p->raw, TRUE, p->literal_selection, "none");
  
  flag_flag_dep_default(p->raw, FALSE, p->automatic);
  flag_flag_dep_default(p->raw, FALSE, p->ordered_res);
  flag_flag_dep_default(p->raw, FALSE, p->ordered_para);
  flag_flag_dep_default(p->raw, FALSE, p->para_into_vars);
  flag_flag_dep_default(p->raw, FALSE, p->para_from_small);
  flag_flag_dep_default(p->raw, FALSE, p->ordered_para);
  flag_flag_dep_default(p->raw, FALSE, p->back_demod);
  flag_flag_dep_default(p->raw, FALSE, p->cac_redundancy);
  flag_parm_dep_default(p->raw, FALSE, p->backsub_check);
  flag_flag_dep_default(p->raw, FALSE, p->lightest_first);
  flag_stringparm_dep_default(p->raw, FALSE, p->literal_selection);

  // production mode

  flag_flag_dependency(p->production,   TRUE,  p->raw,               TRUE);
  flag_flag_dependency(p->production,   TRUE,  p->eval_rewrite,      TRUE);
  flag_flag_dependency(p->production,   TRUE,  p->hyper_resolution,  TRUE);
  flag_flag_dependency(p->production,   TRUE,  p->back_subsume,     FALSE);
  
  return p;
  
}  // init_prover_options

/*************
 *
 *   init_prover_attributes()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_prover_attributes(void)
{
  Att.label            = register_attribute("label",        STRING_ATTRIBUTE);
  Att.bsub_hint_wt     = register_attribute("bsub_hint_wt",    INT_ATTRIBUTE);
  Att.answer           = register_attribute("answer",         TERM_ATTRIBUTE);
  Att.properties       = register_attribute("props",          TERM_ATTRIBUTE);

  Att.action           = register_attribute("action",         TERM_ATTRIBUTE);
  Att.action2          = register_attribute("action2",        TERM_ATTRIBUTE);

  declare_term_attribute_inheritable(Att.answer);
  declare_term_attribute_inheritable(Att.action2);
}  // Init_prover_attributes

/*************
 *
 *   get_attrib_id()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int get_attrib_id(char *str)
{
  if (str_ident(str, "label"))
    return Att.label;
  else if (str_ident(str, "bsub_hint_wt"))
    return Att.bsub_hint_wt;
  else if (str_ident(str, "answer"))
    return Att.answer;
  else if (str_ident(str, "action"))
    return Att.action;
  else if (str_ident(str, "action2"))
    return Att.action2;
  else {
    fatal_error("get_attrib_id, unknown attribute string");
    return -1;
  }
}  /* get_attrib_id */

/*************
 *
 *   update_stats()
 *
 *************/

static
void update_stats(void)
{
  Stats.demod_attempts = demod_attempts() + fdemod_attempts();
  Stats.demod_rewrites = demod_rewrites() + fdemod_rewrites();
  Stats.res_instance_prunes = res_instance_prunes();
  Stats.para_instance_prunes = para_instance_prunes();
  Stats.basic_para_prunes = basic_paramodulation_prunes();
  Stats.sos_removed = 0; // control_sos_removed();
  Stats.nonunit_fsub = nonunit_fsub_tests();
  Stats.nonunit_bsub = nonunit_bsub_tests();
  Stats.usable_size = Glob.usable ? Glob.usable->length : 0;
  Stats.sos_size = Glob.sos ? Glob.sos->length : 0;
  Stats.demodulators_size = Glob.demods ? Glob.demods->length : 0;
  Stats.limbo_size = Glob.limbo ? Glob.limbo->length : 0;
  Stats.disabled_size = Glob.disabled ? Glob.disabled->length : 0;
  Stats.hints_size = Glob.hints ? Glob.hints->length : 0;
  Stats.kbyte_usage = bytes_palloced() / 1000;
}  /* update_stats */

/*************
 *
 *   fprint_prover_stats()
 *
 *************/

static
void fprint_prover_stats(FILE *fp, struct prover_stats s, char *stats_level)
{
  fprintf(fp,"\nGiven=%u. Generated=%u. Kept=%u. proofs=%u.\n",
	  s.given, s.generated, s.kept, s.proofs);
  fprintf(fp,"Usable=%u. Sos=%u. Demods=%u. Limbo=%d, "
	  "Disabled=%u. Hints=%u.\n",
	  s.usable_size, s.sos_size, s.demodulators_size,
	  s.limbo_size, s.disabled_size, s.hints_size);
  
  if (str_ident(stats_level, "lots") || str_ident(stats_level, "all")) {

    fprintf(fp,"Kept_by_rule=%u, Deleted_by_rule=%u.\n",
	    s.kept_by_rule, s.deleted_by_rule);
    fprintf(fp,"Forward_subsumed=%u. Back_subsumed=%u.\n",
	    s.subsumed, s.back_subsumed);
    fprintf(fp,"Sos_limit_deleted=%u. Sos_displaced=%u. Sos_removed=%u.\n",
	    s.sos_limit_deleted, s.sos_displaced, s.sos_removed);
    fprintf(fp,"New_demodulators=%u (%u lex), Back_demodulated=%u. Back_unit_deleted=%u.\n",
	    s.new_demodulators, s.new_lex_demods, s.back_demodulated, s.back_unit_deleted);
    fprintf(fp,"Demod_attempts=%u. Demod_rewrites=%u.\n",
	    s.demod_attempts, s.demod_rewrites);
    fprintf(fp,"Res_instance_prunes=%u. Para_instance_prunes=%u. Basic_paramod_prunes=%u.\n",
	    s.res_instance_prunes, s.para_instance_prunes, s.basic_para_prunes);
    fprintf(fp,"Nonunit_fsub_feature_tests=%u. ", s.nonunit_fsub);
    fprintf(fp,"Nonunit_bsub_feature_tests=%u.\n", s.nonunit_bsub);
  }

  fprintf(fp,"Megabytes=%.2f.\n", s.kbyte_usage / 1000.0);

#if 1
  fprintf(fp,"User_CPU=%.2f, System_CPU=%.2f, Wall_clock=%u.\n",
	  user_seconds(), system_seconds(), wallclock());
#else  /* some debugging junk */
  {
    double user_sec = user_seconds();
    fprintf(fp,"User_CPU=%.2f, System_CPU=%.2f, Wall_clock=%u, "
	    "Mega_mem_calls/sec=%.2f, "
	    "Mega_next_calls/sec=%.2f, "
	    "Mega_sub_calls/sec=%.2f.\n",
	    user_sec, system_seconds(), wallclock(),
	    user_sec == 0 ? 0.0 : mega_mem_calls() / user_sec,
	    user_sec == 0 ? 0.0 : mega_next_calls() / user_sec,
	    user_sec == 0 ? 0.0 : mega_sub_calls() / user_sec);
  }
#endif
}  /* fprint_prover_stats */

/*************
 *
 *   fprint_prover_clocks()
 *
 *************/

/* DOCUMENTATION
Given an arroy of clock values (type double) indexed by
the ordinary clock indexes, print a report to a file.
*/

/* PUBLIC */
void fprint_prover_clocks(FILE *fp, struct prover_clocks clks)
{
  if (clocks_enabled()) {
    fprintf(fp, "\n");
    fprint_clock(fp, clks.pick_given);
    fprint_clock(fp, clks.infer);
    fprint_clock(fp, clks.preprocess);
    fprint_clock(fp, clks.demod);
    fprint_clock(fp, clks.unit_del);
    fprint_clock(fp, clks.redundancy);
    fprint_clock(fp, clks.conflict);
    fprint_clock(fp, clks.weigh);
    fprint_clock(fp, clks.hints);
    fprint_clock(fp, clks.subsume);
    fprint_clock(fp, clks.semantics);
    fprint_clock(fp, clks.back_subsume);
    fprint_clock(fp, clks.back_demod);
    fprint_clock(fp, clks.back_unit_del);
    fprint_clock(fp, clks.index);
    fprint_clock(fp, clks.disable);
  }
}  /* fprint_prover_clocks */

/*************
 *
 *   fprint_all_stats()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void fprint_all_stats(FILE *fp, char *stats_level)
{
  update_stats();

  print_separator(fp, "STATISTICS", TRUE);

  fprint_prover_stats(fp, Stats, stats_level);

  fprint_prover_clocks(fp, Clocks);

  if (str_ident(stats_level, "all")) {
    print_memory_stats(fp);
    selector_report();
    /* p_sos_dist(); */
  }
  print_separator(fp, "end of statistics", TRUE);
  fflush(fp);
}  // fprint_all_stats

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
  case MAX_PROOFS_EXIT:  message = "max_proofs";  break;
  case FATAL_EXIT:       message = "fatal_error"; break;
  case SOS_EMPTY_EXIT:   message = "sos_empty";   break;
  case MAX_MEGS_EXIT:    message = "max_megs";    break;
  case MAX_SECONDS_EXIT: message = "max_seconds"; break;
  case MAX_GIVEN_EXIT:   message = "max_given";   break;
  case MAX_KEPT_EXIT:    message = "max_kept";    break;
  case ACTION_EXIT:      message = "action";      break;
  case SIGSEGV_EXIT:     message = "SIGSEGV";     break;
  case SIGINT_EXIT:      message = "SIGINT";      break;
  default: message = "???";
  }
  return message;
}  /* exit_string */

/*************
 *
 *   exit_with_message()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void exit_with_message(FILE *fp, int code)
{
  int proofs = Glob.initialized ? Stats.proofs : -1;
  
  if (proofs == -1)
    fprintf(fp, "\nExiting.\n");
  else if (proofs == 0)
    fprintf(fp, "\nExiting with failure.\n");
  else
    fprintf(fp, "\nExiting with %d proof%s.\n",
	    proofs, proofs == 1 ? "" : "s");
  
  if (!Opt || !flag(Opt->quiet)) {
    fflush(stdout);
    fprintf(stderr, "\n------ process %d exit (%s) ------\n",
	    my_process_id(), exit_string(code));
    if (!Opt || flag(Opt->bell))
      bell(stderr);
  }

  if (Opt && parm(Opt->report_stderr) > 0)
    report(stderr, "some");

  fprintf(fp, "\nProcess %d exit (%s) %s",
	  my_process_id(), exit_string(code), get_date());
  
  fflush(fp);
  exit(code);
}  /* exit_with_message */

/*************
 *
 *   report()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void report(FILE *fp, char *level)
{
  double seconds = user_seconds();

  if (fp != stderr)
    fprintf(fp, "\nNOTE: Report at %.2f seconds, %s", seconds, get_date());

  if (str_ident(level, ""))
    level = (Opt ? stringparm1(Opt->stats) : "lots");

  fprint_all_stats(fp, level);

  if (!flag(Opt->quiet) && fp != stderr) {
    fflush(stdout);
    fprintf(stderr, "A report (%.2f seconds) has been sent to the output.\n",
	    seconds);
  }
  fflush(fp);
  fflush(stderr);
}  /* report */

/*************
 *
 *   possible_report()
 *
 *************/

static
void possible_report(void)
{
  static int Next_report, Next_report_stderr;
  int runtime;

  runtime = user_time() / 1000;

  if (parm(Opt->report) > 0) {
    if (Next_report == 0)
      Next_report = parm(Opt->report);
    if (runtime >= Next_report) {
      report(stdout, stringparm1(Opt->stats));
      while (runtime >= Next_report)
	Next_report += parm(Opt->report);
    }
  }

  if (parm(Opt->report_stderr) > 0) {
    if (Next_report_stderr == 0)
      Next_report_stderr = parm(Opt->report_stderr);
    if (runtime >= Next_report_stderr) {
      report(stderr, "some");
      while (runtime >= Next_report_stderr)
	Next_report_stderr += parm(Opt->report_stderr);
    }
  }
}  /* possible_report */

/*************
 *
 *   done_with_search()
 *
 *************/

static
void done_with_search(int return_code)
{
  fprint_all_stats(stdout, Opt ? stringparm1(Opt->stats) : "lots");
  /* If we need to return 0, we have to encode it as something else. */
  longjmp(Jump_env, return_code == 0 ? INT_MAX : return_code);
}  /* done_with_search */

/*************
 *
 *   exit_if_over_limit()
 *
 *************/

static
void exit_if_over_limit(void)
{
  /* max_megs is handled elsewhere */

  if (over_parm_limit(Stats.kept, Opt->max_kept))
    done_with_search(MAX_KEPT_EXIT);
  else if (over_parm_limit(Stats.given, Opt->max_given))
    done_with_search(MAX_GIVEN_EXIT);
  else if (at_parm_limit(user_seconds(), Opt->max_seconds))
    done_with_search(MAX_SECONDS_EXIT);
}  // exit_if_over_limit

/*************
 *
 *   inferences_to_make()
 *
 *************/

static
BOOL inferences_to_make(void)
{
  return givens_available();
}  // inferences_to_make

/*************
 *
 *   index_clashable()
 *
 *   Insert/delete a clause into/from resolution index.
 *
 *************/

static
void index_clashable(Topform c, Indexop operation)
{
  if (Glob.use_clash_idx) {
    clock_start(Clocks.index);
    lindex_update(Glob.clashable_idx, c, operation);
    clock_stop(Clocks.index);
  }
}  /* index_clashable */

/*************
 *
 *   restricted_denial()
 *
 *************/

static
BOOL restricted_denial(Topform c)
{
  /* At one time we also required all clauses to be Horn. */
  return
    flag(Opt->restrict_denials) &&
    negative_clause(c->literals);
}  /* restricted_denial */

/*************
 *
 *   disable_clause()
 *
 *************/

static
void disable_clause(Topform c)
{
  // Assume c is in Usable, Sos, Denials, or none of those.
  // Also, c may be in Demodulators.
  //
  // Unindex c according to which lists it is on and
  // the flags that are set, remove c from the lists,
  // and append c to Disabled.  Make sure you don't
  // have a Clist_pos for c during the call, because
  // it will be freed during the call.

  clock_start(Clocks.disable);

  // printf("disabling %d\n", c->id);

  if (clist_member(c, Glob.demods)) {
    index_demodulator(c, demodulator_type(c,
					  parm(Opt->lex_dep_demod_lim),
					  flag(Opt->lex_dep_demod_sane)),
		      DELETE, Clocks.index);
    clist_remove(c, Glob.demods);
  }
  
  if (clist_member(c, Glob.usable)) {
    index_literals(c, DELETE, Clocks.index, FALSE);
    index_back_demod(c, DELETE, Clocks.index, flag(Opt->back_demod));
    if (!restricted_denial(c))
      index_clashable(c, DELETE);
    clist_remove(c, Glob.usable);
  }
  else if (clist_member(c, Glob.sos)) {
    index_literals(c, DELETE, Clocks.index, FALSE);
    index_back_demod(c, DELETE, Clocks.index, flag(Opt->back_demod));
    remove_from_sos2(c, Glob.sos);
  }
  else if (clist_member(c, Glob.limbo)) {
    index_literals(c, DELETE, Clocks.index, FALSE);
    clist_remove(c, Glob.limbo);
  }

  /* printf(" (compressing %d)\n", c->id); */

  /* compress_clause(c); */

  clist_append(c, Glob.disabled);
  clock_stop(Clocks.disable);
}  // disable_clause

/*************
 *
 *   free_search_memory()
 *
 *   This frees memory so that we can check for memory leaks.
 *
 *************/

/* DOCUMENTATION
This is intended for debugging only.
*/

/* PUBLIC */
void free_search_memory(void)
{
  // Demodulators

  while (Glob.demods->first) {
    Topform c = Glob.demods->first->c;
    index_demodulator(c, demodulator_type(c,
					  parm(Opt->lex_dep_demod_lim),
					  flag(Opt->lex_dep_demod_sane)),
		      DELETE, Clocks.index);
    clist_remove(c, Glob.demods);
    if (c->containers == NULL)
      delete_clause(c);
  }
  clist_free(Glob.demods);
  
  destroy_demodulation_index();

  // Lightest

  // zap_sos();

  // Usable, Sos, Limbo

  while (Glob.usable->first)
    disable_clause(Glob.usable->first->c);
  clist_free(Glob.usable);
  Glob.usable = NULL;

  while (Glob.sos->first)
    disable_clause(Glob.sos->first->c);
  clist_free(Glob.sos);
  Glob.sos = NULL;

  while (Glob.limbo->first)
    disable_clause(Glob.limbo->first->c);
  clist_free(Glob.limbo);
  Glob.limbo = NULL;

  destroy_literals_index();
  destroy_back_demod_index();
  lindex_destroy(Glob.clashable_idx);
  Glob.clashable_idx = NULL;

  delete_clist(Glob.disabled);
  Glob.disabled = NULL;

  if (Glob.hints->first) {
    Clist_pos p;
    for(p = Glob.hints->first; p; p = p->next)
      unindex_hint(p->c);
    done_with_hints();
  }
  delete_clist(Glob.hints);
  Glob.hints = NULL;

}  // free_search_memory

/*************
 *
 *   handle_proof_and_maybe_exit()
 *
 *************/

static
void handle_proof_and_maybe_exit(Topform empty_clause)
{
  Term answers;
  Plist proof, p;

  mark_parents_as_used(empty_clause);
  assign_clause_id(empty_clause);
  proof = get_clause_ancestors(empty_clause);
  uncompress_clauses(proof);

  if (!flag(Opt->reuse_denials) && Glob.horn) {
    Topform c = first_negative_clause(proof);
    if (clause_member_plist(Glob.desc_to_be_disabled, c)) {
      printf("%% Redundant proof: ");
      f_clause(empty_clause);
      return;
    }
    else
      /* Descendants of c will be disabled when it is safe to do so. */
      Glob.desc_to_be_disabled = plist_prepend(Glob.desc_to_be_disabled, c);
  }
  
  Glob.empties = plist_append(Glob.empties, empty_clause);
  Stats.proofs++;

  answers = get_term_attributes(empty_clause->attributes, Att.answer);

  /* message to stderr */

  if (!flag(Opt->quiet)) {
    fflush(stdout);
    if (flag(Opt->bell))
      bell(stderr);
    fprintf(stderr, "-------- Proof %d -------- ", Stats.proofs);
    if (answers != NULL)
      fwrite_term_nl(stderr, answers);
    else
      fprintf(stderr, "\n");
  }

  /* print proof to stdout */

  fflush(stderr);
  if (flag(Opt->print_proofs)) {
    print_separator(stdout, "PROOF", TRUE);
    printf("\n%% Proof %d at %.2f (+ %.2f) seconds",
	   Stats.proofs, user_seconds(), system_seconds());
    if (answers != NULL) {
      printf(": ");
      fwrite_term(stdout, answers);
    }
    printf(".\n");

    printf("%% Length of proof is %d.\n", proof_length(proof));
    printf("%% Level of proof is %d.\n", clause_level(empty_clause));
    printf("%% Maximum clause weight is %.3f.\n", max_clause_weight(proof));
    printf("%% Given clauses %d.\n\n", Stats.given);
    for (p = proof; p; p = p->next)
      fwrite_clause(stdout, p->v, CL_FORM_STD);
    print_separator(stdout, "end of proof", TRUE);
  }  /* print_proofs */
  else {
    printf("\n-------- Proof %d at (%.2f + %.2f seconds) ",
	   Stats.proofs, user_seconds(), system_seconds());
    if (answers != NULL)
      fwrite_term_nl(stdout, answers);
    else
      fprintf(stdout, "\n");
  }
  fflush(stdout);
  if (answers)
    zap_term(answers);

  actions_in_proof(proof, &Att);  /* this can exit */

  if (at_parm_limit(Stats.proofs, Opt->max_proofs))
    done_with_search(MAX_PROOFS_EXIT);  /* does not return */

  zap_plist(proof);
}  // handle_proof_and_maybe_exit

/*************
 *
 *   clause_wt_with_adjustments()
 *
 *************/

static
void clause_wt_with_adjustments(Topform c)
{
  clock_start(Clocks.weigh);
  c->weight = clause_weight(c->literals);
  clock_stop(Clocks.weigh);

  if (!clist_empty(Glob.hints)) {
    clock_start(Clocks.hints);
    if (!c->normal_vars)
      renumber_variables(c, MAX_VARS);
    adjust_weight_with_hints(c,
			     flag(Opt->degrade_hints),
			     flag(Opt->breadth_first_hints));
    clock_stop(Clocks.hints);
  }

  if (c->weight > floatparm(Opt->default_weight) && 
      c->weight <= floatparm(Opt->max_weight))
    c->weight = floatparm(Opt->default_weight);
}  /* clause_wt_with_adjustments */

/*************
 *
 *   cl_process()
 *
 *   Process a newly inferred (or input) clause.
 *
 *   It is likely that a pointer to this routine was passed to
 *   an inference rule, and that inference rule called this routine
 *   with a new clause.
 *
 *   If this routine decides to keep the clause, it is appended
 *   to the Limbo list rather than to Sos.  Clauses in Limbo have been
 *   kept, but operations that can delete clauses (back subsumption,
 *   back demoulation) have not yet been applied.  The Limbo list
 *   is processed after the inference rule is finished.
 *
 *   Why use the Limbo list?  Because we're not allowed to delete
 *   clauses (back subsumption, back demodulation, back unit deletion)
 *   while inferring clauses.
 *
 *   Why not infer the whole batch of clauses, and then process them?
 *   Because there can be too many.  We have to do demodulation and
 *   subsumption right away, and get kept clauses indexed for
 *   forward demodulation and forward subsumption right away,
 *   so they can be used on the next inferred clause.
 *
 *************/

/* First, some helper routines. */

static
void cl_process_simplify(Topform c)
{
  if (flag(Opt->eval_rewrite)) {
    int count = 0;
    clock_start(Clocks.demod);
    rewrite_with_eval(c);
    if (flag(Opt->print_gen)) {
      printf("rewrites %d:     ", count);
      fwrite_clause(stdout, c, CL_FORM_STD);
    }
    clock_stop(Clocks.demod);
  }
  else if (!clist_empty(Glob.demods)) {
    if (flag(Opt->lex_order_vars)) {
      renumber_variables(c, MAX_VARS);
      c->normal_vars = FALSE;  // demodulation can make vars non-normal
    }
    clock_start(Clocks.demod);
      demodulate_clause(c,
			parm(Opt->demod_step_limit),
			parm(Opt->demod_increase_limit),
			!flag(Opt->quiet),
			flag(Opt->lex_order_vars));
    if (flag(Opt->print_gen)) {
      printf("rewrite:     ");
      fwrite_clause(stdout, c, CL_FORM_STD);
    }
    clock_stop(Clocks.demod);
  }

  orient_equalities(c, TRUE);
  simplify_literals2(c);  // with x=x, and simplify tautologies to $T
  merge_literals(c);

  if (flag(Opt->unit_deletion)) {
    clock_start(Clocks.unit_del);
    unit_deletion(c);
    clock_stop(Clocks.unit_del);
  }

  if (flag(Opt->cac_redundancy)) {
    clock_start(Clocks.redundancy);
    // If comm or assoc, make a note of it.
    // Also simplify C or AC redundant literals to $T.
    if (cac_redundancy(c, !flag(Opt->quiet)))
      Glob.cac_clauses = plist_prepend(Glob.cac_clauses, c);
    clock_stop(Clocks.redundancy);
  }
}  // cl_process_simplify

static
void cl_process_keep(Topform c)
{
  Stats.kept++;
  if (!c->normal_vars)
    renumber_variables(c, MAX_VARS);
  if (c->id == 0)
    assign_clause_id(c);  // unit conflict or input: already has ID
  mark_parents_as_used(c);
  mark_maximal_literals(c->literals);
  mark_selected_literals(c->literals, stringparm1(Opt->literal_selection));
  if (c->matching_hint != NULL)
    keep_hint_matcher(c);

  if (flag(Opt->print_clause_properties))
      c->attributes = set_term_attribute(c->attributes,
					 Att.properties,
					 topform_properties(c));
  if (flag(Opt->print_kept) || flag(Opt->print_gen) ||
      (!Glob.searching && flag(Opt->print_initial_clauses))) {
    printf("kept:      ");
    fwrite_clause(stdout, c, CL_FORM_STD);
  }
  else if (flag(Opt->print_labeled) &&
	   get_string_attribute(c->attributes, Att.label, 1)) {
    printf("\nkept:      ");
    fwrite_clause(stdout, c, CL_FORM_STD);
  }
  statistic_actions("kept", clause_ids_assigned());  /* Note different stat */
}  // cl_process_keep

static
void cl_process_conflict(Topform c, BOOL denial)
{
  if (number_of_literals(c->literals) == 1) {
    if (!c->normal_vars)
      renumber_variables(c, MAX_VARS);
    clock_start(Clocks.conflict);
    unit_conflict(c, handle_proof_and_maybe_exit);
    clock_stop(Clocks.conflict);
  }
}  // cl_process_conflict

static
void cl_process_new_demod(Topform c)
{
  // If the clause should be a demodulator, make it so.
  if (flag(Opt->back_demod)) {
    int type = demodulator_type(c,
				parm(Opt->lex_dep_demod_lim),
				flag(Opt->lex_dep_demod_sane));
    if (type != NOT_DEMODULATOR) {
      if (flag(Opt->print_kept)) {
	char *s;
	switch(type) {
	case ORIENTED:     s = ""; break;
	case LEX_DEP_LR:   s = " (lex_dep_lr)"; break;
	case LEX_DEP_RL:   s = " (lex_dep_rl)"; break;
	case LEX_DEP_BOTH: s = " (lex_dep_both)"; break;
	default:           s = " (?)";
	}
	printf("    new demodulator%s: %d.\n", s, c->id);
      }
      clist_append(c, Glob.demods);
      index_demodulator(c, type, INSERT, Clocks.index);
      Stats.new_demodulators++;
      if (type != ORIENTED)
	Stats.new_lex_demods++;
      back_demod_hints(c, type, flag(Opt->lex_order_vars));
    }
  }
}  // cl_process_new_demod

static
BOOL skip_black_white_tests(Topform c)
{
  return (!Glob.searching ||
	  c->used ||
	  restricted_denial(c) ||
	  (c->matching_hint  != NULL && !flag(Opt->limit_hint_matchers)));
}  /* skip_black_white_tests */

static
BOOL cl_process_delete(Topform c)
{
  // Should the clause be deleted (tautology, limits, subsumption)?

  if (true_clause(c->literals)) {  // tautology
    if (flag(Opt->print_gen))
      printf("tautology\n");
    Stats.subsumed++;
    return TRUE;  // delete
  }

  clause_wt_with_adjustments(c);  // possibly sets c->matching_hint

  // White-black tests

  if (!skip_black_white_tests(c)) {
    if (white_tests(c)) {
      if (flag(Opt->print_gen))
	printf("keep_rule applied.\n");
      Stats.kept_by_rule++;
    }
    else {
      if (black_tests(c)) {
	if (flag(Opt->print_gen))
	  printf("delete_rule applied.\n");
	Stats.deleted_by_rule++;
	return TRUE;  //delete
      }
      else if (!sos_keep2(c, Glob.sos, Opt)) {
	if (flag(Opt->print_gen))
	  printf("sos_limit applied.\n");
	Stats.sos_limit_deleted++;
	return TRUE;  // delete
      }
    }
  }
      
  // Forward subsumption

  {
    Topform subsumer;
    clock_start(Clocks.subsume);
    subsumer = forward_subsumption(c);
    clock_stop(Clocks.subsume);
    if (subsumer != NULL && !c->used) {
      if (flag(Opt->print_gen))
	printf("subsumed by %d.\n", subsumer->id);
      Stats.subsumed++;
      return TRUE;  // delete
    }
    else
      return FALSE;  // keep the clause
  }
}  // cl_process_delete

static
void cl_process(Topform c)
{
  // If the infer_clock is running, stop it and restart it when done.

  BOOL infer_clock_stopped = FALSE;
  if (clock_running(Clocks.infer)) {
    clock_stop(Clocks.infer);
    infer_clock_stopped = TRUE;
  }
  clock_start(Clocks.preprocess);

  exit_if_over_limit();
  if (parm(Opt->report) > 0 || parm(Opt->report_stderr) > 0)
    possible_report();

  Stats.generated++;
  statistic_actions("generated", Stats.generated);
  if (flag(Opt->print_gen)) {
    printf("\ngenerated: ");
    fwrite_clause(stdout, c, CL_FORM_STD);
  }

  cl_process_simplify(c);            // all simplification

  if (number_of_literals(c->literals) == 0)    // empty clause
    handle_proof_and_maybe_exit(c);
  else {
    // Do safe unit conflict before any deletion checks.
    if (flag(Opt->safe_unit_conflict))
      cl_process_conflict(c, FALSE);  // marked as used if conflict

    if (cl_process_delete(c))
      delete_clause(c);
    else {
      cl_process_keep(c);
      // Ordinary unit conflict.
      if (!flag(Opt->safe_unit_conflict))
	cl_process_conflict(c, FALSE);
      cl_process_new_demod(c);
      // We insert c into the literal index now so that it will be
      // available for unit conflict and forward subsumption while
      // it's in limbo.  (It should not be back subsumed while in limbo.
      // See fatal error in limbo_process).
      index_literals(c, INSERT, Clocks.index, FALSE);
      clist_append(c, Glob.limbo);
    }  // not deleted
  }  // not empty clause
  
  clock_stop(Clocks.preprocess);
  if (infer_clock_stopped)
    clock_start(Clocks.infer);
}  // cl_process

/*************
 *
 *   back_demod()
 *
 *   For each clause that can be back demodulated, make a copy,
 *   disable the original, cl_process the copy (as if it
 *   had just been inferred).
 *
 *************/

static
void back_demod(Topform demod)
{
  Plist results, p, prev;

  clock_start(Clocks.back_demod);
  results = back_demodulatable(demod,
			       demodulator_type(demod,
						parm(Opt->lex_dep_demod_lim),
						flag(Opt->lex_dep_demod_sane)),
			       flag(Opt->lex_order_vars));
  clock_stop(Clocks.back_demod);
  p = results;
  while(p != NULL) {
    Topform old = p->v;
    if (!clist_member(old, Glob.disabled)) {
      Topform new;
      if (flag(Opt->basic_paramodulation))
	new = copy_clause_with_flag(old, nonbasic_flag());
      else
	new = copy_clause(old);
      Stats.back_demodulated++;
      if (flag(Opt->print_kept))
	printf("        %d back demodulating %d.\n", demod->id, old->id);
      new->justification = back_demod_just(old);
      new->attributes = inheritable_att_instances(old->attributes, NULL);
      disable_clause(old);
      cl_process(new);
    }
    prev = p;
    p = p->next;
    free_plist(prev);
  }
}  // back_demod

/*************
 *
 *   back_unit_deletion()
 *
 *   For each clause that can be back unit deleted, make a copy,
 *   disable the original, cl_process the copy (as if it
 *   had just been inferred).
 *
 *************/

static
void back_unit_deletion(Topform unit)
{
  Plist results, p, prev;

  clock_start(Clocks.back_unit_del);
  results = back_unit_deletable(unit);
  clock_stop(Clocks.back_unit_del);
  p = results;
  while(p != NULL) {
    Topform old = p->v;
    if (!clist_member(old, Glob.disabled)) {
      Topform new;
      if (flag(Opt->basic_paramodulation))
	new = copy_clause_with_flag(old, nonbasic_flag());
      else
	new = copy_clause(old);
      Stats.back_unit_deleted++;
      if (flag(Opt->print_kept))
	printf("        %d back unit deleting %d.\n", unit->id, old->id);
      new->justification = back_unit_deletion_just(old);
      new->attributes = inheritable_att_instances(old->attributes, NULL);
      disable_clause(old);
      cl_process(new);
    }
    prev = p;
    p = p->next;
    free_plist(prev);
  }
}  // back_unit_deletion

/*************
 *
 *   back_cac_simplify()
 *
 *   For each clause that can be back unit deleted, make a copy,
 *   disable the original, cl_process the copy (as if it
 *   had just been inferred).
 *
 *************/

static
void back_cac_simplify(void)
{
  Plist to_delete = NULL;
  Plist a;
  Clist_pos p;
  for (p = Glob.sos->first; p; p = p->next) {
    if (cac_tautology(p->c->literals))
      to_delete = plist_prepend(to_delete, p->c);
  }
  for (p = Glob.usable->first; p; p = p->next) {
    if (cac_tautology(p->c->literals))
      to_delete = plist_prepend(to_delete, p->c);
  }
  for (p = Glob.limbo->first; p; p = p->next) {
    if (cac_tautology(p->c->literals))
      to_delete = plist_prepend(to_delete, p->c);
  }
  for (a = to_delete; a; a = a->next) {
    printf("%% back CAC tautology: "); f_clause(a->v);
    disable_clause(a->v);
  }
  zap_plist(to_delete);  /* shallow */
}  // back_cac_simplify

/*************
 *
 *   disable_to_be_disabled()
 *
 *************/

static
void disable_to_be_disabled(void)
{
  if (Glob.desc_to_be_disabled) {

    Plist descendants = NULL;
    Plist p;

    sort_clist_by_id(Glob.disabled);

    for (p = Glob.desc_to_be_disabled; p; p = p->next) {
      Topform c = p->v;
      Plist x = neg_descendants(c,Glob.usable,Glob.sos,Glob.disabled);
      descendants = plist_cat(descendants, x);
    }
    
#if 1
    {
      int n = 0;
      printf("\n%% Disable descendants (x means already disabled):\n");
      for (p = descendants; p; p = p->next) {
	Topform d = p->v;
	printf(" %d%s", d->id, clist_member(d, Glob.disabled) ? "x" : "");
	if (++n % 10 == 0)
	  printf("\n");
      }
      printf("\n");
    }
#endif

    for (p = descendants; p; p = p->next) {
      Topform d = p->v;
      if (!clist_member(d, Glob.disabled))
	disable_clause(d);
    }

    zap_plist(descendants);
    zap_plist(Glob.desc_to_be_disabled);
    Glob.desc_to_be_disabled = NULL;
  }
}  /* disable_to_be_disabled */

/*************
 *
 *   limbo_process()
 *
 *   Apply back subsumption and back demodulation to the Limbo
 *   list.  Since back demodulated clauses are cl_processed,
 *   the Limbo list can grow while it is being processed.
 *
 *   If this prover had a hot-list, or any other feature that
 *   generates clauses from newly kept clauses, it probably would
 *   be done here.
 *
 *   The Limbo list operates as a queue.  An important property
 *   of the Limbo list is that if A is ahead of B, then A does
 *   not simplify or subsume B.  However, B can simplify or subsume A.
 *
 *************/

static
void limbo_process(BOOL pre_search)
{
  while (Glob.limbo->first) {
    Topform c = Glob.limbo->first->c;

    // factoring

    if (flag(Opt->factor))
      binary_factors(c, cl_process);

    // Try to apply new_constant rule.

    if (!at_parm_limit(Stats.new_constants, Opt->new_constants)) {
      Topform new = new_constant(c, INT_MAX);
      if (new) {
	Stats.new_constants++;
	if (!flag(Opt->quiet)) {
	  printf("\nNOTE: New constant: ");
	  f_clause(new);
	  printf("NOTE: New ");
	  print_fsym_precedence(stdout);
	}
	if (Glob.interps)
	  update_semantics_new_constant(new);
	cl_process(new);
      }
    }

    // fold denial (for input clauses only)

    if (parm(Opt->fold_denial_max) > 1 &&
	(has_input_just(c) || has_copy_just(c))) {
      Topform new = fold_denial(c, parm(Opt->fold_denial_max));
      if (new) {
	if (!flag(Opt->quiet)) {
	  printf("\nNOTE: Fold denial: ");
	  f_clause(new);
	}
	cl_process(new);
      }
    }

    // Disable clauses subsumed by c (back subsumption).

    if (flag(Opt->back_subsume)) {
      Plist subsumees;
      clock_start(Clocks.back_subsume);
      subsumees = back_subsumption(c);
      if (subsumees != NULL)
	c->subsumer = TRUE;
      while (subsumees != NULL) {
	Topform d = subsumees->v;
	if (clist_member(d, Glob.limbo))  // See comment in cl_process.
	  fatal_error("back subsume limbo clause");
	Stats.back_subsumed++;
	if (flag(Opt->print_kept))
	  printf("    %d back subsumes %d.\n", c->id, d->id);
	disable_clause(d);
	subsumees = plist_pop(subsumees);
      }
      clock_stop(Clocks.back_subsume);
    }

    // If demodulator, rewrite other clauses (back demodulation).

    if (clist_member(c, Glob.demods)) {
      if (flag(Opt->print_kept))
	printf("    starting back demodulation with %d.\n", c->id);
      back_demod(c);  // This calls cl_process on rewritable clauses.
    }

    // If unit, use it to simplify other clauses (back unit_deletion)

    if (flag(Opt->unit_deletion) && unit_clause(c->literals)) {
      back_unit_deletion(c);  // This calls cl_process on rewritable clauses.
    }

    // Check if we should do back CAC simplification.

    if (plist_member(Glob.cac_clauses, c)) {
      back_cac_simplify();
    }

    // Remove from limbo

    clist_remove(c, Glob.limbo);

    // If restricted_denial, appdend to usable, else append to sos.

    if (restricted_denial(c)) {
      // do not index_clashable!  disable_clause should not unindex_clashable!
      clist_append(c, Glob.usable);
      index_back_demod(c, INSERT, Clocks.index, flag(Opt->back_demod));
    }
    else {
      // Move to Sos and index to be found for back demodulation.
      if (parm(Opt->sos_limit) != -1 &&
	  clist_length(Glob.sos) >= parm(Opt->sos_limit)) {
	sos_displace2(disable_clause);
	Stats.sos_displaced++;
      }
      if (pre_search)
	c->initial = TRUE;
      else
	c->initial = FALSE;
      insert_into_sos2(c, Glob.sos);
      index_back_demod(c, INSERT, Clocks.index, flag(Opt->back_demod));
    }
  }
  // Now it is safe to disable descendants of desc_to_be_disabled clauses.
  disable_to_be_disabled();
}  // limbo_process

/*************
 *
 *   infer_outside_loop()
 *
 *************/

static
void infer_outside_loop(Topform c)
{
  /* If simplification changes the clause, we want to do a "copy"
   inference first, so that a proof does not contain a justification
   like  [assumption,rewrite[...],...]. */
  Topform copy = copy_inference(c);  /* Note: c has no ID yet. */
  cl_process_simplify(copy);
  if (copy->justification->next == NULL) {
    /* Simplification does nothing, so we can just process the original. */
    delete_clause(copy);
    cl_process(c);
  }
  else {
    assign_clause_id(c);
    copy->justification->u.id = c->id;
    clist_append(c, Glob.disabled);
    cl_process(copy);  /* This re-simplifies, but that's ok. */
  }

  limbo_process(FALSE);
}  /* infer_outside_loop */

/*************
 *
 *   given_infer()
 *
 *   Make given_clause inferences according to the flags that are set.
 *   Inferred clauses are sent to cl_process().
 *
 *   We could process the Limbo list after each inference rule,
 *   and this might improve performance in some cases.  But it seems
 *   conceptually simplier if we process the Limbo clauses after
 *   all of the inferences have been made.
 *
 *************/

static
void given_infer(Topform given)
{
  clock_start(Clocks.infer);

  if (flag(Opt->binary_resolution))
    binary_resolution(given,
		      ANY_RES,
		      Glob.clashable_idx,
		      cl_process);

  if (flag(Opt->neg_binary_resolution))
    binary_resolution(given,
		      NEG_RES,
		      Glob.clashable_idx,
		      cl_process);

  if (flag(Opt->pos_hyper_resolution))
    hyper_resolution(given, POS_RES, Glob.clashable_idx, cl_process);

  if (flag(Opt->neg_hyper_resolution))
    hyper_resolution(given, NEG_RES, Glob.clashable_idx, cl_process);

  if (flag(Opt->pos_ur_resolution))
    ur_resolution(given, POS_RES, Glob.clashable_idx, cl_process);

  if (flag(Opt->neg_ur_resolution))
    ur_resolution(given, NEG_RES, Glob.clashable_idx, cl_process);

  if (flag(Opt->paramodulation) &&
      !over_parm_limit(number_of_literals(given->literals),
		       Opt->para_lit_limit)) {
    // This paramodulation does not use indexing.
    Context cf = get_context();
    Context ci = get_context();
    Clist_pos p;
    for (p = Glob.usable->first; p; p = p->next) {
      if (!restricted_denial(p->c) &&
	  !over_parm_limit(number_of_literals(p->c->literals),
			   Opt->para_lit_limit)) {
	para_from_into(given, cf, p->c, ci, FALSE, cl_process);
	para_from_into(p->c, cf, given, ci, TRUE, cl_process);
      }
    }
    free_context(cf);
    free_context(ci);
  }
  clock_stop(Clocks.infer);
}  // given_infer

/*************
 *
 *   rebuild_sos_index()
 *
 *************/

static
void rebuild_sos_index(void)
{
  fatal_error("rebuild_sos_index not implemented for given_selection");
#if 0
  Clist_pos p;
  printf("\nNOTE: Reweighing all SOS clauses and rebuilding SOS indexes.\n");
  zap_picker_indexes();
  update_picker_ratios(Opt);  /* in case they've been changed */
  for (p = Glob.sos->first; p; p = p->next) {
    Topform c = p->c;
    clause_wt_with_adjustments(c); /* weigh the clause (wt stored in c) */
    update_pickers(c, TRUE); /* insert (lower-level) into picker indexes */
#endif
}  /* rebuild_sos_index */

/*************
 *
 *   make_inferences()
 *
 *   Assume that there are inferences to make.
 *
 *   If we had the option of using the pair algorithm instead of
 *   the given algorithm, we could make that decision here.
 *
 *************/

static
void make_inferences(void)
{
  Topform given_clause;
  char *selection_type;

  clock_start(Clocks.pick_given);
  given_clause = get_given_clause2(Glob.sos,Stats.given, Opt, &selection_type);
  clock_stop(Clocks.pick_given);

  if (given_clause != NULL) {
    static int level = 0;             // NOTE: STATIC VARIABLE
    static int last_of_level = 0;     // NOTE: STATIC VARIABLE

    // Print "level" message for breadth-first; also "level" actions.

    if (flag(Opt->breadth_first) &&
	parm(Opt->true_part) == 0 &&
	parm(Opt->false_part) == 0 &&
	parm(Opt->weight_part) == 0 &&
	parm(Opt->random_part) == 0 &&
	str_ident(selection_type, "A") &&
	given_clause->id > last_of_level) {
      level++;
      last_of_level = clause_ids_assigned();
      if (!flag(Opt->quiet)) {
	printf("\nNOTE: Starting on level %d, last clause "
	       "of level %d is %d.\n",
	       level, level-1, last_of_level);
	fflush(stdout);
	fprintf(stderr, "\nStarting on level %d, last clause "
		"of level %d is %d.\n",
		level, level-1, last_of_level);
	fflush(stderr);
      }
      statistic_actions("level", level);
    }

    Stats.given++;

    // Maybe disable back subsumption.

    if (over_parm_limit(Stats.given, Opt->max_given))
      done_with_search(MAX_GIVEN_EXIT);

    if (Stats.given == parm(Opt->backsub_check)) {
      int ratio = (Stats.back_subsumed == 0 ?
		   INT_MAX :
		   Stats.kept / Stats.back_subsumed);
      if (ratio > 20) {
	clear_flag(Opt->back_subsume, TRUE);
	printf("\nNOTE: Back_subsumption disabled, ratio of kept"
	       " to back_subsumed is %d (%.2f of %.2f sec).\n",
	       ratio, clock_seconds(Clocks.back_subsume), user_seconds());
	fflush(stdout);
      }
    }
    
    if (flag(Opt->print_given)) {
      if (given_clause->weight == round(given_clause->weight))
	printf("\ngiven #%d (%s,wt=%d): ",
	       Stats.given, selection_type, (int) given_clause->weight);
      else
	printf("\ngiven #%d (%s,wt=%.3f): ",
	       Stats.given, selection_type, given_clause->weight);
      fwrite_clause(stdout, given_clause, CL_FORM_STD);
    }

    statistic_actions("given", Stats.given);

    clist_append(given_clause, Glob.usable);
    index_clashable(given_clause, INSERT);
    given_infer(given_clause);
  }
}  // make_inferences

/*************
 *
 *   orient_input_eq()
 *
 *   This is designed for input clauses, and it's a bit tricky.  If any
 *   equalities are flipped, we make the flip operations info inferences
 *   so that proofs are complete.  This involves replacing and hiding
 *   (disabling) the original clause.
 *
 *************/

static
Topform orient_input_eq(Topform c)
{
  Topform new = copy_inference(c);
  orient_equalities(new, TRUE);
  if (clause_ident(c->literals, new->literals)) {
    delete_clause(new);
    /* the following puts "oriented" marks on c */
    orient_equalities(c, TRUE);
    return c;
  }
  else {
    /* Replace c with new in Usable. */
    assign_clause_id(new);
    mark_parents_as_used(new);
    clist_swap(c, new);
    clist_append(c, Glob.disabled);
    return new;
  }
}  /* orient_input_eq */

/*************
 *
 *   auto_inference()
 *
 *   This looks at the clauses and decides which inference rules to use.
 *
 *************/

static
void auto_inference(Clist sos, Clist usable, Prover_options opt)
{
  BOOL print = !flag(opt->quiet);
  if (print)
    printf("\nAuto_inference settings:\n");

  if (Glob.equality) {
    if (print)
      printf("  %% set(paramodulation).  %% (positive equality literals)\n");
    set_flag(opt->paramodulation, print);
  }

  if (!Glob.equality || !Glob.unit) {
    if (Glob.horn) {
      Plist clauses = NULL;
      clauses = prepend_clist_to_plist(clauses, sos);
      clauses = prepend_clist_to_plist(clauses, usable);

      if (Glob.equality) {
	if (print)
	  printf("  %% set(hyper_resolution)."
		 "  %% (nonunit Horn with equality)\n");
	set_flag(opt->hyper_resolution, print);
	if (print)
	  printf("  %% set(neg_ur_resolution)."
		 "  %% (nonunit Horn with equality)\n");
	set_flag(opt->neg_ur_resolution, print);

	if (parm(opt->para_lit_limit) == -1) {
	  int para_lit_limit = most_literals(clauses);
	  if (print)
	    printf("  %% assign(para_lit_limit, %d)."
		   "  %% (nonunit Horn with equality)\n",
		   para_lit_limit);
	  assign_parm(opt->para_lit_limit, para_lit_limit, print);
	}
      }
      else {
	int diff = neg_pos_depth_difference(clauses);
	if (diff > 0) {
	  if (print)
	    printf("  %% set(hyper_resolution)."
		   "  %% (HNE depth_diff=%d)\n", diff);
	  set_flag(opt->hyper_resolution, print);
	}
	else {
	  if (print)
	    printf("  %% set(neg_binary_resolution)."
		   "  %% (HNE depth_diff=%d)\n", diff);
	  set_flag(opt->neg_binary_resolution, print);
	  if (print)
	    printf("  %% clear(ordered_res)."
		   "  %% (HNE depth_diff=%d)\n", diff);
	  clear_flag(opt->ordered_res, print);
	  if (print)
	    printf("  %% set(ur_resolution)."
		   "  %% (HNE depth_diff=%d)\n", diff);
	  set_flag(opt->ur_resolution, print);
	}
      }
      zap_plist(clauses);
    }
    else {
      // there are nonhorn clauses
      if (print) {
	printf("  %% set(binary_resolution).  %% (non-Horn)\n");
      }
      set_flag(opt->binary_resolution, print);
      if (Glob.number_of_clauses < 100) {
	if (print)
	  printf("  %% set(neg_ur_resolution)."
		 "  %% (non-Horn, less than 100 clauses)\n");
	set_flag(opt->neg_ur_resolution, print);
      }
    }
  }
}  /* auto_inference */

/*************
 *
 *   auto_process()
 *
 *   This looks at the clauses and decides some processing options.
 *
 *************/

static
void auto_process(Clist sos, Clist usable, Prover_options opt)
{
  BOOL print = !flag(opt->quiet);
  Plist clauses;
  BOOL horn;

  clauses = prepend_clist_to_plist(NULL, sos);
  clauses = prepend_clist_to_plist(clauses, usable);

  horn  = all_clauses_horn(clauses);

  if (print)
    printf("\nAuto_process settings:");

  if (horn) {
    if (neg_nonunit_clauses(clauses) > 0) {
      if (print)
	printf("\n  %% set(unit_deletion)."
	       "  %% (Horn set with negative nonunits)\n");
      set_flag(opt->unit_deletion, print);
    }
    else {
      if (print)
	printf("  (no changes).\n");
    }
  }

  else {
    // there are nonhorn clauses
    if (print)
      printf("\n  %% set(factor).  %% (non-Horn)\n");
    set_flag(opt->factor, print);
    if (print)
      printf("  %% set(unit_deletion).  %% (non-Horn)\n");
    set_flag(opt->unit_deletion, print);
  }
  zap_plist(clauses);
}  /* auto_process */

/*************
 *
 *   auto_denials()
 *
 *************/

static
void auto_denials(Clist sos, Clist usable, Prover_options opt)
{
  int changes = 0;

  printf("\nAuto_denials:");

  if (Glob.horn) {
    Plist neg_clauses = plist_cat(neg_clauses_in_clist(sos),
				  neg_clauses_in_clist(usable));
    Plist p;
    for (p = neg_clauses; p; p = p->next) {
      Topform c = p->v;
      char *label = get_string_attribute(c->attributes, Att.label, 1);
      Term answer = get_term_attribute(c->attributes, Att.answer, 1);
      if (label && !answer) {
	Term t = get_rigid_term(label, 0);
	c->attributes = set_term_attribute(c->attributes, Att.answer, t);
	printf("%s", changes == 0 ? "\n" : "");
	printf("  %% copying label %s to answer in negative clause\n", label);
	changes++;
      }
    }

    if (Glob.number_of_neg_clauses > 1 && parm(opt->max_proofs) == 1) {
      printf("%s", changes == 0 ? "\n" : "");
      printf("  %% assign(max_proofs, %d)."
	     "  %% (Horn set with more than one neg. clause)\n",
	     Glob.number_of_neg_clauses);
      assign_parm(opt->max_proofs, Glob.number_of_neg_clauses, TRUE);
      check_constant_sharing(neg_clauses);
      changes++;
    }
    zap_plist(neg_clauses);
  }

  if (changes == 0)
    printf("  (%sno changes).\n", Glob.horn ? "" : "non-Horn, ");
}  /* auto_denials */

/*************
 *
 *   init_search()
 *
 *************/

static
void init_search(void)
{
  // Initialize clocks.
  
  Clocks.pick_given    = clock_init("pick_given");
  Clocks.infer         = clock_init("infer");
  Clocks.preprocess    = clock_init("preprocess");
  Clocks.demod         = clock_init("demod");
  Clocks.unit_del      = clock_init("unit_deletion");
  Clocks.redundancy    = clock_init("redundancy");
  Clocks.conflict      = clock_init("conflict");
  Clocks.weigh         = clock_init("weigh");
  Clocks.hints         = clock_init("hints");
  Clocks.subsume       = clock_init("subsume");
  Clocks.semantics     = clock_init("semantics");
  Clocks.back_subsume  = clock_init("back_subsume");
  Clocks.back_demod    = clock_init("back_demod");
  Clocks.back_unit_del = clock_init("back_unit_del");
  Clocks.index         = clock_init("index");
  Clocks.disable       = clock_init("disable");

  init_actions(Glob.actions,
	       rebuild_sos_index, done_with_search, infer_outside_loop);
  init_weight(Glob.weights,
	      floatparm(Opt->variable_weight),
	      floatparm(Opt->constant_weight),
	      floatparm(Opt->not_weight),
	      floatparm(Opt->or_weight),
	      floatparm(Opt->sk_constant_weight),
	      floatparm(Opt->prop_atom_weight),
	      floatparm(Opt->nest_penalty),
	      floatparm(Opt->depth_penalty),
	      floatparm(Opt->var_penalty),
	      floatparm(Opt->complexity));

  if (Glob.given_selection == NULL)
    Glob.given_selection = selector_rules_from_options(Opt);
  else if (flag(Opt->input_sos_first))
    Glob.given_selection = plist_prepend(Glob.given_selection,
					 selector_rule_term("I", "high", "age",
							    "initial",
							    INT_MAX));
  init_giv_select(Glob.given_selection);

  Glob.delete_rules = plist_cat(delete_rules_from_options(Opt),
				Glob.delete_rules);

  init_white_black(Glob.keep_rules, Glob.delete_rules);

  // Term ordering

  printf("\nTerm ordering decisions:\n");

  if (stringparm(Opt->order, "lpo")) {
    assign_order_method(LPO_METHOD);
    all_symbols_lrpo_status(LRPO_LR_STATUS);
    set_lrpo_status(str_to_sn(eq_sym(), 2), LRPO_MULTISET_STATUS);
  }
  else if (stringparm(Opt->order, "rpo")) {
    assign_order_method(RPO_METHOD);
    all_symbols_lrpo_status(LRPO_MULTISET_STATUS);
  }
  else if (stringparm(Opt->order, "kbo")) {
    assign_order_method(KBO_METHOD);
  }

  symbol_order(Glob.usable, Glob.sos, Glob.demods, TRUE);

  if (Glob.kbo_weights) {
    if (!stringparm(Opt->order, "kbo")) {
      assign_stringparm(Opt->order, "kbo", TRUE);
      printf("assign(order, kbo), because KB weights were given.\n");
    }
    init_kbo_weights(Glob.kbo_weights);
    print_kbo_weights(stdout);
  }
  else if (stringparm(Opt->order, "kbo")) {
    auto_kbo_weights(Glob.usable, Glob.sos);
    print_kbo_weights(stdout);
  }

  if (!flag(Opt->quiet)) {
    print_rsym_precedence(stdout);
    print_fsym_precedence(stdout);
  }

  if (flag(Opt->inverse_order)) {
    if (exists_preliminary_precedence(FUNCTION_SYMBOL)) {  // lex command
      if (!flag(Opt->quiet))
	printf("Skipping inverse_order, because there is a function_order (lex) command.\n");
    }
    else if (stringparm(Opt->order, "kbo")) {
      if (!flag(Opt->quiet))
	printf("Skipping inverse_order, because term ordering is KBO.\n");
    }
    else {
      BOOL change = inverse_order(Glob.sos);
      if (!flag(Opt->quiet)) {
	printf("After inverse_order: ");
	if (change)
	  print_fsym_precedence(stdout);
	else
	  printf(" (no changes).\n");
      }
    }
  }

  if (stringparm(Opt->eq_defs, "unfold")) {
    if (exists_preliminary_precedence(FUNCTION_SYMBOL))  // lex command
      printf("Skipping unfold_eq, because there is a function_order (lex) command.\n");
    else
      unfold_eq_defs(Glob.sos, INT_MAX, 3, !flag(Opt->quiet));
  }
  else if (stringparm(Opt->eq_defs, "fold")) {
    if (exists_preliminary_precedence(FUNCTION_SYMBOL))  // lex command
      printf("Skipping fold_eq, because there is a function_order (lex) command.\n");
    else {
      BOOL change = fold_eq_defs(Glob.sos, stringparm(Opt->order, "kbo"));
      if (!flag(Opt->quiet)) {
	printf("After fold_eq: ");
	if (change)
	  print_fsym_precedence(stdout);
	else
	  printf(" (no changes).\n");
      }
    }
  }

  // Automatic inference and processing settings
  
  if (flag(Opt->auto_inference))
    auto_inference(Glob.sos, Glob.usable, Opt);

  if (flag(Opt->auto_process))
    auto_process(Glob.sos, Glob.usable, Opt);

  // Tell packages about options and other things.

  resolution_options(flag(Opt->ordered_res),
		     flag(Opt->check_res_instances),
		     flag(Opt->initial_nuclei),
		     parm(Opt->ur_nucleus_limit),
		     flag(Opt->eval_rewrite));
		     
  paramodulation_options(flag(Opt->ordered_para),
			 flag(Opt->check_para_instances),
			 FALSE,
			 flag(Opt->basic_paramodulation),
			 flag(Opt->para_from_vars),
			 flag(Opt->para_into_vars),
			 flag(Opt->para_from_small));

}  /* init_search */

/*************
 *
 *   index_and_process_initial_clauses()
 *
 *************/

static
void index_and_process_initial_clauses(void)
{
  Clist_pos p;
  Clist temp_sos;

  // Index Usable clauses if hyper, UR, or binary-res are set.

  Glob.use_clash_idx = (flag(Opt->binary_resolution) ||
			flag(Opt->neg_binary_resolution) ||
			flag(Opt->pos_hyper_resolution) ||
			flag(Opt->neg_hyper_resolution) ||
			flag(Opt->pos_ur_resolution) ||
			flag(Opt->neg_ur_resolution));

  // Allocate and initialize indexes (even if they won't be used).

  init_literals_index();  // fsub, bsub, fudel, budel, ucon

  init_demodulator_index(DISCRIM_BIND, ORDINARY_UNIF, 0);

  init_back_demod_index(FPA, ORDINARY_UNIF, 10);

  Glob.clashable_idx = lindex_init(FPA, ORDINARY_UNIF, 10,
				   FPA, ORDINARY_UNIF, 10);

  init_hints(ORDINARY_UNIF, Att.bsub_hint_wt,
	     flag(Opt->collect_hint_labels),
	     flag(Opt->back_demod_hints),
	     demodulate_clause);
  init_semantics(Glob.interps, Clocks.semantics,
		 stringparm1(Opt->multiple_interps),
		 parm(Opt->eval_limit),
		 parm(Opt->eval_var_limit));

  // Do Sos and Denials last, in case we PROCESS_INITIAL_SOS.

  ////////////////////////////////////////////////////////////////////////////
  // Usable

  for (p = Glob.usable->first; p != NULL; p = p->next) {
    Topform c = p->c;
    assign_clause_id(c);
    mark_maximal_literals(c->literals);
    mark_selected_literals(c->literals, stringparm1(Opt->literal_selection));
    if (flag(Opt->dont_flip_input))
      orient_equalities(c, FALSE);  // mark, but don't allow flips
    else
      c = orient_input_eq(c);  /* this replaces c if any flipping occurs */
    index_literals(c, INSERT, Clocks.index, FALSE);
    index_back_demod(c, INSERT, Clocks.index, flag(Opt->back_demod));
    index_clashable(c, INSERT);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Demodulators

  if (!clist_empty(Glob.demods) && !flag(Opt->eval_rewrite)) {
    fflush(stdout);
    bell(stderr);
    fprintf(stderr,
	    "\nWARNING: The use of input demodulators is not well tested\n"
	    "and discouraged.  You might need to clear(process_initial_sos)\n"
	    "so that sos clauses are not rewritten and deleted.\n");
    fflush(stderr);
  }

  for (p = Glob.demods->first; p != NULL; p = p->next) {
    Topform c = p->c;
    assign_clause_id(c);
    if (flag(Opt->eval_rewrite)) {
      if (c->is_formula) {
	/* make it into a pseudo-clause */
	c->literals = new_literal(TRUE, formula_to_term(c->formula));
	upward_clause_links(c);
	zap_formula(c->formula);
	c->formula = NULL;
	c->is_formula = FALSE;
	clause_set_variables(c, MAX_VARS);
	mark_oriented_eq(c->literals->atom);
      }
    }
    else {
      if (!pos_eq_unit(c->literals))
	fatal_error("input demodulator is not equation");
      else {
	int type;
	if (flag(Opt->dont_flip_input))
	  orient_equalities(c, FALSE);  /* don't allow flips */
	else
	  c = orient_input_eq(c);  /* this replaces c if any flipping occurs */
	if (c->justification->next != NULL) {
	  printf("\nNOTE: input demodulator %d has been flipped.\n", c->id);
	  fflush(stdout);
	  fprintf(stderr, "\nNOTE: input demodulator %d has been flipped.\n",
		  c->id);
	  if (flag(Opt->bell))
	    bell(stderr);
	  fflush(stderr);
	}
	type = demodulator_type(c,
				parm(Opt->lex_dep_demod_lim),
				flag(Opt->lex_dep_demod_sane));
	if (flag(Opt->dont_flip_input) &&
	    type != ORIENTED &&
	    !renamable_flip_eq(c->literals->atom)) {
	  type = ORIENTED;  /* let the user beware */
	  mark_oriented_eq(c->literals->atom);
	  bell(stderr);
	  fprintf(stderr,"\nWARNING: demodulator does not satisfy term order\n");
	  fflush(stderr);
	  printf("\nWARNING: demodulator does not satisfy term order: ");
	  f_clause(c);
	  fflush(stdout);
	}
	else if (type == NOT_DEMODULATOR) {
	  Term a = ARG(c->literals->atom,0);
	  Term b = ARG(c->literals->atom,1);
	  printf("bad input demodulator: "); f_clause(c);
	  if (term_ident(a, b))
	    fatal_error("input demodulator is instance of x=x");
	  else if (!variables_subset(a, b) && !variables_subset(b, a))
	    fatal_error("input demoulator does not have var-subset property");
	  else
	    fatal_error("input demoulator not allowed");
	}
	index_demodulator(c, type, INSERT, Clocks.index);
      }
    }
  }

  if (flag(Opt->eval_rewrite))
    init_dollar_eval(Glob.demods);

  ////////////////////////////////////////////////////////////////////////////
  // Hints
  
  if (Glob.hints->first) {
    for (p = Glob.hints->first; p != NULL; p = p->next) {
      Topform h = p->c;
      // assign_clause_id(h);  // This should be optional
      orient_equalities(h, TRUE);
      renumber_variables(h, MAX_VARS);
      index_hint(h);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Sos

  // Move Sos to a temporary list, then process that temporary list,
  // putting the clauses back into Sos in the "correct" way, either
  // by calling cl_process() or doing it here.

  temp_sos = Glob.sos;                    // move Sos to a temporary list
  name_clist(temp_sos, "temp_sos");       // not really necessary
  Glob.sos = clist_init("sos");           // get a new (empty) Sos list

  if (flag(Opt->process_initial_sos)) {

    if (flag(Opt->print_initial_clauses))
      printf("\n");

    while (temp_sos->first) {
      Topform c = temp_sos->first->c;
      Topform new;
      clist_remove(c, temp_sos);
      clist_append(c, Glob.disabled);
      
      new = copy_inference(c);  // c has no ID, so this is tricky
      cl_process_simplify(new);
      if (new->justification->next == NULL) {
	// No simplification occurred, so make it a clone of the parent.
	zap_just(new->justification);
	new->justification = copy_justification(c->justification);
	// Get all attributes, not just inheritable ones.
	zap_attributes(new->attributes);
	new->attributes = copy_attributes(c->attributes);
      }
      else {
	// Simplification occurs, so make it a child of the parent.
	assign_clause_id(c);
	new->justification->u.id = c->id;
	if (flag(Opt->print_initial_clauses)) {
	  printf("           ");
	  fwrite_clause(stdout, c, CL_FORM_STD);
	}
      }
      cl_process(new);  // This re-simplifies, but that's ok.
    }
    // This will put processed clauses back into Sos.
    limbo_process(TRUE);  // back subsumption and back demodulation.

  }
  else {
    /* not processing initial sos */
    fflush(stdout);
    bell(stderr);
    fprintf(stderr,
	    "\nWARNING: clear(process_initial_sos) is not well tested.\n"
	    "We usually recommend against using it.\n");
    fflush(stderr);
    
    /* not applying full processing to initial sos */
    while (temp_sos->first) {
      Topform c = temp_sos->first->c;
      clist_remove(c, temp_sos);

      if (number_of_literals(c->literals) == 0)
	/* in case $F is in input, or if predicate elimination finds proof */
	handle_proof_and_maybe_exit(c);
      else {
	assign_clause_id(c);
	if (flag(Opt->dont_flip_input))
	  orient_equalities(c, FALSE);
	else
	  c = orient_input_eq(c);
	mark_maximal_literals(c->literals);
	mark_selected_literals(c->literals,
			       stringparm1(Opt->literal_selection));
	c->weight = clause_weight(c->literals);
	if (!clist_empty(Glob.hints)) {
	  clock_start(Clocks.hints);
	  adjust_weight_with_hints(c,
				   flag(Opt->degrade_hints),
				   flag(Opt->breadth_first_hints));
	  clock_stop(Clocks.hints);
	}

	c->initial = TRUE;
	insert_into_sos2(c, Glob.sos);
	index_literals(c, INSERT, Clocks.index, FALSE);
	index_back_demod(c, INSERT, Clocks.index, flag(Opt->back_demod));
      }
    }
  }

  clist_zap(temp_sos);  // free the temporary list

  ////////////////////////////////////////////////////////////////////////////
  // Print

  print_separator(stdout, "end of process initial clauses", TRUE);

  print_separator(stdout, "CLAUSES FOR SEARCH", TRUE);

  if (flag(Opt->print_initial_clauses)) {
    printf("\n%% Clauses after input processing:\n");
    fwrite_clause_clist(stdout,Glob.usable,  CL_FORM_STD);
    fwrite_clause_clist(stdout,Glob.sos,     CL_FORM_STD);
    fwrite_demod_clist(stdout,Glob.demods,   CL_FORM_STD);
  }
  if (Glob.hints->length > 0) {
      int redundant = redundant_hints();
      printf("\n%% %d hints (%d processed, %d redundant).\n",
	     Glob.hints->length - redundant, Glob.hints->length, redundant);
    }

  print_separator(stdout, "end of clauses for search", TRUE);

}  // index_and_process_initial_clauses

/*************
 *
 *   fatal_setjmp()
 *
 *************/

static
void fatal_setjmp(void)
{
  int return_code = setjmp(Jump_env);
  if (return_code != 0)
    fatal_error("longjmp called outside of search");
}  /* fatal_setjmp */

/*************
 *
 *   collect_prover_results()
 *
 *************/

static
Prover_results collect_prover_results(BOOL xproofs)
{
  Plist p;
  Prover_results results = calloc(1, sizeof(struct prover_results));

  for (p = Glob.empties; p; p = p->next) {
    Plist proof = get_clause_ancestors(p->v);
    uncompress_clauses(proof);
    results->proofs = plist_append(results->proofs, proof);
    if (xproofs) {
      Plist xproof = proof_to_xproof(proof);
      results->xproofs = plist_append(results->xproofs, xproof);
    }
  }
  update_stats();  /* puts package stats into Stats */
  results->stats = Stats;  /* structure copy */
  results->user_seconds = user_seconds();
  results->system_seconds = system_seconds();
  results->return_code = Glob.return_code;
  return results;
}  /* collect_prover_results */

/*************
 *
 *   zap_prover_results()
 *
 *************/

/* DOCUMENTATION
Free the dynamically allocated memory associated with a Prover_result.
*/

/* PUBLIC */
void zap_prover_results(Prover_results results)
{
  Plist a, b;  /* results->proofs is a Plist of Plist of clauses */
  for (a = results->proofs; a; a = a->next) {
    for (b = a->v; b; b = b->next) {
      Topform c = b->v;
      /* There is a tricky thing going on with the ID.  If you try
	 to delete a clause with an ID not in the clause ID table,
	 a fatal error occurs.  If IDs in these clauses came from
	 a child process, they will not be in the table.  Setting
	 the ID to 0 gets around that problem.
       */
      c->id = 0;
      delete_clause(c);  /* zaps justification, attributes */
    }
  }
  free(results);
}  /* zap_prover_results */

/*************
 *
 *   basic_clause_properties()
 *
 *************/

static
void basic_clause_properties(Clist sos, Clist usable)
{
  Plist sos_temp    = copy_clist_to_plist_shallow(sos);
  Plist usable_temp = copy_clist_to_plist_shallow(usable);

  Glob.equality = 
    pos_equality_in_clauses(sos_temp) || pos_equality_in_clauses(usable_temp);
    
  Glob.unit =
    all_clauses_unit(sos_temp) && all_clauses_unit(usable_temp);

  Glob.horn =
    all_clauses_horn(sos_temp) && all_clauses_horn(usable_temp);

  Glob.number_of_clauses =
    plist_count(sos_temp) + plist_count(usable_temp);

  Glob.number_of_neg_clauses =
    negative_clauses(sos_temp) + negative_clauses(usable_temp);

  zap_plist(sos_temp);
  zap_plist(usable_temp);
}  /* basic_clause_properties */

/*************
 *
 *   search()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Prover_results search(Prover_input p)
{
  int return_code = setjmp(Jump_env);
  if (return_code != 0) {
    // we just landed from longjmp(); fix return code and return
    print_separator(stdout, "end of search", TRUE);
    Glob.return_code = (return_code == INT_MAX ? 0 : return_code);
    fatal_setjmp();  /* This makes longjmps cause a fatal_error. */
    return collect_prover_results(p->xproofs);
  }
  else {
    // search for a proof

    print_separator(stdout, "PROCESS INITIAL CLAUSES", TRUE);

    Opt = p->options;          // put options into a global variable
    Glob.initialized = TRUE;   // this signifies that Glob is being used

    Glob.start_time  = user_seconds();
    Glob.start_ticks = bogo_ticks();

    if (flag(Opt->sort_initial_sos) && plist_count(p->sos) <= 100)
      p->sos = sort_plist(p->sos,
			  (Ordertype (*)(void*, void*)) clause_compare_m4);

    // Move clauses and term lists into Glob; do not assign IDs to clauses.

    Glob.usable  = move_clauses_to_clist(p->usable, "usable", FALSE);
    Glob.sos     = move_clauses_to_clist(p->sos, "sos", FALSE);
    Glob.demods  = move_clauses_to_clist(p->demods,"demodulators",FALSE);
    Glob.hints   = move_clauses_to_clist(p->hints, "hints", FALSE);

    Glob.weights          = tlist_copy(p->weights);
    Glob.kbo_weights      = tlist_copy(p->kbo_weights);
    Glob.actions          = tlist_copy(p->actions);
    Glob.interps          = tlist_copy(p->interps);
    Glob.given_selection  = tlist_copy(p->given_selection);
    Glob.keep_rules       = tlist_copy(p->keep_rules);
    Glob.delete_rules     = tlist_copy(p->delete_rules);

    // Allocate auxiliary clause lists.

    Glob.limbo    = clist_init("limbo");
    Glob.disabled = clist_init("disabled");
    Glob.empties  = NULL;

    if (flag(Opt->print_initial_clauses)) {
      printf("\n%% Clauses before input processing:\n");
      fwrite_clause_clist(stdout, Glob.usable,  CL_FORM_STD);
      fwrite_clause_clist(stdout, Glob.sos,     CL_FORM_STD);
      fwrite_clause_clist(stdout, Glob.demods,  CL_FORM_STD);
      if (Glob.hints->length > 0)
	printf("\n%% %d hints input.\n", Glob.hints->length);
    }
    
    // Predicate elimination (may add to sos and move clauses to disabled)

    if (flag(p->options->predicate_elim) && clist_empty(Glob.usable)) {
      print_separator(stdout, "PREDICATE ELIMINATION", TRUE);
      predicate_elimination(Glob.sos, Glob.disabled, !flag(Opt->quiet));
      print_separator(stdout, "end predicate elimination", TRUE);
    }

    basic_clause_properties(Glob.sos, Glob.usable);
    
    // Possible special treatment for denials (negative in Horn sets)

    if (flag(Opt->auto_denials))
      auto_denials(Glob.sos, Glob.usable, Opt);

    init_search();  // init clocks, ordering, auto-mode, init packages
    
    index_and_process_initial_clauses();

    print_separator(stdout, "SEARCH", TRUE);

    printf("\n%% Starting search at %.2f seconds.\n", user_seconds());
    fflush(stdout);
    Glob.start_time = user_seconds();
    Glob.searching = TRUE;

    // ****************************** Main Loop ******************************

    while (inferences_to_make()) {

      // make_inferences: each inferred clause is cl_processed, which
      // does forward demodulation and subsumption; if the clause is kept
      // it is put on the Limbo list, and it is indexed so that it can be
      // used immediately with subsequent newly inferred clauses.

      make_inferences();

      // limbo_process: this applies back subsumption, back demodulation,
      // and other operations that can disable clauses.  Limbo clauses
      // are moved to the Sos list.

      limbo_process(FALSE);

    }  // ************************ end of main loop ************************

    fprint_all_stats(stdout, Opt ? stringparm1(Opt->stats) : "lots");
    print_separator(stdout, "end of search", TRUE);
    fatal_setjmp();  /* This makes longjmps cause a fatal_error. */
    Glob.return_code = SOS_EMPTY_EXIT;
    return collect_prover_results(p->xproofs);
  }
}  /* search */

/*************
 *
 *   forking_search()
 *
 *************/

/* DOCUMENTATION
This is similar to search(), except that a child process is created
to do the search, and the child sends its results to the parent on
a pipe.

<P>
The parameters and results are the same as search().
As in search(), the Plists lists of objets (the parameters) are not changed.
*/

/* PUBLIC */
Prover_results forking_search(Prover_input input)
{
  Prover_results results;

  int rc;
  int fd[2];          /* pipe for child -> parent data */

  rc = pipe(fd);
  if (rc != 0) {
    perror("");
    fatal_error("forking_search: pipe creation failed");
  }

  fflush(stdout);
  fflush(stderr);
  rc = fork();
  if (rc < 0) {
    perror("");
    fatal_error("forking_search: fork failed");
  }

  {  /* kludge to get labels that might be introduced by child into symtab */
    int i = str_to_sn("flip_matches_hint", 0);
    i++;  /* prevents warning about unused variable */
  }

  if (rc == 0) {

    /*********************************************************************/
    /* This is the child process.  Search, send results to parent, exit. */
    /*********************************************************************/

    int to_parent = fd[1];  /* fd for writing data to parent */
    close(fd[0]);           /* close "read" end of pipe */

    fprintf(stdout,"\nChild search process %d started.\n", my_process_id());

    /* Remember how many symbols are in the symbol table.  If new symbols
       are introduced during the search, we have to send them to the
       parent so that clauses sent to the parent can be reconstructed.
    */

    mark_for_new_symbols();

    /* search */
    
    results = search(input);

    /* send results to the parent */

    {
      /* Format of data (all integers) sent to parent:
	---------------------- 
	nymber-of-new-symbols
	  symnum
	  arity
          ...
	number-of-proofs
	  number-of-steps
	    [clauses-in-proof]
	  number-of-steps
	    [clauses-in-proof]
          ...
        [same for xproofs]

	stats  (MAX_STATS of them)
	user_milliseconds
	system_milliseconds
	return_code

      */

      Ibuffer ibuf = ibuf_init();
      Plist p, a;
      I2list new_symbols, q;

      /* collect and write new_symbols */

      new_symbols = new_symbols_since_mark();
      ibuf_write(ibuf, i2list_count(new_symbols));
      for (q = new_symbols; q; q = q->next) {
	ibuf_write(ibuf, q->i);
	ibuf_write(ibuf, q->j);
      }
      zap_i2list(new_symbols);

      /* collect and write proofs */

      ibuf_write(ibuf, plist_count(results->proofs));  /* number of proofs */
      for (p = results->proofs; p; p = p->next) {
	ibuf_write(ibuf, plist_count(p->v));  /* steps in this proof */
	for (a = p->v; a; a = a->next) {
	  put_clause_to_ibuf(ibuf, a->v);
	}
      }
      
      /* collect and write xproofs */

      ibuf_write(ibuf, plist_count(results->xproofs));  /* number of xproofs */
      for (p = results->xproofs; p; p = p->next) {
	ibuf_write(ibuf, plist_count(p->v));  /* steps in this proof */
	for (a = p->v; a; a = a->next)
	  put_clause_to_ibuf(ibuf, a->v);
      }
      
      {
	/* collect stats (shortcut: handle stats struct as sequence of ints) */
	int *x = (void *) &(results->stats);
	int n = sizeof(struct prover_stats) / sizeof(int);
	int i;
	for (i = 0; i < n; i++)
	  ibuf_write(ibuf, x[i]);
      }

      /* collect clocks */
      ibuf_write(ibuf, (int) (results->user_seconds * 1000));
      ibuf_write(ibuf, (int) (results->system_seconds * 1000));
      /* collect return_code */
      ibuf_write(ibuf, results->return_code);

      /* write the data to the pipe */

      rc = write(to_parent,
		 ibuf_buffer(ibuf),
		 ibuf_length(ibuf) * sizeof(int));
      if (rc == -1) {
	perror("");
	fatal_error("forking_search, write error");
      }
      else if (rc != ibuf_length(ibuf) * sizeof(int))
	fatal_error("forking_search, incomplete write from child to parent");

      rc = close(to_parent);
      
      ibuf_free(ibuf);  /* not necessary, because we're going to exit now */
    }

    /* child exits */

    exit_with_message(stdout, results->return_code);
    
    return NULL;  /* won't happen */

  }  /* end of child code */

  else {

    /*********************************************************************/
    /* This is the parent process.  Get results from child, then return. */
    /*********************************************************************/

    int from_child = fd[0];  /* fd for reading data from child */
    close(fd[1]);            /* close "write" end of pipe */

    /* read results from child (read waits until data are available) */

    {
      Ibuffer ibuf = fd_read_to_ibuf(from_child);
      int num_proofs, num_steps, i, j;
      int num_new_symbols;
      I2list new_syms = NULL;

      results = calloc(1, sizeof(struct prover_results));
      
      /* read new_symbols */

      num_new_symbols = ibuf_read(ibuf);
      for (i = 0; i < num_new_symbols; i++) {
	int symnum = ibuf_read(ibuf);
	int arity = ibuf_read(ibuf);
	new_syms = i2list_append(new_syms, symnum, arity);
      }
      add_new_symbols(new_syms);  /* add new symbols to symbol table */
      zap_i2list(new_syms);

      /* read proofs */

      num_proofs = ibuf_read(ibuf);
      for (i = 0; i < num_proofs; i++) {
	Plist proof = NULL;
	num_steps = ibuf_read(ibuf);
	for (j = 0; j < num_steps; j++) {
	  Topform c = get_clause_from_ibuf(ibuf);
	  proof = plist_prepend(proof, c);  /* build backward, reverse later */
	}
	results->proofs = plist_append(results->proofs, reverse_plist(proof));
      }

      /* read xproofs */

      num_proofs = ibuf_read(ibuf);
      for (i = 0; i < num_proofs; i++) {
	Plist proof = NULL;
	num_steps = ibuf_read(ibuf);
	for (j = 0; j < num_steps; j++) {
	  Topform c = get_clause_from_ibuf(ibuf);
	  proof = plist_prepend(proof, c);  /* build backward, reverse later */
	}
	results->xproofs = plist_append(results->xproofs,reverse_plist(proof));
      }

      {
	/* read stats (shortcut: handle stats struct as sequence of ints) */
	int *x = (void *) &(results->stats);
	int n = sizeof(struct prover_stats) / sizeof(int);
	int i;
	for (i = 0; i < n; i++)
	  x[i] = ibuf_read(ibuf);
      }

      /* read clocks */
      results->user_seconds = ibuf_read(ibuf) / 1000.0;
      results->system_seconds = ibuf_read(ibuf) / 1000.0;
      /* read return_code */
      results->return_code = ibuf_read(ibuf);
    }

    /* Wait for child to exit and get the exit code.  We should not
       have to wait long, because we already have its results. */

    {
      int child_status, child_exit_code;
      wait(&child_status);
      if (!WIFEXITED(child_status))
	fatal_error("forking_search: child terminated abnormally");
      child_exit_code = WEXITSTATUS(child_status);
      results->return_code = child_exit_code;
    }

    rc = close(from_child);

    return results;
  }  /* end of parent code */
}  /* forking_search */
