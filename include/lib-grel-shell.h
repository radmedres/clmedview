/*
 * Copyright (C) 2015 Marc Geerlings <m.geerlings@mumc.nl>
 *
 * This file is part of clmedview.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GREL_SHELL_H
#define GREL_SHELL_H

#include <libguile.h>

#include "lib-pixeldata.h"
#include "lib-memory-tree.h"
#include "lib-grel-interpreter.h"

/**
 * @ingroup grel
 * @{
 *   @defgroup grel_shell Shell
 *   @{
 *
 * This module provides an interactive shell for GREL. It is the front-end
 * for the @ref grel_interpreter "GREL::Interpreter" module.
 *
 * Be aware that the GREL Shell uses a GREL::Interpreter module to capture
 * return values.
 */


/**
 * A structure to keep GREL-related variables organized together.
 */
typedef struct
{
  GRELInterpreter *interpreter; /*< An interpreter that is used to get data. */  
} GRELShell;


/**
 * This function starts a guile shell, allowing us to use GREL interactively.
 * It will expose some application functions to the Guile interpreter. 
 *
 * This function is called by grel_shell_start(), so you shouldn't need to call
 * it yourself.
 */
void grel_shell_prepare ();


/**
 * This function starts a Guile shell.
 */
void grel_shell_start ();


/**
 *    @}
 * @}
 */

#endif//GREL_SHELL_H
