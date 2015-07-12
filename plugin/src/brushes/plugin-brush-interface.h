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
 */
 
/**
 * The draw action is triggered by calling this function, so every brush plugin should implement this
 * function.
 *
 * @param ps_Original      Data of the original (raw) image.
 * @param ps_Mask          Data of the mask to draw on.
 * @param ps_Selection     Data of the selection mask which should be respected.
 * @param ts_Point         The coordinate in the image data to draw.
 * @param ui32_BrushScale  The scaling factor format the brush.
 * @param ui32_DrawValue   The value to set selected pixels to.
 * @param te_Action        The action to apply (draw or erase).
 */
void CLM_Plugin_Brush_Apply (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                             Coordinate ts_Point, unsigned int ui32_BrushScale,
                             unsigned int ui32_DrawValue, PixelAction te_Action);

/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_BRUSHES_INTERFACE_H
