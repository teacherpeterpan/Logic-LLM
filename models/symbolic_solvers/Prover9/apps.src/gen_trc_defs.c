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

#include <signal.h>
#include "../ladr/top_input.h"
#include "../ladr/flatdemod.h"

#define PROGRAM_NAME    "genterms"
#include "../VERSION_DATE.h"

// static char Help_string[] = "";

static Mindex Demod_index;  /* demodulator index */

static unsigned Generated = 0;
static unsigned Kept      = 0;

#define N0 4   /* 4 constants */
#define N1 1   /* 1 unary op */
#define N2 3   /* 3 binary ops */

struct syms {
  int s0[N0], c0[N0];
  int s1[N1], c1[N1];
  int s2[N2], c2[N2];
};

/*************
 *
 *   catalan()
 *
 *************/

static
Plist catalan(int n)
{
  if (n == 1) {
    Term t = get_rigid_term("_", 0);
    return plist_append(NULL, t);
  }
  else {
    Plist results = NULL;  /* collect results */
    int i, j;

    for (i = 1, j = n-1; i < n; i++, j--) {
      Plist l, r;
      Plist left  = catalan(i);
      Plist right = catalan(j);
      for (l = left; l; l = l->next) {
	for (r = right; r; r = r->next) {
	  Term c = build_binary_term_safe("o", copy_term(l->v), copy_term(r->v));
	  results = plist_prepend(results, c);
	}
      }
      zap_plist_of_terms(left);
      zap_plist_of_terms(right);
    }
    return reverse_plist(results);
  }
}  /* catalan */

/*************
 *
 *   rewritable_top()
 *
 *************/

static
BOOL rewritable_top(Flatterm f)
{
  if (ARITY(f) == 0)
    return FALSE;
  else {
    Context subst = get_context();
    Discrim_pos dpos;
    Term t = discrim_flat_retrieve_first(f, Demod_index->discrim_tree, subst, &dpos);
    if (t)
      discrim_flat_cancel(dpos);
    free_context(subst);
    return t != NULL;
  }
}  /* rewritable_top */

/*************
 *
 *   rewritable()
 *
 *************/

static
BOOL rewritable(Flatterm head)
{
  Flatterm f;
  for (f = head; f != head->end->next; f = f->next) {
    if (rewritable_top(f))
      return TRUE;
  }
  return FALSE;
}  /* rewritable */

/*************
 *
 *   candidate()
 *
 *************/

static
void candidate(Flatterm f)
{
  Generated++;
  if (!rewritable(f)) {
    Kept++;
    print_flatterm(f);
    printf(" = C.\n");
    fflush(stdout);
  }
}  /* candidate */

/*************
 *
 *   genterms()
 *
 *************/

static
void genterms(Flatterm f, Flatterm head, struct syms s)
{
  if (ARITY(f) == 0) {
    int i;
    for (i = 0; i < N0; i++) {
      if (s.c0[i] > 0) {
	s.c0[i]--;
	f->private_symbol = -s.s0[i];
	if (f->next == NULL)
	  candidate(head);
	else
	  genterms(f->next, head, s);
	s.c0[i]++;
      }
    }
  }
  else if (ARITY(f) == 1) {
    int i;
    for (i = 0; i < N1; i++) {
      if (s.c1[i] > 0) {
	s.c1[i]--;
	f->private_symbol = -s.s1[i];
	genterms(f->next, head, s);
	s.c1[i]++;
      }
    }
  }
  else if (ARITY(f) == 2) {
    int i;
    for (i = 0; i < N2; i++) {
      if (s.c2[i] > 0) {
	s.c2[i]--;
	f->private_symbol = -s.s2[i];
	genterms(f->next, head, s);
	s.c2[i]++;
      }
    }
  }
  else
    fatal_error("genterms, bad arity");
}  /* genterms */

/*************
 *
 *   num_constants()
 *
 *************/

static
int num_constants(struct syms s)
{
  int n = 0;
  int i;
  for (i = 0; i < N0; i++)
    n += s.c0[i];
  return n;
}  /* num_constants */

/*************
 *
 *   num_binaries()
 *
 *************/

static
int num_binaries(struct syms s)
{
  int n = 0;
  int i;
  for (i = 0; i < N2; i++)
    n += s.c2[i];
  return n;
}  /* num_constants */

/*************
 *
 *   check_counts()
 *
 *************/

static
void check_counts(struct syms s)
{
  if (num_constants(s) != num_binaries(s) + 1)
    fatal_error("check_counts, number of constants and binaries do not match");
}  /* check_counts */

/*************
 *
 *   unary_occurrences()
 *
 *************/

static
int unary_occurrences(struct syms s)
{
  int i;
  int n = 0;
  for (i = 0; i < N1; i++)
    n += s.c1[i];
  return n;
}  /* unary_occurrences */

/*************
 *
 *   insert_unaries()
 *
 *************/

static
void insert_unaries(Flatterm f, int n, struct syms s)
{
  if (n == 0) {
    Flatterm x = f;
    while (x->prev)
      x = x->prev;
    /* printf("genterms: "); p_flatterm(x); */
    genterms(x, x, s);
  }
  else {
    Flatterm u = get_flatterm();
    u->arity = 1;
    u->private_symbol = -str_to_sn("unary", 1);
    u->end = f->end; u->next = f; u->prev = f->prev;
    if (f->prev)
      f->prev->next = u;
    f->prev = u;
    insert_unaries(f, n-1, s);
    if (u->prev)
      u->prev->next = f;
    f->prev = u->prev;
    free_flatterm(u);
    if (f->next)
      insert_unaries(f->next, n, s);
  }
}  /* insert_unaries */

/*************
 *
 *   unary_gen()
 *
 *************/

static
void unary_gen(Term t, struct syms s)
{
  Flatterm f = term_to_flatterm(t);
  insert_unaries(f, unary_occurrences(s), s);
  zap_flatterm(f);
}  /* unary_gen */

/*************
 *
 *   lookfor()
 *
 *************/

static
int lookfor(char *s, int argc, char **argv)
{
  int n;
  int i = which_string_member(s, argv, argc);
  if (i == -1)
    return 0;
  else if (argc > i+1 && str_to_int(argv[i+1], &n))
    return n;
  else {
    fatal_error("lookfor: bad arg list");
    return 0;
  }
}  /* lookfor */

int main(int argc, char **argv)
{
  struct syms s;
  int n, i;
  Plist demods;

  init_standard_ladr();
  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */

  n = lookfor("-A",  argc, argv);  s.s0[0] = str_to_sn("A",  0);  s.c0[0] = n;
  n = lookfor("-E",  argc, argv);  s.s0[1] = str_to_sn("E",  0);  s.c0[1] = n;
  n = lookfor("-P1", argc, argv);  s.s0[2] = str_to_sn("P1", 0);  s.c0[2] = n;
  n = lookfor("-P2", argc, argv);  s.s0[3] = str_to_sn("P2", 0);  s.c0[3] = n;

  n = lookfor("-K",  argc, argv);  s.s1[0] = str_to_sn("K",  1);  s.c1[0] = n;

  n = lookfor("-a",  argc, argv);  s.s2[0] = str_to_sn("a",  2);  s.c2[0] = n;
  n = lookfor("-p",  argc, argv);  s.s2[1] = str_to_sn("p",  2);  s.c2[1] = n;
  n = lookfor("-f",  argc, argv);  s.s2[2] = str_to_sn("f",  2);  s.c2[2] = n;

  check_counts(s);

  Demod_index = mindex_init(DISCRIM_BIND, ORDINARY_UNIF, 0);

  i = which_string_member("-demod", argv, argc);

  if (i == -1)
    demods = NULL;
  else {
    FILE *head_fp = fopen(argv[i+1], "r");
    if (head_fp == NULL)
      fatal_error("demod file cannot be opened for reading");
    demods = read_clause_list(head_fp, stderr, TRUE);
    fclose(head_fp);

    Plist p;
    for (p = demods; p != NULL; p = p->next) {
      /* assume positive equality unit */
      Topform d = p->v;
      Literals lit = d->literals;
      Term alpha = lit->atom->args[0];
      mark_oriented_eq(lit->atom);     /* do not check for termination */
      mindex_update(Demod_index, alpha, INSERT);
    }
  }
  
  Plist forms = catalan(num_constants(s));

  n = 0;
  Plist p;
  for (p = forms; p; p = p->next) {
    n++;
    Term t = p->v;
    unary_gen(t, s);
  }
  printf("%% Generated=%u, Kept=%u.\n", Generated, Kept);
  exit(0);
}  /* main */

