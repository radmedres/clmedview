#ifndef PLUGIN_SELECTION_TOOL_INTERFACE_H
#define PLUGIN_SELECTION_TOOL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plugin-interface.h"
#include "lib-pixeldata.h"
#include "lib-common-list.h"


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
 * The draw action is triggered by calling this function, so every brush plugin should implement this
 * function.
 *
 * @param ps_Original      Data of the original (raw) image.
 * @param ps_Mask          Data of the mask to draw on.
 * @param ps_Selection     Data of the selection mask which should be respected.
 * @param ts_Point         The coordinate to be added to polygon coordinate list.
 * @param pll_Coordinates  A pointer to a list of coordinates.
 * @param ui32_DrawValue    The value to set selected pixels to.
 * @param te_Action        The action to apply (draw or erase).
 */
void CLM_Plugin_Line_Apply (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                            Coordinate ts_Point, List **pll_Coordinates, unsigned int ui32_DrawValue,
                            PixelAction te_Action);
/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_SELECTION_TOOL_INTERFACE_H
