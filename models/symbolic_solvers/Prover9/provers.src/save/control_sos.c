#include "control_sos.h"

/* Private definitions and types */

static int Number_removed = 0;    /* disabled because limit is reduced */

/*************
 *
 *   pick_method()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int pick_method(Prover_options opt)
{
  if (parm(opt->true_part) != 0 || parm(opt->false_part) != 0)
    return parm(opt->age_part) != 0 ? BY_RATIO : BY_WEIGHT;
  else if (parm(opt->age_part) != 0)
    return BY_AGE;
  else {
    fatal_error("pick_method, all parts (age, true, false) are 0");
    return 0;
  }
}  /* pick_method */

/*************
 *
 *   pick_set()
 *
 *   Which Sos set to use?   If we're using both true_part and
 *   false_part, some clauses go into SOS2 and others
 *   go into SOS1.  Otherwise, all go into SOS1.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int pick_set(Topform c, Prover_options opt)
{
  if (parm(opt->true_part) != 0 && parm(opt->false_part) != 0)
    return (c->semantics ? SOS1 : SOS2);
  else
    return SOS1;
}  /* pick_set */

/*************
 *
 *   sos_keep() -- is a clause with the given weight good enough to keep
 *                 w.r.t. the sos limit_size?
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL sos_keep(Topform c, Clist sos, Prover_options opt)
{
  static int low_water = INT_MAX;
  int limit_size = parm(opt->sos_limit) == -1 ? INT_MAX : parm(opt->sos_limit);
  double x = .5;  /* When sos is at (x * limit), start being selective. */
  int current_size = clist_length(sos);
  if (current_size < x * limit_size)
    return TRUE;
  else if (current_size > limit_size) {
    char mess[100];
    sprintf(mess, "sos_keep, sos too big, limit is %d", parm(opt->sos_limit));
    fatal_error(mess);
    return FALSE;
  }
  else {
    double part = (1 + x) - ((double) current_size / limit_size);
    /* x <= part <= 1 */
    int limit = wt_of_clause_at(pick_set(c, opt), part);
    if (c->weight >= limit && c->weight < low_water) {
      low_water = c->weight;
      printf("\nLow Water (keep): wt=%d, part=%.2f, limit=%d\n",
	     c->weight, part, limit);
      fflush(stdout);
    }
    return c->weight < limit;
  }
}  /* sos_keep */

/*************
 *
 *   sos_displace() - if sos+limbo is at or above the limit, delete the worst
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sos_displace(Clist sos,
		  Prover_options opt,
		  void (*disable_proc) (Topform))
{
  static int low_water = INT_MAX;
  Topform worst;
  worst = worst_sos_clause(sos, pick_method(opt));
  if (worst == NULL) {
    printf("worst == NULL\n");
  }
  else {
    if (worst->weight < low_water) {
      low_water = worst->weight;
      printf("\nLow Water (displace): id=%d, wt=%d\n",
	     worst->id, worst->weight);
      fflush(stdout);
    }
    disable_proc(worst);
  }
}  /* sos_displace */

/*************
 *
 *   get_given_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform get_given_clause(Clist sos, int num_given,
			Prover_options opt, char **type)
{
  if (clist_empty(sos))
    return NULL;
  else {
    Topform giv;

    if (flag(opt->input_sos_first) && initial_clause(sos->first->c)) {
      giv = sos->first->c;
      *type = "I";
    }
    else {
      int age = parm(opt->age_part);
      int wt1 = parm(opt->false_part);
      int wt2 = parm(opt->true_part);
      int x = num_given % (age+wt1+wt2);

      if (x < age) {
	giv = first_sos_clause(sos);
	*type = "A";
      }
      else if (x < age + wt1) {
	giv = lightest_sos_clause(SOS2);
	*type = "F";
      }
      else {
	giv = lightest_sos_clause(SOS1);
	*type = "T";
      }
    }
    
    remove_from_sos(giv, sos, pick_set(giv, opt));

    return giv;
  }
}  // get_given_clause

/*************
 *
 *   control_sos_limit()
 *
 *************/

/*
This is called when the sos_limit_size changes.
If there are too many clauses in sos, the worst
clauses are disbaled (newest first).  Return the
number disabled.
*/

static
int control_sos_limit(Clist sos,
		      int limit_size,
		      int method,
		      void (*disable_proc) (Topform))
{
  int excess = clist_length(sos) - limit_size;

  if (excess <= 0)
    return 0;
  else {
    int i;
    int wt = 0;
    for (i = 0; i < excess; i++) {
      Topform c = worst_sos_clause(sos, method);
      wt = c->weight;
      (*disable_proc)(c);
      Number_removed++;
    }
    return excess;
  }
}  /* control_sos_limit */

/*************
 *
 *   sos_limit()
 *
 *************/

static
int sos_limit(int given, double spent, int limit)
{
  if (spent <= 0)
    return INT_MAX;
  else {
    int can_do = given * (limit / spent);
    return IMAX(can_do - given, 0);
  }
}  /* sos_limit */

/*************
 *
 *   adjust_sos_limit()
 *
 *   Possibly asjust the sos limit size.  If it is changed, and if there
 *   are too many clauses in sos, remove and disable the worst ones.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void adjust_sos_limit(Clist sos,
		      int num_given,
		      int ticks_spent,
		      Prover_options opt,
		      void (*disable_proc) (Topform))
{
  int ticks_limit = parm(opt->lrs_ticks);

  if (ticks_limit == -1)
    return;  /* no limit */
  else if (num_given % parm(opt->lrs_interval) != 0)
    return;  /* not the right time to check */
  else if (parm(opt->sos_limit) != -1 &&
	   parm(opt->sos_limit) <= parm(opt->min_sos_limit))
    return;  /* already at the minimum sos_limit */
  else if (ticks_spent > ticks_limit / 2)
    return;  /* don't adjust if we've already spent half of the limit */
  else {

    int new_limit = sos_limit(num_given, ticks_spent, ticks_limit);

    new_limit = IMAX(new_limit,parm(opt->min_sos_limit));

    if (new_limit != parm(opt->sos_limit)) {

      int num_disabled = control_sos_limit(sos,
					   new_limit,
					   pick_method(opt),
					   disable_proc);

      assign_parm(opt->sos_limit, new_limit, TRUE);

      if (!flag(opt->quiet)) {
	printf("\n%% SOS Control: given=%d, spent=%d/%d (ticks), "
	       "new_sos_limit=%d, disabled=%d, user_seconds=%.2f.\n",
	       num_given, ticks_spent, ticks_limit,
	       new_limit, num_disabled, user_seconds());
	fflush(stdout);
      }
    }
  }
}  /* adjust_sos_limit */
      
/*************
 *
 *   control_sos_removed()
 *
 *************/

/* DOCUMENTATION
Return the number of sos clasues disabled because the sos_limit_size
was reduced.
*/

/* PUBLIC */
int control_sos_removed(void)
{
  return Number_removed;
}  /* control_sos_removed */

