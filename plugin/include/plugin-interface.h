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

#include "libpixeldata.h"
#include <stdbool.h>

/**
 * @file   plugin/plugin-interface.h
 * @brief  The global plugin interface definition.
 * @author Roel Janssen
 */

/**
 * @defgroup plugin Plugin
 * @{
 *
 * This module provides a global plugin interface definition. Each plugin
 * should use or implement the functions defined in this interface.
 *
 * Plugins may have their own internal state which they can advertise in the
 * metadata.
 */


/**
 * A structure containing properties of a plugin to identify, show and
 * configure a plugin.
 */
typedef struct
{
  char *name;        /*< The name of the plugin. */
  int version;       /*< The version of the plugin. */
  void *properties;  /*< A pointer to the plugin's properties. */

} PluginMetaData;


/**
 * This function allows clmedview to gather some information from the plugin.
 * @note This function _must_ be implemented by each plugin.
 *
 * @param metadata A pointer to a pointer to a PluginMetaData object.
 */
void plugin_get_metadata (PluginMetaData **metadata);


/**
 * The 'apply' action is triggered by calling this function, so every plugin
 * should implement this function.
 *
 * @param metadata   The metadata of the plugin.
 * @param original   Data of the original (raw) image.
 * @param mask       Data of the mask to draw on.
 * @param selection  Data of the selection mask which should be respected.
 * @param point      The coordinate in the image data to draw.
 */
void plugin_apply (PluginMetaData *metadata, PixelData *original,
		   PixelData *mask, PixelData *selection,
		   Coordinate point);


/**
 * This function can be used to get a configuration value. Which configuration
 * values are implemented by a plugin is up to the plugin itself and advertised
 * in the metadata of the plugin.
 *
 * @param metadata  The PluginMetaData object.
 * @param property  The property to get the value of.
 *
 * @return A pointer to the value of the property on success, or NULL on 
 *         failure.
 */
void* plugin_get_property (PluginMetaData *metadata, const char *property);


/**
 * This function can be used to set a configuration value. Which configuration
 * values are implemented by a plugin is up to the plugin itself and advertised
 * in the metadata of the plugin.
 *
 * @param metadata  The PluginMetaData object.
 * @param property  The property to get the value of.
 * @param value     A pointer to the value that should be set.
 *
 * @return A pointer to the value of the property on success, or NULL on 
 *         failure.
 */
bool plugin_set_property (PluginMetaData *metadata, const char *property,
			  void *value);


/**
 * For the sake of abstracting the complexities of changing the value of a
 * pixel this function should be used to perform this task.
 *
 * @param mask        The mask to use for drawing.
 * @param selection   The selection mask to use when the value has been set.
 * @param point       The coordinate to determine the pixel.
 * @param value       The value to set the pixel to.
 * @param action      The action to apply (draw or erase)
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
bool plugin_set_voxel_at_point (PixelData *mask, PixelData *selection,
				Coordinate point, unsigned int value,
				PixelAction action);


/**
 * For the sake of abstracting the complexities of getting the value of a pixel
 * this function should be used to perform this task.
 *
 * @param mask        The mask to use for drawing.
 * @param point       The coordinate to get the pixel value for.
 * @param value       A pointer to the value of the pixel. The type of the
 *                    value is unknown. 
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
bool plugin_get_voxel_at_point (PixelData *mask, Coordinate point,
				void *value);


/**
 * A function to destroy the plugin.
 *
 * @param metadata  The plugin to destroy.
 */
void plugin_destroy (PluginMetaData *metadata);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_INTERFACE_H
