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

#ifndef PIXELDATA_PLUGIN
#define PIXELDATA_PLUGIN

#include "libcommon.h"
#include "libcommon-list.h"
#include "libpixeldata.h"
#include "plugin-interface.h"


/**
 * @file   include/lib-pixeldata-plugin.h
 * @brief  A plug-in interface to manipulate pixel data.
 * @author Marc Geerlings, Jos Slenter, Roel Janssen
 */


/**
 * @ingroup pixeldata
 * @{
 *   @defgroup pixeldata_plugin Plugin
 *   @{
 *
 * This module provides functions to manage plug-ins.
 */


/**
 * This linked list is a generalisation for plug-ins.
 * The fp_Callback is specified as a pointer to void because it has the same
 * size as the specific callback handler.
 */
typedef struct
{
  PluginType te_Type;
  char *pc_Name;
  unsigned char *pc_Icon;
  void *fp_Callback;
  void *v_LibraryHandler;
  int i32_Size;
  int i32_Value;
} Plugin;


/**
 * This linked list can contain dynamically loaded brushes.
 */
typedef struct
{
  PluginType te_Type;
  char *pc_Name;
  unsigned char *pc_Icon;
  void (*fp_Callback) (PixelData*, PixelData*, PixelData*, Coordinate, unsigned int, PixelAction);
  void *v_LibraryHandler;
  int i32_Size;
  int i32_Value;
} PluginBrush;


/**
 * This linked list can contain dynamically loaded selection tools.
 */
typedef struct
{
  PluginType te_Type;
  char *pc_Name;
  unsigned char *pc_Icon;
  void (*fp_Callback) (PixelData*, PixelData*, PixelData*, Coordinate, Coordinate, unsigned int, PixelAction);
  void *v_LibraryHandler;
  int i32_Size;
  int i32_Value;
} PluginSelection;


/**
 * This linked list can contain dynamically loaded line tools.
 */
typedef struct
{
  PluginType te_Type;
  char *pc_Name;
  unsigned char *pc_Icon;
  void (*fp_Callback) (PixelData*, PixelData*, PixelData*, Coordinate, List**, unsigned int, PixelAction);
  void *v_LibraryHandler;
  int i32_Size;
  int i32_Value;
} PluginLine;


/**
 * This function can load a shared object that contains a brush function.
 *
 * @param pll_PluginList  A list of already loaded plugins or NULL.
 * @param pc_Filename     The filename (including the path to it) of the plugin to load.
 *
 * @return A list item with a valid callback function for the loaded plugin on success,
 *         or an unmodified version of pll_PluginList on failure.
 */
List* pixeldata_plugin_load_file (List *pll_PluginList, const char *pc_Filename);


/**
 * This function loads plugins from a directory.
 *
 * @param path             The path to load a plugin from.
 * @param ppll_PluginList  The list to add the plugins to. 
 *
 * @return 1 when no errors occured, 0 when an error occured.
 */
short int pixeldata_plugin_load_from_directory (const char *path, List **ppll_PluginList);


/**
 * This function destroys a Plugin item. The library handle will be closed
 * by a call to dlclose() when this function is called.
 *
 * @param plugin  The plugin list's data to destroy.
 */
void pixeldata_plugin_destroy (void *plugin);


/**
 *   @}
 * @}
 */

#endif//PIXELDATA_PLUGIN
