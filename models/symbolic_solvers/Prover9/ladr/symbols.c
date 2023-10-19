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

#include "symbols.h"

/* Private definitions and types*/

struct symbol {
  int         symnum;           /* unique identifier */
  char        *name;            /* the print symbol */
  int         arity;            /* 0 for constants */
  Symbol_type type;             /* function, relation, unspecified */
  Parsetype   parse_type;       /* infix, prefix, etc. */
  int         parse_prec;       /* precedence for parsing/printing */
  Unif_theory unif_theory;      /* e.g., associative-commutative */
  int         occurrences;      /* how often it occurs somewhere */
  int         lex_val;          /* precedence for term orderings */
  int         kb_weight;        /* for Knuth-Bendix ordering */
  Lrpo_status lrpo_status;      /* for LRPO, LPO, RPO */
  BOOL        skolem;
  BOOL        unfold;
  BOOL        auxiliary;        /* not part of theory, e.g., in hints only */

  /* IF YOU ADD MORE FIELDS, MAKE SURE TO INITIALIZE THEM ! */
};

#define SYM_TAB_SIZE  50000

static Plist By_id[SYM_TAB_SIZE];   /* for access by symnum (ID) */
static Plist By_sym[SYM_TAB_SIZE];  /* for access by string/arity */

static unsigned Symbol_count;

/* Logic symbols when in Term form */

static char *True_sym   =  "$T";
static char *False_sym  =  "$F";
static char *And_sym    =  "&";
static char *Or_sym     =  "|";
static char *Not_sym    =  "-";
static char *Iff_sym    =  "<->";
static char *Imp_sym    =  "->";
static char *Impby_sym  =  "<-";
static char *All_sym    =  "all";
static char *Exists_sym =  "exists";
static char *Quant_sym  =  "$quantified";

/* Other symbols when in Term form */

static char *Attrib_sym =  "#";    /* operator for attaching attributes */
static char *Eq_sym     =  "=";    /* for equality inference rules */
static char *Neq_sym    =  "!=";   /* abbreviation for negation of Eq_sym */

/*************
 *
 *   true_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *true_sym(void)
{
  return True_sym;
}  /* true_sym */

/*************
 *
 *   false_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *false_sym()
{
  return False_sym;
}  /* false_sym */

/*************
 *
 *   and_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *and_sym()
{
  return And_sym;
}  /* and_sym */

/*************
 *
 *   or_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *or_sym()
{
  return Or_sym;
}  /* or_sym */

/*************
 *
 *   not_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *not_sym()
{
  return Not_sym;
}  /* not_sym */

/*************
 *
 *   iff_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *iff_sym()
{
  return Iff_sym;
}  /* iff_sym */

/*************
 *
 *   imp_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *imp_sym()
{
  return Imp_sym;
}  /* imp_sym */

/*************
 *
 *   impby_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *impby_sym()
{
  return Impby_sym;
}  /* impby_sym */

/*************
 *
 *   all_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *all_sym()
{
  return All_sym;
}  /* all_sym */

/*************
 *
 *   exists_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *exists_sym()
{
  return Exists_sym;
}  /* exists_sym */

/*************
 *
 *   quantified_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *quant_sym()
{
  return Quant_sym;
}  /* quantified_sym */

/*************
 *
 *   attrib_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *attrib_sym()
{
  return Attrib_sym;
}  /* attrib_sym */

/*************
 *
 *   eq_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *eq_sym()
{
  return Eq_sym;
}  /* eq_sym */

/*************
 *
 *   neq_sym()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *neq_sym()
{
  return Neq_sym;
}  /* neq_sym */

/*************
 *
 *   set_operation_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_operation_symbol(char *operation, char *symbol)
{
  if (str_ident(operation, "true"))
    True_sym = symbol;
  else if (str_ident(operation, "false"))
    False_sym = symbol;
  else if (str_ident(operation, "conjunction"))
    And_sym = symbol;
  else if (str_ident(operation, "disjunction"))
    Or_sym = symbol;
  else if (str_ident(operation, "negation"))
    Not_sym = symbol;
  else if (str_ident(operation, "implication"))
    Imp_sym = symbol;
  else if (str_ident(operation, "backward_implication"))
    Impby_sym = symbol;
  else if (str_ident(operation, "equivalence"))
    Iff_sym = symbol;
  else if (str_ident(operation, "universal_quantification"))
    All_sym = symbol;
  else if (str_ident(operation, "existential_quantification"))
    Exists_sym = symbol;
  else if (str_ident(operation, "quantification"))
    Quant_sym = symbol;
  else if (str_ident(operation, "attribute"))
    Attrib_sym = symbol;
  else if (str_ident(operation, "equality"))
    Eq_sym = symbol;
  else if (str_ident(operation, "negated_equality"))
    Neq_sym = symbol;
  else {
    printf("The unknown operation is %s\n", operation);
    fatal_error("set_operation_symbol, unknown operation");
  }
}  /* set_operation_symbol */

/*************
 *
 *   get_operation_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *get_operation_symbol(char *operation)
{
  if (str_ident(operation, "true"))
    return True_sym;
  else if (str_ident(operation, "false"))
    return False_sym;
  else if (str_ident(operation, "conjunction"))
    return And_sym;
  else if (str_ident(operation, "disjunction"))
    return Or_sym;
  else if (str_ident(operation, "negation"))
    return Not_sym;
  else if (str_ident(operation, "implication"))
    return Imp_sym;
  else if (str_ident(operation, "backward_implication"))
    return Impby_sym;
  else if (str_ident(operation, "equivalence"))
    return Iff_sym;
  else if (str_ident(operation, "universal_quantification"))
    return All_sym;
  else if (str_ident(operation, "existential_quantification"))
    return Exists_sym;
  else if (str_ident(operation, "quantification"))
    return Quant_sym;
  else if (str_ident(operation, "attribute"))
    return Attrib_sym;
  else if (str_ident(operation, "equality"))
    return Eq_sym;
  else if (str_ident(operation, "negated_equality"))
    return Neq_sym;
  else {
    printf("The unknown operation is %s\n", operation);
    fatal_error("get_operation_symbol, unknown operation");
    return "";
  }
}  /* get_operation_symbol */

/*************
 *
 *   symbol_in_use()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL symbol_in_use(char *str)
{
  if (str_ident(str, True_sym))
    return TRUE;
  else if (str_ident(str, False_sym))
    return TRUE;
  else if (str_ident(str, And_sym))
    return TRUE;
  else if (str_ident(str, Or_sym))
    return TRUE;
  else if (str_ident(str, Not_sym))
    return TRUE;
  else if (str_ident(str, Imp_sym))
    return TRUE;
  else if (str_ident(str, Impby_sym))
    return TRUE;
  else if (str_ident(str, Iff_sym))
    return TRUE;
  else if (str_ident(str, All_sym))
    return TRUE;
  else if (str_ident(str, Exists_sym))
    return TRUE;
  else if (str_ident(str, Quant_sym))
    return TRUE;
  else if (str_ident(str, Attrib_sym))
    return TRUE;
  else if (str_ident(str, Eq_sym))
    return TRUE;
  else if (str_ident(str, Neq_sym))
    return TRUE;
  else
    return FALSE;
}  /* symbol_in_use */

/****************************************************************************/
/* This section is about the symbol table.                                  */
/****************************************************************************/

/*************
 *
 *    Symbol get_symbol()
 *
 *************/

static
Symbol get_symbol(void)
{
  Symbol p = malloc(sizeof(struct symbol));
  
  p->name = "";
  p->symnum = 0;
  p->parse_type = NOTHING_SPECIAL;
  p->parse_prec = 0;
  p->arity = -1;
  p->unif_theory = EMPTY_THEORY;
  p->occurrences = -1;
  p->lex_val = INT_MAX;
  p->lrpo_status = LRPO_LR_STATUS;
  p->kb_weight = 1;
  p->type = UNSPECIFIED_SYMBOL;
  p->skolem = FALSE;
  p->unfold = FALSE;
  p->auxiliary = FALSE;
  return(p);
}  /* get_symbol */

/*************
 *
 *    int new_symnum()
 *
 *    Return the next available symbol number.  It is always POSITIVE.
 *
 *************/

static
int new_symnum(void)
{
  Symbol_count++;
  return(Symbol_count);
}  /* new_symnum */

/*************
 *
 *   hash_sym()
 *
 *************/

static
unsigned hash_sym(char *s, int arity)
{
  unsigned x = arity;
  while (*s != '\0') {
    unsigned c = *s;
    x = (x << 4) | c;
    s++;
  }
  return abs(x) % SYM_TAB_SIZE;
}  /* hash_sym */

/*************
 *
 *   hash_id()
 *
 *************/

static
unsigned hash_id(int id)
{
  return abs(id) % SYM_TAB_SIZE;
}  /* hash_id */

/*************
 *
 *   lookup_by_id()
 *
 *************/

static
Symbol lookup_by_id(int symnum)
{
  Plist p;
  for (p = By_id[hash_id(symnum)]; p; p = p->next) {
    Symbol s = p->v;
    if (s->symnum == symnum)
      return s;
  }
  return NULL;
}  /* lookup_by_id */

/*************
 *
 *   lookup_by_sym()
 *
 *************/

static
Symbol lookup_by_sym(char *name, int arity)
{
  Plist p;
  for (p = By_sym[hash_sym(name,arity)]; p; p = p->next) {
    Symbol s = p->v;
    if (s->arity == arity && str_ident(s->name, name))
      return s;
  }
  return NULL;
}  /* lookup_by_sym */

/*************
 *
 *    int str_to_sn()
 *
 *************/

/* DOCUMENTATION
This routine takes a string and an arity, and returns an integer
identifier for the pair.  If the pair is not already in the symbol
table, a new entry is inserted into the table.  A pair, say ("f",2),
is sometimes written as f/2, which is a different symbol from f/3.
There is no limit on the length of the string (which is copied).
*/

/* PUBLIC */
int str_to_sn(char *str, int arity)
{
  Symbol s = lookup_by_sym(str, arity);

  if (s == NULL) {
    s = get_symbol();
    s->name = new_str_copy(str);
    s->arity = arity;
    s->symnum = new_symnum();
    
    /* printf("New Symbol: %s/%d, sn=%d\n", str, arity, s->symnum); */

    /* insert into both hash tables */

    {
      int hashval_id = hash_id(s->symnum);
      int hashval_sym = hash_sym(str, arity);
      By_sym[hashval_sym] = plist_prepend(By_sym[hashval_sym], s);
      By_id[hashval_id]   = plist_prepend(By_id[hashval_id], s);
    }
  }
  return(s->symnum);
}  /* str_to_sn */

/*************
 *
 *    fprint_syms(file_ptr) -- Display the symbol list.
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) the symbol table, including many
of the attributes of each symbol.
*/

/* PUBLIC */
void fprint_syms(FILE *fp)
{
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      fprintf(fp, "%d  %s/%d %s, lex_val=%d, kb_weight=%d\n",
	      s->symnum, s->name, s->arity,
	      s->type == FUNCTION_SYMBOL ? "function" :
                         s->type == PREDICATE_SYMBOL ? "relation" : "",
	      s->lex_val,
	      s->kb_weight);
    }
  }
}  /* fprint_syms */

/*************
 *
 *    p_syms()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) the symbol table, including all
of the attributes of each symbol.
*/

/* PUBLIC */
void p_syms(void)
{
  fprint_syms(stdout);
}  /* p_syms */

/*************
 *
 *    fprint_sym()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) the string associated with a symbol ID.
A newline is NOT printed.
*/

/* PUBLIC */
void fprint_sym(FILE *fp, int symnum)
{
  fprintf(fp, "%s", sn_to_str(symnum));
}  /* fprint_sym */

/*************
 *
 *    sprint_sym()
 *
 *************/

/* DOCUMENTATION
This routine appends, to String_buf sb, the string associated with
a symbol ID.  A newline is NOT printed.
*/

/* PUBLIC */
void sprint_sym(String_buf sb, int symnum)
{
  sb_append(sb, sn_to_str(symnum));
}  /* fprint_sym */

/*************
 *
 *    p_sym()
 *
 *************/

/* DOCUMENTATION
This routine prints (stdout) the string associated with a symbol ID.
A newline is NOT printed.
*/

/* PUBLIC */
void p_sym(int symnum)
{
  fprint_sym(stdout, symnum);
}  /* p_sym */

/*************
 *
 *   str_exists()
 *
 *************/

/* DOCUMENTATION
This function checks if the given string occurs in the
symbol table (with any arity).  This should be used judiciously,
because the whole table is scanned.
*/

/* PUBLIC */
BOOL str_exists(char *str)
{
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if (str_ident(str, s->name))
	return TRUE;
    }
  }
  return FALSE;
}  /* str_exists */

/*************
 *
 *   greatest_symnum()
 *
 *************/

/* DOCUMENTATION
This function returns the greatest symnum (symbol ID) currently in use.
This can be used if you need to dynamnically allocate an array
of objects to be indexed by symnum.
*/

/* PUBLIC */
int greatest_symnum(void)
{
  return Symbol_count;
}  /* greatest_symnum */

/****************************************************************************/
/* This section is about miscellaneous properties of symbols.               */
/****************************************************************************/

/*************
 *
 *    char *sn_to_str(symnum)  --  given a symbol number, return the name
 *
 *************/

/* DOCUMENTATION
This routine returns the string assocated with a symbol ID.
*/

/* PUBLIC */
char *sn_to_str(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  
  if (p == NULL)
    return("");
  else
    return(p->name);
}  /* sn_to_str */

/*************
 *
 *    int is_symbol(symbol, str, arity)
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if a given symbol ID matches a given
(string,arity) pair.
*/

/* PUBLIC */
BOOL is_symbol(int symnum, char *str, int arity)
{
  Symbol n = lookup_by_id(symnum);
  if (n == NULL)
    return FALSE;
  else
    return (n->arity == arity && str_ident(n->name, str));
}  /* is_symbol */

/*************
 *
 *    int sn_to_arity(symnum)  --  given a symbol number, return the arity
 *
 *************/

/* DOCUMENTATION
This routine returns the arity associated with a symbol ID.
*/

/* PUBLIC */
int sn_to_arity(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    return(-1);
  else
    return(p->arity);
}  /* sn_to_arity */

/*************
 *
 *    int sn_to_occurrences(symnum)
 *
 *************/

/* DOCUMENTATION
This routine returns the occurrences associated with a symbol ID.
*/

/* PUBLIC */
int sn_to_occurrences(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    return(-1);
  else
    return(p->occurrences);
}  /* sn_to_occurrences */

/*************
 *
 *    set_unfold_symbol()
 *
 *************/

/* DOCUMENTATION
This routine declares that a symbol is a Skolem function (or constant).
*/

/* PUBLIC */
void set_unfold_symbol(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  p->unfold = TRUE;
}  /* set_unfold_symbol */

/*************
 *
 *   is_unfold_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL is_unfold_symbol(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  return p->unfold;
}  /* is_unfold_symbol */

/*************
 *
 *   declare_aux_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_aux_symbols(Ilist syms)
{
  Ilist p;
  for (p = syms; p; p = p->next) {
    Symbol s = lookup_by_id(p->i);
    s->auxiliary = TRUE;
  }
}  /* declare_aux_symbols */

/****************************************************************************/
/* This section is about parse properties of symbols.                       */
/****************************************************************************/

/*************
 *
 *   parse_type_to_str()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *parse_type_to_str(Parsetype type)
{
  switch (type) {
  case INFIX_LEFT: return "infix_left";
  case INFIX_RIGHT: return "infix_right";
  case INFIX: return "infix";
  case PREFIX: return "prefix";
  case PREFIX_PAREN: return "prefix_paren";
  case POSTFIX: return "postfix";
  case POSTFIX_PAREN: return "postfix_paren";
  case NOTHING_SPECIAL: return "ordinary";
  }
  return "???";
}  /* parse_type_to_str */

/*************
 *
 *   clear_parse_type_for_all_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void clear_parse_type_for_all_symbols(void)
{
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      s->parse_type = NOTHING_SPECIAL;
      s->parse_prec = 0;
    }
  }
}  /* clear_parse_type_for_all_symbols */

/*************
 *
 *   clear_parse_type()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void clear_parse_type(char *str)
{
  Symbol p;
  p = lookup_by_sym(str, 1);
  if (p != NULL) {
    p->parse_type = NOTHING_SPECIAL;
    p->parse_prec = 0;
  }
  p = lookup_by_sym(str, 2);
  if (p != NULL) {
    p->parse_type = NOTHING_SPECIAL;
    p->parse_prec = 0;
  }
}  /* clear_parse_type */

/*************
 *
 *    check_diff_type_same_prec()
 *
 *************/

static
void check_diff_type_same_prec(char *str, int prec, Parsetype type)
{
  if (type != NOTHING_SPECIAL) {
    int i;
    for (i = 0; i < SYM_TAB_SIZE; i++) {
      Plist p;
      for (p = By_id[i]; p; p = p->next) {
	Symbol s = p->v;
	Parsetype type2 = s->parse_type;
	int prec2 = s->parse_prec;
	char *name = s->name;
	if (type2 != type && prec == prec2 && !str_ident(name, str)) {
	  printf("\nConflicting declarations (the first may be built in):\n");
	  printf("  op(%d, %s, \"%s\").\n", prec2,parse_type_to_str(type2),name);
	  printf("  op(%d, %s, \"%s\").\n", prec,parse_type_to_str(type),str);
	  fatal_error("cannot declare different parse types with same "
		      "precedence (see output file)");
	}
      }
    }
  }
}  /* check_diff_type_same_prec */

/*************
 *
 *   set_parse_type()
 *
 *************/

/* DOCUMENTATION
This routine sets the parse/print properties of a binary or unary symbol.
The types for binary infix symbols are
INFIX_LEFT,
INFIX_RIGHT,
INFIX.
The types for prefix unary symbols are
PREFIX,
PREFIX_PAREN.
The types for postfix unary symbols are
POSTFIX,
POSTFIX_PAREN.
<P>
If the precedence is out of range [MIN_PRECEDENCE ... MAX_PRECEDENCE],
a fatal error occurs.
*/

/* PUBLIC */
void set_parse_type(char *str, int precedence, Parsetype type)
{
  if (precedence < MIN_PRECEDENCE || precedence > MAX_PRECEDENCE)
    fatal_error("set_parse_type: precedence out of range");
  else {
    Symbol p = NULL;
    clear_parse_type(str);  /* in case it has parse type of diff. arity */
    check_diff_type_same_prec(str, precedence, type);
    switch (type) {
    case INFIX_LEFT:
    case INFIX_RIGHT:
    case INFIX:
      p = lookup_by_id(str_to_sn(str, 2));
      p->parse_type = type;
      p->parse_prec = precedence;
      break;
    case PREFIX:
    case PREFIX_PAREN:
    case POSTFIX:
    case POSTFIX_PAREN:
      p = lookup_by_id(str_to_sn(str, 1));
      p->parse_type = type;
      p->parse_prec = precedence;
      break;
    case NOTHING_SPECIAL:
      /* already cleared above */
      break;
    }
  }
}  /* set_parse_type */

/*************
 *
 *   binary_parse_type()
 *
 *************/

/* DOCUMENTATION
This routine gets the parse/print properties for a binary symbol.
If *str is a binary symbol, TRUE is returned and the properties are filled in.
If *str is a not a binary symbol, FALSE is returned.
*/

/* PUBLIC */
BOOL binary_parse_type(char *str, int *precedence, Parsetype *type)
{
  Symbol p = lookup_by_sym(str, 2);
  if (p == NULL || p->parse_type == NOTHING_SPECIAL)
    return FALSE;
  else {
    *precedence = p->parse_prec;
    *type = p->parse_type;
    return TRUE;
  }
}  /* binary_parse_type */

/*************
 *
 *   unary_parse_type()
 *
 *************/

/* DOCUMENTATION
This routine gets the parse/print properties for a unary symbol.
If *str is a unary symbol, TRUE is returned and the properties are filled in.
If *str is a not a unary symbol, FALSE is returned.
*/

/* PUBLIC */
BOOL unary_parse_type(char *str, int *precedence, Parsetype *type)
{
  Symbol p = lookup_by_sym(str, 1);
  if (p == NULL || p->parse_type == NOTHING_SPECIAL)
    return FALSE;
  else {
    *precedence = p->parse_prec;
    *type = p->parse_type;
    return TRUE;
  }
}  /* unary_parse_type */

/*************
 *
 *   special_parse_type()
 *
 *************/

/* DOCUMENTATION
Is the string a unary or binary "special_parse_type"
(e.g., PREFIX or INFIX)?  If so, return the arity; otherwise
return -1.
*/

/* PUBLIC */
int special_parse_type(char *str)
{
  int prec;
  Parsetype type;
  if (binary_parse_type(str, &prec, &type))
    return 2;
  else if (unary_parse_type(str, &prec, &type))
    return 1;
  else
    return -1;
}  /* special_parse_type */

/****************************************************************************/
/* This section is about associativity-commutativity properties.            */
/****************************************************************************/

static int Assoc_comm_symbols = 0;  /* number declared */
static int Comm_symbols = 0;        /* number decoared */

/*************
 *
 *    set_assoc_comm()
 *
 *************/

/* DOCUMENTATION
This routine declares a string to be a (binary) symbol with the
logical property "associative-commutative".  This property is used for
AC unification/matching/identity.
(If you wish to print AC expressions without parentheses, see
set_parse_type().)

*/

/* PUBLIC */
void set_assoc_comm(char *str, BOOL set)
{
  int sn = str_to_sn(str, 2);
  Symbol p = lookup_by_id(sn);
  if (set) {
    p->unif_theory = ASSOC_COMMUTE;
    Assoc_comm_symbols++;
  }
  else {
    p->unif_theory = EMPTY_THEORY;
    Assoc_comm_symbols--;
  }
}  /* set_assoc_comm */

/*************
 *
 *    set_commutative()
 *
 *************/

/* DOCUMENTATION
This routine declares a string to be a (binary) symbol with the
logical property "commutative".  This property is used for commutative
unification/matching/identity.
*/

/* PUBLIC */
void set_commutative(char *str, BOOL set)
{
  int sn = str_to_sn(str, 2);
  Symbol p = lookup_by_id(sn);
  if (set) {
    p->unif_theory = COMMUTE;
    Comm_symbols++;
  }
  else {
    p->unif_theory = EMPTY_THEORY;
    Comm_symbols--;
  }
}  /* set_commutative */

/*************
 *
 *    assoc_comm_symbols()
 *
 *************/

/* DOCUMENTATION
This function tells you if any symbol has been declared to be
associative-commutative;
*/

/* PUBLIC */
BOOL assoc_comm_symbols(void)
{
  return Assoc_comm_symbols != 0;
}  /* assoc_comm_symbols */

/*************
 *
 *    comm_symbols()
 *
 *************/

/* DOCUMENTATION
This function tells you if any symbol has been declared to be
commutative.
*/

/* PUBLIC */
BOOL comm_symbols(void)
{
  return Comm_symbols != 0;
}  /* comm_symbols */

/*************
 *
 *    is_assoc_comm
 *
 *    NOTE: may wish to avoid hash if no AC symbols exist.
 *
 *************/

/* DOCUMENTATION
This function checks if a symbol ID has the
associative-commutative property.  Note that set_assoc_comm() takes a
string, but this routine takes a symbol ID.  Recall that str_to_sn()
and sn_to_str() translate between the two forms.
*/

/* PUBLIC */
BOOL is_assoc_comm(int sn)
{
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? 0 : p->unif_theory == ASSOC_COMMUTE);
}  /* is_assoc_comm */

/*************
 *
 *    is_commutative
 *
 *    NOTE: may wish to avoid hash if no C symbols exist.
 *
 *************/

/* DOCUMENTATION
This function checks if a symbol ID has the commutative property.
Note that set_commutative() takes a string, but this routine
takes a symbol ID.  Recall that str_to_sn() and sn_to_str()
translate between the two forms.
*/

/* PUBLIC */
BOOL is_commutative(int sn)
{
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? 0 : p->unif_theory == COMMUTE);
}  /* is_commutative */

/****************************************************************************/
/* This section is about some built-in logic symbols.                       */
/****************************************************************************/

static int Eq_symnum = 0;
static int Or_symnum = 0;
static int Not_symnum = 0;

/*************
 *
 *    int is_eq_symbol(symbol)
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if a given symbol ID is for eq_sym()/2.
One could use is_symbol(symnum, eq_sym(), 2) instead, but this
should be a bit faster.
*/

/* PUBLIC */
BOOL is_eq_symbol(int symnum)
{
  if (Eq_symnum == 0) {
    Eq_symnum = str_to_sn(eq_sym(), 2);
  }
  return (symnum == Eq_symnum ? TRUE : FALSE);
}  /* is_eq_symbol */

/*************
 *
 *   not_symnum()
 *
 *************/

/* DOCUMENTATION
Return the symnum for not_sym()/1.
*/

/* PUBLIC */
int not_symnum(void)
{
  if (Not_symnum == 0)
    Not_symnum = str_to_sn(not_sym(), 1);
  return Not_symnum;
}  /* not_symnum */

/*************
 *
 *   or_symnum()
 *
 *************/

/* DOCUMENTATION
Return the symnum for or_sym()/2.
*/

/* PUBLIC */
int or_symnum(void)
{
  if (Or_symnum == 0)
    Or_symnum = str_to_sn(or_sym(), 2);
  return Or_symnum;
}  /* or_symnum */

/*************
 *
 *   declare_base_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_base_symbols(void)
{
  int sn;
  sn = str_to_sn(false_sym(), 0);
  sn = str_to_sn(true_sym(), 0);
  sn = str_to_sn("false", 0);
  sn = str_to_sn("true", 0);
}  /* declare_base_symbols */

/****************************************************************************/
/* This section is about variables. */
/****************************************************************************/

static Variable_style Var_style = STANDARD_STYLE;

/*************
 *
 *   set_variable_style()
 *
 *************/

/* DOCUMENTATION
This routine determines how variables are parsed and printed.
*/

/* PUBLIC */
void set_variable_style(Variable_style style)
{
  Var_style = style;
}  /* set_variable_style */

/*************
 *
 *   variable_style()
 *
 *************/

/* DOCUMENTATION
This routine gives the current variable style.
*/

/* PUBLIC */
Variable_style variable_style(void)
{
  return Var_style;
}  /* set_variable_style */

/*************
 *
 *   variable_name()
 *
 *************/

/* DOCUMENTATION
Is the given name a variable?  Formulas can have free
variables (not explicitly quantified), so we have a rule to
distinguish variables from constants.  This is it.
*/

/* PUBLIC */
BOOL variable_name(char *s)
{
  if (variable_style() == PROLOG_STYLE)
    return (*s >= 'A' && *s <= 'Z');
  else if (variable_style() == INTEGER_STYLE)
    return (*s >= '0' && *s <= '9');
  else
    return (*s >= 'u' && *s <= 'z');
}  /* variable_name */

/*************
 *
 *   symbol_for_variable()
 *
 *************/

/* DOCUMENTATION
Given a pointer to a string and a variable index,
fill in the string with the variable symbol.
The variable symbol is determined by the current
variable style (standard, prolog, integer, etc.),
which can be changed with set_variable_style().
*/

/* PUBLIC */
void symbol_for_variable(char *str, int varnum)
{
  if (variable_style() == INTEGER_STYLE)
    /* 0,1,2,3,4,5,6,7,... */
    sprintf(str, "%d", varnum);
  else if (variable_style() == PROLOG_STYLE) {
    /* A,B,C,D,E,F,V6,V7,V8,... */
    if (varnum < 6)
      sprintf(str, "%c", 'A' + varnum);
    else
      sprintf(str, "V%d", varnum);
  }
  else {
    /* x,y,z,u,w,v5,v6,v7,v8,... */
    if (varnum < 3)
      sprintf(str, "%c", 'x' + varnum);
    else if (varnum == 3)
      sprintf(str, "%c", 'u');
    else if (varnum == 4)
      sprintf(str, "%c", 'w');
    else
      sprintf(str, "v%d", varnum);
  }
}  /* symbol_for_variable */

/*************
 *
 *   variable_symbols()
 *
 *************/

/* DOCUMENTATION
Given a Plist of symbols (symnums), return a (new) list of the
symnums that correspond to variables.
*/

/* PUBLIC */
Ilist variable_symbols(Ilist syms)
{
  if (syms == NULL)
    return NULL;
  else {
    Ilist work = variable_symbols(syms->next);
    if (sn_to_arity(syms->i) == 0 && variable_name(sn_to_str(syms->i)))
      work = ilist_prepend(work, syms->i);
    return work;
  }
}  /* variable_symbols */

/*************
 *
 *   remove_variable_symbols()
 *
 *************/

/* DOCUMENTATION
Given a Plist of symbols (symnums), remove the ones that
correspond to variables.
*/

/* PUBLIC */
Ilist remove_variable_symbols(Ilist syms)
{
  Ilist vars = variable_symbols(syms);
  Ilist result = ilist_subtract(syms,vars);
  zap_ilist(syms);
  zap_ilist(vars);
  return result;
}  /* remove_variable_symbols */

/****************************************************************************/
/* This section is about FUNCTION/RELATION distinction.                     */
/****************************************************************************/

/*************
 *
 *   set_symbol_type()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_symbol_type(int symnum, Symbol_type type)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    fatal_error("set_symbol_type: bad symnum");
  p->type = type;
}  /* set_symbol_type */

/*************
 *
 *   get_symbol_type()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Symbol_type get_symbol_type(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    fatal_error("get_symbol_type: bad symnum");
  return p->type;
}  /* get_symbol_type */

/*************
 *
 *   function_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL function_symbol(int symnum)
{
  return get_symbol_type(symnum) == FUNCTION_SYMBOL;
}  /* function_symbol */

/*************
 *
 *   relation_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL relation_symbol(int symnum)
{
  return get_symbol_type(symnum) == PREDICATE_SYMBOL;
}  /* relation_symbol */

/*************
 *
 *   function_or_relation_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL function_or_relation_symbol(int symnum)
{
  Symbol_type t = get_symbol_type(symnum);
  return t == PREDICATE_SYMBOL || t == FUNCTION_SYMBOL;
}  /* function_or_relation_symbol */

/*************
 *
 *   declare_functions_and_relations()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_functions_and_relations(Ilist fsyms, Ilist rsyms)
{
  Ilist p;
  for (p = fsyms; p; p = p->next)
    set_symbol_type(p->i, FUNCTION_SYMBOL);
  for (p = rsyms; p; p = p->next)
    set_symbol_type(p->i, PREDICATE_SYMBOL);
}  /* declare_functions_and_relations */

/*************
 *
 *   function_or_relation_sn()
 *
 *************/

/* DOCUMENTATION
If there is a function or relation symbol in the table
with the given string, return the symnum; otherwise
return -1.  (If there is more than one, the first one
found is returned.)
*/

/* PUBLIC */
int function_or_relation_sn(char *str)
{
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if ((s->type == FUNCTION_SYMBOL || s->type == PREDICATE_SYMBOL) &&
	  str_ident(str, s->name))
	return s->symnum;
    }
  }
  return -1;
}  /* function_or_relation_sn */

/*************
 *
 *   all_function_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist all_function_symbols(void)
{
  Ilist syms = NULL;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if (s->type == FUNCTION_SYMBOL)
	syms = ilist_append(syms, s->symnum);
    }
  }
  return syms;
}  /* all_function_symbols */

/*************
 *
 *   all_relation_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist all_relation_symbols(void)
{
  Ilist syms = NULL;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if (s->type == PREDICATE_SYMBOL)
	syms = ilist_append(syms, s->symnum);
    }
  }
  return syms;
}  /* all_relation_symbols */

/****************************************************************************/
/* This section is all about LRPO status.                                   */
/****************************************************************************/

/*************
 *
 *   set_lrpo_status()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_lrpo_status(int symnum, Lrpo_status status)
{
  Symbol p = lookup_by_id(symnum);
  p->lrpo_status = status;
}  /* set_lrpo_status */

/*************
 *
 *   all_symbols_lrpo_status()
 *
 *************/

/* DOCUMENTATION
Assign all symbols the given lrpo status:
LRPO_LR_STATUS or LRPO_MULTISET_STATUS.
*/

/* PUBLIC */
void all_symbols_lrpo_status(Lrpo_status status)
{
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      s->lrpo_status = status;
    }
  }
}  /* all_symbols_lrpo_status */

/*************
 *
 *    sn_to_lrpo_status
 *
 *************/

/* DOCUMENTATION
This routine returns the LRPO status associated with a symbol ID.
The default value is LRPO_LR_STATUS.  See order.h for the possible
values.  If the symbol ID is not valid, 0 is returned.
*/

/* PUBLIC */
Lrpo_status sn_to_lrpo_status(int sn)
{
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? 0 : p->lrpo_status);
}  /* sn_to_lrpo_status */

/****************************************************************************/
/* This section is all about KB weights.                                    */
/****************************************************************************/

static BOOL Zero_wt_kb = FALSE;  /* is there symbol with kb_weight=0? */

/*************
 *
 *   set_kb_weight()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_kb_weight(int symnum, int weight)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL) {
    fatal_error("set_kb_weight, symbol not found");
  }

  if (weight == 0) {
    if (Zero_wt_kb)
      fatal_error("set_kb_weight, more than one symbol of weight 0");
    else if (p->arity != 1 || p->type != FUNCTION_SYMBOL)
      fatal_error("set_kb_weight, weight 0 symbols must be unary"
		  " function symbols");
    else
      Zero_wt_kb = TRUE;
  }
  p->kb_weight = weight;
}  /* set_kb_weight */

/*************
 *
 *   zero_wt_kb()
 *
 *************/

/* DOCUMENTATION
Is there already a symbol with KB weight 0?
*/

/* PUBLIC */
BOOL zero_wt_kb(void)
{
  return Zero_wt_kb;
}  /* zero_wt_kb */

/*************
 *
 *    int sn_to_kb_wt()
 *
 *************/

/* DOCUMENTATION
This routine returns the Knuth-Bendix weight associated with a symbol ID.
*/

/* PUBLIC */
int sn_to_kb_wt(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    return(-1);
  else
    return(p->kb_weight);
}  /* sn_to_kb_wt */

/*************
 *
 *   print_kbo_weights()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void print_kbo_weights(FILE *fp)
{
  Ilist fsyms = current_fsym_precedence();
  Ilist p;

  fprintf(fp, "Function symbol KB weights: ");
  for (p = fsyms; p; p = p->next)
    fprintf(fp, " %s=%d.", sn_to_str(p->i), sn_to_kb_wt(p->i));
  fprintf(fp, "\n");
  zap_ilist(fsyms);
}  /* print_kbo_weights */

/****************************************************************************/
/* This section is all about Skolem symbols.                                */
/****************************************************************************/

static char *Skolem_constant_prefix = "c";
static char *Skolem_function_prefix = "f";
static int Next_skolem_constant = 1;      /* counter for c1, c2, ... */
static int Next_skolem_function = 1;      /* counter for f1, f2, ... */
static BOOL Skolem_check = TRUE;  /* make sure Skolem symbols are unique */

/*************
 *
 *    set_skolem()
 *
 *************/

/* DOCUMENTATION
This routine declares that a symbol is a Skolem function (or constant).
*/

/* PUBLIC */
void set_skolem(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  p->skolem = TRUE;
  p->type = FUNCTION_SYMBOL;
}  /* set_skolem */

/*************
 *
 *   skolem_check()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void skolem_check(BOOL flag)
{
  Skolem_check = flag;
}  /* skolem_check */

/*************
 *
 *   skolem_ok()
 *
 *************/

static
BOOL skolem_ok(char *name, int arity)
{
  if (!Skolem_check)
    return TRUE;
  else {
    Symbol s = lookup_by_sym(name, arity);
    if (s == NULL)
      return TRUE;
    else
      return s->auxiliary;
  }
}  /* skolem_ok */

/*************
 *
 *   next_skolem_symbol()
 *
 *************/

/* DOCUMENTATION
This routine returns a fresh symbol ID, which is intended to be
used as a Skolem symbol.  
The symbols are c1, c2, c3, ... for arity 0 (constants) and
f1, f2, f3, ... for arity != 0.
If some of those symbols already exist
in the symbol table (with any arity), they will be skipped.
*/

/* PUBLIC */
int next_skolem_symbol(int arity)
{
  char name[20];

  do {
    if (arity == 0) {
      sprintf(name, "%s%d", Skolem_constant_prefix, Next_skolem_constant);
      Next_skolem_constant++;
    }
    else {
      sprintf(name, "%s%d", Skolem_function_prefix, Next_skolem_function);
      Next_skolem_function++;
    }
  } while (!skolem_ok(name,arity));
  
  {
    int symnum = str_to_sn(name, arity);
    set_skolem(symnum);
    return symnum;
  }
}  /* next_skolem_symbol */

/*************
 *
 *   skolem_symbols()
 *
 *************/

/* DOCUMENTATION
Return the list of SYMNUMs (increasing) that have
been declared to be Skolem symbols.
*/

/* PUBLIC */
Ilist skolem_symbols(void)
{
  Ilist g = NULL;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if (s->skolem)
	g = ilist_prepend(g, s->symnum);
    }
  }
  return reverse_ilist(g);
}  /* skolem_symbols */

/*************
 *
 *   is_skolem()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL is_skolem(int symnum)
{
  Symbol p = lookup_by_id(symnum);
  return p->skolem;
}  /* is_skolem */

/*************
 *
 *   skolem_reset()
 *
 *************/

/* DOCUMENTATION
Reset the Skolem symbol counters (constant and function) to 1.
*/

/* PUBLIC */
void skolem_reset(void)
{
  Next_skolem_constant = 1;
  Next_skolem_function = 1;
}  /* skolem_reset */

/*************
 *
 *   decommission_skolem_symbols()
 *
 *************/

/* DOCUMENTATION
For each symbol in the symbol table, if it is marked
"skolem", unmark it and set the type to "unspecified".
*/

/* PUBLIC */
void decommission_skolem_symbols(void)
{
  Ilist fsyms = all_function_symbols();
  Ilist p;

  for (p = fsyms; p; p = p->next) {
    Symbol n = lookup_by_id(p->i);
    if (n->skolem) {
      n->skolem = FALSE;
      n->type = UNSPECIFIED_SYMBOL;
    }
  }
  zap_ilist(fsyms);
}  /* decommission_skolem_symbols */

/*************
 *
 *   set_skolem_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_skolem_symbols(Ilist symnums)
{
  Ilist p;
  for (p = symnums; p; p = p->next) {
    Symbol sym = lookup_by_id(p->i);
    if (sym == NULL)
      fatal_error("set_skolem_symbols, symbol not found");
    sym->skolem = TRUE;
  }
}  /* set_skolem_symbols */

/****************************************************************************/
/* This section is all about lex_val and symbol precedence.                 */
/****************************************************************************/

static Ilist Preliminary_prec_func = NULL;
static Ilist Preliminary_prec_pred  = NULL;

/*************
 *
 *    set_lex_val()
 *
 *************/

/* DOCUMENTATION
This routine is used to assign a lexical value to a symbol.
The value can be retrieved later with sn_to_lex_val();
*/

/* PUBLIC */
void set_lex_val(int symnum, int lex_val)
{
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)
    fatal_error("set_lex_val, invalid symnum");
  p->lex_val = lex_val;
  /* printf("set_lex_val %s/%d, %d\n", p->name, p->arity, lex_val); */
}  /* set_lex_val */

/*************
 *
 *    sn_to_lex_val
 *
 *************/

/* DOCUMENTATION
This routine returns the lexical value associated with a symbol ID.
The default value is INT_MAX.  If the symbol ID is not valid, INT_MIN
is returned.
*/

/* PUBLIC */
int sn_to_lex_val(int sn)
{
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? INT_MIN : p->lex_val);
}  /* sn_to_lex_val */

/*************
 *
 *    int sym_precedence(symnum_1, symnum_2)
 *
 *************/

/* DOCUMENTATION
This routine compares two symbol IDs by looking at their lex_val
in the symbol table.  The range of return values is<BR>
{SAME_AS, GREATER_THAN, LESS_THAN, NOT_COMPARABLE}.
*/

/* PUBLIC */
Ordertype sym_precedence(int symnum_1, int symnum_2)
{
  int p1, p2;

  if (symnum_1 == symnum_2)
    return SAME_AS;
  else {
    p1 = sn_to_lex_val(symnum_1);
    p2 = sn_to_lex_val(symnum_2);
	
    if (p1 == INT_MAX || p2 == INT_MAX)
      return NOT_COMPARABLE;
    else if (p1 > p2)
      return GREATER_THAN;
    else if (p1 < p2)
      return LESS_THAN;
    else
      return SAME_AS;
  }
}  /* sym_precedence */

/*************
 *
 *   syms_with_lex_val()
 *
 *************/

/* DOCUMENTATION
Return an Ilist containing symnums of symbols to which lex_vals have
been assigned.
*/

/* PUBLIC */
Ilist syms_with_lex_val(void)
{
  Ilist g = NULL;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = By_id[i]; p; p = p->next) {
      Symbol s = p->v;
      if (s->lex_val != INT_MAX)
	g = ilist_append(g, s->symnum);
    }
  }
  return g;
}  /* syms_with_lex_val */

/*************
 *
 *   exists_preliminary_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL exists_preliminary_precedence(Symbol_type type)
{
  if (type == FUNCTION_SYMBOL)
    return Preliminary_prec_func != NULL;
  else if (type == PREDICATE_SYMBOL)
    return Preliminary_prec_pred != NULL;
  else
    return FALSE;
}  /* exists_preliminary_precedence */

/*************
 *
 *   preliminary_lex_compare()
 *
 *************/

/* DOCUMENTATION
Compare the given strings with respect to the list of strings
given to the set_preliminary_precedence call.  Strings without
preliminary_precedence are smaller than those with.  Two different
strings without preliminary_precedence are NOT_COMPARABLE.
<P>
Return LESS_THAN, GREATER_THAN, SAME_AS, NOT_COMPARABLE.
*/

/* PUBLIC */
Ordertype preliminary_lex_compare(Symbol a, Symbol b)
{
  int ai = -1;
  int bi = -1;
  if (a->type != b->type)
    return NOT_COMPARABLE;
  else if (a->type == UNSPECIFIED_SYMBOL)
    return NOT_COMPARABLE;
  else if (a->type == FUNCTION_SYMBOL) {
    ai = position_in_ilist(a->symnum, Preliminary_prec_func);
    bi = position_in_ilist(b->symnum, Preliminary_prec_func);
  }
  else if (a->type == PREDICATE_SYMBOL) {
    ai = position_in_ilist(a->symnum, Preliminary_prec_pred);
    bi = position_in_ilist(b->symnum, Preliminary_prec_pred);
  }
  /* printf("%s=%d, %s=%d\n", a, ai, b, bi); */
  if (ai == -1)
    ai = INT_MIN;
  if (bi == -1)
    bi = INT_MIN;

  if (ai < bi)
    return LESS_THAN;
  else if (ai > bi)
    return GREATER_THAN;
  else if (ai == INT_MIN)
    return NOT_COMPARABLE;  /* neither in preliminary_precedence */
  else
    return SAME_AS;
}  /* preliminary_lex_compare */

/*************
 *
 *   lex_compare_base()
 *
 *************/

static
Ordertype lex_compare_base(Symbol s1, Symbol s2)
{
  if (s1 == s2)
    return SAME_AS;
  else if (s1 == NULL)
    return LESS_THAN;
  else if (s2 == NULL)
    return GREATER_THAN;

  /* FUNCTION < RELATION < others (don't know if there can be others) */

  else if (s1->type == FUNCTION_SYMBOL && s2->type != FUNCTION_SYMBOL)
    return LESS_THAN;
  else if (s1->type != FUNCTION_SYMBOL && s2->type == FUNCTION_SYMBOL)
    return GREATER_THAN;

  else if (s1->type == PREDICATE_SYMBOL && s2->type != PREDICATE_SYMBOL)
    return LESS_THAN;
  else if (s1->type != PREDICATE_SYMBOL && s2->type == PREDICATE_SYMBOL)
    return GREATER_THAN;

  /* Now they have the same type (FUNCTION, RELATION, other). */
  /* Check for preliminary order (lex command). */

  else if (preliminary_lex_compare(s1, s2) == LESS_THAN)
    return LESS_THAN;
  else if (preliminary_lex_compare(s1, s2) == GREATER_THAN)
    return GREATER_THAN;

  /* = < other relations */

  else if (s1->type == PREDICATE_SYMBOL && is_eq_symbol(s2->symnum))
    return GREATER_THAN;
  else if (s1->type == PREDICATE_SYMBOL && is_eq_symbol(s1->symnum))
    return LESS_THAN;

  /* if arities same:
     (1) Skolems > non-Skolems
     (2) if both Skolems, use sumnum
     (3) more-occurrences < fewer-occurrences
     (4) Use UNIX's strcomp, which is lexical ascii ordering.
  */

  else if (s1->arity == s2->arity) {

    if (s1->skolem || s2->skolem) {
      if (!s2->skolem)
	return GREATER_THAN;
      else if (!s1->skolem)
	return LESS_THAN;
      else if (s1->symnum > s2->symnum)
	return GREATER_THAN;
      else if (s1->symnum < s2->symnum)
	return LESS_THAN;
      else
	return SAME_AS;
    }
    
    else if (s1->occurrences < s2->occurrences)
      return GREATER_THAN;
    else if (s1->occurrences > s2->occurrences)
      return LESS_THAN;
    else {
      int i = strcmp(s1->name, s2->name);
      if (i < 0)
	return LESS_THAN;
      else if (i > 0)
	return GREATER_THAN;
      else
	return SAME_AS;
    }
  }

  /* the type is the same, but arities are different */

  else
    return NOT_COMPARABLE;  /* code for "not yet decided" */
}  /* lex_compare_base */

/*************
 *
 *   lex_compare_arity_0123()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype lex_compare_arity_0123(Symbol s1, Symbol s2)
{
  Ordertype base = lex_compare_base(s1, s2);

  if (base != NOT_COMPARABLE)
    return base;
  else
    /* symbols same type, but with different arities */
    return s1->arity < s2->arity ? LESS_THAN : GREATER_THAN;
}  /* lex_compare_arity_0123 */

/*************
 *
 *   lex_compare_arity_0213()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype lex_compare_arity_0213(Symbol s1, Symbol s2)
{
  Ordertype base = lex_compare_base(s1, s2);

  if (base != NOT_COMPARABLE)
    return base;
  else {
    /* Symbols same type, but with different arities.
       Relations: order by arity.
       Functions: constants < arity-2 < arity-1 < arity-3 < arity-4 ... .
    */

    if (s1->type == PREDICATE_SYMBOL)
      return s1->arity < s2->arity ? LESS_THAN : GREATER_THAN;
    else if (s1->arity == 1)
      return s2->arity >= 3 ? LESS_THAN : GREATER_THAN;
    else if (s2->arity == 1)
      return s1->arity < 3 ? LESS_THAN : GREATER_THAN;
    else
      return s1->arity < s2->arity ? LESS_THAN : GREATER_THAN;
  }
}  /* lex_compare_arity_0213 */

/*************
 *
 *   lex_order()
 *
 *************/

/* DOCUMENTATION
Assign a total order on lex_vals of (fsyms U rsyms).
If a list of strings was previously given to
set_preliminary_precedence, that order is maintained
for symbols that have those strings.
For the other rules, see *comp_proc (lex_compare*).
*/

/* PUBLIC */
void lex_order(Ilist fsyms, Ilist rsyms,
	       I2list fsyms_multiset, I2list rsyms_multiset,
	       Ordertype (*comp_proc) (Symbol, Symbol))
{
  int n = ilist_count(fsyms) + ilist_count(rsyms);
  Symbol *a = malloc(n * sizeof(void *));
  Ilist p;
  int i = 0;
  for (p = fsyms; p; p = p->next, i++) {
    a[i] = lookup_by_id(p->i);
    a[i]->occurrences = multiset_occurrences(fsyms_multiset, p->i);
  }
  for (p = rsyms; p; p = p->next, i++) {
    a[i] = lookup_by_id(p->i);
    a[i]->occurrences = multiset_occurrences(rsyms_multiset, p->i);
  }

  add_skolems_to_preliminary_precedence();

  merge_sort((void **) a, n, (Ordertype (*)(void*, void*)) comp_proc);
  
  for (i = 0; i < n; i++)
    a[i]->lex_val = i;
  free(a);
}  /* lex_order */

/*************
 *
 *   insert_by_lex_val()
 *
 *************/

static
Ilist insert_by_lex_val(Ilist head, Ilist tail)
{
  if (tail == NULL) {
    head->next = NULL;
    return head;
  }
  else if (sn_to_lex_val(head->i) < sn_to_lex_val(tail->i)) {
    head->next = tail;
    return head;
  }
  else {
    tail->next = insert_by_lex_val(head, tail->next);
    return tail;
  }
}  /* insert_by_lex_val */

/*************
 *
 *   sort_by_lex_val()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist sort_by_lex_val(Ilist p)
{
  if (p == NULL)
    return NULL;
  else {
    return insert_by_lex_val(p, sort_by_lex_val(p->next));
  }
}  /* sort_by_lex_val */

/*************
 *
 *   remove_syms_without_lex_val()
 *
 *************/

static
Ilist remove_syms_without_lex_val(Ilist syms)
{
  if (syms == NULL)
    return NULL;
  else {
    syms->next = remove_syms_without_lex_val(syms->next);
    if (sn_to_lex_val(syms->i) == INT_MAX) {
      Ilist rest = syms->next;
      free_ilist(syms);
      return rest;
    }
    else 
      return syms;
  }
}  /* remove_syms_without_lex_val */

/*************
 *
 *   current_fsym_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist current_fsym_precedence()
{
  Ilist syms = all_function_symbols();
  syms = remove_syms_without_lex_val(syms);
  syms = sort_by_lex_val(syms);
  return syms;
}  /* current_fsym_precedence */

/*************
 *
 *   current_rsym_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist current_rsym_precedence()
{
  Ilist syms = all_relation_symbols();
  syms = remove_syms_without_lex_val(syms);
  syms = sort_by_lex_val(syms);
  return syms;
}  /* current_rsym_precedence */

/*************
 *
 *   not_in_preliminary_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist not_in_preliminary_precedence(Ilist syms, Symbol_type type)
{
  Ilist missing = NULL;
  Ilist p;
  for (p = syms; p; p = p->next) {
    if (type == FUNCTION_SYMBOL &&
	position_in_ilist(p->i, Preliminary_prec_func) == -1)
      missing = ilist_append(missing, p->i);

    else if (type == PREDICATE_SYMBOL &&
	     position_in_ilist(p->i, Preliminary_prec_pred) == -1)
      missing = ilist_append(missing, p->i);
  }
  return missing;
}  /* not_in_preliminary_precedence */

/*************
 *
 *   print_fsym_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void print_fsym_precedence(FILE *fp)
{
  Ilist fsyms = current_fsym_precedence();
  Ilist p;

  printf("Function symbol precedence:  function_order([");
  for (p = fsyms; p; p = p->next)
    printf(" %s%s", sn_to_str(p->i), p->next ? "," : "");
  printf(" ]).\n");
  
  zap_ilist(fsyms);
}  /* print_fsym_precedence */

/*************
 *
 *   print_rsym_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void print_rsym_precedence(FILE *fp)
{
  Ilist rsyms = current_rsym_precedence();
  Ilist p;

  printf("Predicate symbol precedence:  predicate_order([");
  for (p = rsyms; p; p = p->next)
    printf(" %s%s", sn_to_str(p->i), p->next ? "," : "");
  printf(" ]).\n");
  
  zap_ilist(rsyms);
}  /* print_rsym_precedence */

/*************
 *
 *   min_lex_val()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int min_lex_val(void)
{
  Ilist a = syms_with_lex_val();
  Ilist p;
  int min = INT_MAX;
  for (p = a; p; p = p->next) {
    int x = sn_to_lex_val(p->i);
    min = IMIN(min, x);
  }
  zap_ilist(a);
  return min;
}  /* min_lex_val */

/*************
 *
 *   max_lex_val()
 *
 *************/

static
int max_lex_val(void)
{
  Ilist a = syms_with_lex_val();
  Ilist p;
  int max = INT_MIN;
  for (p = a; p; p = p->next) {
    int x = sn_to_lex_val(p->i);
    max = IMAX(max, x);
  }
  zap_ilist(a);
  return max;
}  /* min_lex_val */

/*************
 *
 *   assign_greatest_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void assign_greatest_precedence(int symnum)
{
  set_lex_val(symnum, max_lex_val() + 1);
}  /* assign_greatest_precedence */

/*************
 *
 *   has_greatest_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL has_greatest_precedence(int symnum)
{
  return sn_to_lex_val(symnum) == max_lex_val();
}  /* has_greatest_precedence */

/*************
 *
 *   lex_insert_after_initial_constants()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void lex_insert_after_initial_constants(Ilist syms)
{
  if (syms) {
    Ilist all = current_fsym_precedence();
    Ilist a, s;
    int val = 1;

    for (a = all; a && sn_to_arity(a->i) == 0; a = a->next) {
      if (!ilist_member(syms, a->i))
	set_lex_val(a->i, val++);
    }
  
    syms = sort_by_lex_val(syms);  /* so that relative order is unchanged */

    for (s = syms; s; s = s->next)
      set_lex_val(s->i, val++);

    for (; a; a = a->next) {
      if (!ilist_member(syms, a->i))
	set_lex_val(a->i, val++);
    }
  }
}  /* lex_insert_after_initial_constants */

/*************
 *
 *   skolem_insert()
 *
 *************/

static
Ilist skolem_insert(Ilist prec, int i)
{
  if (prec == NULL)
    return ilist_append(NULL, i);
  else if (sn_to_arity(prec->i) > sn_to_arity(i))
    return ilist_prepend(prec, i); 
  else {
    prec->next = skolem_insert(prec->next, i);
    return prec;
  }
}  /* skolem_insert */

/*************
 *
 *   add_skolems_to_preliminary_precedence()
 *
 *************/

/* DOCUMENTATION
If there is a preliminary precedence, add the skolem symbols
to it in the following way.  For each Skolem symbol of arity-n,
add it to Preliminary_precedence just before the first symbol
of higher arity (else at the end).
*/

/* PUBLIC */
void add_skolems_to_preliminary_precedence(void)
{
  if (Preliminary_prec_func != NULL) {
    Ilist skolems = skolem_symbols();
    Ilist p;
    /* printf("Before adding skolems: "); p_ilist(Preliminary_precedence); */
    for (p = skolems; p; p = p->next) {
      if (!ilist_member(Preliminary_prec_func, p->i))
	Preliminary_prec_func = skolem_insert(Preliminary_prec_func, p->i);
    }
    /* printf("After adding skolems: "); p_ilist(Preliminary_precedence); */
    zap_ilist(skolems);
  }
}  /* add_skolems_to_preliminary_precedence */

/****************************************************************************/
/* This section is all about symbols generated on the fly. */
/****************************************************************************/

static unsigned Mark_for_new_symbols = 0;  /* */

/*************
 *
 *   fresh_symbol()
 *
 *************/

/* DOCUMENTATION
This routine returns a symbol ID for a new symbol with the given
arity.  The symbol is made up of the given prefix followed by the
smallest natural number that results in a new symbol (regardless of
arity).  The prefix must be less than MAX_NAME characters.
*/

/* PUBLIC */
int fresh_symbol(char *prefix, int arity)
{
  char name[MAX_NAME+20];
  int i = 0;

  if (strlen(prefix) > MAX_NAME) {
    fatal_error("fresh_symbol, prefix is too big.");
  }

  do {
    sprintf(name, "%s%d", prefix, i);
    i++;
  } while (str_exists(name));

  return str_to_sn(name, arity);
}  /* fresh_symbol */

/*************
 *
 *   gen_new_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int gen_new_symbol(char *prefix, int arity, Ilist syms)
{
  char name[MAX_NAME+20];
  int symnum;
  int i = 0;

  if (strlen(prefix) > MAX_NAME)
    fatal_error("gen_new_symbol, prefix is too big.");

  sprintf(name, "%s%d", prefix, i);
  symnum = str_to_sn(name, arity);

  while (ilist_member(syms, symnum)) {
    i++;
    sprintf(name, "%s%d", prefix, i);
    symnum = str_to_sn(name, arity);
  }

  return symnum;
}  /* gen_new_symbol */

/*************
 *
 *   mark_for_new_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_for_new_symbols(void)
{
  Mark_for_new_symbols = Symbol_count + 1;  /* next symnum */
}  /* mark_for_new_symbols */

/*************
 *
 *   new_symbols_since_mark()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list new_symbols_since_mark(void)
{
  I2list p = NULL;
  int sn;
  for (sn = Mark_for_new_symbols; sn <= Symbol_count; sn++) {
    if (function_or_relation_symbol(sn)) {
      p = i2list_append(p, sn, sn_to_arity(sn));
    }
  }
  return p;
}  /* new_symbols_since_mark */

/*************
 *
 *   add_new_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void add_new_symbols(I2list syms)
{
  I2list p;
  for (p = syms; p; p = p->next) {
    int sn;
    int symnum = p->i;
    int arity = p->j;
    if (symnum != Symbol_count+1)
      fatal_error("add_new_symbols, bad symnum");
    sn = fresh_symbol("child_symbol_", arity);
    if (sn != symnum)
      fatal_error("add_new_symbols, symnums do not match");
  }
}  /* add_new_symbols */

/*************
 *
 *   new_constant_properties()
 *
 *************/

/* DOCUMENTATION
In the symbol table entry for the given symbol number, set
type=function, kb_weight=1, lex_val=(after initial constants)
*/

/* PUBLIC */
void new_constant_properties(int sn)
{
  Symbol s = lookup_by_id(sn);

  if (s == NULL || s->arity != 0)
    fatal_error("new_constant_properties, bad symbol number");
  s->type = FUNCTION_SYMBOL;
  s->kb_weight = 1;
  {
    Ilist syms = ilist_append(NULL, sn);
    lex_insert_after_initial_constants(syms);
    zap_ilist(syms);
  }
}  /* new_constant_properties */

/*************
 *
 *   collect_multiples()
 *
 *************/

static
Ilist collect_multiples(Ilist syms)
{
  /* Example: Given  (f/0, f/1, g/0, h/2), 
              return (f/0, f/1).
  */
  Ilist p1;
  Ilist p2;
  Ilist bad_syms = NULL;
  for (p1 = syms; p1; p1 = p1->next) {
    char *s1 = sn_to_str(p1->i);
    for (p2 = p1->next; p2; p2 = p2->next) {
      char *s2 = sn_to_str(p2->i);
      if (str_ident(s1, s2)) {
	if (!ilist_member(bad_syms, p1->i))
	  bad_syms = ilist_prepend(bad_syms, p1->i);
	if (!ilist_member(bad_syms, p2->i))
	  bad_syms = ilist_prepend(bad_syms, p2->i);
      }
    }
  }
  return bad_syms;
}  /* collect_multiples */

/*************
 *
 *   arity_check()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist arity_check(Ilist fsyms, Ilist rsyms)
{
  Ilist syms = ilist_cat(ilist_copy(fsyms), ilist_copy(rsyms));
  Ilist bad_syms = collect_multiples(syms);
  return bad_syms;
}  /* arity_check */

/*************
 *
 *   symbol_with_string()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int symbol_with_string(Ilist syms, char *str)
{
  if (syms == NULL)
    return -1;
  else if (str_ident(str, sn_to_str(syms->i)))
    return syms->i;
  else
    return symbol_with_string(syms->next, str);
}  /* symbol_with_string */

/*************
 *
 *   process_skolem_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void process_skolem_list(Plist skolem_strings, Ilist fsyms)
{
  Ilist skolems = NULL;
  Plist p;
  for (p = skolem_strings; p; p = p->next) {
    int sn = symbol_with_string(fsyms, p->v);
    if (sn == -1)
      fprintf(stderr, "WARNING, declared Skolem symbol not found in formulas: %s\n", (char *) p->v);
    else
      skolems = ilist_append(skolems, sn);
  }
  set_skolem_symbols(skolems);
  zap_ilist(skolems);
}  /* process_skolem_list */

/*************
 *
 *   process_lex_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void process_lex_list(Plist lex_strings, Ilist syms, Symbol_type type)
{
  Ilist lexs = NULL;
  Plist p;
  Plist not_in_formulas = NULL;
  for (p = lex_strings; p; p = p->next) {
    int sn = symbol_with_string(syms, p->v);
    if (sn == -1)
      not_in_formulas = plist_append(not_in_formulas, p->v);
    else
      lexs = ilist_append(lexs, sn);
  }
  if (not_in_formulas) {
    char *s = (type == FUNCTION_SYMBOL ? "function" : "predicate");
    fprintf(stderr,
	    "WARNING, %s symbols in %s_order (lex) command not found in formulas: ", s, s);
    fprintf(stdout,
	    "WARNING, %s symbols in %s_order (lex) command not found in formulas: ", s, s);
    for (p = not_in_formulas; p; p = p->next) {
      fprintf(stderr, "%s%s", (char *) p->v, p->next ? ", " : ".\n");
      fprintf(stdout, "%s%s", (char *) p->v, p->next ? ", " : ".\n");
    }
  }

  if (type == FUNCTION_SYMBOL)
    Preliminary_prec_func = lexs;
  else
    Preliminary_prec_pred = lexs;
    
  zap_plist(not_in_formulas);
}  /* process_lex_list */

/*************
 *
 *   symnums_of_arity()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist symnums_of_arity(Ilist p, int i)
{
  if (p == NULL)
    return NULL;
  else if (sn_to_arity(p->i) != i) {
    Ilist r = p->next;
    free_ilist(p);
    return symnums_of_arity(r, i);
  }
  else {
    p->next = symnums_of_arity(p->next, i);
    return p;
  }
}  /* symnums_of_arity */

