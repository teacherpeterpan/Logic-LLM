#include "fpa.h"

/* Private definitions and types */

static unsigned FPA_ID_COUNT = 0;

typedef struct fpa_trie * Fpa_trie;

struct fpa_trie {
  Fpa_trie   parent, next, kids;
  int        label;
  Fpa_chunk  terms;
#ifdef FPA_DEBUG
  Ilist      path;
#endif
};

struct fpa_index {
  Fpa_trie   root;
  int        depth;
  Fpa_index  next;
};

struct fpa_state {
  int              type;
  Fpa_state        left, right;
  Term             left_term, right_term;
  struct fposition fpos;
#ifdef FPA_DEBUG
  Ilist            path;
#endif
};

/* A path is a sequence of integers.  We have to do some operations
   to the end of a path, and an Ilist is singly-linked; but we can
   get away with just keeping a pointer to the end of the list.
 */

struct path {
  Ilist first;
  Ilist last;
};

enum { LEAF, UNION, INTERSECT };  /* types of fpa_state (node in FPA tree) */

/* for a mutual recursion */

static Fpa_state build_query(Term t, Context c, Querytype type,
			     struct path *p, int bound, Fpa_trie index);
			     
/*
 * memory management
 */

static unsigned Fpa_trie_gets, Fpa_trie_frees;
static unsigned Fpa_state_gets, Fpa_state_frees;
static unsigned Fpa_index_gets, Fpa_index_frees;

#define BYTES_FPA_TRIE sizeof(struct fpa_trie)
#define PTRS_FPA_TRIE BYTES_FPA_TRIE%BPP == 0 ? BYTES_FPA_TRIE/BPP : BYTES_FPA_TRIE/BPP + 1

#define BYTES_FPA_STATE sizeof(struct fpa_state)
#define PTRS_FPA_STATE BYTES_FPA_STATE%BPP == 0 ? BYTES_FPA_STATE/BPP : BYTES_FPA_STATE/BPP + 1

#define BYTES_FPA_INDEX sizeof(struct fpa_index)
#define PTRS_FPA_INDEX BYTES_FPA_INDEX%BPP == 0 ? BYTES_FPA_INDEX/BPP : BYTES_FPA_INDEX/BPP + 1

/*************
 *
 *   Fpa_trie get_fpa_trie()
 *
 *************/

static
Fpa_trie get_fpa_trie(void)
{
  Fpa_trie p = get_mem(PTRS_FPA_TRIE);
  p->label = -1;
  Fpa_trie_gets++;
  return(p);
}  /* get_fpa_trie */

/*************
 *
 *    free_fpa_trie()
 *
 *************/

static
void free_fpa_trie(Fpa_trie p)
{
  free_mem(p, PTRS_FPA_TRIE);
  Fpa_trie_frees++;
}  /* free_fpa_trie */

/*************
 *
 *   Fpa_state get_fpa_state()
 *
 *************/

static
Fpa_state get_fpa_state(void)
{
  Fpa_state p = get_mem(PTRS_FPA_STATE);
  Fpa_state_gets++;
  return(p);
}  /* get_fpa_state */

/*************
 *
 *    free_fpa_state()
 *
 *************/

static
void free_fpa_state(Fpa_state p)
{
  free_mem(p, PTRS_FPA_STATE);
  Fpa_state_frees++;
}  /* free_fpa_state */

/*************
 *
 *   Fpa_index get_fpa_index()
 *
 *************/

static
Fpa_index get_fpa_index(void)
{
  Fpa_index p = get_mem(PTRS_FPA_INDEX);
  p->depth = -1;
  Fpa_index_gets++;
  return(p);
}  /* get_fpa_index */

/*************
 *
 *    free_fpa_index()
 *
 *************/

static
void free_fpa_index(Fpa_index p)
{
  free_mem(p, PTRS_FPA_INDEX);
  Fpa_index_frees++;
}  /* free_fpa_index */

/*************
 *
 *   fprint_fpa_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the fpa package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_fpa_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_FPA_TRIE;
  fprintf(fp, "fpa_trie (%4d)     %11u%11u%11u%9.1f K\n",
          n, Fpa_trie_gets, Fpa_trie_frees,
          Fpa_trie_gets - Fpa_trie_frees,
          ((Fpa_trie_gets - Fpa_trie_frees) * n) / 1024.);

  n = BYTES_FPA_STATE;
  fprintf(fp, "fpa_state (%4d)    %11u%11u%11u%9.1f K\n",
          n, Fpa_state_gets, Fpa_state_frees,
          Fpa_state_gets - Fpa_state_frees,
          ((Fpa_state_gets - Fpa_state_frees) * n) / 1024.);

  n = BYTES_FPA_INDEX;
  fprintf(fp, "fpa_index (%4d)    %11u%11u%11u%9.1f K\n",
          n, Fpa_index_gets, Fpa_index_frees,
          Fpa_index_gets - Fpa_index_frees,
          ((Fpa_index_gets - Fpa_index_frees) * n) / 1024.);

}  /* fprint_fpa_mem */

/*************
 *
 *   p_fpa_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the fpa package.
*/

/* PUBLIC */
void p_fpa_mem()
{
  fprint_fpa_mem(stdout, TRUE);
}  /* p_fpa_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   fprint_path()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) an FPA Path.  A newline is NOT printed.
*/

/* PUBLIC */
void fprint_path(FILE *fp, Ilist p)
{
  int i;
  fprintf(fp, "(");
  for (i = 0; p != NULL; p = p->next, i++) {
    if (i%2 == 0) {
      if (p->i == 0)
	fprintf(fp, "*");
      else
	fprint_sym(fp, p->i);
    }
    else
      fprintf(fp, "%d", p->i);

    if (p->next != NULL)
      fprintf(fp, " ");
  }
  fprintf(fp, ")");
}  /* fprint_path */

/*************
 *
 *   p_path()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) an FPA Path.  A newline is NOT printed.
*/

/* PUBLIC */
void p_path(Ilist p)
{
  fprint_path(stdout, p);
  fflush(stdout);
}  /* p_path */

/*************
 *
 *    fprint_fpa_trie -- Print an FPA trie to stdout.
 *
 *************/

static
void fprint_fpa_trie(FILE *fp, Fpa_trie p, int depth)
{
  int i;
  Fpa_trie q;
  for (i = 0; i < depth; i++)
    fprintf(fp, " - ");
  if (depth == 0)
    fprintf(fp, "root");
  else if (depth % 2 == 1) {
    if (p->label == 0)
      fprintf(fp, "*");
    else
      fprint_sym(fp, p->label);
  }
  else
    fprintf(fp, "%2d", p->label);

  if (p->terms != NULL) {
#if 0
    Plist g;
    fprintf(fp, " [");
    for (g = p->terms; g != NULL; g = g->next)
      fprintf(fp, "%u%s", (unsigned) FPA_ID(g->v),
	      g->next == NULL ? "" : ",");
    fprintf(fp, "]");
#endif
  }
#ifdef FPA_DEBUG
  if (p->path != NULL)
    fprint_path(fp, p->path);
#endif  
  fprintf(fp, "\n");
  for (q = p->kids; q != NULL; q = q->next)
    fprint_fpa_trie(fp, q, depth+1);
}  /* fprint_fpa_trie */

/*************
 *
 *   fprint_fpa_index()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) an Fpa_index.
WARNING: An Fpa_index can be very big, and it is printed as a tree,
so if you use this routine, I suggest trying it on a small index first.
*/

/* PUBLIC */
void fprint_fpa_index(FILE *fp, Fpa_index idx)
{
  fprintf(fp, "FPA/Path index, depth is %d.\n", idx->depth);
  fprint_fpa_trie(fp, idx->root, 0);
}  /* fprint_fpa_index */

/*************
 *
 *   p_fpa_index()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) an Fpa_index.
WARNING: An Fpa_index can be very big, and it is printed as a tree,
so if you use this routine, I suggest trying it on a small index first.
*/

/* PUBLIC */
void p_fpa_index(Fpa_index idx)
{
  fprint_fpa_index(stdout, idx);
}  /* p_fpa_index */

/*************
 *
 *    fpa_trie_member_insert (recursive) -- This routine takes a trie
 *    and a path, and looks for a node in the trie that corresponds
 *    to the path.  If such a node does not exist, it is created, and
 *    the trie is updated.
 *
 *************/

static
Fpa_trie fpa_trie_member_insert(Fpa_trie node, Ilist path)
{
  if (path == NULL)
    return node;
  else {
    /* Find child node that matches first member of path;
     * if it doesn't exist, create it. Children are in increasing order.
     */
    int val = path->i;
    Fpa_trie curr = node->kids;
    Fpa_trie prev = NULL;
    while (curr != NULL && curr->label < val) {
      prev = curr;
      curr = curr->next;
    }
    if (curr != NULL && curr->label == val)
      return fpa_trie_member_insert(curr, path->next);
    else {
      /* Get a new node and insert it before curr (which may be NULL). */
      Fpa_trie new = get_fpa_trie();
      new->parent = node;
      new->label = val;
      new->next = curr;
      if (prev == NULL)
	node->kids = new;
      else
	prev->next = new;
      return fpa_trie_member_insert(new, path->next);
    }
  }
}  /* fpa_trie_member_insert */

/*************
 *
 *    fpa_trie_member (recursive) -- This routine looks for a trie
 *    node that corresponds to a given path.
 *
 *************/

static
Fpa_trie fpa_trie_member(Fpa_trie node, Ilist path)
{
  if (path == NULL)
    return node;
  else {
    /* Find child node that matches first member of path;
     * Children are in increasing order.
     */
    int val = path->i;
    Fpa_trie curr = node->kids;
    while (curr != NULL && curr->label < val)
      curr = curr->next;
    if (curr != NULL && curr->label == val)
      return fpa_trie_member(curr, path->next);
    else
      return NULL;
  }
}  /* fpa_trie_member */

/*************
 *
 *    fpa_trie_possible_delete (recursive) -- This routine checks if
 *    a trie node should be deleted.  If so, it is deleted, and a
 *    recursive call is made on the parent node.  The trie node should
 *    be deleted if (1) it is not the root, (2) there is no FPA list,
 *    and (3) it has no children.
 *
 *************/

static
void fpa_trie_possible_delete(Fpa_trie node)
{
  if (node->parent != NULL && node->terms == NULL && node->kids == NULL) {
    if (node->parent->kids == node)
      node->parent->kids = node->next;
    else {
      Fpa_trie p = node->parent->kids;
      while (p->next != node)
	p = p->next;
      p->next = node->next;
    }
    fpa_trie_possible_delete(node->parent);
    free_fpa_trie(node);
  }
}  /* fpa_trie_delete */

/*************
 *
 *    path_insert -- Given (term,path,index), insert a pointer
 *    to the term into the path list of the index.
 *
 *************/

static
void path_insert(Term t, Ilist path, Fpa_trie index)
{
  Fpa_trie node = fpa_trie_member_insert(index, path);

#ifdef FPA_DEBUG
  if (node->path == NULL)
    node->path = copy_ilist(path);
#endif

  node->terms = flist_insert(node->terms, t);
}  /* path_insert */

/*************
 *
 *    path_delete -- Given (term,path,index), try to delete a pointer
 *    to the term from the path list of the index.
 *
 *************/

static
void path_delete(Term t, Ilist path, Fpa_trie index)
{
  Fpa_trie node = fpa_trie_member(index, path);

  if (node == NULL) {
    fatal_error("path_delete, trie node not found.");
  }

  node->terms = flist_delete(node->terms, t);

#ifdef FPA_DEBUG
  if (node->terms == NULL) {
    zap_ilist(node->path);
    node->path = NULL;
  }
#endif
  fpa_trie_possible_delete(node);
}  /* path_delete */

/*************
 *
 *    path_push -- append an integer to a path.   "save" is used because
 *    we have a pointer to the last member, but the list is singly linked.
 *
 *************/

static
Ilist path_push(struct path *p, int i)
{
  Ilist new = get_ilist();
  Ilist save = p->last;

  new->i = i;
  if (p->last == NULL)
    p->first = new;
  else
    p->last->next = new;
  p->last = new;
  return save;
}  /* path_push */

/*************
 *
 *    path_restore -- pop and throw away the last member of a path.
 *    We assume that "save" points to the next-to-last member.
 *
 *************/

static
void path_restore(struct path *p, Ilist save)
{
  free_ilist(p->last);
  p->last = save;
  if (save != NULL)
    save->next = NULL;
  else
    p->first = NULL;
}  /* path_restore */

/*************
 *
 *  fpa_paths (recursive) -- This routine traverses a term, keeping a
 *  path, and either inserts or deletes pointers to the term into/from the
 *  appropriate path lists of an FPA/PATH index.
 *
 *************/

static
void fpa_paths(Term root, Term t, struct path *p, int bound,
	       Indexop op, Fpa_trie index)
{
  Ilist save1;

  if (VARIABLE(t))
    save1 = path_push(p, 0);
  else
    save1 = path_push(p, SYMNUM(t));
  if (COMPLEX(t) && bound > 0 && !is_assoc_comm(SYMNUM(t))) {
    int i;
    Ilist save2 = path_push(p, 0);

    for (i = 0; i < t->arity; i++) {
      p->last->i = i+1;  /* Count arguments from 1. */
      fpa_paths(root, t->args[i], p, bound-1, op, index);
    }
    path_restore(p, save2);
  }
  else {
    /* printf("    ");  p_path(p->first); */
    
    if (op == INSERT)
      path_insert(root, p->first, index);
    else
      path_delete(root, p->first, index);
  }
  path_restore(p, save1);
}  /* fpa_paths */

/*************
 *
 *   fpa_init_index()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty FPA/Path index.
Parameter depth tells how deep to index the terms. For example,
depth=0 means to index on the root symbol only.
*/

/* PUBLIC */
Fpa_index fpa_init_index(int depth)
{
  Fpa_index f = get_fpa_index();
  f->depth = depth;
  f->root = get_fpa_trie();
  return f;
}  /* fpa_init_index */

/*************
 *
 *    fpa_update -- Insert/delete a term into/from a FPA-PATH index.
 *
 *************/

/* DOCUMENTATION
This routine inserts (op==INSERT) a term into an Fpa_index or
deletes (op==DELETE) a term from an Fpa_index.
<P>
IMPORTANT:  FPA indexing owns the FPA_ID field of the term.
<P>
A fatal error occurs if you try to delete a term that was not previously
inserted.
*/

/* PUBLIC */
void fpa_update(Term t, Fpa_index idx, Indexop op)
{
  struct path p;

  if (FPA_ID(t) == 0) {
    if (op == INSERT)
      FPA_ID(t) = ++FPA_ID_COUNT;
    else
      fatal_error("fpa_update: FPA_ID=0.");
  }

  p.first = p.last = NULL;
  fpa_paths(t, t, &p, idx->depth, op, idx->root);
}  /* fpa_update */

/*************
 *
 *  query_leaf_full - for testing only
 *
 *  Ordinarily, with query_leaf(), if an FPA list doesn't exist,
 *  the query will be simplified.  If you wish to get the whole
 *  query tree, with NULL leaves for nonexistant FPA lists, you
 *  can use this instead of query_leaf().  This is useful if you
 *  want to print the query tree.
 *
 *************/

#ifdef FPA_DEBUG
static
Fpa_state query_leaf_full(Ilist path, Fpa_trie index)
{
  Fpa_trie n = fpa_trie_member(index, path);
  Fpa_state q = get_fpa_state();
  q->type = LEAF;
  q->terms = (n == NULL ? NULL : n->terms);
  q->path = copy_ilist(path);
  return q;
}  /* query_leaf_full */
#endif

/*************
 *
 *    query_leaf
 *
 *************/

static
Fpa_state query_leaf(Ilist path, Fpa_trie index)
{
  Fpa_trie n;

  /* return query_leaf_full(path, index); */

  n = fpa_trie_member(index, path);
  if (n == NULL)
    return NULL;
  else {
    Fpa_state q = get_fpa_state();
    q->type = LEAF;
    q->fpos = first_fpos(n->terms);
#ifdef FPA_DEBUG
    q->path = copy_ilist(path);
#endif
    return q;
  }
}  /* query_leaf */

/*************
 *
 *    query_intersect
 *
 *************/

static
Fpa_state query_intersect(Fpa_state q1, Fpa_state q2)
{
  /* Assume neither is NULL. */
  Fpa_state q = get_fpa_state();
  q->type = INTERSECT;
  q->left = q1;
  q->right = q2;
  return q;
}  /* query_intersect */

/*************
 *
 *    query_union
 *
 *************/

static
Fpa_state query_union(Fpa_state q1, Fpa_state q2)
{
  if (q1 == NULL)
    return q2;
  else if (q2 == NULL)
    return q1;
  else {
    Fpa_state q = get_fpa_state();
    q->type = UNION;
    q->left = q1;
    q->right = q2;
    return q;
  }
}  /* query_union */

/*************
 *
 *    query_special (recursive)
 *
 *************/

static
Fpa_state query_special(Fpa_trie n)
{
  /* There are 2 kinds of nodes: argument position (1,2,3,...) and
   * symbol (a,b,f,g,h); the two types alternate in a path.  The
   * given node n is a symbol node.  What we wish to do is construct
   * the union of all leaves, excluding those that have an argument
   * position greater than 1.  This should contain all terms that
   * have a path corresponding to node n.
   */

  if (n->kids == NULL) {
    Fpa_state q = get_fpa_state();
    q->type = LEAF;
    q->fpos = first_fpos(n->terms);
#ifdef FPA_DEBUG
    q->path = copy_ilist(n->path);
#endif
    return q;
  }
  else {
    Fpa_state q1 = NULL;
    Fpa_trie pos_child;
    for (pos_child=n->kids; pos_child!=NULL; pos_child=pos_child->next) {
      if (pos_child->label == 1) {
	Fpa_trie sym_child;
	for (sym_child=pos_child->kids;
	     sym_child!=NULL;
	     sym_child=sym_child->next) {
	  Fpa_state q2 = query_special(sym_child);
	  q1 = query_union(q1, q2);
	}
      }
    }
    return q1;
  }
}  /* query_special */

/*************
 *
 *  zap_fpa_state (recursive)
 *
 *  This (recursive) routine frees an Fpa_state.
 *  It should NOT be called if you retrieve all answers to
 *  a query, because the query tree is freed as it is processsed
 *  by fpa_next_answer().  This routine should be called only if
 *  you decide not to get all of the answers.
 *
 *************/

static
void zap_fpa_state(Fpa_state q)
{
  if (q != NULL) {
    zap_fpa_state(q->left);
    zap_fpa_state(q->right);
#ifdef FPA_DEBUG
    zap_ilist(q->path);
#endif
    free_fpa_state(q);
  }
}  /* zap_fpa_state */

/*************
 *
 *   union_commuted()
 *
 *************/

static
Fpa_state union_commuted(Fpa_state q, Term t, Context c,
			 Querytype type,
			 struct path *p, int bound, Fpa_trie index)
{
  Fpa_state q1;
  int empty, i;
#if 0
  printf("enter union_commuted with\n");
  p_fpa_state(q);
#endif
  q1 = NULL;
  empty = 0;

  for (i = 0; i < 2 && !empty; i++) {
    p->last->i = (i == 0 ? 2 : 1);
    /* Skip this arg if VARIABLE && (UNIFY || INSTANCE). */
    if (!VARIABLE(t->args[i]) || type==GENERALIZATION ||
	type==VARIANT || type==IDENTICAL) {
      Fpa_state q2 = build_query(t->args[i], c, type, p, bound-1, index);
      if (q2 == NULL) {
	empty = 1;
	zap_fpa_state(q1);
	q1 = NULL;
      }
      else if (q1 == NULL)
	q1 = q2;
      else
	q1 = query_intersect(q1, q2);
    }
  }
  if (q1 != NULL)
    q1 = query_union(q, q1);
  else
    q1 = q;
#if 0
  printf("exit union_commuted with\n");
  p_fpa_state(q1);
#endif    
  return(q1);
}  /* union_commuted */

/*************
 *
 *   var_in_context()
 *
 *************/

static
BOOL var_in_context(Term t, Context c)
{
  DEREFERENCE(t, c);
  return VARIABLE(t);
}  /* var_in_context */

/*************
 *
 *   all_args_vars_in_context()
 *
 *************/

static
BOOL all_args_vars_in_context(Term t, Context c)
{
  /* Assume t is not a variable. */
  int i = 0;
  BOOL ok = TRUE;
  while (i < t->arity && ok) {
    ok = var_in_context(t->args[i], c);
    i++;
  }
  return ok;
}  /* all_args_vars_in_context */

/*************
 *
 *   build_query()
 *
 *************/

static
Fpa_state build_query(Term t, Context c, Querytype type,
		      struct path *p, int bound, Fpa_trie index)
{
  if (VARIABLE(t)) {
    int i = VARNUM(t);
    if (c != NULL && c->terms[i] != NULL)
      return build_query(c->terms[i], c->contexts[i], type, p, bound, index);
    else if (type == UNIFY || type == INSTANCE) {
      fatal_error("build_query, variable.");
      return NULL;  /* to quiet compiler */
    }
    else {
      Ilist save = path_push(p, 0);
      Fpa_state q = query_leaf(p->first, index);
      path_restore(p, save);
      return q;
    }
  }
  else {  /* non-variable */
    Fpa_state q1 = NULL;
    Ilist save1 = path_push(p, SYMNUM(t));

    if (CONSTANT(t) || bound <= 0 || is_assoc_comm(SYMNUM(t))) {
      q1 = query_leaf(p->first, index);
    }
    else if ((type == INSTANCE || type == UNIFY) &&
	     all_args_vars_in_context(t, c)) {
      Fpa_trie n = fpa_trie_member(index, p->first);
      q1 = (n == NULL ? NULL : query_special(n));
    }
    else {
      Ilist save2 = path_push(p, 0);
      int empty = 0;
      int i;
      for (i = 0; i < t->arity && !empty; i++) {
	p->last->i = i+1;
	/* Skip this arg if VARIABLE && (UNIFY || INSTANCE). */
	if (!var_in_context(t->args[i],c) || type==GENERALIZATION ||
	    type==VARIANT || type==IDENTICAL) {
	  Fpa_state q2 = build_query(t->args[i], c, type, p, bound-1, index);
					      
	  if (q2 == NULL) {
	    empty = 1;
	    zap_fpa_state(q1);
	    q1 = NULL;
	  }
	  else if (q1 == NULL)
	    q1 = q2;
	  else
	    q1 = query_intersect(q1, q2);
	}
      }
      if (is_commutative(SYMNUM(t)) && !term_ident(t->args[0], t->args[1]))
	q1 = union_commuted(q1, t, c, type, p, bound, index);
      path_restore(p, save2);
    }
    if (type == UNIFY || type == GENERALIZATION) {
      Fpa_state q2;
      p->last->i = 0;
      q2 = query_leaf(p->first, index);
      q1 = query_union(q1, q2);
    }
    path_restore(p, save1);
    return q1;
  }
}  /* build_query */

/*************
 *
 *    fprint_fpa_state (recursive)
 *
 *************/

/* DOCUMENTATION
This (recursive) routine prints (to FILE *fp) an Fpa_state tree.
The depth parameter should be 0 on the top call.
This is an AND/OR tree, with lists of terms (ordered by FPA_ID)
at the leaves.  If FPA_DEBUG is not defined in fpa.h, the
paths corresponding to the leaves are not printed, and the
tree is hard to understand without the paths.
*/

/* PUBLIC */
void fprint_fpa_state(FILE *fp, Fpa_state q, int depth)
{
  int i;
  for (i = 0; i < depth; i++)
    fprintf(fp, "- - ");

  switch (q->type) {
  case UNION: fprintf(fp, "OR\n"); break;
  case INTERSECT: fprintf(fp, "AND\n"); break;
  case LEAF:
#ifdef FPA_DEBUG
    fprint_path(fp, q->path);
    fprintf(fp, " ");
#endif
    {
#if 0
      Plist p;
      fprintf(fp, "[");
      for (p = q->terms; p != NULL; p = p->next)
	fprintf(fp, "%u%s", (unsigned) FPA_ID(p->v),
		p->next == NULL ? "" : ",");
      fprintf(fp, "]\n");
#endif
    }
    break;
  }
  fflush(fp);
  if (q->type == UNION || q->type == INTERSECT) {
    fprint_fpa_state(fp, q->right, depth+1);
    fprint_fpa_state(fp, q->left, depth+1);
  }
}  /* fprint_fpa_state */

/*************
 *
 *    p_fpa_state
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) an Fpa_state tree.
See the description of fprint_fpa_state().
*/

/* PUBLIC */
void p_fpa_state(Fpa_state q)
{
  fprint_fpa_state(stdout, q, 0);
}  /* fprint_fpa_state */

/*************
 *
 *    p_fpa_query
 *
 *************/

/* DOCUMENTATION
This routine constructs an fpa_query tree and prints it to stdout.
*/

/* PUBLIC */
void p_fpa_query(Term t, Querytype query_type, Fpa_index idx)
{
  Fpa_state q;
  char *s;
  struct path p;
  p.first = p.last = NULL;

  switch (query_type) {
  case UNIFY:          s = "UNIFY         "; break;
  case INSTANCE:       s = "INSTANCE      "; break;
  case GENERALIZATION: s = "GENERALIZATION"; break;
  case VARIANT:        s = "VARIANT       "; break;
  case IDENTICAL:      s = "IDENTICAL     "; break;
  default:                 s = "FPA_??            "; break;
  }
  printf("\n%s with term %u: ", s, (unsigned) FPA_ID(t)); p_term(t);
  fflush(stdout);

  q = build_query(t, NULL, query_type, &p, idx->depth, idx->root);
  p_fpa_state(q);
  zap_fpa_state(q);
  
}  /* fprint_fpa_query */

/*************
 *
 *    next_term()
 *
 *    Get the first or next term that satisfies a unification condition.
 *    (Unification conditions are provided by build_query.)
 *    `max' should be FPA_ID_MAX on top calls.  A return of NULL indicates
 *    that there are none or no more terms that satisfy (and the tree has
 *    been deallocated).  If you want to stop getting terms before a NULL
 *    is returned, then please deallocate the tree with zap_fpa_state(tree).
 *
 *    Warning: a return of NULL means that the tree has been deallocated.
 *
 *************/

static
Term next_term(Fpa_state q, FPA_ID_TYPE max)
{
  if (q == NULL)
    return NULL;
  else if (q->type == LEAF) {
    Term t = FTERM(q->fpos);
    while (t != NULL && FPA_ID(t) > max) {
      q->fpos = next_fpos(q->fpos);
      t = FTERM(q->fpos);
    }
    if (t == NULL) {
      zap_fpa_state(q);
      return NULL;
    }
    else {
      q->fpos = next_fpos(q->fpos);
      return t;
    }
  }
    
  else if (q->type == INTERSECT) {
    Term t1, t2;
    t1 = next_term(q->left, max);
    if (t1 != NULL)
      t2 = next_term(q->right, FPA_ID(t1));
    else
      t2 = (Term) &t2;  /* anything but NULL */

    while (t1 != t2 && t1 != NULL && t2 != NULL) {
      if (FGT(t1,t2))
	t1 = next_term(q->left, FPA_ID(t2));
      else
	t2 = next_term(q->right, FPA_ID(t1));
    }
    if (t1 == NULL || t2 == NULL) {
      if (t1 == NULL)
	q->left = NULL;
      if (t2 == NULL)
	q->right = NULL;
      zap_fpa_state(q);
      return NULL; 
    }
    else
      return t1;
  }
    
  else {  /* UNION node */
    Term t1, t2;
    /* first get the left term */
    t1 = q->left_term;
    if (t1 == NULL) {
      /* it must be brought up */
      if (q->left) {
	t1 = next_term(q->left, max);
	if (t1 == NULL)
	  q->left = NULL;
      }
    }
    else  /* it was saved from a previous call */
      q->left_term = NULL;
    
    /* now do the same for the right side */
    t2 = q->right_term;
    if (t2 == NULL) {
      if (q->right) {
	t2 = next_term(q->right, max);
	if (t2 == NULL)
	  q->right = NULL;
      }
    }
    else
      q->right_term = NULL;
    
    /* At this point, both left_term and right_term are NULL.
     * Now decide which of t1 and t2 to return.  If both are
     * non-NULL (and different), save the smaller for the next
     * call, and return the larger.
     */
    if (t1 == NULL) {
      if (t2 == NULL) {
	zap_fpa_state(q);
	return NULL;
      }
      else
	return t2;
    }
    else if (t2 == NULL)
      return t1;
    else if (t1 == t2)
      return t1;
    else if (FGT(t1,t2)) {
      q->right_term = t2;  /* save t2 for next time */
      return t1;
    }
    else {
      q->left_term = t1;  /* save t1 for next time */
      return t2;
    }
  }
}  /* next_term */

/*************
 *
 *    fpa_next_answer()
 *
 *************/

/* DOCUMENTATION
This routine extracts and returns the next answer term
from an Fpa_state tree.  If there
are no more answers, NULL is returned, and the tree is freed.
If you wish to stop getting answers before NULL is returned,
call zap_fpa_state(q) to free the Fpa_state tree.
*/

/* PUBLIC */
Term fpa_next_answer(Fpa_state q)
{
  return next_term(q, FPA_ID_MAX);
}  /* fpa_next_answer */

/*************
 *
 *   fpa_first_answer()
 *
 *************/

/* DOCUMENTATION
This routine extracts and returns the first answer term
from an Fpa_state tree.  If there
are no more answers, NULL is returned, and the tree is freed.
If you wish to stop getting answers before NULL is returned,
call zap_fpa_state(q) to free the Fpa_state tree.
<P>
The query types are
UNIFY, INSTANCE, GENERALIZATION, VARIANT, and IDENTICAL.
<P>
If Context c is not NULL, then the instance of the term (in the
context) is used for the query.
*/

/* PUBLIC */
Term fpa_first_answer(Term t, Context c, Querytype query_type,
		      Fpa_index idx, Fpa_state *ppos)
{
  struct path p;
  p.first = p.last = NULL;

  *ppos = build_query(t, c, query_type, &p, idx->depth, idx->root);
  
  return fpa_next_answer(*ppos);
}  /* fpa_first_answer */

/*************
 *
 *    fpa_cancel
 *
 *************/

/* DOCUMENTATION
This routine should be called if you get some, but not all answers
to an fpa query.  See fpa_first_answer() and fpa_next_answer().
*/

/* PUBLIC */
void fpa_cancel(Fpa_state q)
{
  zap_fpa_state(q);
}  /* fpa_cancel */

/*************
 *
 *   zap_fpa_trie()
 *
 *************/

static
void zap_fpa_trie(Fpa_trie n)
{
  Fpa_trie k, prev;

  k = n->kids;
  while (k != NULL) {
    prev = k;
    k = k->next;
    zap_fpa_trie(prev);
  }

  if (n->terms != NULL)
    zap_fpa_chunks(n->terms);

#ifdef FPA_DEBUG
  zap_ilist(n->path);
#endif

  free_fpa_trie(n);
}  /* zap_fpa_trie */

/*************
 *
 *   zap_fpa_index()
 *
 *************/

/* DOCUMENTATION
This routine removes all the entries from an Fpa_index idx and
frees all of the associated memory.
*/

/* PUBLIC */
void zap_fpa_index(Fpa_index idx)
{
  zap_fpa_trie(idx->root);
  free_fpa_index(idx);
}  /* zap_fpa_index */

/*************
 *
 *   fpa_empty()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if an FPA/Path index is empty.
*/

/* PUBLIC */
BOOL fpa_empty(Fpa_index idx)
{
  return (idx == NULL ? TRUE : idx->root->kids == NULL);
}  /* fpa_empty */
