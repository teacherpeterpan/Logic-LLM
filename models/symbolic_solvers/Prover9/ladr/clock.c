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

#include "clock.h"

/* Private definitions and types */

struct clock {
  char       *name;                 /* name of clock */
  unsigned   accum_msec;            /* accumulated time */
  unsigned   curr_msec;             /* time since clock has been turned on */
  int        level;                 /* STARTs - STOPs */
};

static BOOL Clocks_enabled = TRUE;   /* getrusage can be slow */
static unsigned Clock_starts = 0;    /* Keep a count */

static unsigned Wall_start;          /* for measuring wall-clock time */

/* Macro to get the number of user CPU milliseconds since the start of
   the process.  Getrusage() can be VERY slow on some systems.  So I tried
   clock() instead (ignoring the rollover problem), but that didn't seem
   to help on Linux.
*/

#if 1
#define CPU_TIME(msec) { struct rusage r; getrusage(RUSAGE_SELF, &r);\
       msec = r.ru_utime.tv_sec * 1000 + r.ru_utime.tv_usec / 1000; }
#else
#define CPU_TIME(msec) { msec = clock() / 1000; }
#endif

/*
 * memory management
 */

#define PTRS_CLOCK PTRS(sizeof(struct clock))
static unsigned Clock_gets, Clock_frees;

/*************
 *
 *   Clock get_clock()
 *
 *************/

static
Clock get_clock(void)
{
  Clock p = get_cmem(PTRS_CLOCK);
  Clock_gets++;
  return(p);
}  /* get_clock */

/*************
 *
 *    free_clock()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_clock(Clock p)
{
  if (p != NULL) {
    free_mem(p, PTRS_CLOCK);
    Clock_frees++;
  }
}  /* free_clock */

/*
 *  end of memory management
 */

/*************
 *
 *   clock_init()
 *
 *************/

/* DOCUMENTATION
This routine initializes a clock.  You give it a string
(which is not copied), representing the name of the new clock,
and it returns a Clock to be used for all operations on the clock.
<P>
The clock operations are clock_start(), clock_stop(),
clock_seconds(), clock_milliseconds(), and clock_reset().
*/

/* PUBLIC */
Clock clock_init(char *str)
{
  Clock p = get_clock();
  p->name = str;
  p->level = 0;
  p->accum_msec = 0;
  return p;
}  /* clock_init */

/*************
 *
 *   clock_start()
 *
 *************/

/* DOCUMENTATION
This routine starts clock n.  It is okay if the clock is already going.
*/

/* PUBLIC */
void clock_start(Clock p)
{
  if (Clocks_enabled) {
    p->level++;
    if (p->level == 1) {
      CPU_TIME(p->curr_msec);
      Clock_starts++;
    }
  }
}  /* clock_start */

/*************
 *
 *   clock_stop()
 *
 *************/

/* DOCUMENTATION
This routine stops clock n and adds the time to the accumulated total,
<I>unless there have been too many starts and not enough stops</I>.
See the introduction.
*/

/* PUBLIC */
void clock_stop(Clock p)
{
  if (Clocks_enabled) {
    if (p->level <= 0)
      fprintf(stderr,"WARNING, clock_stop: clock %s not running.\n",p->name); 
    else {
      p->level--;
      if (p->level == 0) {
	unsigned msec;
	CPU_TIME(msec);
	p->accum_msec += msec - p->curr_msec;
      }
    }
  }
}  /* clock_stop */

/*************
 *
 *   clock_milliseconds()
 *
 *************/

/* DOCUMENTATION
This routine returns the current value of a clock, in milliseconds.
The value is in milliseconds.
*/

/* PUBLIC */
unsigned clock_milliseconds(Clock p)
{
  if (p == NULL)
    return 0;
  else {
    int i = p->accum_msec;
    if (p->level == 0)
      return i;
    else {
      int msec;
      CPU_TIME(msec);
      return i + (msec - p->curr_msec);
    }
  }
}  /* clock_milliseconds */

/*************
 *
 *   clock_seconds()
 *
 *************/

/* DOCUMENTATION
This routine returns the current value of a clock, in seconds.
The clock need not be stopped.
*/

/* PUBLIC */
double clock_seconds(Clock p)
{
  if (p == NULL)
    return 0.0;
  else {
    int i = p->accum_msec;
    if (p->level == 0)
      return (i / 1000.0);
    else {
      int msec;
      CPU_TIME(msec);
      return (i + (msec - p->curr_msec)) / 1000.0;
    }
  }
}  /* clock_seconds */

/*************
 *
 *   clock_running()
 *
 *************/

/* DOCUMENTATION
This routine tells you whether or not a clock is running.
*/

/* PUBLIC */
BOOL clock_running(Clock p)
{
  return p->level > 0;
}  /* clock_running */

/*************
 *
 *   clock_reset()
 *
 *************/

/* DOCUMENTATION
This routine resets a clock, as if it had just been initialized.
(You should not need this routine under normal circumstances.)
*/
  
/* PUBLIC */
void clock_reset(Clock p)
{
  if (p != NULL) {
    p->level = 0;
    p->accum_msec = 0;
  }
}  /* clock_reset */

/*************
 *
 *   fprint_clock()
 *
 *************/

/* DOCUMENTATION
This routine
*/

/* PUBLIC */
void fprint_clock(FILE *fp, Clock p)
{
  if (p != NULL)
    fprintf(fp, "clock %-15s: %6.2f seconds.\n", p->name, clock_seconds(p));
}  /* fprint_clock */

/*************
 *
 *   get_date()
 *
 *************/

/* DOCUMENTATION
This routine returns a string representation of the current date and time.
*/

/* PUBLIC */
char * get_date(void)
{
  time_t i = time(NULL);
  return asctime(localtime(&i));
}  /* get_date */

/*************
 *
 *   user_time()
 *
 *************/

/* DOCUMENTATION
This routine returns the user CPU time, in milliseconds, that the
current process has used so far.
*/

/* PUBLIC */
unsigned user_time()
{
  struct rusage r;
  unsigned sec, usec;

  getrusage(RUSAGE_SELF, &r);
  sec = r.ru_utime.tv_sec;
  usec = r.ru_utime.tv_usec;

  return((sec * 1000) + (usec / 1000));
}  /* user_time */

/*************
 *
 *   user_seconds()
 *
 *************/

/* DOCUMENTATION
This routine returns the user CPU time, in seconds, that the
current process has used so far.
*/

/* PUBLIC */
double user_seconds()
{
  struct rusage r;
  unsigned sec, usec;

  getrusage(RUSAGE_SELF, &r);
  sec = r.ru_utime.tv_sec;
  usec = r.ru_utime.tv_usec;

  return(sec + (usec / 1000000.0));
}  /* user_seconds */

/*************
 *
 *   system_time()
 *
 *************/

/* DOCUMENTATION
This routine returns the system CPU time, in milliseconds, that has
been spent on the current process.
(System time measures low-level operations such
as system calls, paging, and I/O that the operating systems does
on behalf of the process.)
*/

/* PUBLIC */
unsigned system_time()
{
  struct rusage r;
  unsigned sec, usec;

  getrusage(RUSAGE_SELF, &r);
  sec = r.ru_stime.tv_sec;
  usec = r.ru_stime.tv_usec;

  return((sec * 1000) + (usec / 1000));
}  /* system_time */

/*************
 *
 *   system_seconds()
 *
 *************/

/* DOCUMENTATION
This routine returns the system CPU time, in seconds, that has
been spent on the current process.
(System time measures low-level operations such
as system calls, paging, and I/O that the operating systems does
on behalf of the process.)
*/

/* PUBLIC */
double system_seconds()
{
  struct rusage r;
  unsigned sec, usec;

  getrusage(RUSAGE_SELF, &r);
  sec = r.ru_stime.tv_sec;
  usec = r.ru_stime.tv_usec;

  return(sec + (usec / 1000000.0));
}  /* system_seconds */

/*************
 *
 *   absolute_wallclock()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
unsigned absolute_wallclock(void)
{
  time_t t = time((time_t *) NULL);
  return (unsigned) t;
}  /* absolute_wallclock */

/*************
 *
 *   init_wallclock()
 *
 *************/

/* DOCUMENTATION
This routine initializes the wall-clock timer.
*/

/* PUBLIC */
void init_wallclock()
{
  Wall_start = absolute_wallclock();
}  /* init_wallclock */

/*************
 *
 *   wallclock()
 *
 *************/

/* DOCUMENTATION
This routine returns the number of wall-clock seconds since
init_wallclock() was called.  The result is unsigned.
*/

/* PUBLIC */
unsigned wallclock()
{
  return absolute_wallclock() - Wall_start;
}  /* wallclock */

/*************
 *
 *   disable_clocks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void disable_clocks(void)
{
  Clocks_enabled = FALSE;
}  /* disable_clocks */

/*************
 *
 *   enable_clocks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void enable_clocks(void)
{
  Clocks_enabled = TRUE;
}  /* enable_clocks */

/*************
 *
 *   clocks_enabled()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL clocks_enabled(void)
{
  return Clocks_enabled;
}  /* clocks_enabled */


