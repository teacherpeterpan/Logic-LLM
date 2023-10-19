#ifndef TP_CONTROL_SOS_H
#define TP_CONTROL_SOS_H

#include "search-structures.h"
#include "utilities.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from control_sos.c */

int pick_method(Prover_options opt);

int pick_set(Topform c, Prover_options opt);

BOOL sos_keep(Topform c, Clist sos, Prover_options opt);

void sos_displace(Clist sos,
		  Prover_options opt,
		  void (*disable_proc) (Topform));

Topform get_given_clause(Clist sos, int num_given,
			Prover_options opt, char **type);

void adjust_sos_limit(Clist sos,
		      int num_given,
		      int ticks_spent,
		      Prover_options opt,
		      void (*disable_proc) (Topform));

int control_sos_removed(void);

#endif  /* conditional compilation of whole file */
