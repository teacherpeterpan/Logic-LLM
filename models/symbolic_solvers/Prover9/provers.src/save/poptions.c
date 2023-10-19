#include "../ladr/header.h"
#include "../ladr/options.h"
#include "poptions.h"

/*************
 *
 *    init_prover_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_prover_options(void)
{
  // FLAGS:
  //        internal name           external name             default

  init_flag(BINARY_RESOLUTION,      "binary_resolution",      FALSE);
  init_flag(HYPER_RESOLUTION,       "hyper_resolution",       FALSE);
  init_flag(UR_RESOLUTION,          "ur_resolution",          FALSE);
  init_flag(PARAMODULATION,         "paramodulation",         FALSE);
  init_flag(PARA_UNITS_ONLY,        "para_units_only",        FALSE);
  init_flag(BASIC_PARAMODULATION,   "basic_paramodulation",   FALSE);

  init_flag(PROCESS_INITIAL_SOS,    "process_initial_sos",    TRUE);

  init_flag(BACK_SUBSUME,           "back_subsume",           TRUE);
  init_flag(BACK_DEMOD,             "back_demod",             FALSE);
  init_flag(ORDERED_INFERENCE,      "ordered_inference",      TRUE);
  init_flag(ORDERED_INSTANCE,       "ordered_instance",       TRUE);
  init_flag(LEX_DEP_DEMOD,          "lex_dep_demod",          TRUE);
  init_flag(UNIT_DELETION,          "unit_deletion",          FALSE);
  init_flag(BACK_UNIT_DELETION,     "back_unit_deletion",     FALSE);
  init_flag(FACTOR,                 "factor",                 FALSE);

  init_flag(PRINT_GIVEN,            "print_given",            TRUE);
  init_flag(PRINT_GEN,              "print_gen",              FALSE);
  init_flag(PRINT_LABELED,          "print_labeled",          FALSE);
  init_flag(PRINT_KEPT,             "print_kept",             FALSE);
  init_flag(PRINT_MEMORY,           "print_memory",           FALSE);

  init_flag(DEGRADE_HINTS,          "degrade_hints",          TRUE);
  init_flag(REUSE_GOALS,            "reuse_goals",            FALSE);
  init_flag(CAC_REDUNDANCY,         "cac_redundancy",         TRUE);

  init_flag(CLAUSE_WT_MAX_LITS,     "clause_wt_max_lits",     FALSE);
  init_flag(NO_FAPL,                "no_fapl",                FALSE);

  init_flag(CHECK_FOR_LEAKS,        "check_for_leaks",        FALSE);
  init_flag(PROLOG_STYLE_VARIABLES, "prolog_style_variables", FALSE);
  init_flag(JUSTIFICATION_LAST,     "justification_last",     TRUE);
  init_flag(CLOCKS,                 "clocks",                 TRUE);

  init_flag(PRINT_PROOFS,           "print_proofs",           TRUE);
  init_flag(STANDARD_PROOFS,        "standard_proofs",        TRUE);
  init_flag(X_PROOFS,               "x_proofs",               TRUE);
  init_flag(R_PROOFS,               "r_proofs",               FALSE);
  init_flag(RX_PROOFS,              "rx_proofs",              FALSE);
  init_flag(XR_PROOFS,              "xr_proofs",              FALSE);

  init_flag(AUTO,                   "auto",                   FALSE);
  init_flag(HANDS_OFF_OPTIONS,      "hands_off_options",      FALSE);

  // PARMS:
  //        internal name     external name       default       min       max

  init_parm(MAX_MEGS,         "max_megs",             200,        0,  INT_MAX);
  init_parm(MAX_SECONDS,      "max_seconds",      INT_MAX,        0,  INT_MAX);
  init_parm(MAX_PROOFS,       "max_proofs",             1,       -1,  INT_MAX);
  init_parm(MAX_KEPT,         "max_kept",         INT_MAX,        0,  INT_MAX);
  init_parm(MAX_GIVEN,        "max_given",        INT_MAX,        0,  INT_MAX);
  init_parm(MAX_LITERALS,     "max_literals",     INT_MAX,        1,  INT_MAX);
  init_parm(MAX_WEIGHT,       "max_weight",       INT_MAX,  INT_MIN,  INT_MAX);
  init_parm(MIN_MAX_WEIGHT,   "min_max_weight",   INT_MIN,  INT_MIN,  INT_MAX);
  init_parm(PICK_GIVEN_RATIO, "pick_given_ratio",      -1,       -1,  INT_MAX);
  init_parm(BSUB_HINT_ADD_WT, "bsub_hint_add_wt",   -1000,  INT_MIN,  INT_MAX);
  init_parm(BSUB_HINT_WT,     "bsub_hint_wt",     INT_MAX,  INT_MIN,  INT_MAX);
  init_parm(CONSTANT_WEIGHT,  "constant_weight",        1,  INT_MIN,  INT_MAX);
  init_parm(VARIABLE_WEIGHT,  "variable_weight",        1,  INT_MIN,  INT_MAX);
  init_parm(LRS_INTERVAL,     "lrs_interval",          50,        1,  INT_MAX);
  init_parm(LRS_SECONDS,      "lrs_seconds",           -1,       -1,  INT_MAX);
  init_parm(LRS_CLAUSES,      "lrs_clauses",           -1,       -1,  INT_MAX);
  init_parm(LRS_GIVEN,        "lrs_given",             -1,       -1,  INT_MAX);
  init_parm(LRS_MEGS,         "lrs_megs",              -1,       -1,  INT_MAX);
  init_parm(SOS_TARGET_SIZE,  "sos_target_size",  INT_MAX,        0,  INT_MAX);
  init_parm(DEFAULT_WEIGHT,   "default_weight",   INT_MAX,  INT_MIN,  INT_MAX);
  init_parm(REPORT,           "report",                -1,       -1,  INT_MAX);

  // STRINGPARMS:
  // (internal-name, external-name, number-of-strings, str1, str2, ... )
  // str1 is always the default

  init_stringparm(NEG_SELECTION, "neg_selection", 5,
		  "maximal",
		  "first",
		  "first_maximal",
		  "minimal",
		  "all");

  init_stringparm(STATS, "stats", 4,
		  "all",
		  "lots",
		  "some",
		  "none");

  // Flag and parm Dependencies.  These cause other flags and parms
  // to be changed.  These happen immediately and can be undone
  // by later settings in the input.

  flag_flag_dependency(PARAMODULATION, TRUE, BACK_DEMOD, TRUE);
  flag_flag_dependency(BACK_UNIT_DELETION, TRUE, UNIT_DELETION, TRUE);

  flag_parm_dependency(AUTO, TRUE, PICK_GIVEN_RATIO, 3);

  // flag_stringparm_dependency(AUTO_INFERENCE,TRUE,NEG_SELECTION,"maximal");

}  // init_prover_options

