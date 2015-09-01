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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libcommon-tree.h"
#include "libcommon-list.h"

#include "libmemory-serie.h"
//#include "lib-pixeldata-plugin.h"

/**
 * @file include/libconfiguration.h
 * @brief A program-wide configurable state.
 * @author Roel Janssen
 */


/**
 * @ingroup configuration
 * @{
 *
 * This module provides a singleton to give an independent way to store the
 * program's state. The need for this module has come since there are multiple
 * user interfaces to interact with.
 */


#define KEY_TOGGLE_FOLLOW       0
#define KEY_TOGGLE_AUTOCLOSE    1
#define KEY_UNDO                2
#define KEY_REDO                3
#define KEY_VIEW_AXIAL          4
#define KEY_VIEW_SAGITAL        5
#define KEY_VIEW_CORONAL        6
#define KEY_VIEW_SPLIT          7
#define KEY_TOGGLE_TERMINAL     8
#define KEY_TOGGLE_SIDEBAR      9
#define KEY_TOGGLE_RECORDING    10
#define KEY_REPLAY_RECORDING    11
#define KEY_REPLAY_OVER_TIME    12
#define KEY_REPLAY_OVER_SLICES  13


/**
 * An opaque type providing access to common configuration items.
 * Access to its members should be done using the macro definitions.
 */
typedef struct
{
  Tree *pt_memory;              /* The actively maintained memory tree. */
  Tree *pt_active_study;        /* The active study tree. */
  Tree *pt_active_serie;        /* The active serie tree. */

  Serie *ps_active_serie;       /* The active serie */
  Serie *ps_active_mask;        /* The active study */

  unsigned char c_key_bindings[14]; /*< An array with key bindings. */

} Configuration;


/**
 * Returns the active memory tree.
 */
#define CONFIGURATION_MEMORY_TREE(c)       c->pt_memory

/**
 * Returns the active study tree
 */
#define CONFIGURATION_ACTIVE_STUDY_TREE(c) c->pt_active_study

/**
 * Returns the active serie tree
 */
#define CONFIGURATION_ACTIVE_SERIE_TREE(c) c->pt_active_serie

/**
 * Returns the active serie in the memory tree.
 */
#define CONFIGURATION_ACTIVE_SERIE(c)  c->ps_active_serie

/**
 * Returns the active mask in the memory tree.
 */
#define CONFIGURATION_ACTIVE_MASK(c)  c->ps_active_mask


/**
 * Returns a list of loaded draw tools.
 */
//#define CONFIGURATION_PLUGINS(c)           c->pl_draw_tools

/**
 * Returns the active draw tool list item.
 */
//#define CONFIGURATION_ACTIVE_PLUGIN(c)     c->pl_active_draw_tool

/**
 * Returns the bound character for a key.
 */
#define CONFIGURATION_KEY(c,k)             c->c_key_bindings[k]

/**
 * Returns a list of lookup tables.
 */
//#define CONFIGURATION_LOOKUP_TABLES(c)     c->pl_lookup_tables

/**
 * An instance of the global configuration state.
 */
Configuration configuration_state;


/**
 * A function to get a pointer to the global configuration object.
 *
 * @return a pointer to the global configuration object.
 */
Configuration *configuration_get_default ();


/**
 * This function should be called before using any of this module's
 * functionality.
 */
void configuration_init ();


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//CONFIGURATION_H
