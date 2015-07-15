#ifndef PLUGIN_BRUSHES_INTERFACE_H
#define PLUGIN_BRUSHES_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plugin-interface.h"
#include "libpixeldata.h"

/**
 * @file   plugin/src/brushes/plugin-brush-interface.h
 * @brief  The plugin interface definition for brushes.
 * @author Roel Janssen
 */

/**
 * @ingroup plugin
 * @{
 *
 *   @defgroup plugin_brush Brush
 *   @{
 *
 * This module provides an interface to a variant of the supported plugins.
 * These plugins mainly differ from other plugin types in their stateless usage.
 * When called, the plugin sets a couple of pixels and returns. It is not aware of
 * any previous or next call.
 *
 * This plugin type manipulates the mask layer. It doesn't write to the original
 * data or selection data.
 *
 * Brush plugins implement _at least_ these properties:
 * - scale
 * - value
 * - action
 */


/**
 * A structure containing all properties of a brush.
 */
typedef struct
{
  unsigned int size; /*< The size of the brush to fill around a coordinate. */
  unsigned int value; /*< The value to use when brushing around. */
  PixelAction action; /*< The action to apply (draw or erase). */
} PluginBrushProperties;

/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_BRUSHES_INTERFACE_H
