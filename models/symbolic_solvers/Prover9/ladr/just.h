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

#ifndef TP_JUST_H
#define TP_JUST_H

#include "clauseid.h"
#include "parse.h"

/* INTRODUCTION
*/

/* Public definitions */

typedef enum {
  INPUT_JUST,         /* Primary                              */
  GOAL_JUST,          /* Primary                              */
  DENY_JUST,          /* Primary    int       (ID)            */
  CLAUSIFY_JUST,      /* Primary    int       (ID)            */
  COPY_JUST,          /* Primary    int       (ID)            */
  BACK_DEMOD_JUST,    /* Primary    int       (ID)            */
  BACK_UNIT_DEL_JUST, /* Primary    int       (ID)            */
  NEW_SYMBOL_JUST,    /* Primary    int       (ID)            */
  EXPAND_DEF_JUST,    /* Primary    Ilist     (ID,def-ID)     */
  BINARY_RES_JUST,    /* Primary    Ilist                     */
  HYPER_RES_JUST,     /* Primary    Ilist                     */
  UR_RES_JUST,        /* Primary    Ilist                     */
  FACTOR_JUST,        /* Primary    Ilist     (ID,lit1,lit2)  */
  XXRES_JUST,         /* Primary    Ilist     (ID,lit)        */
  PARA_JUST,          /* Primary    Parajust                  */
  PARA_FX_JUST,       /* Primary    Parajust                  */
  PARA_IX_JUST,       /* Primary    Parajust                  */
  PARA_FX_IX_JUST,    /* Primary    Parajust                  */
  INSTANCE_JUST,      /* Primary    Instancejust              */
  PROPOSITIONAL_JUST, /* Primary    int       (ID)            */

  DEMOD_JUST,         /* Secondary  I3list                    */
  UNIT_DEL_JUST,      /* Secondary  Ilist     (lit,ID)        */
  FLIP_JUST,          /* Secondary  int       (lit)           */
  XX_JUST,            /* Secondary  int       (lit)           */
  MERGE_JUST,         /* Secondary  int       (lit)           */
  EVAL_JUST,          /* Secondary  int       (count)         */

  IVY_JUST,           /* Primary    Ivyjust                   */

  UNKNOWN_JUST        /* probably an error                    */
} Just_type;

typedef struct parajust * Parajust;

struct parajust {
  int from_id;
  int into_id;
  Ilist from_pos;
  Ilist into_pos;
};

typedef struct instancejust * Instancejust;

struct instancejust {
  int parent_id;
  Plist pairs;
};

typedef struct ivyjust * Ivyjust;

struct ivyjust {
  Just_type type;  /* input,resolve,paramod,instantiate,flip,propositional */
  int parent1;
  int parent2;
  Ilist pos1;
  Ilist pos2;
  Plist pairs;  /* for instantiate */
};

typedef struct just * Just;

struct just {
  Just_type type;
  Just next;
  union {
    int id;
    Ilist lst;
    Parajust para;
    I3list demod;
    Instancejust instance;
    Ivyjust ivy;
  } u;
};

/* A justification starts with a Primary part and then
   has a sequence (maybe none) of Secondary parts.
   Each type has some data, an integer or Ilist (integer
   list) giving clause IDs, or positions of literals or
   subterms.
*/

/* End of public definitions */

/* Public function prototypes from just.c */

Just get_just(void);

Parajust get_parajust(void);

Instancejust get_instancejust(void);

void fprint_just_mem(FILE *fp, BOOL heading);

void p_just_mem();

Just ivy_just(Just_type type,
	      int parent1, Ilist pos1,
	      int parent2, Ilist pos2,
	      Plist pairs);

Just input_just(void);

Just goal_just(void);

Just deny_just(Topform tf);

Just clausify_just(Topform tf);

Just expand_def_just(Topform tf, Topform def);

Just copy_just(Topform c);

Just propositional_just(Topform c);

Just new_symbol_just(Topform c);

Just back_demod_just(Topform c);

Just back_unit_deletion_just(Topform c);

Just binary_res_just(Topform c1, int n1, Topform c2, int n2);

Just binary_res_just_by_id(int c1, int n1, int c2, int n2);

Just factor_just(Topform c, int lit1, int lit2);

Just xxres_just(Topform c, int lit);

Just resolve_just(Ilist g, Just_type type);

Just demod_just(I3list steps);

Just para_just(Just_type rule,
	       Topform from, Ilist from_vec,
	       Topform into, Ilist into_vec);

Just instance_just(Topform parent, Plist pairs);

Just para_just_rev_copy(Just_type rule,
			Topform from, Ilist from_vec,
			Topform into, Ilist into_vec);

Just unit_del_just(Topform deleter, int literal_num);

Just flip_just(int n);

Just xx_just(int n);

Just merge_just(int n);

Just eval_just(int n);

Just append_just(Just j1, Just j2);

Just copy_justification(Just j);

char *jstring(Just j);

int jmap1(I3list map, int i);

char *jmap2(I3list map, int i, char *a);

void sb_append_id(String_buf sb, int id, I3list map);

void sb_write_ids(String_buf sb, Ilist p, I3list map);

void sb_write_just(String_buf sb, Just just, I3list map);

void sb_xml_write_just(String_buf sb, Just just, I3list map);

void p_just(Just j);

void zap_just(Just just);

Ilist get_parents(Just just, BOOL all);

Topform first_negative_parent(Topform c);

Plist get_clause_ancestors(Topform c);

int proof_length(Plist proof);

void map_just(Just just, I2list map);

int just_count(Just j);

void mark_parents_as_used(Topform c);

int clause_level(Topform c);

Just term_to_just(Term lst);

BOOL primary_just_type(Topform c, Just_type t);

BOOL has_input_just(Topform c);

BOOL has_copy_just(Topform c);

BOOL has_copy_flip_just(Topform c);

void sb_tagged_write_just(String_buf sb, Just just, I3list map);

#endif  /* conditional compilation of whole file */
