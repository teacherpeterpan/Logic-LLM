#include "attrib.h"

/* Private definitions and types */

/* Attribute: a list of these can be attached to clauses. */

struct attribute {   /*  to form lists of attributes */
  int id;            /* attribute ID (index into Attribute_names array) */
  union {            /* attribute value */
    int i;
    char *s;
    Term t;
  } u;
  Attribute next;
};

/* Attribute_names: data about types of attributes. */

#define MAX_ATTRIBUTE_NAMES 50

static struct {  /* array, indexed by attribute id */
  char *name;           /* name of attribute, e.g., label, answer */
  Attribute_type type;  /* INT_ATTRIBUTE STRING_ATTRIBUTE TERM_ATTRIBUTE etc */
  BOOL inheritable;     /* child gets instance (for term attributes only) */
} Attribute_names[MAX_ATTRIBUTE_NAMES];

/*
 * memory management
 */

static unsigned Attribute_gets, Attribute_frees;

#define BYTES_ATTRIBUTE sizeof(struct attribute)
#define PTRS_ATTRIBUTE BYTES_ATTRIBUTE%BPP == 0 ? BYTES_ATTRIBUTE/BPP : BYTES_ATTRIBUTE/BPP + 1

/*************
 *
 *   Attribute get_attribute()
 *
 *************/

static
Attribute get_attribute(void)
{
  Attribute p = get_mem(PTRS_ATTRIBUTE);
  Attribute_gets++;
  return(p);
}  /* get_attribute */

/*************
 *
 *    free_attribute()
 *
 *************/

static
void free_attribute(Attribute p)
{
  free_mem(p, PTRS_ATTRIBUTE);
  Attribute_frees++;
}  /* free_attribute */

/*************
 *
 *   fprint_attrib_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the attrib package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_attrib_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_ATTRIBUTE;
  fprintf(fp, "attribute (%4d)    %11u%11u%11u%9.1f K\n",
          n, Attribute_gets, Attribute_frees,
          Attribute_gets - Attribute_frees,
          ((Attribute_gets - Attribute_frees) * n) / 1024.);

}  /* fprint_attrib_mem */

/*************
 *
 *   p_attrib_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the attrib package.
*/

/* PUBLIC */
void p_attrib_mem()
{
  fprint_attrib_mem(stdout, TRUE);
}  /* p_attrib_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   register_attribute()
 *
 *************/

/* DOCUMENTATION
This routine associates an attribute name and attribute
type with an integer ID to be used with the attribute
operations (set, get, etc).
*/

/* PUBLIC */
void register_attribute(int id, char *name, Attribute_type type)
{
  if (id < 0 || id >= MAX_ATTRIBUTE_NAMES)
    fatal_error("register_attribute: id out of range");
  else if (Attribute_names[id].name != NULL)
    fatal_error("register_attribute: id already in use");
  else {
    Attribute_names[id].name = new_str_copy(name);
    Attribute_names[id].type = type;
  }
}  /* register_attribute */

/*************
 *
 *   declare_term_attribute_inheritable()
 *
 *************/

/* DOCUMENTATION
This routine makes a term attribute (which has already been
registered) inheritable.  This usually means that when the
clause to which the attribute is attached begets a child,
the child gets a copy of the instantiated attribute.  This
was designed for answer literals and ordering constraints.
*/

/* PUBLIC */
void declare_term_attribute_inheritable(int id)
{
  if (Attribute_names[id].type != TERM_ATTRIBUTE)
    fatal_error("declare_term_attribute_inheritable, bad id");
  Attribute_names[id].inheritable = TRUE;
}  /* declare_term_attribute_inheritable */

/*************
 *
 *   inheritable()
 *
 *************/

static
BOOL inheritable(Attribute a)
{
  return Attribute_names[a->id].inheritable;
}  /* inheritable */

/*************
 *
 *   set_int_attribute()
 *
 *************/

/* DOCUMENTATION
This routine appends an <ID,value> pair to a list of attributes.
<P>
A fatal error occurs if the ID does not refer to an integer type
attribute (see register_attribute).
*/

/* PUBLIC */
Attribute set_int_attribute(Attribute a, int id, int val)
{
  if (Attribute_names[id].type != INT_ATTRIBUTE)
    fatal_error("set_int_attribute, bad id");

  if (a == NULL) {
    Attribute b = get_attribute();
    b->id = id;
    b->u.i = val;
    return b;
  }
  else {
    a->next = set_int_attribute(a->next, id, val);
    return a;
  }
}  /* set_int_attribute */

/*************
 *
 *   get_int_attribute()
 *
 *************/

/* DOCUMENTATION
This routine gets the n-th (counting from 1) attribute value
associated with an attribute ID.
If nothing matches, INT_MAX is returned.
<P>
A fatal error occurs if the ID does not refer to an integer type
attribute (see register_attribute).
*/

/* PUBLIC */
int get_int_attribute(Attribute a, int id, int n)
{
  if (Attribute_names[id].type != INT_ATTRIBUTE)
    fatal_error("get_int_attribute, bad id");

  if (a == NULL)
    return INT_MAX;
  else if (a->id == id && n == 1)
    return a->u.i;
  else if (a->id == id)
    return get_int_attribute(a->next, id, n-1);
  else
    return get_int_attribute(a->next, id, n);
}  /* get_int_attribute */

/*************
 *
 *   exists_attribute()
 *
 *************/

/* DOCUMENTATION
This routine checks if there are any attributes of the given type.
*/

/* PUBLIC */
BOOL exists_attribute(Attribute a, int id)
{
  if (a == NULL)
    return FALSE;
  else if (a->id == id)
    return TRUE;
  else
    return exists_attribute(a->next, id);
}  /* exists_attribute */

/*************
 *
 *   set_term_attribute()
 *
 *************/

/* DOCUMENTATION
This routine appends an <ID,value> pair to a list of attributes.
The term is not copied.
<P>
A fatal error occurs if the ID does not refer to a Term type
attribute (see register_attribute).
*/

/* PUBLIC */
Attribute set_term_attribute(Attribute a, int id, Term val)
{
  if (Attribute_names[id].type != TERM_ATTRIBUTE)
    fatal_error("set_term_attribute, bad ID");

  if (a == NULL) {
    Attribute b = get_attribute();
    b->id = id;
    b->u.t = val;
    return b;
  }
  else {
    a->next = set_term_attribute(a->next, id, val);
    return a;
  }
}  /* set_term_attribute */

/*************
 *
 *   replace_term_attribute()
 *
 *************/

/* DOCUMENTATION
This routine replaces that n-th term attribute for given ID.
The term that is already there is zapped, and the new
term is NOT copied.
<P>
A fatal error occurs if the ID does not refer to a Term type
attribute (see register_attribute), or if there are not
n attributes identified by ID.
*/

/* PUBLIC */
void replace_term_attribute(Attribute a, int id, Term val, int n)
{
  if (Attribute_names[id].type != TERM_ATTRIBUTE)
    fatal_error("replace_term_attribute, bad ID");

  if (a == NULL)
    fatal_error("replace_term_attribute, attribute not found");
  else if (a->id == id && n == 1) {
    zap_term(a->u.t);
    a->u.t = val;
  }
  else if (a->id == id)
    replace_term_attribute(a->next, id, val, n-1);
  else
    replace_term_attribute(a->next, id, val, n);
}  /* replace_term_attribute */

/*************
 *
 *   replace_int_attribute()
 *
 *************/

/* DOCUMENTATION
This routine replaces that n-th int attribute for given attribute ID.
<P>
A fatal error occurs if the ID does not refer to an int type
attribute (see register_attribute), or if there are not
n attributes identified by ID.
*/

/* PUBLIC */
void replace_int_attribute(Attribute a, int id, int val, int n)
{
  if (Attribute_names[id].type != INT_ATTRIBUTE)
    fatal_error("replace_int_attribute, bad ID");

  if (a == NULL)
    fatal_error("replace_int_attribute, attribute not found");
  else if (a->id == id && n == 1) {
    a->u.i = val;
  }
  else if (a->id == id)
    replace_int_attribute(a->next, id, val, n-1);
  else
    replace_int_attribute(a->next, id, val, n);
}  /* replace_int_attribute */

/*************
 *
 *   get_term_attribute()
 *
 *************/

/* DOCUMENTATION
This routine gets the n-th (counting from 1) attribute value
associated with an attribute ID.
If nothing matches, NULL is returned.
<P>
A fatal error occurs if the ID does not refer to a Term type
attribute (see register_attribute).
*/

/* PUBLIC */
Term get_term_attribute(Attribute a, int id, int n)
{
  if (Attribute_names[id].type != TERM_ATTRIBUTE)
    fatal_error("get_term_attribute, bad ID");

  if (a == NULL)
    return NULL;
  else if (a->id == id && n == 1)
    return a->u.t;
  else if (a->id == id)
    return get_term_attribute(a->next, id, n-1);
  else
    return get_term_attribute(a->next, id, n);
}  /* get_term_attribute */

/*************
 *
 *   set_string_attribute()
 *
 *************/

/* DOCUMENTATION
This routine appends an <ID,value> pair to a list of attributes.
The string is not copied.
<P>
A fatal error occurs if the ID does not refer to a string type
attribute (see register_attribute).
*/

/* PUBLIC */
Attribute set_string_attribute(Attribute a, int id, char *val)
{
  if (Attribute_names[id].type != STRING_ATTRIBUTE)
    fatal_error("set_string_attribute, bad ID");

  if (a == NULL) {
    Attribute b = get_attribute();
    b->id = id;
    b->u.s = val;
    return b;
  }
  else {
    a->next = set_string_attribute(a->next, id, val);
    return a;
  }
}  /* set_string_attribute */

/*************
 *
 *   get_string_attribute()
 *
 *************/

/* DOCUMENTATION
This routine gets the n-th (counting from 1) attribute value
associated with an attribute ID.
If nothing matches, NULL is returned.
<P>
A fatal error occurs if the ID does not refer to a string type
attribute (see register_attribute).
*/

/* PUBLIC */
char *get_string_attribute(Attribute a, int id, int n)
{
  if (Attribute_names[id].type != STRING_ATTRIBUTE)
    fatal_error("get_string_attribute, bad ID");

  if (a == NULL)
    return NULL;
  else if (a->id == id && n == 1)
    return a->u.s;
  else if (a->id == id)
    return get_string_attribute(a->next, id, n-1);
  else
    return get_string_attribute(a->next, id, n);
}  /* get_string_attribute */

/*************
 *
 *   zap_attributes()
 *
 *************/

/* DOCUMENTATION
This routine frees a list of attributes and any associated memory.
In particular, the terms in term attributes are zapped.
*/

/* PUBLIC */
void zap_attributes(Attribute a)
{
  if (a != NULL) {
    zap_attributes(a->next);
    /* If there is any memory associted with the attribure, free it here. */
    if (Attribute_names[a->id].type == TERM_ATTRIBUTE)
      zap_term(a->u.t);
    free_attribute(a);
  }
}  /* zap_attributes */

/*************
 *
 *   delete_attributes()
 *
 *************/

/* DOCUMENTATION
This routine frees all attributes of the given type.
*/

/* PUBLIC */
Attribute delete_attributes(Attribute a, int id)
{
  if (a == NULL)
    return NULL;
  else {
    a->next = delete_attributes(a->next, id);
    if (a->id == id) {
      Attribute b = a->next;
      /* If there is any memory associted with the attribure, free it here. */
      if (Attribute_names[a->id].type == TERM_ATTRIBUTE)
	zap_term(a->u.t);
      free_attribute(a);
      return b;
    }
    else
      return a;
  }
}  /* delete_attributes */

/*************
 *
 *   cat_att()
 *
 *************/

/* DOCUMENTATION
Concatenate two lists of attributes.  Return the result.
*/

/* PUBLIC */
Attribute cat_att(Attribute a, Attribute b)
{
  if (a == NULL)
    return b;
  else {
    a->next = cat_att(a->next, b);
    return a;
  }
}  /* cat_att */

/*************
 *
 *   build_attr_term()
 *
 *************/

/* DOCUMENTATION
Given an attribute, build (and return) a term representation of it.
The name of the attribute will be the (unary) function symbol,
and the value will be the argument.
<P>
This is typically used for printing attributes.
*/

/* PUBLIC */
Term build_attr_term(Attribute a)
{
  char *name = Attribute_names[a->id].name;
  Attribute_type type = Attribute_names[a->id].type;
  Term t = get_rigid_term(name, 1);  /* e.g., label(cl_32), answer(assoc) */

  switch (type) {
  case INT_ATTRIBUTE:
    {
      char s[50];
      if (a->u.i < 0) {
	ARG(t,0) = get_rigid_term("-", 1);
	itoa(-(a->u.i), s);
	ARG(ARG(t,0),0) = get_rigid_term(s, 0);
      }
      else {
	itoa(a->u.i, s);
	ARG(t,0) = get_rigid_term(s, 0);
      }
      break;
    }
  case STRING_ATTRIBUTE:
    ARG(t,0) = get_rigid_term(a->u.s, 0);
    break;
  case TERM_ATTRIBUTE:
    ARG(t,0) = copy_term(a->u.t);
    break;
  default:
    fatal_error("build_attr_term: bad attribute type");
  }
  return t;
}  /* build_attr_term */

/*************
 *
 *   attributes_to_term()
 *
 *************/

/* DOCUMENTATION
This routine takes a list of attributes and
constructs a term representation.  It is a
right-associated binary tree with Term forms
of the attributes at the leaves.
*/

/* PUBLIC */
Term attributes_to_term(Attribute a, char *operator)
{
  if (a == NULL)
    return NULL;  /* should happen only on top call */
  else if (a->next == NULL)
    return build_attr_term(a);
  else {
    return build_binary_term(str_to_sn(operator, 2),
			     build_attr_term(a),
			     attributes_to_term(a->next, operator));
  }
}  /* attributes_to_term */

/*************
 *
 *   cat_attributes()
 *
 *************/

static
Attribute cat_attributes(Attribute a0, Attribute a1)
{
  if (a0 == NULL)
    return a1;
  else {
    a0->next = cat_attributes(a0->next, a1);
    return a0;
  }
}  /* cat_attributes */

/*************
 *
 *   attribute_name_to_id()
 *
 *************/

/* DOCUMENTATION
Given an attribute name, return the attribute ID which is used
for the "get" and "set" operations.  Return -1 if the name
has not been registered with "register_attribute".
*/

/* PUBLIC */
int attribute_name_to_id(char *name)
{
  int i;
  for (i = 0; i < MAX_ATTRIBUTE_NAMES; i++) {
    if (Attribute_names[i].name != NULL &&
	str_ident(Attribute_names[i].name, name))
      return i;
  }
  return -1;
}  /* attribute_name_to_id */

/*************
 *
 *   term_to_attributes()
 *
 *************/

/* DOCUMENTATION
This routine takes a term representing a list of
attributes and builds list of attributes.
The input term form is a binary term, constructed
with the given operator, with the attributes at
the leaves.  For example,
<PRE>
    label("hi there!") # answer(XGK(x,y,z)) # hint_wt(32)
</PRE>
If anuthing goes wrong, a fatal error occurs.
*/

/* PUBLIC */
Attribute term_to_attributes(Term t, char *operator)
{
  if (is_term(t, operator, 2)) {
    Attribute a0 = term_to_attributes(ARG(t,0), operator);
    Attribute a1 = term_to_attributes(ARG(t,1), operator);
    return cat_attributes(a0, a1);
  }
  else {
    int id;
    Attribute a;
    if (ARITY(t) != 1)
      fatal_error("term_to_attributes, arity not 1");
    id = attribute_name_to_id(sn_to_str(SYMNUM(t)));
    if (id == -1)
      fatal_error("term_to_attributes, attribute name not found");
    a = get_attribute();
    a->id = id;
    switch (Attribute_names[id].type) {
    case INT_ATTRIBUTE:
      if (!CONSTANT(ARG(t,0)))
	fatal_error("term_to_attributes, bad intger");
      else {
	int i;
	if (!term_to_int(ARG(t,0), &i))
	  fatal_error("term_to_attributes, bad integer");
	a->u.i = i;
      }
      break;
    case STRING_ATTRIBUTE:
      if (!CONSTANT(ARG(t,0)))
	fatal_error("term_to_attributes, bad string");
      else
	a->u.s = sn_to_str(SYMNUM(ARG(t,0)));
      break;
    case TERM_ATTRIBUTE:
      a->u.t = copy_term(ARG(t,0));
      break;
    }
    return a;
  }
}  /* term_to_attributes */

/*************
 *
 *   inheritable_att_instances()
 *
 *************/

/* DOCUMENTATION
Given a list of attributes, this routine copies, instantiates,
and returns the the inheritable attributes.  The Context can be NULL.
*/

/* PUBLIC */
Attribute inheritable_att_instances(Attribute a, Context subst)
{
  if (a == NULL)
    return NULL;
  else if (!inheritable(a))
    return inheritable_att_instances(a->next, subst);
  else {
    Attribute new = get_attribute();
    new->id = a->id;
    new->u.t = subst ? apply(a->u.t, subst) : copy_term(a->u.t);
    new->next = inheritable_att_instances(a->next, subst);
    return new;
  }
}  /* inheritable_att_instances */

/*************
 *
 *   copy_attributes()
 *
 *************/

/* DOCUMENTATION
This routine copies a list of attributes.
*/

/* PUBLIC */
Attribute copy_attributes(Attribute a)
{
  if (a == NULL)
    return NULL;
  else {
    Attribute new = get_attribute();
    new->id = a->id;
    switch (Attribute_names[a->id].type) {
    case INT_ATTRIBUTE: new->u.i = a->u.i; break;
    case STRING_ATTRIBUTE: new->u.s = a->u.s; break;
    case TERM_ATTRIBUTE: new->u.t = copy_term(a->u.t); break;
    default: fatal_error("copy_attribute: unknown attribute");
    }
    new->next = copy_attributes(a->next);
    return new;
  }
}  /* copy_attributes */

/*************
 *
 *   renumber_vars_attributes()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables in the inheritable attribute terms.
*/

/* PUBLIC */
void renumber_vars_attributes(Attribute attrs, int vmap[], int max_vars)
{
  Attribute a;
  for (a = attrs; a; a = a->next) {
    if (inheritable(a)) {
      a->u.t = renum_vars_recurse(a->u.t, vmap, max_vars);
    }
  }
}  /* renumber_vars_attributes */

/*************
 *
 *   set_vars_attributes()
 *
 *************/

/* DOCUMENTATION
This routine sets the variables in the inheritable attribute terms.
*/

/* PUBLIC */
void set_vars_attributes(Attribute attrs, char *vnames[], int max_vars)
{
  Attribute a;
  for (a = attrs; a; a = a->next) {
    if (inheritable(a)) {
      a->u.t = set_vars_recurse(a->u.t, vnames, max_vars);
    }
  }
}  /* set_vars_attributes */
