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

#ifndef TP_ATTRIB_H
#define TP_ATTRIB_H

#include "unify.h"

/* INTRODUCTION
This package is about lists of attributes.  Each attribute is
a pair (attribute-id, attribute-value).  Each attribute-id
is associated with a data type.
<P>
To use an attribute, you first have to call register_attribute(),
giving and ID, name, and type of the attribute.
The ID is used with the "set" and "get"  operations.
<P>
For example,
<PRE>
  register_attribute(label_attr, "label", STRING_ATTRIBUTE);
  ...
  Attribute a = set_string_attribute(NULL, label_attr, "clause_32");
  ...
  char *s = get_string_attribute(a, label_attr, 1);
</PRE>
*/

/* Public definitions */

typedef enum { INT_ATTRIBUTE,
	       STRING_ATTRIBUTE,
               TERM_ATTRIBUTE
             } Attribute_type;

typedef struct attribute * Attribute;

/* End of public definitions */

/* Public function prototypes from attrib.c */

void fprint_attrib_mem(FILE *fp, BOOL heading);

void p_attrib_mem();

int register_attribute(char *name, Attribute_type type);

void declare_term_attribute_inheritable(int id);

Attribute set_int_attribute(Attribute a, int id, int val);

int get_int_attribute(Attribute a, int id, int n);

BOOL exists_attribute(Attribute a, int id);

Attribute set_term_attribute(Attribute a, int id, Term val);

void replace_term_attribute(Attribute a, int id, Term val, int n);

void replace_int_attribute(Attribute a, int id, int val, int n);

Term get_term_attribute(Attribute a, int id, int n);

Term get_term_attributes(Attribute a, int id);

Attribute set_string_attribute(Attribute a, int id, char *val);

char *get_string_attribute(Attribute a, int id, int n);

BOOL string_attribute_member(Attribute a, int id, char *s);

void zap_attributes(Attribute a);

Attribute delete_attributes(Attribute a, int id);

Attribute cat_att(Attribute a, Attribute b);

Term build_attr_term(Attribute a);

Term attributes_to_term(Attribute a, char *operator);

int attribute_name_to_id(char *name);

Attribute term_to_attributes(Term t, char *operator);

Attribute inheritable_att_instances(Attribute a, Context subst);

Attribute copy_attributes(Attribute a);

void instantiate_inheritable_attributes(Attribute a, Context subst);

void renumber_vars_attributes(Attribute attrs, int vmap[], int max_vars);

void set_vars_attributes(Attribute attrs, char *vnames[], int max_vars);

Plist vars_in_attributes(Attribute attrs);

int label_att(void);

BOOL attributes_contain_variables(Attribute a);

Attribute copy_int_attribute(Attribute source, Attribute dest, int attr_id);

Attribute copy_string_attribute(Attribute source, Attribute dest, int attr_id);

Attribute copy_term_attribute(Attribute source, Attribute dest, int attr_id);

#endif  /* conditional compilation of whole file */
