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

#include "memory.h"

#define MALLOC_MEGS        20  /* size of blocks malloced by palloc */
#define DEFAULT_MAX_MEGS  200  /* change with set_max_megs(n) */
#define MAX_MEM_LISTS     500  /* number of lists of available nodes */

static void ** M[MAX_MEM_LISTS];

static BOOL Max_megs_check = TRUE;
static int Max_megs = DEFAULT_MAX_MEGS;  /* change with set_max_megs(n) */
static void (*Exit_proc) (void);         /* set with set_max_megs_proc() */

static int Malloc_calls = 0;   /* number of calls to malloc by palloc */

static unsigned Bytes_palloced = 0;

static void *Block = NULL;        /* location returned by most recent malloc */
static void *Block_pos = NULL;    /* current position in block */

static unsigned Mem_calls = 0;
static unsigned Mem_calls_overflows = 0;

#define BUMP_MEM_CALLS {Mem_calls++; if (Mem_calls==0) Mem_calls_overflows++;}

/*************
 *
 *    void *palloc(n) -- assume n is a multiple of BYTES_POINTER.
 *
 *************/

static
void *palloc(size_t n)
{
  if (n == 0)
    return NULL;
  else {
    void *chunk;
    size_t malloc_bytes = MALLOC_MEGS*1024*1024;

    if (Block==NULL || Block + malloc_bytes - Block_pos < n) {
      /* First call or not enough in the current block, so get a new block. */
      if (n > malloc_bytes) {
	printf("palloc, n=%d\n", (int) n);
	fatal_error("palloc, request too big; reset MALLOC_MEGS");
      }
      else if (Max_megs_check && (Malloc_calls+1)*MALLOC_MEGS > Max_megs) {
	if (Exit_proc)
	  (*Exit_proc)();
	else
	  fatal_error("palloc, Max_megs parameter exceeded");
      }
      else {
	Block_pos = Block = malloc(malloc_bytes);
	Malloc_calls++;
	if (Block_pos == NULL)
	  fatal_error("palloc, operating system is out of memory");
      } 
    }
    chunk = Block_pos; 
    Block_pos += n; 
    Bytes_palloced += n; 
    return(chunk);
  }
}  /* palloc */ 

/*************
 *
 *   get_cmem()
 *
 *************/

/* DOCUMENTATION
Get a chunk of memory that will hold n pointers (NOT n BYTES).
The memory is initialized to all 0.
*/

/* PUBLIC */
void *get_cmem(unsigned n)
{
  if (n == 0)
    return NULL;
  else {
    void **p = NULL;
    BUMP_MEM_CALLS;
    if (n >= MAX_MEM_LISTS)
      return calloc(n, BYTES_POINTER);
    else if (M[n] == NULL)
      p = palloc(n * BYTES_POINTER);
    else {
      /* the first pointer is used for the avail list */
      p = M[n];
      M[n] = *p;
    }
    {
      int i;
      for (i = 0; i < n; i++)
	p[i] = 0;
    }
    return p;
  }
}  /* get_cmem */

/*************
 *
 *   get_mem()
 *
 *************/

/* DOCUMENTATION
Get a chunk of memory that will hold n pointers (NOT n BYTES).
The memory is NOT initialized.
*/


/* PUBLIC */
void *get_mem(unsigned n)
{
  if (n == 0)
    return NULL;
  else {
    void **p = NULL;
    BUMP_MEM_CALLS;
    if (n >= MAX_MEM_LISTS)
      p = malloc(n * BYTES_POINTER);
    else if (M[n] == NULL)
      p = palloc(n * BYTES_POINTER);
    else {
      /* the first pointer is used for the avail list */
      p = M[n];
      M[n] = *p;
    }
    return p;
  }
}  /* get_mem */

/*************
 *
 *   free_mem()
 *
 *************/

/* DOCUMENTATION
Free a chunk of memory that holds n pointers (not n bytes)
that was returned from a previous get_mem() or get_cmem() call.
*/

/* PUBLIC */
void free_mem(void *q, unsigned n)
{
  if (n == 0)
    ;  /* do nothing */
  else {
    /* put it on the appropriate avail list */
    void **p = q;
    if (n >= MAX_MEM_LISTS)
      free(p);
    else {
      /* the first pointer is used for the avail list */
      *p = M[n];
      M[n] = p;
    }
  } 
}  /* free_mem */

/*************
 *
 *   mlist_length()
 *
 *************/

static
int mlist_length(void **p)
{
  int n;
  for (n = 0; p; p = *p, n++);
  return n;
}  /* mlist_length */

/*************
 *
 *   memory_report()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void memory_report(FILE *fp)
{
  int i;
  fprintf(fp, "\nMemory report, %d @ %d = %d megs (%.2f megs used).\n",
	  Malloc_calls, MALLOC_MEGS, Malloc_calls * MALLOC_MEGS,
	  Bytes_palloced / (1024 * 1024.0));
  for (i = 0; i < MAX_MEM_LISTS; i++) {
    int n = mlist_length(M[i]);
    if (n != 0)
      fprintf(fp, "List %3d, length %7d, %8.1f K\n", i, n,
	      i * n * BYTES_POINTER / 1024.);
  }
}  /* memory_report */

/*************
 *
 *    int megs_malloced() -- How many MB have been dynamically allocated?
 *
 *************/

/* DOCUMENTATION
This routine returns the number of megabytes that palloc()
has obtained from the operating system by malloc();
*/

/* PUBLIC */
int megs_malloced(void)
{
  return Malloc_calls * MALLOC_MEGS;
}  /* megs_malloced */

/*************
 *
 *   set_max_megs()
 *
 *************/

/* DOCUMENTATION
This routine changes the limit on the amount of memory obtained
from malloc() by palloc().  The argument is in megabytes.
The default value is DEFAULT_MAX_MEGS.
*/

/* PUBLIC */
void set_max_megs(int megs)
{
  Max_megs = (megs == -1 ? INT_MAX : megs);
}  /* set_max_megs */

/*************
 *
 *   set_max_megs_proc()
 *
 *************/

/* DOCUMENTATION
This routine is used to specify the routine that will be called
if max_megs is exceeded.
*/

/* PUBLIC */
void set_max_megs_proc(void (*proc)(void))
{
  Exit_proc = proc;
}  /* set_max_megs_proc */

/*************
 *
 *   bytes_palloced()
 *
 *************/

/* DOCUMENTATION
How many bytes have been allocated by the palloc() routine?
This includes all of the get_mem() calls.
*/

/* PUBLIC */
int bytes_palloced(void)
{
  return Bytes_palloced;
}  /* bytes_palloced */

/*************
 *
 *   tp_alloc()
 *
 *************/

/* DOCUMENTATION
Allocate n bytes of memory, aligned on a pointer boundary.
The memory is not initialized, and it cannot be freed.
*/

/* PUBLIC */
void *tp_alloc(size_t n)
{
  /* If n is not a multiple of BYTES_POINTER, round up so that it is. */
  if (n % BYTES_POINTER != 0) {
    n += (BYTES_POINTER - (n % BYTES_POINTER));
  }
  return palloc(n);
}  /* tp_alloc */

/*************
 *
 *   mega_mem_calls()
 *
 *************/

/* DOCUMENTATION
 */

/* PUBLIC */
unsigned mega_mem_calls(void)
{
  return
    (Mem_calls / 1000000) +
    ((UINT_MAX / 1000000) * Mem_calls_overflows);
}  /* mega_mem_calls */

/*************
 *
 *   disable_max_megs()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void disable_max_megs(void)
{
  Max_megs_check = FALSE;
}  /* disable_max_megs */

/*************
 *
 *   enable_max_megs()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void enable_max_megs(void)
{
  Max_megs_check = TRUE;
}  /* enable_max_megs */
