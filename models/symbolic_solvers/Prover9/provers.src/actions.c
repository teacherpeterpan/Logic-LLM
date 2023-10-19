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

#include "actions.h"

/* Private definitions and types */

/* These lists are for rules such as
     given = 13 -> set(print_kept).
   This is a slow version, because the rules are parsed
   each time they might be applied, that is, at each
   given, generated, and kept clause.
   We might have to change to a more efficient data structure.
*/

static Plist Given_rules     = NULL;
static Plist Generated_rules = NULL;
static Plist Kept_rules      = NULL;
static Plist Level_rules     = NULL;

/* Some changes to flags or parms require rebuilding the sos index;
   a pointer to the procedure to do so is stored here.
*/

static void (*Rebuild_sos_proc) (void);
static void (*Exit_proc) (int);  /* this is called for "exit" actions */
static void (*Assert_proc) (Topform);  /* for "assert" actions */

/*****************************************************************************/
/* What flags and parms can be changed by actions? */

#define NN 100  /* make sure that each array is <= this! */

/* These flags and parms can simply be changed. */

static char *Changable_flags[NN] = {
  "reuse_denials",
  "print_gen",
  "print_kept",
  "print_given"};

static char *Changable_parms[NN] = {
  "demod_step_limit",
  "demod_size_limit",
  "new_constants",
  "para_lit_limit",
  "stats",
  "max_given",
  "max_weight",
  "max_depth",
  "max_vars",
  "max_proofs",
  "max_literals"};

/* These flags and parms affect previously stored data on weighting and
   selection of the given clause.  They can be changed if the routine
   rebuild_sos_proc() is called, which reweighs sos clauses and rebuilds
   the sos indexes. */

static char *Changable_flags_rebuild[NN] = {
  "breadth_first",
  "lightest_first",
  "breadth_first_hints"};

static char *Changable_parms_rebuild[NN] = {
  "constant_weight",
  "variable_weight",
  "not_weight",
  "or_weight",
  "prop_atom_weight",
  "nest_penalty",
  "depth_penalty",
  "skolem_penalty",
  "default_weight",
  
  "pick_given_ratio",
  "hints_part",
  "age_part",
  "weight_part",
  "false_part",
  "true_part",
  "random_part"};


/*****************************************************************************/

/*************
 *
 *   changable_flag()
 *
 *************/

static
BOOL changable_flag(Term t)
{
  return ((is_term(t, "set", 1) || is_term(t, "clear", 1)) &&
	  string_member(sn_to_str(SYMNUM(ARG(t,0))),
			Changable_flags,
			NN));
}  /* changable_flag */

/*************
 *
 *   changable_parm()
 *
 *************/

static
BOOL changable_parm(Term t)
{
  return (is_term(t, "assign", 2) &&
	  string_member(sn_to_str(SYMNUM(ARG(t,0))),
			Changable_parms,
			NN));
}  /* changable_parm */

/*************
 *
 *   changable_flag_rebuild()
 *
 *************/

static
BOOL changable_flag_rebuild(Term t)
{
  return ((is_term(t, "set", 1) || is_term(t, "clear", 1)) &&
	  string_member(sn_to_str(SYMNUM(ARG(t,0))),
			Changable_flags_rebuild,
			NN));
}  /* changable_flag_rebuild */

/*************
 *
 *   changable_parm_rebuild()
 *
 *************/

static
BOOL changable_parm_rebuild(Term t)
{
  return (is_term(t, "assign", 2) &&
	  string_member(sn_to_str(SYMNUM(ARG(t,0))),
			Changable_parms_rebuild,
			NN));
}  /* changable_parm_rebuild */

/*************
 *
 *   init_action()
 *
 *************/

/* DOCUMENTATION
This is one of two ways to set up an action.
<P>
Also see register_action(), for which the rule Term is not yet built.
*/

/* PUBLIC */
void init_action(Term t)
{
  Term rule = copy_term(t);
  Term trigger, stat;

  if (!is_term(rule, "->", 2))
    fatal_error("init_actions, bad rule");
  trigger = ARG(rule,0);
  if (!is_term(trigger, eq_sym(), 2))
    fatal_error("init_actions, bad trigger");
  stat = ARG(trigger,0);

  if (is_constant(stat, "given"))
    Given_rules = plist_append(Given_rules, rule);
  else if (is_constant(stat, "generated"))
    Generated_rules = plist_append(Generated_rules, rule);
  else if (is_constant(stat, "kept"))
    Kept_rules = plist_append(Kept_rules, rule);
  else if (is_constant(stat, "level"))
    Level_rules = plist_append(Level_rules, rule);
  else
    fatal_error("init_action, bad statistic");
}  /* init_action */

/*************
 *
 *   init_actions()
 *
 *************/

/* DOCUMENTATION
The actions are copied.
*/

/* PUBLIC */
void init_actions(Plist rules,
		  void (*rebuild_sos_proc)(void),
		  void (*exit_proc)(int),
		  void (*assert_proc)(Topform))
{
  /* rule:          trigger -> action
     Example:     given=100 -> assign(max_weight, 25)
  */
  Plist p;
  for (p = rules; p; p = p->next)
    init_action(p->v);

  /* Some changes to flags or parms require rebuilding the sos index. */
  Rebuild_sos_proc = rebuild_sos_proc; 
  Exit_proc = exit_proc;
  Assert_proc = assert_proc;

}  /* init_actions */

/*************
 *
 *   register_action()
 *
 *************/

/* DOCUMENTATION
This is one of two ways to set up an action.
<P>
Also see init_action(), which takes the rule as a Term.
*/

/* PUBLIC */
void register_action(char *stat, char *val, char *op, char *arg1, char *arg2)
{
  Term action, rule;
  
  if (str_ident(op, "set") || str_ident(op, "clear"))
    action = term1(op, term0(arg1));
  else if (str_ident(op, "assign"))
    action = term2(op, term0(arg1), term0(arg2));
  else {
    fatal_error("register_action, operation must be set, clear, or assign");
    action = NULL;  /* to satisfy the compiler */
  }

  rule = term2("->",
	       term2("=",
		     term0(stat),
		     term0(val)),
	       action);
  
  if (str_ident(stat, "given"))
    Given_rules = plist_append(Given_rules, rule);
  else if (str_ident(stat, "generated"))
    Generated_rules = plist_append(Generated_rules, rule);
  else if (str_ident(stat, "kept"))
    Kept_rules = plist_append(Kept_rules, rule);
  else if (str_ident(stat, "level"))
    Level_rules = plist_append(Level_rules, rule);
  else
    fatal_error("register_action, bad statistic");

  printf("\n%% action: "); fwrite_term_nl(stdout, rule);
  
}  /* register_action */

/*************
 *
 *   apply_action()
 *
 *************/

static
BOOL apply_action(Term action)
{
  BOOL rebuild = FALSE;
  if (is_constant(action, "exit"))
    (*Exit_proc)(ACTION_EXIT);
  else if (is_term(action, "set", 1) || is_term(action, "clear", 1)) {
    if (changable_flag(action))
      flag_handler(stdout, action, TRUE, KILL_UNKNOWN);
    else if (changable_flag_rebuild(action)) {
      flag_handler(stdout, action, TRUE, KILL_UNKNOWN);
      rebuild = TRUE;
    }
    else
      fatal_error("apply_action, flag is not changable");
  }
  else if (is_term(action, "assign", 2)) {
    if (changable_parm(action))
      parm_handler(stdout, action, TRUE, KILL_UNKNOWN);
    else if (changable_parm_rebuild(action)) {
      parm_handler(stdout, action, TRUE, KILL_UNKNOWN);
      rebuild = TRUE;
    }
    else
      fatal_error("apply_action, parm is not changable");
  }
  else if (is_term(action, "assert", 1)) {
    Topform c = term_to_clause(ARG(action,0));
    clause_set_variables(c, MAX_VARS);
    c->justification = input_just();
    append_label_attribute(c, "asserted_by_action");
    printf("\nAsserting clause: "); f_clause(c);
    (*Assert_proc)(c);
  }
  else {
    fatal_error("apply_action, unknown action:");
    fwrite_term_nl(stdout, action);
  }
  return rebuild;
}  /* apply_action */

/*************
 *
 *   statistic_actions()
 *
 *************/

/* DOCUMENTATION
Given a statistic and a value, look in the rules
(given to init_actions) and apply as appropriate.

*/

/* PUBLIC */
void statistic_actions(char *stat, int n)
{
  Plist p = NULL;
  BOOL rebuild = FALSE;
  if (str_ident(stat, "given"))
    p = Given_rules;
  else if (str_ident(stat, "generated"))
    p = Generated_rules;
  else if (str_ident(stat, "kept"))
    p = Kept_rules;
  else if (str_ident(stat, "level"))
    p = Level_rules;
  else
    fatal_error("statistic_actions, unknown stat");
  
  for (; p; p = p->next) {
    Term rule = p->v;
    Term trigger = ARG(rule,0);
    Term tval = ARG(trigger,1);
    Term action = ARG(rule,1);
    int ival;
    if (!term_to_int(tval, &ival))
      fatal_error("statistic_actions, bad integer in trigger");
    if (n == ival) {
      printf("\n%% Applying rule: ");
      fwrite_term_nl(stdout, rule);
      if (apply_action(action))
	rebuild = TRUE;
    }
  }
  if (rebuild)
    (*Rebuild_sos_proc)();
}  /* statistic_actions */

/*************
 *
 *   proof_action()
 *
 *   Given a clause and an attribute ID; for each attribute
 *   of that type, look for a "trigger -> action".
 *
 *   The trigger here is "in_proof", and the actions supported are exit,
 *   set(flag), clear(flag), assign(parm, i), assign(stringparm, string).
 *
 *************/

static
void proof_action(Topform c, int attribute_id)
{
  BOOL rebuild = FALSE;
  int i = 0;
  Term t = get_term_attribute(c->attributes, attribute_id, ++i);
  while (t) {
    if (is_term(t, "->", 2) &&
	is_constant(ARG(t,0), "in_proof")) {
      Term action = ARG(t, 1);

      printf("\n%% Proof action: ");
      fwrite_term_nl(stdout, action);

      if (apply_action(action))
	rebuild = TRUE;
    }
    t = get_term_attribute(c->attributes, attribute_id, ++i);
  }
  if (rebuild)
    (*Rebuild_sos_proc)();
}  /* proof_action */

/*************
 *
 *   actions_in_proof()
 *
 *************/

/* DOCUMENTATION
Quick and dirty, until a general mechanism gets designed ...
If the proof contains any #action(in_proof -> <action>) attributes,
perform the action.

Actions supported here:  exit, set, clear, assign.
*/

/* PUBLIC */
void actions_in_proof(Plist proof,
		      Prover_attributes att)
{
  Plist p;
  for (p = proof; p; p = p->next) {
    proof_action(p->v, att->action);
    proof_action(p->v, att->action2);
  }
}  /* actions_in_proof */

