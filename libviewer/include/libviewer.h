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

#ifndef VIEWER_H
#define VIEWER_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include "libpixeldata.h"
#include "libpixeldata-plugin.h"
#include "libcommon.h"
#include "libcommon-list.h"
#include "libmemory-serie.h"
#include "libmemory.h"


/**
 * @file   include/lib-viewer.h
 * @brief  A viewing widget for displaying a single dimension of a medical image.
 * @author Roel Janssen
 */


/**
 * @defgroup viewer Viewer
 * @{
 *
 * This module provides a single dimension viewer for a medical image. It
 * supports three view types: axial, sagital and coronal.
 *
 * This is a module that provides a widget that is compatible with GTK to
 * display a single dimension of a medical image.
 *
 * Signals
 * -------
 *
 * The user actions applied on the Viewer widget can be read by setting a
 * callback function (a.k.a. a signal). There are three signals:
 *
 * * on_pixel_paint
 * * on_focus_change
 * * on_handle_change_callback
 */


/******************************************************************************
 * TYPES
 ******************************************************************************/


/**
 * This enumeration lists all possible actions that can be done by scrolling
 * with the mouse.
 */
typedef enum ScrollMouseAction
{
  SCROLL_NONE,
  SCROLL_PREV_NEXT,
  SCROLL_SCALE
} ScrollMouseAction;


/**
 * A struct to store drawing commands.
 * @note: This should be considered a temporary hack.
 */
typedef struct
{
  Plugin *plugin;

  Coordinate prev_coordinate;
  Coordinate coordinate;

  unsigned int size;
  unsigned int value;
  PixelAction action;

} ViewerCommand;


/**
 * This enumeration lists all possible redraw actions. Choose the redraw mode
 * that does as little as possible, but just enough to be useful.
 */
typedef enum
{
  REDRAW_MINIMAL, /*< Doesn't update pixel buffers. */
  REDRAW_ACTIVE,  /*< Only updates pixel buffers for the active mask. */
  REDRAW_ALL      /*< Updates all pixel buffers. */
} RedrawMode;


/**
 * This structure contains all data needed by 'viewer' to display an image.
 */
typedef struct Viewer
{
  /*--------------------------------------------------------------------------.
   | PROPERTIES                                                               |
   | For each property there should be a getter and a setter.                 |
   '--------------------------------------------------------------------------*/

  MemoryImageOrientation te_Orientation; /*< Intended orientation. */
  List *pll_MaskSeries; /*< A list of all connected masks. */
  List *pll_OverlaySeries; /*< A list of all connected overlays. */
  PixelData *ps_Original; /*< Actual slice data. */
  PixelData *ps_ActiveSelection; /*< A special mask type for selection tools. */
  PixelData *ps_ActiveMask; /*< A pointer to the active mask layer. */
  Serie *ps_ActiveLayer; /*< A pointer to the active layer. */
  short int b_FollowMode_Enabled; /*< A variable to (en|dis)able follow-mode. */
  short int b_AutoClose_Enabled; /*< A variable to (en|dis)able "auto close". */
  short int b_ViewMode_Enabled; /*< A variable to (en|disable)able view-mode. */

  Vector3D ts_NormalVector; /*< The normal vector for the images inside. */
  Vector3D ts_PivotPoint; /*< The pivot point for the images inside. */
  Vector3D ts_UpVector; /*< A vector to determine the direction. */

  /*--------------------------------------------------------------------------.
   | MEMBERS                                                                  |
   | Member variables are private fields that should only be used internally. |
   | If access to a variable here is useful for outsiders, please use a macro |
   | definition to provide access.                                            |
   '--------------------------------------------------------------------------*/

  GtkWidget *c_Embed; /*< A GtkWidget that can be used by the GUI. */
  ClutterActor *c_Stage; /*< A ClutterActor that is the Clutter stage. */
  ClutterActor *c_Actor; /*< A ClutterActor that displays the image. */
  ClutterActor *c_SliceInfo; /*< A ClutterText layer to display image info. */
  ClutterActor *c_Handles; /*< A ClutterActor used for drawing handles. */

  ClutterActor *c_SliceOrientationTop; /*< A ClutterText layer to display orientatiosn info. */
  ClutterActor *c_SliceOrientationBottom; /*< A ClutterText layer to display orientatiosn info. */
  ClutterActor *c_SliceOrientationLeft; /*< A ClutterText layer to display orientatiosn info. */
  ClutterActor *c_SliceOrientationRight; /*< A ClutterText layer to display orientatiosn info. */



  Plane ts_OriginalPlane; /*< The width and height of the original data. */
  Plane ts_ScaledPlane; /*< The ts_OriginalPlane size with voxel scaling. */
  Coordinate ts_ViewPortCoordinates; /*< The previous coordinate. */
  Coordinate ts_PreviousDrawCoordinate; /*< Previous coordinate that was drawn to. */
  Coordinate ts_InitialDrawCoordinate; /*< First coordinate of a stroke. */
  Coordinate ts_WindowLevelBasePoint; /*< Previous coordinate for window/level. */
  Coordinate ts_CurrentMousePosition; /*< The current mouse position. */
  Coordinate ts_PreviousMousePosition; /*< The previous mouse position. */

  List *pll_PolygonPoints; /*< Used by the polygon plug-in.  */
  Plugin *ts_ActivePainter; /*< The active draw plugin to call. */

  float f_OldViewModeRatio; /*< The previous view-mode ratio. */
  float f_ZoomFactor; /*< Factor for the zoom functionality. */

  List *pll_Replay; /*< A list of replayable actions. */
  short int is_recording; /*< A state variable for recording. */

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/

  /**
   * A callback function to call on a "pixel paint" event.
   * @param viewer A pointer to the viewer object that triggered this callback.
   * @param data   Unused. Room for passing additional data.
   */
  void (*on_pixel_paint_callback)(struct Viewer *viewer, void *data);

  /**
   * A callback function to call on a "focus change" event.
   * @param viewer A pointer to the viewer object that triggered this callback.
   * @param data   A pointer to a Coordinate containing the latest position.
   */
  void (*on_focus_change_callback)(struct Viewer *viewer, void *data);

  /**
   * A callback function to call on a "window/level change" event.
   * @param viewer A pointer to the viewer object that triggered this callback.
   * @param data   A pointer to a Range with the latest window/level values.
   */
  void (*on_window_level_change_callback)(struct Viewer *viewer, void *data);

  /**
   * A callback function to call on a "handle change" event.
   * @param viewer A pointer to the viewer object that triggered this callback.
   * @param ratio  The ratio of DX / DY.
   */
  void (*on_handle_change_callback)(struct Viewer *viewer, void *data);

} Viewer;


/******************************************************************************
 * MACROS
 ******************************************************************************/


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can "cast" a Viewer to a ClutterActor.
 */
#define VIEWER_ACTOR(x) (x->c_Actor)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can "cast" a Viewer to a GtkWidget.
 */
#define VIEWER_WIDGET(x) (x->c_Embed)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can get the orientation of a Viewer.
 */
#define VIEWER_ORIENTATION(x) (x->te_Orientation)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can get the PixelData of the original slice.
 */
#define VIEWER_PIXELDATA_ORIGINAL(x) (x->ps_Original)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can get the PixelData of the active mask slice.
 */
#define VIEWER_PIXELDATA_ACTIVE_MASK(x) (x->ps_ActiveMask)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can get the PixelData of the active selection slice.
 */
#define VIEWER_PIXELDATA_ACTIVE_SELECTION(x) (x->ps_ActiveSelection)


/**
 * A macro to mimic the Gtk and Clutter typecasting beauty.
 * With this macro you can get the active slice of the original PixelData.
 */
#define VIEWER_ACTIVE_SLICE(x) (PIXELDATA_ACTIVE_SLICE (x->ps_Original))


/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/


/**
 * This function creates a clutter stage, which is a stand-alone object
 * that can display a slice and its regions of interests.
 *
 * @param ts_Original        The original image Serie.
 * @param ts_Mask            The active mask image Serie.
 * @param pll_Overlays       An optional list of overlays.
 * @param ts_NormalVector    The normal vector.
 * @param ts_PivotPoint      The position vector.
 * @param ts_UpVector        The vector to determine the rotate base position.
 *
 * @return A pointer to a Viewer. Use the VIEWER_WIDGET macro to get a
 *         GtkWidget that can be displayed.
 */
Viewer* viewer_new (Serie *ts_Original, Serie *ts_Mask, List *pll_Overlays,
                    Vector3D ts_NormalVector, Vector3D ts_PivotPoint,
                    Vector3D ts_UpVector);


void v_viewer_set_image_orientation_direction(Viewer *pt_Viewport, char *pc_Top, char *pc_Bottom, char *pc_Left, char *pc_Right);

/**
 * This function creates a clutter stage, which is a stand-alone object
 * that can display a slice and its regions of interests.
 *
 * @param ts_Original        The original image Serie.
 * @param ts_Mask            The active mask image Serie.
 * @param pll_Overlays       An optional list of overlays.
 * @param ts_NormalVector    The normal vector.
 * @param ts_PivotPoint      The position vector.
 * @param ts_UpVector        The vector to determine the rotate base position.
 *
 * @return A pointer to a Viewer. Use the VIEWER_WIDGET macro to get a
 *         GtkWidget that can be displayed.
 */
void
viewer_initialize (Viewer *resources, Serie *ts_Original, Serie *ts_Mask, List *pll_Overlays,
                   Vector3D ts_NormalVector, Vector3D ts_PivotPoint,
                   Vector3D ts_UpVector);


/**
 * This function can be called to resize the actor related to the given
 * Viewer.
 *
 * @param resources  The Viewer to apply this action to.
 * @param width      The width to resize to.
 * @param height     The height to resize to.
 */
void viewer_resize (Viewer *resources, int width, int height);


/**
 * This function triggers a redraw of a Viewer widget. You can do a "full redraw"
 * to reload the image data, or a "partial redraw" which only scales the current
 * image.
 *
 * @param resources    The Viewer resources.
 * @param redraw_mode  The mode of redrawing to apply.
 */
void viewer_redraw (Viewer *resources, RedrawMode redraw_mode);


/**
 * This function cleans up the memory associated with a viewer object.
 *
 * @param data  The Viewer to apply this action to.
 */
void viewer_destroy (void* data);


/**
 * This function adds mask resources to a viewer.
 *
 * @param resources The viewer to add the mask to.
 * @param mask      The mask to add.
 */
void viewer_add_mask (Viewer *resources, PixelData *mask);


/**
 * This function adds mask resources to a viewer. This function
 * differs from viewer_add_mask() in that you can give it a Serie
 * and it will automatically create a lookup table.
 *
 * @param resources The viewer to add the mask to.
 * @param mask      The mask serie to add.
 */
void viewer_add_mask_serie (Viewer *resources, Serie *mask);


/**
 * This function removes a mask serie from a viewer.
 *
 * @param resources The viewer to remove a mask of.
 * @param mask      The mask serie to remove.
 */
void viewer_remove_mask_serie (Viewer *resources, Serie *mask);


/**
 * This function sets the active mask layer for a viewer using a Serie.
 *
 * @param resources The viewer to set an active mask for.
 * @param serie     The serie to use as mask.
 */
void viewer_set_active_mask_serie (Viewer *resources, Serie *serie);


/**
 * This function returns the active mask serie of the Viewer.
 *
 * @param resources The viewer to get the active mask of.
 *
 * @return The serie of the active mask.
 */
Serie* viewer_get_active_mask_serie (Viewer *resources);


/**
 * This function sets the active layer for a viewer using a Serie.
 *
 * @param resources The viewer to set an active layer for.
 * @param serie     The serie to use as active layer.
 */
void viewer_set_active_layer_serie (Viewer *resources, Serie *serie);


/**
 * This function returns the active layer serie of the Viewer.
 *
 * @param resources The viewer to get the active layer of.
 *
 * @return The serie of the active layer.
 */
Serie* viewer_get_active_layer_serie (Viewer *resources);


/**
 * This function sets the active lookup table for a Serie in a Viewer.
 *
 * @param resources The viewer to set an active mask for.
 * @param serie     The serie to use as mask.
 * @param lut_name  The name of the lookup table.
 */
void viewer_set_lookup_table_for_serie (Viewer *resources, Serie *serie, const char *lut_name);


/**
 * This function gets the active lookup table of a Serie in a Viewer.
 *
 * @param resources The viewer to set an active mask for.
 * @param serie     The serie to use as mask.
 *
 * @return The PixelDataLookupTable structure of the active lookup table.
 */
PixelDataLookupTable* viewer_get_active_lookup_table_for_serie (Viewer *resources, Serie *serie);


/**
 * This functions sets the opacity of a Serie in a Viewer.
 *
 * @param resources  The viewer to set the opacity for.
 * @param serie      The Serie to apply the opacity for.
 * @param opacity    The opacity to set.
 */
void viewer_set_opacity_for_serie (Viewer *resources, Serie *serie, unsigned char opacity);


/**
 * This functions gets the opacity of a Serie in a Viewer.
 *
 * @param resources  The viewer to get the opacity of.
 * @param serie      The Serie to get the opacity of.
 *
 * @return The opacity as a value between 0 and 255.
 */
unsigned char viewer_get_opacity_for_serie (Viewer *resources, Serie *serie);


/**
 * This function returns the PixelData for a Serie.
 *
 * @param resources  The viewer to get the PixelData of.
 * @param serie      THe serie to get the PixelData of.
 */
PixelData* viewer_get_pixeldata_for_serie (Viewer *resources, Serie *serie);


/**
 * This functions allows to set a callback.
 *
 * @param resources  The viewer to set the callback for.
 * @param name       The name of the event to set the callback for.
 * @param callback   A function pointer of the callback function.
 */
void viewer_set_callback (Viewer *resources, const char *name, void (*callback)(Viewer *, void *));


/**
 * This function allows to set an active painter, which is usually a plug-in.
 *
 * @param resources The viewer to set the active painter for.
 * @param plugin    The plug-in to set as the active painter.
 */
void viewer_set_active_painter (Viewer *resources, Plugin *plugin);


/**
 * This function sets the base image data for a viewer.
 *
 * @param resources The viewer to add the mask to.
 * @param data      The resources of the base image.
 */
void viewer_set_data (Viewer *resources, PixelData *data);


/**
 * This function adds an overlay serie to the Viewer.
 *
 * @param resources The viewer to add an overlay to.
 * @param overlay   The Serie to add as overlay.
 */
void viewer_add_overlay_serie (Viewer *resources, Serie *overlay);


/**
 * This function removes a overlay serie from a viewer.
 *
 * @param resources The viewer to remove an overlay of.
 * @param overlay   The overlay serie to remove.
 */
void viewer_remove_overlay_serie (Viewer *resources, Serie *overlay);


/**
 * This function can change the displayed slice by providing a slice number.
 *
 * @param resources        The viewer to set the displayed slice for.
 * @param i32_SliceNumber  The slice number to set.
 */
void viewer_set_slice (Viewer *resources, int i32_SliceNumber);


/**
 * Using this function one can (en|dis)able follow-mode.
 *
 * @param resources    The viewer to change the follow-mode setting for.
 * @param b_SetEnabled Set to 1 to enable, 0 to disable.
 */
void viewer_set_follow_mode (Viewer *resources, short int b_SetEnabled);


/**
 * Using this function returns the current follow-mode state.
 *
 * @param resources    The viewer to change the follow-mode setting for.
 * @return The current follow-mode state.
 */
short int viewer_get_follow_mode (Viewer *resources);


/**
 * Using this function one can set the intended orientation.
 *
 * @param resources    The viewer to set the orientation for.
 * @param orientation  The desired orientation.
 */
void viewer_set_orientation (Viewer *resources, MemoryImageOrientation orientation);


/**
 * Using this function one can get the intended orientation.
 *
 * @param resources    The viewer to get the orientation of.
 * @return The intended orientation.
 */
MemoryImageOrientation viewer_get_orientation (Viewer *resources);


/**
 * Using this function one can (en|dis)able "auto close".
 *
 * @param resources    The viewer to change the "auto close" setting for.
 * @param b_SetEnabled Set to 1 to enable, 0 to disable.
 */
void viewer_set_auto_close (Viewer *resources, short int b_SetEnabled);


/**
 * This function returns the current "auto close" state.
 * @param resources    The viewer to get the "auto close" setting of.
 * @return The current "auto close" state.
 */
short int viewer_get_auto_close (Viewer *resources);


/**
 * Using this function one can set the "view mode".
 *
 * @param resources    The viewer to change the "view-mode" setting for.
 * @param b_SetEnabled Set to 1 to enable, 0 to disable.
 */
void viewer_set_view_mode (Viewer *resources, short int b_SetEnabled);


/**
 * This function returns the current "view-mode" state.
 *
 * @param resources    The viewer to get the "view-mode" setting of.
 * @return The current "view-mode" state.
 */
short int viewer_get_view_mode (Viewer *resources);


/**
 * This function can be called when a key has been pressed (or when this
 * should be emulated).
 *
 * @param resources    The viewer to apply the key press for.
 * @param event        The GdkEventKey that was or should be pressed.
 */
void viewer_on_key_press (Viewer *resources, GdkEventKey *event);


/**
 * This function can be called when a key has been released (or when this
 * should be emulated).
 *
 * @param resources    The viewer to apply the key release for.
 * @param event        The GdkEventKey that was or should be released.
 */
void viewer_on_key_release (Viewer *resources, GdkEventKey *event);


/**
 * This function should be called when a normal vector or pivot point has
 * changed.
 *
 * @param resources    The viewer to refresh the data for.
 */
void viewer_refresh_data (Viewer *resources);


/**
 * For data with time series, this function can be used to set a specific
 * time serie.
 *
 * @param resources      The viewer to set the time serie for.
 * @param u16_timepoint  The time serie.
 */
void viewer_set_timepoint (Viewer *resources, unsigned short int u16_timepoint);


/**
 * A function to toggle recording of actions done in the Viewer.
 *
 * @param resources      The viewer to toggle recording of.
 */
void viewer_toggle_recording (Viewer *resources);


/**
 * A function to replay a recording of actions done in the Viewer.
 *
 * @param resources      The viewer to replay the recording of.
 */
void viewer_replay_recording (Viewer *resources);


/**
 * A function to replay a recording of actions done in the Viewer
 * over all time series.
 *
 * @param resources      The viewer to replay the recording of.
 */
void viewer_replay_recording_over_time (Viewer *resources);


/**
 * A function to update the window width and window level of
 * a Serie in a Viewer.
 *
 * @param resources   The viewer to set new window/level settings for.
 * @param serie       The serie to apply the new window/level settings to.
 * @param i32_windowWidth  The window width value.
 * @param i32_windowLevel  The window level value.
 */
void viewer_set_window_level_for_serie (Viewer *resources, Serie *serie, int i32_windowWidth, int i32_windowLevel);

/**
 * @}
 */

#endif//VIEWER_H
