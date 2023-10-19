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

#ifndef TP_SEARCH_H
#define TP_SEARCH_H

#include "search-structures.h"  /* includes all of LADR */

// Local includes

#include "semantics.h"
#include "pred_elim.h"
#include "demodulate.h"
#include "index_lits.h"
#include "forward_subsume.h"
#include "unfold.h"
#include "actions.h"
#include "giv_select.h"
#include "white_black.h"
#include "utilities.h"

/* INTRODUCTION
*/

/* Public definitions */


/* End of public definitions */

/* Public function prototypes from search.c */

Prover_options init_prover_options(void);

void init_prover_attributes(void);

int get_attrib_id(char *str);

void fprint_prover_clocks(FILE *fp, struct prover_clocks clks);

void fprint_all_stats(FILE *fp, char *stats_level);

void exit_with_message(FILE *fp, int code);

void report(FILE *fp, char *level);

void free_search_memory(void);

void zap_prover_results(Prover_results results);

Prover_results search(Prover_input p);

Prover_results forking_search(Prover_input input);

#endif  /* conditional compilation of whole file */
