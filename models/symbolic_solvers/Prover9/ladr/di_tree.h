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

#ifndef TP_DI_TREE_H
#define TP_DI_TREE_H

#include "features.h"
#include "topform.h"

/* INTRODUCTION
*/

/* Public definitions */

typedef struct di_tree * Di_tree;

struct di_tree {       /* node in an integer vector discrimination tree */
  int label;           /* label of node */
  Di_tree   next;      /* sibling */
  union {
    Di_tree kids;      /* for internal nodes */
    Plist data;        /* for leaves */
  } u;
};

/* End of public definitions */

/* Public function prototypes from di_tree.c */

int nonunit_fsub_tests(void);

int nonunit_bsub_tests(void);

Di_tree get_di_tree(void);

void free_di_tree(Di_tree p);

void fprint_di_tree_mem(FILE *fp, BOOL heading);

void p_di_tree_mem(void);

Di_tree init_di_tree(void);

void di_tree_insert(Ilist vec, Di_tree node, void *datum);

BOOL di_tree_delete(Ilist vec, Di_tree node, void *datum);

void zap_di_tree(Di_tree node, int depth);

void p_di_tree(Ilist vec, Di_tree node, int depth);

Topform forward_feature_subsume(Topform d, Di_tree root);

Plist back_feature_subsume(Topform c, Di_tree root);

unsigned mega_sub_calls(void);

#endif  /* conditional compilation of whole file */
