#ifndef TP_SOS_H
#define TP_SOS_H

#include "clist.h"

/* INTRODUCTION
*/

/* Public definitions */

enum { BY_AGE, BY_WEIGHT, BY_RATIO };
enum { SOS1, SOS2 };

/* End of public definitions */

/* Public function prototypes from sos.c */

void p_sos_tree(void);

void p_sos_dist(void);

void index_sos(Topform c, int set);

void insert_into_sos(Topform c, Clist sos, int set);

void remove_from_sos(Topform c, Clist sos, int set);

Topform first_sos_clause(Clist lst);

Topform lightest_sos_clause(int set);

Topform worst_sos_clause(Clist sos, int method);

int wt_of_clause_at(int set, double part);

int clauses_of_weight(int wt, int set);

void zap_sos_index(void);

#endif  /* conditional compilation of whole file */
