#ifndef PLUGIN_SELECTION_TOOL_INTERFACE_H
#define PLUGIN_SELECTION_TOOL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plugin-interface.h"
#include "libpixeldata.h"
#include "libcommon-list.h"


/**
 * @file   plugin/src/line-tools/plugin-line-interface.h
 * @brief  The plugin interface definition for line tools.
 * @author Roel Janssen
 */


/**
 * @ingroup plugin
 * @{
 *
 *   @defgroup plugin_line Line
 *   @{
 *
 * This module provides an interface to a variant of the supported plugins.
 * These plugins mainly differ from other plugin types in their stateful usage.
 * When called, the plugin may act different because of a previous call.
 *
 * This type of plugin is often used to draw lines between points.
 *
 * This plugin type manipulates the mask layer. It doesn't write to the original
 * data or selection data.
 */

/**
 * A structure containing all properties of a line tool.
 */
typedef struct
{
  unsigned int size;
  unsigned int value;
  PixelAction action;

  List *coordinates;
} PluginLineProperties;


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_SELECTION_TOOL_INTERFACE_H
