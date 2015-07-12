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

#include "libviewer.h"
#include "libpixeldata.h"
#include "libmemory-slice.h"

#include "libcommon-debug.h"
#include "libcommon-unused.h"

#include <string.h>
#include <cairo.h>
#include <math.h>
#include <assert.h>

#define SCALING_FILTER CLUTTER_SCALING_FILTER_NEAREST


/******************************************************************************
 * LOCAL FUNCTION DEFINITIONS
 ******************************************************************************/


// Event handlers
gboolean viewer_on_mouse_scroll (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_mouse_scroll_prevnext (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_mouse_scroll_scale (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_mouse_move (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_enter_stage (GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean viewer_on_leave_stage (GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean viewer_on_mouse_press (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_mouse_release (ClutterActor *actor, ClutterEvent *event, gpointer data);
gboolean viewer_on_redraw_update_handles (ClutterCanvas *canvas, cairo_t *cr, int width, int height, void *data);

// Helper functions
Coordinate viewer_get_image_pixel_position (Viewer *resources, Coordinate ts_MousePosition);
Coordinate viewer_get_canvas_pixel_position (Viewer *resources, Coordinate ts_MousePosition);
ClutterActor* viewer_create_actor_from_pixeldata (PixelData *ps_Data, int i32_Width, int i32_Height, short int do_full_redraw);

void viewer_draw_mask (Viewer *resources, Coordinate ts_MousePosition, PixelAction te_Action);
void viewer_update_text (Viewer *resources);
void viewer_set_optimal_fit (Viewer *resources);


/******************************************************************************
 * GTK AND CLUTTER EVENT HANDLERS
 ******************************************************************************/


gboolean
viewer_on_redraw_update_handles (UNUSED ClutterCanvas *canvas, cairo_t *cr,
                                 int width, int height, void *data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;
  assert (resources != NULL);

  /*--------------------------------------------------------------------------.
   | CLEAR PREVIOUS CONTENTS                                                  |
   '--------------------------------------------------------------------------*/
  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_restore (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  // Do not display anything when "view-mode" is not enabled.
  if (resources->b_ViewMode_Enabled == 0) return FALSE;

  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width (cr, 2.0);

  Coordinate ts_Diff;
  Coordinate ts_pivotPoint;

  Coordinate ts_firstPoint, ts_secondPoint, ts_thirthPoint, ts_fourthPoint;
  float f_ratio, f_constant, f_firstXValue,f_firstYValue,f_secondXValue,f_secondYValue;

  ts_pivotPoint.x=width / 2;
  ts_pivotPoint.y=height / 2;

  Coordinate ts_CurrentMousePosition;

  ts_CurrentMousePosition.x = resources->ts_CurrentMousePosition.x;
  ts_CurrentMousePosition.y = resources->ts_CurrentMousePosition.y;

  ts_Diff.x = ts_CurrentMousePosition.x - ts_pivotPoint.x;
  ts_Diff.y = ts_CurrentMousePosition.y - ts_pivotPoint.y;

  if (resources->f_OldViewModeRatio == 0)
  {
    f_firstXValue=0;
    f_firstYValue=ts_pivotPoint.y;

    f_secondXValue=ts_pivotPoint.x;
    f_secondYValue=0;
  }
  else
  {
    f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*resources->f_OldViewModeRatio);
    f_firstXValue=(ts_CurrentMousePosition.y-f_constant)/resources->f_OldViewModeRatio;
    f_firstYValue=resources->f_OldViewModeRatio *resources-> ts_CurrentMousePosition.x + f_constant;

    f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*(1 / -resources->f_OldViewModeRatio));
    f_secondXValue=(ts_CurrentMousePosition.y-f_constant)/(-1/resources->f_OldViewModeRatio);
    f_secondYValue=(-1/resources->f_OldViewModeRatio) *resources-> ts_CurrentMousePosition.x + f_constant;
  }

  const short int position_threshold = 25 + 25 * resources->f_ZoomFactor;
  if (((ts_CurrentMousePosition.x > (f_firstXValue - position_threshold))
       && (ts_CurrentMousePosition.x < (f_firstXValue + position_threshold)))
      || ((ts_CurrentMousePosition.y > (f_firstYValue - position_threshold))
          && (ts_CurrentMousePosition.y < (f_firstYValue + position_threshold))))
  {
    f_ratio=(ts_Diff.x != 0)?(ts_Diff.y/ts_Diff.x):0;
    f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*f_ratio);

    ts_firstPoint.x=0;
    ts_firstPoint.y=f_ratio*ts_firstPoint.x+f_constant;

    ts_secondPoint.x=width;
    ts_secondPoint.y=f_ratio*ts_secondPoint.x+f_constant;


    ts_thirthPoint.x = ts_firstPoint.y;
    ts_thirthPoint.y = -ts_firstPoint.x;

    ts_fourthPoint.x = ts_secondPoint.y;
    ts_fourthPoint.y = -ts_secondPoint.x;


    f_constant=(f_ratio == 0) ? 0 : ts_pivotPoint.y - (ts_pivotPoint.x*(-1 / f_ratio));

    ts_thirthPoint.x=0;
    ts_thirthPoint.y=(-1/f_ratio)*ts_thirthPoint.x+f_constant;

    ts_fourthPoint.x=width;
    ts_fourthPoint.y=(-1/f_ratio)*ts_fourthPoint.x+f_constant;

    resources->on_handle_change_callback (resources, (void *)&f_ratio);
    resources->f_OldViewModeRatio = f_ratio;

  }
  else if (((ts_CurrentMousePosition.x > (f_secondXValue - position_threshold))
            && (ts_CurrentMousePosition.x < (f_secondXValue + position_threshold)))
           || ((ts_CurrentMousePosition.y > (f_secondYValue - position_threshold))
               && (ts_CurrentMousePosition.y < (f_secondYValue + position_threshold))))
  {
    f_ratio=(ts_Diff.x != 0)?(ts_Diff.y/ts_Diff.x):0;
    f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*f_ratio);

    ts_thirthPoint.x=0;
    ts_thirthPoint.y=f_ratio*ts_thirthPoint.x+f_constant;

    ts_fourthPoint.x=width;
    ts_fourthPoint.y=f_ratio*ts_fourthPoint.x+f_constant;

    f_constant=(f_ratio == 0) ? 0 : ts_pivotPoint.y - (ts_pivotPoint.x*(-1 / f_ratio));

    ts_firstPoint.x=0;
    ts_firstPoint.y=(-1/f_ratio)*ts_firstPoint.x+f_constant;

    ts_secondPoint.x=width;
    ts_secondPoint.y=(-1/f_ratio)*ts_secondPoint.x+f_constant;

    float f_NewRatio = (-1/f_ratio);
    resources->on_handle_change_callback (resources, (void *)&f_NewRatio);

    resources->f_OldViewModeRatio = (-1/f_ratio);
  }
  else
  {
    if (resources->f_OldViewModeRatio == 0)
    {
      ts_firstPoint.x=0;
      ts_firstPoint.y=ts_pivotPoint.y;

      ts_secondPoint.x=width;
      ts_secondPoint.y=ts_pivotPoint.y;

      ts_thirthPoint.x=ts_pivotPoint.x;
      ts_thirthPoint.y=0;

      ts_fourthPoint.x=ts_pivotPoint.x;
      ts_fourthPoint.y=height;
    }
    else
    {
      f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*resources->f_OldViewModeRatio);

      ts_firstPoint.x=0;
      ts_firstPoint.y=resources->f_OldViewModeRatio*ts_firstPoint.x+f_constant;

      ts_secondPoint.x=width;
      ts_secondPoint.y=resources->f_OldViewModeRatio*ts_secondPoint.x+f_constant;

      f_constant = ts_pivotPoint.y - (ts_pivotPoint.x*(-1 / resources->f_OldViewModeRatio));

      ts_thirthPoint.x=0;
      ts_thirthPoint.y=(-1/resources->f_OldViewModeRatio)*ts_thirthPoint.x+f_constant;

      ts_fourthPoint.x=width;
      ts_fourthPoint.y=(-1/resources->f_OldViewModeRatio)*ts_fourthPoint.x+f_constant;
    }
  }


  /*--------------------------------------------------------------------------.
   | HORIZONTAL LINE                                                          |
   '--------------------------------------------------------------------------*/
  cairo_set_source_rgba (cr, 0.1, 1, 0.1, 0.8);

  cairo_move_to (cr, ts_firstPoint.x, ts_firstPoint.y);
  cairo_line_to (cr, ts_secondPoint.x, ts_secondPoint.y);

  cairo_stroke (cr);


  /*--------------------------------------------------------------------------.
   | VERTICAL LINE                                                            |
   '--------------------------------------------------------------------------*/
  cairo_set_source_rgba (cr, 1, 0.1, 1, 0.8);

  cairo_move_to (cr, ts_thirthPoint.x,ts_thirthPoint.y);
  cairo_line_to (cr, ts_fourthPoint.x,ts_fourthPoint.y);

  cairo_stroke (cr);

  return FALSE;
}


gboolean
viewer_on_mouse_scroll (UNUSED ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;
  assert (resources != NULL);

  ClutterModifierType state = clutter_event_get_state (event);
  ScrollMouseAction action = SCROLL_PREV_NEXT;

  gboolean CtrlPressed = (state & CLUTTER_CONTROL_MASK ? TRUE : FALSE);
  if (CtrlPressed) action = SCROLL_SCALE;

  switch (action)
  {
    case SCROLL_PREV_NEXT:
      viewer_on_mouse_scroll_prevnext (resources->c_Actor, event, data);
      break;
    case SCROLL_SCALE:
      viewer_on_mouse_scroll_scale (resources->c_Actor, event, data);
      break;
    case SCROLL_NONE:
      break;
  }

  return FALSE;
}


void
viewer_update_slices (List *pll_MaskSeries, double f_Z)
{
  if (list_length (pll_MaskSeries) < 1) return;

  pll_MaskSeries = list_nth (pll_MaskSeries, 1);
  while (pll_MaskSeries != NULL)
  {
    PixelData *current = pll_MaskSeries->data;
    assert (current != NULL);

    Slice *current_slice = PIXELDATA_ACTIVE_SLICE (current);

    PIXELDATA_ACTIVE_SLICE (current) =
      memory_slice_get_nth (current_slice, f_Z);

    pll_MaskSeries = list_next (pll_MaskSeries);
  }
}


gboolean
viewer_on_mouse_scroll_prevnext (UNUSED ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  if (data == NULL) return FALSE;

  ClutterScrollDirection direction = clutter_event_get_scroll_direction (event);

  Viewer *resources = (Viewer *)data;
  assert (resources != NULL);

  double f_Z = 0;
  double deltaX, deltaY;

  switch (direction)
  {
    /*------------------------------------------------------------------------.
     | SCROLL SMOOTH                                                          |
     '------------------------------------------------------------------------*/
    case CLUTTER_SCROLL_SMOOTH:

      clutter_event_get_scroll_delta(event, &deltaX, &deltaY);

      if (deltaY < 0)
      {
        PIXELDATA_ACTIVE_SLICE (resources->ps_Original) =
         memory_slice_get_previous (PIXELDATA_ACTIVE_SLICE (resources->ps_Original));
      }
      else if (deltaY > 0)
      {
        PIXELDATA_ACTIVE_SLICE (resources->ps_Original) =
          memory_slice_get_next (PIXELDATA_ACTIVE_SLICE (resources->ps_Original));
      }
      else
      {
        return FALSE;
      }
      break;
    /*------------------------------------------------------------------------.
     | SCROLL UP                                                              |
     '------------------------------------------------------------------------*/
    case CLUTTER_SCROLL_UP:
      PIXELDATA_ACTIVE_SLICE (resources->ps_Original) =
        memory_slice_get_next (PIXELDATA_ACTIVE_SLICE (resources->ps_Original));

      f_Z = PIXELDATA_ACTIVE_SLICE (resources->ps_Original)->matrix.i16_z;

      break;
     /*-----------------------------------------------------------------------.
      | SCROLL DOWN                                                           |
      '-----------------------------------------------------------------------*/
    case CLUTTER_SCROLL_DOWN:
      PIXELDATA_ACTIVE_SLICE (resources->ps_Original) =
        memory_slice_get_previous (PIXELDATA_ACTIVE_SLICE (resources->ps_Original));

      f_Z = PIXELDATA_ACTIVE_SLICE (resources->ps_Original)->matrix.i16_z;

      break;
    default: break;
  }

  f_Z = PIXELDATA_ACTIVE_SLICE (resources->ps_Original)->matrix.i16_z;

  viewer_update_slices (resources->pll_MaskSeries, f_Z);
  viewer_update_slices (resources->pll_OverlaySeries, f_Z);

  viewer_update_text (resources);
  viewer_redraw (resources, REDRAW_ALL);

  return FALSE;
}


gboolean
viewer_on_mouse_scroll_scale (UNUSED ClutterActor *actor, ClutterEvent *event,
                              gpointer data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;
  ClutterScrollDirection direction;
  direction = clutter_event_get_scroll_direction (event);
  if (direction != CLUTTER_SCROLL_SMOOTH)
    return FALSE;

  Coordinate ts_AbsoluteMousePosition;
  clutter_event_get_coords (event, &ts_AbsoluteMousePosition.x, &ts_AbsoluteMousePosition.y);

  Coordinate ts_PixelPosition;
  ts_PixelPosition = viewer_get_image_pixel_position (resources, ts_AbsoluteMousePosition);

  double deltaX, deltaY;
  clutter_event_get_scroll_delta(event, &deltaX, &deltaY);

  if (((deltaX < 0 ) || (deltaY < 0 )) && (resources->f_ZoomFactor > 0.5))
  {
    resources->f_ZoomFactor *= 0.90;
  }

  if ((deltaX > 0 ) || (deltaY > 0 ))
  {
    resources->f_ZoomFactor *= 1.1;
  }
  else
  {
    resources->f_ZoomFactor *= 1.0;
  }

  Plane ts_NewActorSize;
  ts_NewActorSize.width = resources->ts_OriginalPlane.width * resources->f_ZoomFactor;
  ts_NewActorSize.height = resources->ts_OriginalPlane.height * resources->f_ZoomFactor;

  clutter_actor_set_size (resources->c_Actor, ts_NewActorSize.width, ts_NewActorSize.height);

  Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);

  Coordinate ts_Position;
  ts_Position.x = ts_PixelPosition.x * resources->f_ZoomFactor * slice->f_ScaleFactorX;
  ts_Position.y = ts_PixelPosition.y * resources->f_ZoomFactor * slice->f_ScaleFactorY;

  Coordinate ts_ActorPosition;
  ts_ActorPosition.x = ts_AbsoluteMousePosition.x - ts_Position.x;
  ts_ActorPosition.y = ts_AbsoluteMousePosition.y - ts_Position.y;

  clutter_actor_set_position (resources->c_Actor, ts_ActorPosition.x, ts_ActorPosition.y);

  viewer_update_text (resources);
  viewer_redraw (resources, REDRAW_MINIMAL);
  return FALSE;
}


gboolean
viewer_on_mouse_release (UNUSED ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;
  assert (resources != NULL);

  ClutterModifierType c_modifiers = clutter_event_get_state (event);
  gboolean ShiftPressed = (c_modifiers & CLUTTER_SHIFT_MASK) ? TRUE : FALSE;
  if (resources->b_AutoClose_Enabled)
  {
    (ShiftPressed)
      ? viewer_draw_mask (resources, resources->ts_InitialDrawCoordinate, ACTION_ERASE)
      : viewer_draw_mask (resources, resources->ts_InitialDrawCoordinate, ACTION_SET);
  }

  resources->ts_PreviousDrawCoordinate.x = 0;
  resources->ts_PreviousDrawCoordinate.y = 0;

  resources->ts_InitialDrawCoordinate.x = 0;
  resources->ts_InitialDrawCoordinate.y = 0;

  resources->ts_CurrentMousePosition.x = 0;
  resources->ts_CurrentMousePosition.y = 0;

  return FALSE;
}


/******************************************************************************
 * VIEWER EVENT HANDLERS
 ******************************************************************************/


void
viewer_update_text (Viewer *resources)
{
  debug_functions ();

  Coordinate ts_PixelPosition;
  ts_PixelPosition = viewer_get_image_pixel_position (resources, resources->ts_CurrentMousePosition);

  PixelData *pixeldata = resources->ps_Original;
  Slice *slice = PIXELDATA_ACTIVE_SLICE (pixeldata);

  char *pc_PixelValue = pixeldata_get_pixel_value_as_string (pixeldata, ts_PixelPosition);

  char *text = calloc (1, 110);
  sprintf (text, "Slice:\t\t %d\n"
                 "Window/Level:\t %d / %d\n"
                 "Position:\t\t %.0f , %.0f\n"
                 "Zoom:\t\t %.0f%%\n"
                 "Value:\t\t %s\n"
                 "Macro:\t\t %s\n",
                 slice->matrix.i16_z,
                 pixeldata->ts_WWWL.i32_windowWidth, pixeldata->ts_WWWL.i32_windowLevel,
                 ts_PixelPosition.x, ts_PixelPosition.y,
                 resources->f_ZoomFactor * 100,
                 pc_PixelValue,
                 (resources->is_recording) ? "Recording" : "");

  free (pc_PixelValue);
  clutter_text_set_text (CLUTTER_TEXT (resources->c_SliceInfo), text);
  free (text);
}


void
viewer_on_key_press (Viewer *resources, GdkEventKey *event)
{
  debug_functions ();
  debug_events ();

  assert (resources != NULL);

  (event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R)
    //? viewer_set_view_mode (resources, 1)
    ? viewer_set_view_mode (resources, 0)
    : viewer_set_view_mode (resources, 0);
}



void
viewer_on_key_release (Viewer *resources, GdkEventKey *event)
{
  debug_functions ();
  debug_events ();

  assert (resources != NULL);

  if (event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R)
  {
    viewer_set_view_mode (resources, 0);
  }
}


/******************************************************************************
 * VIEWER HELPER FUNCTIONS
 ******************************************************************************/


void
viewer_redraw_child_series (Viewer *resources, List *pll_Series, int i32_Width,
                            int i32_Height, RedrawMode redraw_mode)
{
  pll_Series = list_nth (pll_Series, 1);

  ClutterActor *child;
  PixelData *ps_Data;

  while (pll_Series != NULL)
  {
    ps_Data = pll_Series->data;
    assert (ps_Data != NULL);

    // Prevent doing too many full redraws.
    child = (redraw_mode != REDRAW_ACTIVE || resources->ps_ActiveMask == pll_Series->data)
      ? viewer_create_actor_from_pixeldata (ps_Data, i32_Width, i32_Height, 1)
      : viewer_create_actor_from_pixeldata (ps_Data, i32_Width, i32_Height, 0);

    // Skip a broken layer.
    if (child == NULL)
    {
      pll_Series = list_next (pll_Series);
      continue;
    }

    clutter_actor_add_child (resources->c_Actor, child);
    pll_Series = list_next (pll_Series);
  }
}


void
viewer_redraw (Viewer *resources, RedrawMode redraw_mode)
{
  debug_functions ();

  /*--------------------------------------------------------------------------.
   | DETERMINE THE SCALING FACTOR                                             |
   '--------------------------------------------------------------------------*/
  Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
  assert (slice != NULL);

  int i32_Width  = slice->matrix.i16_x;
  int i32_Height = slice->matrix.i16_y;

  /*--------------------------------------------------------------------------.
   | NON-MINIMAL REDRAW                                                       |
   '--------------------------------------------------------------------------*/
  if (redraw_mode != REDRAW_MINIMAL)
  {
    assert (resources->c_Stage != NULL);

    /*------------------------------------------------------------------------.
     | REMOVE OLD DRAWING DATA                                                |
     '------------------------------------------------------------------------*/
    ClutterActor *child = clutter_actor_get_first_child (resources->c_Actor);
    while (child != NULL)
    {
      ClutterActor *temp = clutter_actor_get_next_sibling (child);
      clutter_actor_destroy (child);
      child = temp;
    }

    /*------------------------------------------------------------------------.
     | DISPLAYING BASE IMAGE                                                  |
     '------------------------------------------------------------------------*/
    ClutterContent *c_Slice = clutter_image_new ();
    assert (c_Slice != NULL);

    ClutterActor *c_BaseImage;
    c_BaseImage = viewer_create_actor_from_pixeldata (resources->ps_Original,
                                                      slice->matrix.i16_x,
                                                      slice->matrix.i16_y,
                                                      (redraw_mode == REDRAW_ALL));

    clutter_actor_add_child (resources->c_Actor, c_BaseImage);
    /*------------------------------------------------------------------------.
     | APPLY PARENT SETTINGS                                                  |
     '------------------------------------------------------------------------*/
    clutter_actor_set_width (resources->c_Actor, slice->matrix.i16_x);
    clutter_actor_set_height (resources->c_Actor, slice->matrix.i16_y);
    clutter_actor_set_scale (resources->c_Actor,
                             slice->f_ScaleFactorX * resources->f_ZoomFactor,
                             slice->f_ScaleFactorY * resources->f_ZoomFactor);

    /*------------------------------------------------------------------------.
     | DISPLAYING OVERLAYS                                                    |
     '------------------------------------------------------------------------*/
    viewer_redraw_child_series (resources, resources->pll_OverlaySeries,
    				i32_Width, i32_Height, redraw_mode);

    /*------------------------------------------------------------------------.
     | DISPLAYING MASKS                                                       |
     '------------------------------------------------------------------------*/
    viewer_redraw_child_series (resources, resources->pll_MaskSeries,
    				i32_Width, i32_Height, redraw_mode);



  }

  /*--------------------------------------------------------------------------.
   | MINIMAL REDRAW                                                           |
   '--------------------------------------------------------------------------*/
  else
  {
    Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
    clutter_actor_set_width (CLUTTER_ACTOR (resources->c_Actor), slice->matrix.i16_x);
    clutter_actor_set_height (CLUTTER_ACTOR (resources->c_Actor), slice->matrix.i16_y);
    clutter_actor_set_scale (resources->c_Actor,
                             slice->f_ScaleFactorX * resources->f_ZoomFactor,
                             slice->f_ScaleFactorY * resources->f_ZoomFactor);

    ClutterActor *child = clutter_actor_get_first_child (resources->c_Actor);
    while (child != NULL)
    {
      clutter_actor_set_width (child, clutter_actor_get_width (resources->c_Actor));
      clutter_actor_set_height (child, clutter_actor_get_height (resources->c_Actor));
      child = clutter_actor_get_next_sibling (child);
    }
  }

  /*------------------------------------------------------------------------.
   | DISPLAYING HANDLES                                                     |
   '------------------------------------------------------------------------*/
  if (resources->c_Handles != NULL && resources->b_ViewMode_Enabled == 1)
  {
    ClutterContent *c_Canvas = clutter_actor_get_content (resources->c_Handles);
    if (c_Canvas == NULL) return;

    Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
    float f_width = (i32_Width * slice->f_ScaleFactorX * resources->f_ZoomFactor);
    float f_height = (i32_Height * slice->f_ScaleFactorY * resources->f_ZoomFactor);

    clutter_actor_set_size (resources->c_Handles, f_width, f_height);
    clutter_canvas_set_size (CLUTTER_CANVAS (c_Canvas), f_width, f_height);

    Coordinate ts_ActorPosition;
    clutter_actor_get_position (resources->c_Actor, &ts_ActorPosition.x, &ts_ActorPosition.y);
    clutter_actor_set_position (resources->c_Handles, ts_ActorPosition.x, ts_ActorPosition.y);
  }
}


ClutterActor*
viewer_create_actor_from_pixeldata (PixelData *pixeldata, int width,
                                    int height, short int do_full_redraw)
{
  debug_functions ();

  assert (pixeldata != NULL);

  Slice *slice = PIXELDATA_ACTIVE_SLICE (pixeldata);
  assert (slice != NULL);

  ClutterActor *c_Mask = clutter_actor_new ();
  if (c_Mask == NULL) return NULL;

  ClutterColor c_TransparentColor = { 0, 0, 0, 0 };
  clutter_actor_set_background_color (c_Mask, &c_TransparentColor);

  ClutterContent *mask_Content = clutter_image_new ();

  // Don't ask for a new RGB pixel buffer when this is not needed.
  unsigned int* pixbuf = (do_full_redraw || PIXELDATA_RGB (pixeldata) == NULL)
    ? pixeldata_create_rgb_pixbuf (pixeldata)
    : PIXELDATA_RGB (pixeldata);

  if (pixbuf != NULL)
  {
    GError *error = NULL;
    if (!clutter_image_set_data (CLUTTER_IMAGE (mask_Content),
                                 (guint8 *)pixbuf,
                                 COGL_PIXEL_FORMAT_RGBA_8888,
                                 slice->matrix.i16_x,
                                 slice->matrix.i16_y,
                                 (slice->matrix.i16_x * 4),
                                 &error))
    {
      debug_warning ("Could not load pixels to buffer: %s\n", error->message);
    }

    clutter_actor_set_content (c_Mask, CLUTTER_CONTENT (mask_Content));
    g_object_unref (mask_Content);
  }

  clutter_actor_set_opacity (c_Mask, pixeldata_get_alpha (pixeldata));
  clutter_actor_set_width (c_Mask, width);
  clutter_actor_set_height (c_Mask, height);
  clutter_actor_set_content_scaling_filters (c_Mask, SCALING_FILTER, SCALING_FILTER);

  return c_Mask;
}


Coordinate
viewer_get_image_pixel_position (Viewer *resources, Coordinate ts_MousePosition)
{
  debug_functions ();

  Coordinate co_PixelPosition;
  Plane ts_ActorSize;
  Coordinate ts_ActorPosition;
  float f_WindowScale_Factor;

  clutter_actor_get_transformed_size (resources->c_Actor, &ts_ActorSize.width, &ts_ActorSize.height);
  clutter_actor_get_transformed_position (resources->c_Actor, &ts_ActorPosition.x, &ts_ActorPosition.y);

  f_WindowScale_Factor = resources->ts_OriginalPlane.width / ts_ActorSize.width;

  //Slice *slice = VIEWER_ACTIVE_SLICE (resources);

  co_PixelPosition.x = f_WindowScale_Factor * (ts_MousePosition.x - ts_ActorPosition.x);// slice->f_ScaleFactorX;
  f_WindowScale_Factor = resources->ts_OriginalPlane.height / ts_ActorSize.height;

  co_PixelPosition.y = f_WindowScale_Factor * (ts_MousePosition.y - ts_ActorPosition.y);// slice->f_ScaleFactorY;
  return co_PixelPosition;
}


Coordinate
viewer_get_canvas_pixel_position (Viewer *resources, Coordinate ts_MousePosition)
{
  debug_functions ();

  Coordinate co_PixelPosition;
  Plane ts_ActorSize;
  Coordinate ts_ActorPosition;

  clutter_actor_get_size (resources->c_Handles, &ts_ActorSize.width, &ts_ActorSize.height);
  clutter_actor_get_position (resources->c_Handles, &ts_ActorPosition.x, &ts_ActorPosition.y);

  co_PixelPosition.x = ts_MousePosition.x - ts_ActorPosition.x;
  co_PixelPosition.y = ts_MousePosition.y - ts_ActorPosition.y;

  return co_PixelPosition;
}


void
viewer_draw_mask (Viewer *resources, Coordinate ts_MousePosition, PixelAction te_Action)
{
  debug_functions ();

  assert (resources != NULL);
  if (resources->ts_ActivePainter == NULL)
  {
    debug_warning ("There's no active painter.", NULL);
    return;
  }

  Plugin *plugin = resources->ts_ActivePainter;
  if (plugin->fp_Callback == NULL)
  {
    debug_warning ("The active painter doesn't have a callback.", NULL);
    return;
  }

  Coordinate ts_PixelPosition;
  ts_PixelPosition = viewer_get_image_pixel_position (resources, ts_MousePosition);

  // The pixel position should be rounded to a pixel. The best visual result is
  // cutting off the decimal part.
  ts_PixelPosition.x = (int)ts_PixelPosition.x;
  ts_PixelPosition.y = (int)ts_PixelPosition.y;

  // Record the action when needed.
  if (resources->is_recording == 1)
  {
    ViewerCommand *command = calloc (1, sizeof (ViewerCommand));
    assert (command != NULL);

    command->plugin = resources->ts_ActivePainter;
    command->coordinate = ts_PixelPosition;
    command->size = plugin->i32_Size;
    command->value = plugin->i32_Value;
    command->action = te_Action;

    resources->pll_Replay = list_append (resources->pll_Replay, command);
  }

  switch (plugin->te_Type)
  {
    case PLUGIN_TYPE_BRUSH:
      {
	pixeldata_apply_brush (resources->ps_Original, resources->ps_ActiveMask,
			       resources->ps_ActiveSelection, resources->ts_PreviousDrawCoordinate,
			       ts_PixelPosition, plugin->i32_Size, plugin->i32_Value, te_Action,
			       plugin->fp_Callback);
      }
      break;
    case PLUGIN_TYPE_SELECTION:
      {
        Coordinate ts_InitialPixelPosition;
        ts_InitialPixelPosition = viewer_get_image_pixel_position (resources, resources->ts_InitialDrawCoordinate);

        ts_InitialPixelPosition.x = abs (ts_InitialPixelPosition.x);
        ts_InitialPixelPosition.y = abs (ts_InitialPixelPosition.y);

        ((PluginSelection *)plugin)->fp_Callback (resources->ps_Original, resources->ps_ActiveMask,
                                                  resources->ps_ActiveSelection, ts_InitialPixelPosition,
                                                  ts_PixelPosition, plugin->i32_Value, te_Action);
      }
      break;
    case PLUGIN_TYPE_LINE:
      {
        ((PluginLine *)plugin)->fp_Callback (resources->ps_Original, resources->ps_ActiveMask,
                                             resources->ps_ActiveSelection, ts_PixelPosition,
                                             &resources->pll_PolygonPoints, plugin->i32_Value, te_Action);
      }
      break;
    default:
      {
        debug_warning ("Encountered an unsupported plugin type.", NULL);
      }
      break;
  }

  resources->ts_PreviousDrawCoordinate = ts_PixelPosition;

  if (resources->on_pixel_paint_callback != NULL)
  {
    resources->on_pixel_paint_callback (resources, NULL);
  }

  viewer_redraw (resources, REDRAW_ACTIVE);
}


void
viewer_resize (Viewer *resources, int width, int height)
{
  debug_functions ();
  assert (resources != NULL);

  viewer_set_optimal_fit (resources);

  int i32_Width = clutter_actor_get_width (resources->c_Actor);
  int i32_Height = clutter_actor_get_height (resources->c_Actor);

  // Only position the actor when it's smaller than the stage.
  if ((i32_Width - 2) > width && (i32_Height - 2) > height) return;

  if (resources->ts_ScaledPlane.width < width || resources->ts_ScaledPlane.height < height)
  {
    Coordinate ts_Position;
    ts_Position.x = (width - resources->ts_ScaledPlane.width * resources->f_ZoomFactor) / 2;
    ts_Position.y = (height - resources->ts_ScaledPlane.height * resources->f_ZoomFactor) / 2;
    clutter_actor_set_position (resources->c_Actor, ts_Position.x, ts_Position.y);
    viewer_redraw (resources, REDRAW_MINIMAL);
  }
}


gboolean
viewer_coordinate_within_image (Viewer *resources, Coordinate ts_Point)
{
  assert (resources != NULL);

  Slice *slice = VIEWER_ACTIVE_SLICE (resources);
  Plane ts_AbsolutePixelSize;
  ts_AbsolutePixelSize.width = slice->matrix.i16_x * slice->f_ScaleFactorX * resources->f_ZoomFactor;
  ts_AbsolutePixelSize.height = slice->matrix.i16_y * slice->f_ScaleFactorY * resources->f_ZoomFactor;

  Coordinate ts_ActorPosition;
  clutter_actor_get_position (resources->c_Actor, &ts_ActorPosition.x, &ts_ActorPosition.y);
  if (ts_Point.x < ts_ActorPosition.x || ts_Point.y < ts_ActorPosition.y) return FALSE;
  if (ts_Point.x > ts_ActorPosition.x + ts_AbsolutePixelSize.width) return FALSE;
  if (ts_Point.y > ts_ActorPosition.y + ts_AbsolutePixelSize.height) return FALSE;

  return FALSE;
}


gboolean
viewer_on_mouse_press (UNUSED ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  ClutterModifierType c_modifiers = clutter_event_get_state (event);
  guint button_pressed = clutter_event_get_button (event);

  Viewer *resources = (Viewer *)data;

  // Always update the ViewPortCoordinates.
  clutter_event_get_coords (event, &resources->ts_ViewPortCoordinates.x,
                            &resources->ts_ViewPortCoordinates.y);

  resources->ts_InitialDrawCoordinate = resources->ts_ViewPortCoordinates;

  /*--------------------------------------------------------------------------.
   | LEFT MOUSE BUTTON CLICK                                                  |
   '--------------------------------------------------------------------------*/
  if (button_pressed == 1)
  {
    if (!(c_modifiers & CLUTTER_CONTROL_MASK))
    {
      PixelAction te_Action =
        (c_modifiers & CLUTTER_SHIFT_MASK)
          ? ACTION_ERASE
          : ACTION_SET;

      resources->ts_PreviousDrawCoordinate.x = 0;
      resources->ts_PreviousDrawCoordinate.y = 0;

      if (resources->b_ViewMode_Enabled == 0)
      {
	debug_extra ("%s: calling viewer_draw_mask.", __func__);
        viewer_draw_mask (resources, resources->ts_ViewPortCoordinates, te_Action);
      }
    }
  }

  /*--------------------------------------------------------------------------.
   | RIGHT MOUSE BUTTON CLICK                                                 |
   '--------------------------------------------------------------------------*/
  else if (button_pressed == 3)
  {
    clutter_event_get_coords (event, &resources->ts_WindowLevelBasePoint.x,
                              &resources->ts_WindowLevelBasePoint.y);

    resources->ts_PreviousMousePosition.x = resources->ts_WindowLevelBasePoint.x;
    resources->ts_PreviousMousePosition.y = resources->ts_WindowLevelBasePoint.y;

    list_free_all (resources->pll_PolygonPoints, free);
    resources->pll_PolygonPoints = NULL;
  }

  return FALSE;
}


gboolean
viewer_on_mouse_move (UNUSED ClutterActor *actor, ClutterEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  ClutterModifierType c_modifiers = clutter_event_get_state (event);
  Viewer *resources = (Viewer *)data;

  Coordinate ts_CurrentMousePosition;
  clutter_event_get_coords (event, &ts_CurrentMousePosition.x, &ts_CurrentMousePosition.y);

  /*--------------------------------------------------------------------------.
   | LEFT MOUSE BUTTON CLICK | DRAGGING                                       |
   '--------------------------------------------------------------------------*/
  if (c_modifiers & CLUTTER_BUTTON1_MASK)
  {
    gboolean CtrlPressed = (c_modifiers & CLUTTER_CONTROL_MASK ? TRUE : FALSE);

    /*------------------------------------------------------------------------.
     | CTRL PRESSED | DRAGGING                                                |
     '------------------------------------------------------------------------*/
    if (CtrlPressed)
    {
      Coordinate ts_ActorPosition;
      clutter_actor_get_position (resources->c_Actor, &ts_ActorPosition.x, &ts_ActorPosition.y);

      ts_ActorPosition.x -= (resources->ts_ViewPortCoordinates.x - ts_CurrentMousePosition.x);
      ts_ActorPosition.y -= (resources->ts_ViewPortCoordinates.y - ts_CurrentMousePosition.y);
      clutter_actor_set_position (resources->c_Actor, ts_ActorPosition.x, ts_ActorPosition.y);

      resources->ts_ViewPortCoordinates.x = ts_CurrentMousePosition.x;
      resources->ts_ViewPortCoordinates.y = ts_CurrentMousePosition.y;

      viewer_redraw (resources, REDRAW_MINIMAL);
    }

    /*------------------------------------------------------------------------.
     | ALT PRESSED | CHANGE HANDLES                                           |
     '------------------------------------------------------------------------*/
    else if (resources->b_ViewMode_Enabled == 1)
    {
      resources->ts_CurrentMousePosition = viewer_get_canvas_pixel_position (resources, ts_CurrentMousePosition);
      ClutterContent *c_Content = clutter_actor_get_content (resources->c_Handles);
      clutter_content_invalidate (CLUTTER_CONTENT (c_Content));
    }

    /*------------------------------------------------------------------------.
     | DEFAULT | MASK DRAWING                                                 |
     '------------------------------------------------------------------------*/
    else
    {
      gboolean ShiftPressed = (c_modifiers & CLUTTER_SHIFT_MASK ? TRUE : FALSE);
      PixelAction te_Action = (ShiftPressed) ? ACTION_ERASE : ACTION_SET;

      viewer_draw_mask (resources, ts_CurrentMousePosition, te_Action);
    }
  }

  /*--------------------------------------------------------------------------.
   | RIGHT MOUSE BUTTON CLICK | WINDOW LEVELING                               |
   '--------------------------------------------------------------------------*/
  else if (c_modifiers & CLUTTER_BUTTON3_MASK && resources->ps_ActiveLayer != NULL)
  {
    Coordinate ts_AccellerationFactor;
    Coordinate ts_Diff;
    WWWL ts_WWWL;


    // to create a non linear relation between mouse movement and reaction on
    // mouse action, the delta between the start point and current mouse position
    // determines a certain acceleration factor.

    ts_AccellerationFactor.x = abs(ts_CurrentMousePosition.x - resources->ts_WindowLevelBasePoint.x);
    ts_AccellerationFactor.y = abs(ts_CurrentMousePosition.y - resources->ts_WindowLevelBasePoint.y);

    ts_AccellerationFactor.x /= 5;
    ts_AccellerationFactor.y /= 5;

    ts_AccellerationFactor.x = (ts_AccellerationFactor.x < 5) ? 1 : ts_AccellerationFactor.x;
    ts_AccellerationFactor.y = (ts_AccellerationFactor.y < 5) ? 1 : ts_AccellerationFactor.y;

    // Get the difference in X and Y.

    ts_Diff.x = (ts_CurrentMousePosition.x - resources->ts_PreviousMousePosition.x)*ts_AccellerationFactor.x;
    ts_Diff.y = (ts_CurrentMousePosition.y - resources->ts_PreviousMousePosition.y)*ts_AccellerationFactor.y;

    resources->ts_PreviousMousePosition.x = ts_CurrentMousePosition.x;
    resources->ts_PreviousMousePosition.y = ts_CurrentMousePosition.y;

    PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, resources->ps_ActiveLayer);

    ts_WWWL.i32_windowWidth = (int)(ts_Diff.x);
    ts_WWWL.i32_windowLevel = (int)(ts_Diff.y);


    pixeldata_calculate_window_width_level (pixeldata, ts_WWWL.i32_windowWidth, ts_WWWL.i32_windowLevel);

    // Trigger a full redraw.
    viewer_redraw (resources, REDRAW_ALL);

    // Execute the window/level update callback.

    if (resources->on_window_level_change_callback != NULL)
    {
      resources->on_window_level_change_callback (resources, &ts_WWWL);
    }

  }

  resources->ts_CurrentMousePosition = ts_CurrentMousePosition;
  viewer_update_text (resources);

  if (resources->on_focus_change_callback != NULL)
  {
    Coordinate ts_PixelPosition;
    ts_PixelPosition = viewer_get_image_pixel_position (resources, ts_CurrentMousePosition);
    resources->on_focus_change_callback (resources, &ts_PixelPosition);
  }

  return FALSE;
}


gboolean
viewer_on_enter_stage (UNUSED GtkWidget *widget, UNUSED GdkEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;

  GdkWindow *window = gtk_widget_get_parent_window (resources->c_Embed);
  GdkDisplay *display =  gdk_display_get_default ();
  GdkCursor* cursor = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);
  gdk_window_set_cursor (window, cursor);
  g_object_unref (cursor);

  return FALSE;
}


gboolean
viewer_on_leave_stage (UNUSED GtkWidget *widget, UNUSED GdkEvent *event, gpointer data)
{
  debug_functions ();
  debug_events ();

  Viewer *resources = (Viewer *)data;

  GdkWindow *window = gtk_widget_get_parent_window (resources->c_Embed);
  if (window != NULL)
    gdk_window_set_cursor (window, NULL);

  PixelData *pixeldata = NULL;
  /*
  if (resources->ps_ActiveLayer == NULL)
    pixeldata = viewer_get_pixeldata_for_serie (resources, resources->ps_ActiveLayer);
  else
  */
  pixeldata = VIEWER_PIXELDATA_ORIGINAL (resources);

  Slice *slice = VIEWER_ACTIVE_SLICE (resources);
  char *text = calloc (1, 110);
  sprintf (text, "Slice:\t\t %d\n"
                 "Window/Level:\t %d / %d\n"
                 "Position:\nZoom:\nValue:\n"
                 "Macro:\t\t %s\n",
                 slice->matrix.i16_z,
                 pixeldata->ts_WWWL.i32_windowWidth, pixeldata->ts_WWWL.i32_windowLevel,
                 (resources->is_recording) ? "Recording" : "");

  clutter_text_set_text (CLUTTER_TEXT (resources->c_SliceInfo), text);
  free (text);

  return FALSE;
}

/******************************************************************************
 * OTHER STUFF
 ******************************************************************************/

void
viewer_set_optimal_fit (Viewer *resources)
{
  debug_functions ();
  assert (resources != NULL);

  float f_Border = 50;

  double width = gtk_widget_get_allocated_width (resources->c_Embed);
  double height = gtk_widget_get_allocated_height (resources->c_Embed);

  if (width > 2 && height > 2)
  {

    double f_ScaleFactorWidth = (width - 2 * f_Border) / resources->ts_ScaledPlane.width;
    double f_ScaleFactorHeight = (height - 2 * f_Border) / resources->ts_ScaledPlane.height;

    resources->f_ZoomFactor = (f_ScaleFactorWidth > f_ScaleFactorHeight)
      ? f_ScaleFactorHeight
      : f_ScaleFactorWidth;
  }
}

void v_viewer_set_image_orientation_direction(Viewer *pt_Viewport, char *pc_Top, char *pc_Bottom, char *pc_Left, char *pc_Right)
{
  if (pt_Viewport == NULL)
  {
    return;
  }

  if ((pt_Viewport->c_SliceOrientationTop == NULL) || (pt_Viewport->c_SliceOrientationBottom == NULL) || (pt_Viewport->c_SliceOrientationLeft == NULL) || (pt_Viewport->c_SliceOrientationRight == NULL))
  {
    return;
  }

  clutter_text_set_text (CLUTTER_TEXT (pt_Viewport->c_SliceOrientationTop), pc_Top);
  clutter_text_set_text (CLUTTER_TEXT (pt_Viewport->c_SliceOrientationBottom), pc_Bottom);
  clutter_text_set_text (CLUTTER_TEXT (pt_Viewport->c_SliceOrientationLeft), pc_Left);
  clutter_text_set_text (CLUTTER_TEXT (pt_Viewport->c_SliceOrientationRight), pc_Right);
}



void
viewer_initialize (Viewer *resources, Serie *ts_Original, Serie *ts_Mask, List *pll_Overlays,
                   Vector3D ts_NormalVector, Vector3D ts_PivotPoint,
                   Vector3D ts_UpVector)
{
  debug_functions ();

  assert (ts_Original != NULL);
  assert (ts_Mask != NULL);

  /*--------------------------------------------------------------------------.
   | INITIALIZE VIEWER RESOURCES                                              |
   '--------------------------------------------------------------------------*/
  assert (resources != NULL);

  resources->ts_NormalVector.x = ts_NormalVector.x;
  resources->ts_NormalVector.y = ts_NormalVector.y;
  resources->ts_NormalVector.z = ts_NormalVector.z;

  resources->ts_PivotPoint.x = ts_PivotPoint.x;
  resources->ts_PivotPoint.y = ts_PivotPoint.y;
  resources->ts_PivotPoint.z = ts_PivotPoint.z;

  resources->ts_UpVector.x = ts_UpVector.x;
  resources->ts_UpVector.y = ts_UpVector.y;
  resources->ts_UpVector.z = ts_UpVector.z;

  resources->f_ZoomFactor = 1.0;
  resources->is_recording = 0;

  int i32_windowWidth = ts_Original->i32_MaximumValue - ts_Original->i32_MinimumValue;
  int i32_windowLevel = i32_windowWidth / 2;


  Slice *slice = memory_slice_new (ts_Original);
  assert (slice != NULL);

  memory_slice_set_NormalVector(slice,&resources->ts_NormalVector);
  memory_slice_set_PivotPoint(slice,&resources->ts_PivotPoint);
  memory_slice_set_UpVector(slice,&resources->ts_UpVector);

  // Set the default window and level.
  Serie *ps_serie=slice->serie;
  PixelDataLookupTable *default_lut;

  if (ps_serie->i32_MaximumValue < 16)
  {
    default_lut = pixeldata_lookup_table_get_default_mask();
    i32_windowWidth = 15;
    i32_windowLevel = 8;
  }
  else
  {
    default_lut = pixeldata_lookup_table_get_default ();
  }

  resources->ps_Original = pixeldata_new_with_lookup_table (default_lut, i32_windowWidth ,
                                                            i32_windowLevel, slice, ts_Original);

  assert (resources->ps_Original != NULL);

  // Set the default width and height.
  resources->ts_OriginalPlane.width = slice->matrix.i16_x;
  resources->ts_OriginalPlane.height = slice->matrix.i16_y;

  resources->ts_ScaledPlane.width = slice->matrix.i16_x * slice->f_ScaleFactorX;
  resources->ts_ScaledPlane.height = slice->matrix.i16_y * slice->f_ScaleFactorY;





  // Set up the display slice resources for the mask.
  viewer_add_mask_serie (resources, ts_Mask);
  resources->ps_ActiveMask = list_last (resources->pll_MaskSeries)->data;

  // Add overlays.
  while (pll_Overlays != NULL)
  {
    viewer_add_overlay_serie (resources, pll_Overlays->data);
    pll_Overlays = list_next (pll_Overlays);
  }


  /*--------------------------------------------------------------------------.
   | CLUTTER AND GTK INITALIZATION                                            |
   '--------------------------------------------------------------------------*/

  assert (resources->c_Embed != NULL);
  assert (resources->c_Stage != NULL);
  assert (resources->c_Actor != NULL);
  assert (resources->c_SliceInfo != NULL);

  assert (resources->c_Handles != NULL);

  clutter_actor_set_width (resources->c_Actor, slice->matrix.i16_x);
  clutter_actor_set_height (resources->c_Actor, slice->matrix.i16_y);
  clutter_actor_set_content_scaling_filters (resources->c_Actor, SCALING_FILTER, SCALING_FILTER);

  viewer_set_optimal_fit (resources);

  float f_ActorWidth, f_ActorHeight;
  f_ActorWidth = resources->ts_OriginalPlane.width * resources->f_ZoomFactor* slice->f_ScaleFactorX;
  f_ActorHeight = resources->ts_OriginalPlane.height * resources->f_ZoomFactor* slice->f_ScaleFactorY;
  clutter_actor_set_size (resources->c_Actor, f_ActorWidth, f_ActorHeight);

  float f_StageWidth, f_StageHeight;
  f_StageWidth = clutter_actor_get_width(resources->c_Stage);
  f_StageHeight = clutter_actor_get_height(resources->c_Stage);


  float f_StartPositionX, f_StartPositionY;

  f_StartPositionX = (f_StageWidth - f_ActorWidth)  / 2;
  f_StartPositionY = (f_StageHeight - f_ActorHeight) / 2;
  clutter_actor_set_position(resources->c_Actor, f_StartPositionX, f_StartPositionY );




  /*--------------------------------------------------------------------------.
   | CLUTTER-GTK PARTS                                                        |
   '--------------------------------------------------------------------------*/

  viewer_set_slice (resources, 0);


}

Viewer*
viewer_new (Serie *ts_Original, Serie *ts_Mask, List *pll_Overlays,
            Vector3D ts_NormalVector, Vector3D ts_PivotPoint,
            Vector3D ts_UpVector)
{
  debug_functions ();

  assert (ts_Original != NULL);
  assert (ts_Mask != NULL);

  /*--------------------------------------------------------------------------.
   | INITIALIZE VIEWER RESOURCES                                              |
   '--------------------------------------------------------------------------*/
  Viewer *resources;
  resources = calloc (1, sizeof (Viewer));
  assert (resources != NULL);

  resources->ts_NormalVector.x = ts_NormalVector.x;
  resources->ts_NormalVector.y = ts_NormalVector.y;
  resources->ts_NormalVector.z = ts_NormalVector.z;

  resources->ts_PivotPoint.x = ts_PivotPoint.x;
  resources->ts_PivotPoint.y = ts_PivotPoint.y;
  resources->ts_PivotPoint.z = ts_PivotPoint.z;

  resources->ts_UpVector.x = ts_UpVector.x;
  resources->ts_UpVector.y = ts_UpVector.y;
  resources->ts_UpVector.z = ts_UpVector.z;

  resources->f_ZoomFactor = 1.0;
  resources->is_recording = 0;

  int i32_windowWidth = ts_Original->i32_MaximumValue - ts_Original->i32_MinimumValue;
  int i32_windowLevel = i32_windowWidth / 2;


  Slice *slice = memory_slice_new (ts_Original);
  assert (slice != NULL);




  memory_slice_set_NormalVector(slice,&resources->ts_NormalVector);
  memory_slice_set_PivotPoint(slice,&resources->ts_PivotPoint);
  memory_slice_set_UpVector(slice,&resources->ts_UpVector);

  // Set the default window and level.

  PixelDataLookupTable *default_lut;

  if (ts_Original->i32_MaximumValue < 16)
  {
    default_lut = pixeldata_lookup_table_get_default_mask();
    i32_windowWidth = 15;
    i32_windowLevel = 8;
  }
  else
  {
    default_lut = pixeldata_lookup_table_get_default ();
  }

  resources->ps_Original = pixeldata_new_with_lookup_table (default_lut, i32_windowWidth ,
                                                            i32_windowLevel, slice, ts_Original);

  assert (resources->ps_Original != NULL);

  // Set the default width and height.
  resources->ts_OriginalPlane.width = slice->matrix.i16_x;
  resources->ts_OriginalPlane.height = slice->matrix.i16_y;

  resources->ts_ScaledPlane.width = slice->matrix.i16_x * slice->f_ScaleFactorX;
  resources->ts_ScaledPlane.height = slice->matrix.i16_y * slice->f_ScaleFactorY;

  // Set up the display slice resources for the mask.
  viewer_add_mask_serie (resources, ts_Mask);
  resources->ps_ActiveMask = list_last (resources->pll_MaskSeries)->data;

  // Add overlays.
  while (pll_Overlays != NULL)
  {
    viewer_add_overlay_serie (resources, pll_Overlays->data);
    pll_Overlays = list_next (pll_Overlays);
  }

  /*--------------------------------------------------------------------------.
   | CLUTTER AND GTK INITALIZATION                                            |
   '--------------------------------------------------------------------------*/

  resources->c_Embed = gtk_clutter_embed_new ();
  assert (resources->c_Embed != NULL);

  resources->c_Stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (resources->c_Embed));
  assert (resources->c_Stage != NULL);

  resources->c_Actor = clutter_actor_new ();
  assert (resources->c_Actor != NULL);

  resources->c_SliceInfo = clutter_text_new_full ("Sans 10",
                                                  "Slice:\n"
                                                  "Window/Level:\n"
                                                  "Position:\n"
                                                  "Value:\n",
                                                  CLUTTER_COLOR_White);

  resources->c_SliceOrientationTop    = clutter_text_new_full("Sans 12", "", CLUTTER_COLOR_Yellow);
  resources->c_SliceOrientationBottom = clutter_text_new_full("Sans 12", "", CLUTTER_COLOR_Yellow);
  resources->c_SliceOrientationLeft   = clutter_text_new_full("Sans 12", "", CLUTTER_COLOR_Yellow);
  resources->c_SliceOrientationRight  = clutter_text_new_full("Sans 12", "", CLUTTER_COLOR_Yellow);

  assert (resources->c_SliceInfo != NULL);

  resources->c_Handles = clutter_actor_new ();
  assert (resources->c_Handles != NULL);

  ClutterContent *c_Canvas = clutter_canvas_new ();
  clutter_actor_set_content (resources->c_Handles, CLUTTER_CONTENT (c_Canvas));
  g_signal_connect (c_Canvas, "draw", G_CALLBACK (viewer_on_redraw_update_handles), resources);

  clutter_actor_set_background_color (resources->c_Stage, CLUTTER_COLOR_Black);
  clutter_actor_set_width (resources->c_Actor, slice->matrix.i16_x);
  clutter_actor_set_height (resources->c_Actor, slice->matrix.i16_y);
  clutter_actor_set_content_scaling_filters (resources->c_Actor, SCALING_FILTER, SCALING_FILTER);

  /*--------------------------------------------------------------------------.
   | CLUTTER-GTK PARTS                                                        |
   '--------------------------------------------------------------------------*/

  clutter_actor_add_child (resources->c_Stage, resources->c_Actor);

  clutter_actor_insert_child_above (resources->c_Stage, resources->c_SliceInfo, NULL);

  clutter_actor_insert_child_above (resources->c_Stage, resources->c_SliceOrientationTop, NULL);
  clutter_actor_insert_child_above (resources->c_Stage, resources->c_SliceOrientationBottom, NULL);
  clutter_actor_insert_child_above (resources->c_Stage, resources->c_SliceOrientationLeft, NULL);
  clutter_actor_insert_child_above (resources->c_Stage, resources->c_SliceOrientationRight, NULL);

  clutter_actor_insert_child_above (resources->c_Stage, resources->c_Handles, NULL);

  // Align the text widget to the bottom left corner.
  clutter_actor_set_pivot_point (resources->c_SliceInfo, 0.0, 1.0);

  clutter_actor_set_pivot_point (resources->c_SliceOrientationTop, 0.5, 0.0);
  clutter_actor_set_pivot_point (resources->c_SliceOrientationBottom, 0.5, 1.0);
  clutter_actor_set_pivot_point (resources->c_SliceOrientationLeft, 0.0, 0.5);
  clutter_actor_set_pivot_point (resources->c_SliceOrientationRight, 1.0, 0.5);

  ClutterConstraint *align_x_constraint;
  ClutterConstraint *align_y_constraint;

  align_x_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_X_AXIS, 0.0);
  align_y_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_Y_AXIS, 1.0);
  clutter_actor_add_constraint (resources->c_SliceInfo, align_x_constraint);
  clutter_actor_add_constraint (resources->c_SliceInfo, align_y_constraint);

  align_x_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_X_AXIS, 0.5);
  align_y_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_Y_AXIS, 0.0);
  clutter_actor_add_constraint (resources->c_SliceOrientationTop, align_x_constraint);
  clutter_actor_add_constraint (resources->c_SliceOrientationTop, align_y_constraint);

  align_x_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_X_AXIS, 0.5);
  align_y_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_Y_AXIS, 1.0);
  clutter_actor_add_constraint (resources->c_SliceOrientationBottom, align_x_constraint);
  clutter_actor_add_constraint (resources->c_SliceOrientationBottom, align_y_constraint);

  align_x_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_X_AXIS, 0.0);
  align_y_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_Y_AXIS, 0.5);
  clutter_actor_add_constraint (resources->c_SliceOrientationLeft, align_x_constraint);
  clutter_actor_add_constraint (resources->c_SliceOrientationLeft, align_y_constraint);

  align_x_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_X_AXIS, 1.0);
  align_y_constraint = clutter_align_constraint_new (resources->c_Stage, CLUTTER_ALIGN_Y_AXIS, 0.5);
  clutter_actor_add_constraint (resources->c_SliceOrientationRight, align_x_constraint);
  clutter_actor_add_constraint (resources->c_SliceOrientationRight, align_y_constraint);


  // Place it 10 pixels off the left and bottom.
  clutter_actor_set_margin_left (resources->c_SliceInfo, 10);
  clutter_actor_set_margin_bottom (resources->c_SliceInfo, 10);

  clutter_actor_set_margin_top (resources->c_SliceOrientationTop, 10);
  clutter_actor_set_margin_bottom (resources->c_SliceOrientationBottom, 10);
  clutter_actor_set_margin_left (resources->c_SliceOrientationLeft, 10);
  clutter_actor_set_margin_right (resources->c_SliceOrientationRight, 10);


  // TODO: Calculate what the middle slice is and set it.
  viewer_set_slice (resources, 0);

  /*--------------------------------------------------------------------------.
   | SIGNALS & EVENT HANDLING                                                 |
   '--------------------------------------------------------------------------*/
  clutter_actor_set_reactive (resources->c_Actor, TRUE);
  clutter_actor_set_reactive (resources->c_Stage, TRUE);

  g_signal_connect (resources->c_Stage, "motion-event",
                    G_CALLBACK (viewer_on_mouse_move), resources);

  g_signal_connect (resources->c_Stage, "button-release-event",
                    G_CALLBACK (viewer_on_mouse_release), resources);

  g_signal_connect (resources->c_Stage, "button-press-event",
                    G_CALLBACK (viewer_on_mouse_press), resources);

  g_signal_connect (resources->c_Actor, "scroll-event",
                    G_CALLBACK (viewer_on_mouse_scroll), resources);

  g_signal_connect (resources->c_Embed, "enter-notify-event",
                    G_CALLBACK (viewer_on_enter_stage), resources);

  g_signal_connect (resources->c_Embed, "leave-notify-event",
                    G_CALLBACK (viewer_on_leave_stage), resources);

  viewer_set_optimal_fit (resources);

  return resources;
}


void
viewer_add_mask_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();

  // Set up the display slice resources for the mask.
  Slice *mask_slice = memory_slice_new (serie);
  assert (mask_slice != NULL);

  memory_slice_set_NormalVector(mask_slice, &resources->ts_NormalVector);
  memory_slice_set_PivotPoint(mask_slice, &resources->ts_PivotPoint);
  memory_slice_set_UpVector(mask_slice, &resources->ts_UpVector);

  if (mask_slice->data != NULL)
    free (mask_slice->data), mask_slice->data = NULL;

  mask_slice->data = memory_slice_get_data (mask_slice);
  assert (mask_slice->data != NULL);

  PixelDataLookupTable *mask_lut = pixeldata_lookup_table_get_default_mask ();
  PixelData *mask = pixeldata_new_with_lookup_table (mask_lut, 0, 255, mask_slice, serie);
  assert (mask != NULL);

  pixeldata_set_alpha (mask, 120);

  resources->pll_MaskSeries = list_append (resources->pll_MaskSeries, mask);
}


void
viewer_remove_mask_serie (Viewer *resources, Serie *mask)
{
  assert (resources != NULL);
  assert (mask != NULL);

  List *pll_MaskSeries = list_nth (resources->pll_MaskSeries, 1);
  while (pll_MaskSeries != NULL)
  {
    assert (pll_MaskSeries->data != NULL);

    Serie *serie = PIXELDATA_ACTIVE_SERIE (((PixelData *)pll_MaskSeries->data));
    assert (serie != NULL);

    if (serie->id == mask->id)
    {
      debug_extra ("The mask to remove has been found.");

      pixeldata_destroy (pll_MaskSeries->data);

      pll_MaskSeries = list_remove (pll_MaskSeries);
      resources->pll_MaskSeries = pll_MaskSeries;

      break;
    }

    pll_MaskSeries = list_next (pll_MaskSeries);
  }

  if (pll_MaskSeries == NULL)
    debug_error ("%s:%d (%s) :: Trying to remove a mask from a Viewer that"
                 " isn't known to the Viewer.", __func__, __LINE__, __FILE__);
  else
    resources->pll_MaskSeries = pll_MaskSeries;

  viewer_redraw (resources, REDRAW_ALL);
}


void
viewer_remove_overlay_serie (Viewer *resources, Serie *overlay)
{
  assert (resources != NULL);
  assert (overlay != NULL);

  List *pll_MaskSeries = list_nth (resources->pll_OverlaySeries, 1);
  while (pll_MaskSeries != NULL)
  {
    assert (pll_MaskSeries->data != NULL);

    Serie *serie = PIXELDATA_ACTIVE_SERIE (((PixelData *)pll_MaskSeries->data));
    assert (serie != NULL);

    if (serie->id == overlay->id)
    {
      debug_extra ("The overlay to remove has been found.");

      pixeldata_destroy (pll_MaskSeries->data);

      pll_MaskSeries = list_remove (pll_MaskSeries);
      resources->pll_OverlaySeries = pll_MaskSeries;

      break;
    }

    pll_MaskSeries = list_next (pll_MaskSeries);
  }

  resources->pll_OverlaySeries = pll_MaskSeries;
  viewer_redraw (resources, REDRAW_ALL);
}


void
viewer_set_active_layer_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();
  assert (resources != NULL);

  resources->ps_ActiveLayer = serie;
}


Serie*
viewer_get_active_layer_serie (Viewer *resources)
{
  debug_functions ();
  assert (resources != NULL);

  return resources->ps_ActiveLayer;
}


void
viewer_set_active_mask_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();

  assert (resources != NULL);
  assert (resources->ps_Original != NULL);

  Slice *slice = memory_slice_new (serie);
  assert (slice != NULL);

  memory_slice_set_NormalVector(slice, &resources->ts_NormalVector);
  memory_slice_set_PivotPoint(slice, &resources->ts_PivotPoint);
  memory_slice_set_UpVector(slice, &resources->ts_UpVector);

  slice->i16_ViewportChange = 1;


  Slice *original_slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
  if (original_slice != NULL)
    slice->matrix.i16_z = original_slice->matrix.i16_z;

  if (resources->ps_ActiveMask == NULL)
  {
    PixelDataLookupTable *mask_lut = pixeldata_lookup_table_get_default_mask ();
    resources->ps_ActiveMask = pixeldata_new_with_lookup_table (mask_lut, 0, 255, slice, serie);
  }
  else
  {
    List *pll_MaskSeries = list_nth (resources->pll_MaskSeries, 1);
    while (pll_MaskSeries != NULL)
    {
      PixelData *pixeldata = pll_MaskSeries->data;
      if (PIXELDATA_ACTIVE_SERIE (pixeldata)->id == serie->id)
      {
        debug_extra ("Found active pixeldata mask.");
        resources->ps_ActiveMask = pixeldata;
        pixeldata_set_slice (resources->ps_ActiveMask, slice);
        break;
      }

      pll_MaskSeries = list_next (pll_MaskSeries);
    }
  }

  viewer_redraw (resources, REDRAW_MINIMAL);
}


Serie*
viewer_get_active_mask_serie (Viewer *resources)
{
  assert (resources != NULL);
  return PIXELDATA_ACTIVE_SERIE (resources->ps_ActiveMask);
}

void
viewer_set_active_selection_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();

  puts (__func__);

  assert (resources != NULL);
  assert (resources->ps_Original != NULL);

  if (resources->ps_ActiveSelection != NULL)
  {
    pixeldata_destroy (resources->ps_ActiveSelection);
    resources->ps_ActiveSelection = NULL;
  }

  Slice *slice = memory_slice_new (serie);
  slice->ps_NormalVector = &resources->ts_NormalVector;
  slice->ps_PivotPoint = &resources->ts_PivotPoint;
  slice->i16_ViewportChange = 1;


  if (resources->ps_ActiveSelection != NULL)
    pixeldata_destroy (resources->ps_ActiveSelection);

  PixelDataLookupTable *selection_lut = pixeldata_lookup_table_get_default_mask ();
  resources->ps_ActiveSelection = pixeldata_new_with_lookup_table (selection_lut, 0, 255, slice, slice->serie);

  viewer_redraw (resources, REDRAW_MINIMAL);
}


void
viewer_set_active_selection (Viewer *resources, PixelData *selection)
{
  debug_functions ();

  puts (__func__);

  assert (resources != NULL);
  resources->ps_ActiveSelection = selection;
}


void
viewer_add_overlay_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();

  assert (resources != NULL);
  assert (serie != NULL);

  // Set up the display slice resources for the mask.
  PixelData *overlay;
  Slice *overlay_slice = memory_slice_new (serie);


  memory_slice_set_NormalVector(overlay_slice, &resources->ts_NormalVector);
  memory_slice_set_PivotPoint(overlay_slice, &resources->ts_PivotPoint);
  memory_slice_set_UpVector(overlay_slice, &resources->ts_UpVector);

  if (serie->i32_MaximumValue == 0)
    serie->i32_MaximumValue = 255;

  PixelDataLookupTable *overlay_lut;

  int i32_windowWidth, i32_windowLevel;

  i32_windowWidth = serie->i32_MaximumValue - serie->i32_MinimumValue;
  i32_windowLevel = serie->i32_MaximumValue/2;

  if (i32_windowLevel == 0)
  {
    i32_windowLevel = 1;
  }

  if (serie->i32_MaximumValue < 16)
  {
    overlay_lut = pixeldata_lookup_table_get_default_mask();
  }
  else if (serie->i32_MaximumValue < 255)
  {
    overlay_lut = pixeldata_lookup_table_get_default_overlay ();
  }
  else
  {
    overlay_lut = pixeldata_lookup_table_get_default();
  }


  overlay = pixeldata_new_with_lookup_table (overlay_lut,
					     i32_windowWidth,
					     i32_windowLevel,
					     overlay_slice,
					     serie);
  assert (overlay != NULL);

  resources->pll_OverlaySeries = list_append (resources->pll_OverlaySeries,
					      overlay);
  viewer_redraw (resources, REDRAW_ALL);
}


void
viewer_set_active_painter (Viewer *resources, Plugin *plugin)
{
  debug_functions ();

  assert (resources != NULL);
  resources->ts_ActivePainter = plugin;
}


void
viewer_set_callback (Viewer *resources, const char *name, void (*callback)(Viewer *, void *))
{
  debug_functions ();
  assert (resources != NULL);

  if (!strcmp (name, "handle-change"))
    resources->on_handle_change_callback = callback;

  else if (!strcmp (name, "focus-change"))
    resources->on_focus_change_callback = callback;

  else if (!strcmp (name, "pixel-paint"))
    resources->on_pixel_paint_callback = callback;

  else if (!strcmp (name, "window-level-change"))
    resources->on_window_level_change_callback = callback;

  else
  {
    debug_error ("Event '%s' is not implemented for Viewer.", name);
  }
}


void
viewer_set_follow_mode (Viewer *resources, short int b_SetEnabled)
{
  debug_functions ();

  assert (resources != NULL);
  resources->b_FollowMode_Enabled = b_SetEnabled;
}


short int
viewer_get_follow_mode (Viewer *resources)
{
  debug_functions ();

  assert (resources != NULL);
  return resources->b_FollowMode_Enabled;
}


void
viewer_set_auto_close (Viewer *resources, short int b_SetEnabled)
{
  debug_functions ();

  assert (resources != NULL);
  resources->b_AutoClose_Enabled = b_SetEnabled;
}


short int
viewer_get_auto_close (Viewer *resources)
{
  debug_functions ();

  assert (resources != NULL);
  return resources->b_AutoClose_Enabled;
}


void
viewer_set_data (Viewer *resources, PixelData *data)
{
  debug_functions ();

  assert (resources != NULL);
  if (resources->ps_Original != NULL)
  {
    pixeldata_destroy (resources->ps_Original);
    resources->ps_Original = NULL;
  }

  resources->ps_Original = data;
}


void
viewer_set_slice (Viewer *resources, int i32_SliceNumber)
{
  debug_functions ();

  assert (resources != NULL);

  // Set the original data to the proper slice.
  Slice *ts_CurrentSlice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
  ts_CurrentSlice = memory_slice_get_nth (ts_CurrentSlice, i32_SliceNumber);

  // Set the masks to the proper slice.
  List *pll_Masks = resources->pll_MaskSeries;
  while (pll_Masks != NULL)
  {
    PixelData *ts_MaskPixelData = (PixelData *)(pll_Masks->data);

    Slice *ts_MaskSlice = PIXELDATA_ACTIVE_SLICE (ts_MaskPixelData);
    ts_MaskSlice = memory_slice_get_nth (ts_MaskSlice, i32_SliceNumber);

    pll_Masks = list_next (pll_Masks);
  }

  viewer_redraw (resources, REDRAW_ALL);
}


void
viewer_destroy (void *data)
{
  debug_functions ();

  Viewer *resources = data;
  if (resources == NULL) return;

  pixeldata_destroy (resources->ps_Original);
  resources->ps_Original = NULL;

  list_free_all (resources->pll_MaskSeries, pixeldata_destroy);
  resources->pll_MaskSeries = NULL;

  list_free_all (resources->pll_OverlaySeries, pixeldata_destroy);
  resources->pll_OverlaySeries = NULL;

  list_free_all (resources->pll_Replay, free);
  resources->pll_Replay = NULL;

  clutter_actor_destroy (resources->c_SliceInfo);
  clutter_actor_destroy (resources->c_SliceOrientationTop);
  clutter_actor_destroy (resources->c_SliceOrientationBottom);
  clutter_actor_destroy (resources->c_SliceOrientationLeft);
  clutter_actor_destroy (resources->c_SliceOrientationRight);

  clutter_actor_destroy (resources->c_Handles);
  clutter_actor_destroy (resources->c_Actor);

  g_object_ref_sink (resources->c_Embed);
  gtk_widget_destroy (resources->c_Embed);
  g_object_unref (resources->c_Embed);

  free (resources);
}


void
viewer_set_lookup_table_for_serie (Viewer *resources, Serie *serie, const char *lut_name)
{
  debug_functions ();
  assert (resources != NULL);
  assert (serie != NULL);
  assert (lut_name != NULL);

  PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, serie);
  if (pixeldata == NULL)
  {
    debug_error ("Couldn't set the lookup table.");
    return;
  }

  pixeldata_set_color_lookup_table (pixeldata, lut_name);
  viewer_redraw (resources, REDRAW_ALL);
}

void viewer_set_window_level_for_serie (Viewer *resources, Serie *serie, int i32_WindowWidth, int i32_WindowLevel)
{
  debug_functions ();
  assert (resources != NULL);
  assert (serie != NULL);

  PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, serie);
  if (pixeldata == NULL)
  {
    debug_error ("Couldn't set the lookup table.");
    return;
  }

  pixeldata_calculate_window_width_level(pixeldata, i32_WindowWidth, i32_WindowLevel);

  viewer_redraw (resources, REDRAW_ALL);
}



PixelDataLookupTable*
viewer_get_active_lookup_table_for_serie (Viewer *resources, Serie *serie)
{
  debug_functions ();

  assert (resources != NULL);
  assert (serie != NULL);

  PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, serie);
  if (pixeldata != NULL)
  {
    PixelDataLookupTable *lut = PIXELDATA_LOOKUP_TABLE (pixeldata);
    if (lut == NULL)
    {
      debug_error ("This layer doesn't seem to have a lookup table.");
      return NULL;
    }

    return lut;
  }
  else
  {
    debug_error ("Couldn't get the lookup table.");
  }

  return NULL;
}


void
viewer_set_opacity_for_serie (Viewer *resources, Serie *serie, unsigned char opacity)
{
  assert (resources != NULL);
  assert (serie != NULL);

  PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, serie);
  if (pixeldata == NULL)
  {
    debug_error ("Couldn't set alpha for this layer.");
    return;
  }

  pixeldata_set_alpha (pixeldata, opacity);
  viewer_redraw (resources, REDRAW_ALL);
}


unsigned char
viewer_get_opacity_for_serie (Viewer *resources, Serie *serie)
{
  assert (resources != NULL);
  assert (serie != NULL);

  PixelData *pixeldata = viewer_get_pixeldata_for_serie (resources, serie);
  if (pixeldata == NULL)
  {
    debug_error ("Couldn't get alpha for this layer.");
    return 255;
  }

  return pixeldata_get_alpha (pixeldata);
}


PixelData*
viewer_get_pixeldata_for_serie (Viewer *resources, Serie *serie)
{
  assert (resources != NULL);
  assert (serie != NULL);

  /*--------------------------------------------------------------------------.
   | CHECK FOR ORIGINAL                                                       |
   '--------------------------------------------------------------------------*/

  if (serie == PIXELDATA_ACTIVE_SERIE (resources->ps_Original))
    return resources->ps_Original;

  if (serie == PIXELDATA_ACTIVE_SERIE (resources->ps_ActiveMask))
    return resources->ps_ActiveMask;

  /*--------------------------------------------------------------------------.
   | CHECK FOR OVERLAYS                                                       |
   '--------------------------------------------------------------------------*/

  List *overlays = list_nth (resources->pll_OverlaySeries, 1);
  while (overlays != NULL)
  {
    PixelData *overlay = overlays->data;
    if (serie == PIXELDATA_ACTIVE_SERIE (overlay))
      return overlay;

    overlays = list_next (overlays->next);
  }

  /*--------------------------------------------------------------------------.
   | CHECK FOR MASKS                                                          |
   '--------------------------------------------------------------------------*/

  List *masks = list_nth (resources->pll_MaskSeries, 1);
  while (masks != NULL)
  {
    PixelData *mask = masks->data;
    if (serie == PIXELDATA_ACTIVE_SERIE (mask))
      return mask;

    masks = list_next (masks->next);
  }

  return NULL;
}

void
viewer_set_orientation (Viewer *resources, MemoryImageOrientation orientation)
{
  debug_functions ();

  assert (resources != NULL);
  resources->te_Orientation = orientation;
}


MemoryImageOrientation
viewer_get_orientation (Viewer *resources)
{
  debug_functions ();

  assert (resources != NULL);
  return resources->te_Orientation;
}


void
viewer_set_view_mode (Viewer *resources, short int b_SetEnabled)
{
  debug_functions ();

  assert (resources != NULL);
  resources->b_ViewMode_Enabled = b_SetEnabled;

  ClutterContent *c_Content = clutter_actor_get_content (CLUTTER_ACTOR (resources->c_Handles));
  clutter_content_invalidate (CLUTTER_CONTENT (c_Content));
}


short int
viewer_get_view_mode (Viewer *resources)
{
  debug_functions ();

  assert (resources != NULL);
  return resources->b_ViewMode_Enabled;
}


void
viewer_refresh_data (Viewer *resources)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  memory_slice_set_NormalVector(slice, &resources->ts_NormalVector);

  slice->data = memory_slice_get_data (slice);

  List *pll_MaskSeries = resources->pll_MaskSeries;
  if (list_length (pll_MaskSeries) > 0)
  {
    pll_MaskSeries = list_nth (pll_MaskSeries, 1);
    PixelData *ps_Data;

    while (pll_MaskSeries != NULL)
    {
      ps_Data = pll_MaskSeries->data;

      Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Data);

      if (slice->data != NULL)
        free (slice->data), slice->data = NULL;

      memory_slice_set_NormalVector(slice, &resources->ts_NormalVector);
      slice->data = memory_slice_get_data (slice);

      pll_MaskSeries = list_next (pll_MaskSeries);
    }
  }
}


void
viewer_set_timepoint (Viewer *resources, unsigned short int u16_timepoint)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
  memory_slice_set_timepoint (slice,u16_timepoint);

  List *pll_MaskSeries = resources->pll_MaskSeries;
  if (list_length (pll_MaskSeries) > 0)
  {
    pll_MaskSeries = list_nth (pll_MaskSeries, 1);
    PixelData *ps_Data;

    while (pll_MaskSeries != NULL)
    {
      ps_Data = pll_MaskSeries->data;

      Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Data);
      memory_slice_set_timepoint (slice,u16_timepoint);

      pll_MaskSeries = list_next (pll_MaskSeries);
    }
  }
}

void
viewer_toggle_recording (Viewer *resources)
{
  assert (resources != NULL);

  if (resources->is_recording == 0)
  {
    if (resources->pll_Replay != NULL)
    {
      list_free_all (resources->pll_Replay, free);
      resources->pll_Replay = NULL;
    }
    resources->is_recording = 1;
  }
  else
    resources->is_recording = 0;
}


void
viewer_replay_recording (Viewer *resources)
{
  List *commands = list_nth (resources->pll_Replay, 1);

  while (commands != NULL)
  {
    ViewerCommand *command = commands->data;
    if (command == NULL)
    {
      commands = list_next (commands);
      continue;
    }

    Plugin *plugin = command->plugin;
    if (plugin == NULL)
    {
      commands = list_next (commands);
      continue;
    }

    switch (plugin->te_Type)
    {
      case PLUGIN_TYPE_BRUSH:
      {
	pixeldata_apply_brush (resources->ps_Original, resources->ps_ActiveMask,
			       resources->ps_ActiveSelection, command->prev_coordinate,
			       command->coordinate, command->size, command->value,
                               command->action, plugin->fp_Callback);
      }
      break;
      case PLUGIN_TYPE_SELECTION:
      {
        Coordinate ts_InitialPixelPosition;
        ts_InitialPixelPosition = viewer_get_image_pixel_position (resources, resources->ts_InitialDrawCoordinate);

        ts_InitialPixelPosition.x = abs (ts_InitialPixelPosition.x);
        ts_InitialPixelPosition.y = abs (ts_InitialPixelPosition.y);

        ((PluginSelection *)plugin)->fp_Callback (resources->ps_Original, resources->ps_ActiveMask,
                                                  resources->ps_ActiveSelection, command->prev_coordinate,
                                                  command->coordinate, command->value, command->action);
      }
      break;
      /*
      case PLUGIN_TYPE_LINE:
      {
        ((PluginLine *)plugin)->fp_Callback (resources->ps_Original, resources->ps_ActiveMask,
                                             resources->ps_ActiveSelection, command->coordinate,
                                             &resources->pll_PolygonPoints, plugin->i32_Value, te_Action);
      }
      break;
      */
      default:
      {
        debug_warning ("Encountered an unsupported plugin type.", NULL);
      }
      break;
    }

    commands = list_next (commands);
  }

}


void
viewer_replay_recording_over_time (Viewer *resources)
{
  assert (resources != NULL);

  Serie *serie = resources->ps_Original->serie;
  assert (serie != NULL);

  Slice *slice = PIXELDATA_ACTIVE_SLICE (resources->ps_Original);
  assert (slice != NULL);

  short int original_time_point = slice->u16_timePoint;

  short unsigned int counter;
  for (counter = 0; counter < serie->num_time_series; counter++)
  {
    viewer_set_timepoint (resources, counter);
    viewer_refresh_data (resources);
    viewer_replay_recording (resources);
  }

  // Restore the original time point.
  viewer_set_timepoint (resources, original_time_point);
  viewer_refresh_data (resources);
  viewer_redraw (resources, REDRAW_ALL);
}

