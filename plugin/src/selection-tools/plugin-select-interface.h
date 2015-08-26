#ifndef PLUGIN_SELECTION_TOOL_INTERFACE_H
#define PLUGIN_SELECTION_TOOL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plugin-interface.h"
#include "libpixeldata.h"


/**
 * @file   plugin/src/selection-tools/plugin-select-interface.h
 * @brief  The plugin interface definition for selection tools.
 * @author Roel Janssen
 */

/**
 * @ingroup plugin
 * @{
 *
 *   @defgroup plugin_select Selection
 *   @{
 *
 * This module provides an interface to a variant of the supported plugins.
 * These plugins mainly differ from other plugin types in that they operate on 
 * the selection layer, not a mask layer.
 */

/**
 * The draw action is triggered by calling this function, so every brush plugin should implement this
 * function.
 *
 * @param ps_Original      Data of the original (raw) image.
 * @param ps_Mask          Data of the mask to draw on.
 * @param ps_Selection     Data of the selection mask which should be respected.
 * @param ts_Start         The first coordinate for this selection.
 * @param ts_Current       The current coordinate for this selection.
 * @param ui32_DrawValue   The value to set selected pixels to.
 * @param te_Action        The action to apply.
 */
void CLM_Plugin_Select_Apply (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                              Coordinate ts_Start, Coordinate ts_Current, unsigned int ui32_DrawValue,
                              PixelAction te_Action);


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//PLUGIN_SELECTION_TOOL_INTERFACE_H
