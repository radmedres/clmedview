/*
 * Copyright (C) 2014  Roel Janssen <roel@roelserve.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib-pixeldata.h"


/**
 * @file   plugin/plugin-interface.h
 * @brief  The global plugin interface definition.
 * @author Roel Janssen
 */

/**
 * @defgroup plugin Plugin
 * @{
 *
 * This module provides a global plugin interface definition. Each plugin should 
 * use or implement the functions defined in this interface.
 * 
 * @note The prefix for this namespace is "CLM_Plugin" instead of the expected 
 *       "Plugin_", because the plugins are tied to the CLMedview application.
 */

/**
 * This enumeration contains all known plugin types. If you need a new type, please
 * add it here. Each type should have its own interface file. Please describe why
 * the new plugin type is different from the others.
 */
typedef enum {
  PLUGIN_TYPE_UNDEFINED,
  PLUGIN_TYPE_BRUSH,
  PLUGIN_TYPE_SELECTION,
  PLUGIN_TYPE_LINE
} PluginType;


/**
 * This function allows clmedview to gather some information from the plugin.
 * @note This function _must_ be implemented by each plugin.
 *
 * @param name     A pointer to a string. On input this string is unallocated. 
 *                 On output this string should be allocated.
 * @param icon     A pointer to a icon blob. On input this string is unallocated.
 *                 On output this string should be allocated.
 * @param version  A pointer to an int. On input this integer has to be allocated.
 *                 On output the value should have changed to the version of the plugin.
 * @param type     A pointer to a PluginType enum. On input this enum has to be allocated.
 *                 On output the value should have changed to the actual plugin type.
 */
void CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type);


/**
 * For the sake of abstracting the complexities of changing the value of a
 * pixel this function should be used to perform this task.
 *
 * @param ps_Mask        The mask to use for drawing.
 * @param ps_Selection   The selection mask to use for whether the value may be set.
 * @param ts_Point       The coordinate to determine the pixel.
 * @param ui32_DrawValue The value to set the pixel to.
 * @param te_Action      The action to apply (draw or erase)
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
short int CLM_Plugin_DrawPixelAtPoint (PixelData *ps_Mask, PixelData *ps_Selection,
                                       Coordinate ts_Point, unsigned int ui32_DrawValue,
                                       PixelAction te_Action);


/**
 * For the sake of abstracting the complexities of getting the value of a pixel
 * this function should be used to perform this task.
 *
 * @param ps_Mask        The mask to use for drawing.
 * @param ts_Point       The coordinate to get the pixel value for.
 * @param pv_PixelValue  A pointer to the value of the pixel. 
 *                       The type of the value is unknown. 
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
short int CLM_Plugin_GetPixelAtPoint (PixelData *ps_Mask, Coordinate ts_Point, void *pv_PixelValue);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_INTERFACE_H
