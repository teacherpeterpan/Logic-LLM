#include "../ladr/header.h"

#include "../ladr/ioutil.h"
#include "../ladr/commands.h"
#include "../ladr/mindex.h"
#include "../ladr/demod.h"
#include "../ladr/interp.h"

#define PROGRAM_NAME    "latgen"
#define PROGRAM_VERSION "2004-F"
#define PROGRAM_DATE    "Aug 2004"

/* Flag IDs */

#define VERBOSE 0

/* Global vars */

static int Meet_sym, Join_sym;

static Term procterm(Term t, Mindex idx)
{
  Ilist just = NULL;
  ac_canonical(t, -1);
  t = demodulate(t, idx, &just);
  zap_ilist(just);
  if (is_term(t, "junk", 0)) {
    zap_term(t);
    return NULL;
  }
  else
    return t;
}  /* procterm */

static Plist gen_terms(Mindex idx, int max)
{

  Plist usable, sos;

  usable = NULL;

  sos = NULL;
  /* sos = plist_append(sos, get_rigid_term("u", 0)); */
  sos = plist_append(sos, get_rigid_term("z", 0));
  sos = plist_append(sos, get_rigid_term("y", 0));
  sos = plist_append(sos, get_rigid_term("x", 0));

  while (sos) {
    Plist p = sos;
    Term s = p->v;
    sos = sos->next;
    
    free_plist(p);

    for (p = usable; p; p = p->next) {
      Term u = p->v;
      
      if (symbol_count(s) + symbol_count(u) + 1 <= max) {
	Term new;
	new = procterm(build_binary_term(Meet_sym, copy_term(s), copy_term(u)), idx);
	if (new)
	  sos = plist_prepend(sos, new);
	new = procterm(build_binary_term(Join_sym, copy_term(s), copy_term(u)), idx);
	if (new)
	  sos = plist_prepend(sos, new);
      }
    }
    usable = plist_prepend(usable, s);
  }
  return usable;
}  /* gen_terms */

static Term variablize(Term t)
{
  if (CONSTANT(t)) {
    if (is_term(t, "x", 0)) {
      zap_term(t);
      t = get_variable_term(0);
      t->private_flags = 1;
    }
    else if (is_term(t, "y", 0)){
      zap_term(t);
      t = get_variable_term(1);
      t->private_flags = 2;
    }
    else if (is_term(t, "z", 0)){
      zap_term(t);
      t = get_variable_term(2);
      t->private_flags = 4;
    }
    else if (is_term(t, "u", 0)){
      zap_term(t);
      t = get_variable_term(3);
      t->private_flags = 8;
    }
    return t;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      ARG(t,i) = variablize(ARG(t,i));
      t->private_flags = t->private_flags | ARG(t,i)->private_flags;
    }
    return t;
  }
}  /* variablize */

int main(int argc, char **argv)
{
  FILE *head_fp;
  Clist demodulators;
  Clist_pos cp;
  Mindex idx;
  Term t;
  int max, generated, ba;
  Plist lat_terms, top_terms, p, q;
  Interp interp;

  declare_standard_parse_types();
  translate_neg_equalities(TRUE);
  init_options();
  init_flag(VERBOSE, "verbose", FALSE);

  max = atoi(argv[2]);

  head_fp = fopen(argv[1], "r");
  if (head_fp == NULL)
    fatal_error("demodulator file can't be opened for reading");

  /* for building terms */

  Meet_sym = str_to_sn("m", 2);
  Join_sym = str_to_sn("j", 2);

  /* for writing terms */

  fast_set_defaults();

  t = read_commands(head_fp, stderr, FALSE, KILL_UNKNOWN);

  if (!is_term(t, "clauses", 1))
    fatal_error("clauses(demodulators) not found");

  /* Read list of demodulators. */

  demodulators = read_clause_list(head_fp, stderr, "demodulators", TRUE);

  /* Read interpretation. */

  t = read_term(head_fp, stderr);  /* get first interpretation */
  interp = compile_interp(t);
  zap_term(t);

  fclose(head_fp);

  /* AC-canonicalize and index the demodulators. */

  idx = mindex_init(DISCRIM, BACKTRACK_UNIF, 0);
  for (cp = demodulators->first; cp != NULL; cp = cp->next) {
    /* assume positive equality unit */
    Literal lit = cp->c->literals;
    Term alpha = lit->atom->args[0];
    mark_oriented_eq(lit->atom);  /* don not check for termination */
    if (assoc_comm_symbols())
      ac_canonical(lit->atom, -1);
    mindex_update(idx, alpha, INSERT);
  }

  lat_terms = gen_terms(idx, max);
#if 0
  fwrite_term_list(stdout, lat_terms, "lattice");
#endif

  top_terms = NULL;
  for (q = lat_terms; q; q = q->next) {
    Term t = q->v;
    t = variablize(t);
    /* printf("%d:  ", t->private_flags); p_term(t); */
    if (t->private_flags == 7)
      top_terms = plist_prepend(top_terms, q->v);
  }

  /* printf("lat_terms=%d, top_terms=%d.\n", plist_count(lat_terms), plist_count(top_terms)); */

  generated = ba = 0;

  {
    Term eq = get_rigid_term("=", 2);
    Literal lit = new_literal(TRUE, eq);
    Clause c = get_clause();
    append_literal(c, lit);
    for (p = top_terms; p; p = p->next) {
      /* top of left side must be join */
      Term t = p->v;
      if (SYMNUM(t) == Join_sym) {
	for (q = p->next; q; q = q->next) {
	  ARG(eq,0) = p->v;
	  ARG(eq,1) = q->v;
	  generated++;
	  if (eval_clause(c, interp)) {
	    fast_fwrite_term_nl(stdout,eq);
	    ba++;
	  }
	}
      }
    }
  }

  fprintf(stdout,
	  "%% latgen %s %s, gen=%d, output=%d, %.2f seconds.\n",
	  argv[1], argv[2], generated, ba,
	  user_time() / 1000.0);
}  /* main */
