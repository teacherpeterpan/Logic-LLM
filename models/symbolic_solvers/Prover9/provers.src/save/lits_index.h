#ifndef TP_LITS_INDEX_H
#define TP_LITS_INDEX_H

#include "prover9.h"
#include "poptions.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from lits_index.c */

void init_lits_index(Mindextype mtype,
		     Uniftype utype,
		     int fpa_depth);

void lits_destroy_index(void);

void index_literals_old(Clause c, Indexop op, Clock clock);

Plist back_subsumption_old(Clause c);

Plist unit_conflict_old(Clause c, int max);

Plist back_unit_deletable_old(Clause c);

#endif  /* conditional compilation of whole file */
