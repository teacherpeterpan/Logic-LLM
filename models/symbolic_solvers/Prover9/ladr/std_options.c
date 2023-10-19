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

#include "std_options.h"

/* Private definitions and types */

/* Flags */

static int Prolog_style_variables = -1;       /* delayed effect */
static int Ignore_option_dependencies = -1;   /* immediate effect */
static int Clocks = -1;                       /* delayed effect */

/*************
 *
 *   init_standard_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_standard_options(void)
{
  /* Flags */
  Prolog_style_variables     = init_flag("prolog_style_variables",     FALSE);
  Ignore_option_dependencies = init_flag("ignore_option_dependencies", FALSE);
  Clocks                     = init_flag("clocks",                     FALSE);

}  /* init_standard_options */

/*************
 *
 *   process_standard_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void process_standard_options(void)
{
  if (flag(Clocks))
    enable_clocks();
  else
    disable_clocks();

  if (flag(Prolog_style_variables))
    set_variable_style(PROLOG_STYLE);
  else
    set_variable_style(STANDARD_STYLE);

  /* Flag gnore_option_dependencies is handled internally by
     the options package.  It takes effect immediately.
   */

}  /* process_standard_options */

/*************
 *
 *   clocks_id()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clocks_id(void)
{
  return Clocks;
}  /* clocks_id */

/*************
 *
 *   prolog_style_variables_id()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int prolog_style_variables_id(void)
{
  return Prolog_style_variables;
}  /* prolog_style_variables_id */
