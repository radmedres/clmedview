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

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

/**
 * @file include/libcommon-debug.h
 * @brief A generic interface for multiple debugging modes.
 * @author Roel Janssen
 */


/**
 * @ingroup common
 * @{
 * 
 *   @defgroup common_debug Debug
 *   @{
 *
 * Functions in this module provide an interface for multiple debugging modes.
 * This module makes it easier to tag debug messages and format them in a 
 * standardized way.
 *
 * The following compiler flags enable debug messages:
 *
 * * -DENABLE_DEBUG_FUNCTIONS: Print the name of a function when called.
 * * -DENABLE_DEBUG_EVENTS:    Print the name of an event handler when called.
 * * -DENABLE_DEBUG_EXTRA:     Print extra debug messages.
 * * -DENABLE_DEBUG_WARNING:   Print warning messages.
 * * -DENABLE_DEBUG_ERROR:     Print error messages.
 * * -DENABLE_DEBUG:           Enable all of the above.
 *
 * Typically you can enable debugging by running:
 * <pre>
 * ./configure CFLAGS="-DENABLE_DEBUG"
 * </pre>
 */


/******************************************************************************
 * TYPES
 ******************************************************************************/


/**
 * An enumeration of all debug modes known to this program.
 */
typedef enum
{
  DEBUG_MODE_FUNCTIONS,
  DEBUG_MODE_EVENTS,
  DEBUG_MODE_EXTRA,
  DEBUG_MODE_WARNING,
  DEBUG_MODE_ERROR,
} DebugMode;


/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/


/**
 * This function provides a way to show debugging information.
 * It is up to the user which messages are shown.
 *
 * @param mode    The mode in which the message should be visible.
 * @param format  A format specifier for the output message.
 * @param ...     Extra parameters.
 */
void common_debug (DebugMode mode, const char *format, ...);


/******************************************************************************
 * MACRO DEFINITIONS
 ******************************************************************************/


#define DEBUG_MAX_MESSAGE_LENGTH 255


#if defined(ENABLE_DEBUG_FUNCTIONS) || defined (ENABLE_DEBUG)
#  define debug_functions() common_debug (DEBUG_MODE_FUNCTIONS, "%s:%d (%s)", __func__, __LINE__, __FILE__)
#else
#  define debug_functions()
#endif


#if defined(ENABLE_DEBUG_EVENTS) || defined (ENABLE_DEBUG)
#  define debug_events()    common_debug (DEBUG_MODE_EVENTS, "%s:%d (%s)", __func__, __LINE__, __FILE__)
#else
#  define debug_events()
#endif

// The syntax below may look odd. The macro allows to pass a variable number of
// arguments with the "..." and the __VA_ARGS__ macro.
//
// To allow leaving out the variable arguments entirely, we have to get rid of
// the ",". This is exactly what ## does. Please refer to the GCC Manual for more
// information. This construction has been tested with GCC and CLANG.
#if defined(ENABLE_DEBUG_EXTRA) || defined (ENABLE_DEBUG)
#define debug_extra(x, ...)     common_debug (DEBUG_MODE_EXTRA, x, ## __VA_ARGS__)
#else
#  define debug_extra(x, ...)
#endif


#if defined(ENABLE_DEBUG_WARNING) || defined (ENABLE_DEBUG)
#  define debug_warning(x, ...) common_debug (DEBUG_MODE_WARNING, x, ## __VA_ARGS__)
#else
#  define debug_warning(x, ...)
#endif


#if defined(ENABLE_DEBUG_ERROR) || defined (ENABLE_DEBUG)
#  define debug_error(x, ...)   common_debug (DEBUG_MODE_ERROR, x, ## __VA_ARGS__)
#else
#  define debug_error(x, ...)
#endif


#ifdef __cplusplus
}
#endif

/**
 *   @}
 * @}
 */

#endif//COMMON_DEBUG_H
