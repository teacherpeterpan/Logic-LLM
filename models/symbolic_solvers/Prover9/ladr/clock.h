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

#ifndef TP_CLOCK_H
#define TP_CLOCK_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "string.h"

/* INTRODUCTION
This package is for timing various operations.  Say you need to time
an operation P.  You first call clock_init() to set up a clock,
then you can start and stop the clock as you wish, then you
can get the accumulated time with clock_value().
These clocks measure the user CPU time.
<P>
An unusual feature of these clocks is that they can be used
inside of recursive routines.  For example, you can start
the clock at the beginning of a recursive routine and stop
it at the end.  If you start it 3 times then stop it
three times, it will really stop only on the third call.
This works by a counter of the number of extra
times the clock has been started, and clock_stop() will stop
the clock only when the count is 0.   (This feature probably isn't
very useful, and most people can ignore it.)
<P>
Also here are some routines for getting process system/user CPU time, 
elapsed wall-clock time, and the time/date.
*/

/* Public definitions */

typedef struct clock * Clock;

/* End of public definitions */

/* Public function prototypes from clock.c */

void free_clock(Clock p);

Clock clock_init(char *str);

void clock_start(Clock p);

void clock_stop(Clock p);

unsigned clock_milliseconds(Clock p);

double clock_seconds(Clock p);

BOOL clock_running(Clock p);

void clock_reset(Clock p);

void fprint_clock(FILE *fp, Clock p);

char * get_date(void);

unsigned user_time();

double user_seconds();

unsigned system_time();

double system_seconds();

unsigned absolute_wallclock(void);

void init_wallclock();

unsigned wallclock();

void disable_clocks(void);

void enable_clocks(void);

BOOL clocks_enabled(void);

#endif  /* conditional compilation of whole file */
