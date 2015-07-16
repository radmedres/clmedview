#include "plugin-interface.h"
#include "plugin-interface-line.h"
#include "libcommon-unused.h"

#include <stdlib.h>
#include <string.h>


static Coordinate *ac_1 = NULL;
static Coordinate *ac_2 = NULL;

static void plugin_draw (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate point);
static void plugin_bresenham_line (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate first, Coordinate second);


void
plugin_get_metadata (PluginMetaData **metadata)
{
  if (metadata == NULL) return;
  PluginMetaData *meta = *metadata;

  if (meta == NULL)
    meta = calloc (1, sizeof (PluginMetaData));

  meta->name = calloc (8, sizeof (char));
  meta->name = memcpy (meta->name, "polygon", 8);

  meta->version = 2;

  meta->properties = calloc (1, sizeof (PluginLineProperties));
  if (meta->properties != NULL)
  {
    PluginLineProperties *properties = meta->properties;
    properties->size = 1;
    properties->value = 1;
    properties->action = 1;
  }

  *metadata = meta;
}


void
plugin_apply (PluginMetaData *metadata, UNUSED PixelData *original,
	      PixelData *mask, PixelData *selection,
	      Coordinate point)
{
  if (metadata == NULL) return;

  PluginLineProperties *properties = metadata->properties;
  if (properties == NULL) return;

  plugin_draw (metadata, mask, selection, point);

  // When the coordinates have been reset, reset the internal variables too.
  if (properties->coordinates == NULL)
  {
    ac_1 = NULL;
    ac_2 = NULL;
  }
  
  if (ac_1 != NULL && ac_2 != NULL)
  {
    if (properties->action == ACTION_ERASE)
    {
      properties->action = ACTION_SET;
      plugin_bresenham_line (metadata, mask, selection, *ac_1, *ac_2);
      properties->action = ACTION_ERASE;
    }
    else
    {
      properties->action = ACTION_ERASE;
      plugin_bresenham_line (metadata, mask, selection, *ac_1, *ac_2);
      properties->action = ACTION_SET;
    }
  }

  // Add the new coordinate.
  Coordinate *ts_NewCoordinate = calloc (1, sizeof (Coordinate));
  memcpy (ts_NewCoordinate, &point, sizeof (Coordinate));
  properties->coordinates = list_prepend (properties->coordinates, (void *)ts_NewCoordinate);

  List *pll_Points = properties->coordinates;

  // Draw all previous lines.
  List *pll_LastElement = NULL;
  while (pll_Points != NULL)
  {
    Coordinate *ts_CurrentPoint = pll_Points->data;
    assert (ts_CurrentPoint != NULL);

    if (pll_LastElement != NULL)
    {
      Coordinate *ts_LastCoordinate = pll_LastElement->data;
      plugin_bresenham_line (metadata, mask, selection, *ts_LastCoordinate, *ts_CurrentPoint);
    }

    pll_LastElement = pll_Points;
    pll_Points = list_next (pll_Points);
  }

  ac_1 = (properties->coordinates)->data;
  ac_2 = pll_LastElement->data;

  plugin_bresenham_line (metadata, mask, selection, *ac_1, *ac_2);
}


void *
plugin_get_property (PluginMetaData *metadata, const char *property)
{
  if (metadata == NULL)
    return NULL;

  PluginLineProperties *properties = metadata->properties;
  if (properties == NULL)
    return NULL;

  if (!strcmp ("name", property))
    return &(metadata->name);

  else if (!strcmp ("size", property))
    return &(properties->size);

  else if (!strcmp ("value", property))
    return &(properties->value);

  else if (!strcmp ("action", property))
    return &(properties->action);

  return NULL;
}


bool
plugin_set_property (PluginMetaData *metadata, const char *property,
		     void *value)
{
  if (metadata == NULL)
    return false;

  PluginLineProperties *properties = metadata->properties;
  if (properties == NULL)
    return false;

  if (!strcmp ("size", property))
    properties->size = *(unsigned int *)value;

  else if (!strcmp ("value", property))
    properties->value = *(unsigned int *)value;

  else if (!strcmp ("action", property))
    properties->action = *(PixelAction*)value;

  else if (!strcmp ("reset", property))
  {
    list_free (properties->coordinates);
    properties->coordinates = NULL;
  }
  else
    return false;

  return true;
}


void
plugin_destroy (PluginMetaData *metadata)
{
  if (metadata == NULL) return;

  free (metadata->name);
  metadata->name = NULL;

  free (metadata->properties);
  metadata->properties = NULL;

  free (metadata);
}


static void
plugin_bresenham_line (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate first, Coordinate second)
{
  Coordinate ts_Start, ts_End;

  ts_Start = first;
  ts_End = second;

  /*--------------------------------------------------------------------------.
   | The following is an optimized version of the bresenham line algorithm.   |
   | It basically calculates a straight line of pixels for two given points.  |
   | For each pixel it selects, fp_BrushCallback will be called.              |
   '--------------------------------------------------------------------------*/

  int i32_DifferenceInX = abs (ts_End.x - ts_Start.x);
  int i32_DirectionOfX = (ts_Start.x < ts_End.x) ? 1 : -1;

  int i32_DifferenceInY = abs (ts_End.y - ts_Start.y);
  int i32_DirectionOfY = (ts_Start.y < ts_End.y) ? 1 : -1;

  int i32_Error = ((i32_DifferenceInX > i32_DifferenceInY) ? i32_DifferenceInX : -i32_DifferenceInY) / 2;
  int i32_PreviousError;

  for (;;)
  {
    plugin_draw (metadata, mask, selection, ts_Start);
    if (ts_Start.x == ts_End.x && ts_Start.y == ts_End.y) break;

    i32_PreviousError = i32_Error;

    if (i32_PreviousError > -i32_DifferenceInX)
    {
      i32_Error -= i32_DifferenceInY; ts_Start.x += i32_DirectionOfX;
    }

    if (i32_PreviousError < i32_DifferenceInY)
    {
      i32_Error += i32_DifferenceInX; ts_Start.y += i32_DirectionOfY;
    }
  }
}

static void
plugin_draw (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginLineProperties *properties = metadata->properties;

  int lower;
  int upper;
  int size;

  if (properties->size > 1)
  {
    size = properties->size / 2;
    lower = -1 * size;
    upper = size;
  }
  else
  {
    plugin_set_voxel_at_point (mask, selection, point, properties->value,
			       properties->action);
    return;
  }

  /* Center the brush position. */
  point.y = point.y - size - 1;

  int counter;
  int row = 1;
  int column;
  Coordinate current_point;

  for (counter = lower; counter < upper; counter++)
  {
    for (column = lower; column < upper; column++)
    {
      current_point.x = point.x + column;
      current_point.y = point.y + row;

      plugin_set_voxel_at_point (mask, selection, current_point,
				 properties->value, properties->action);
    }

    row++;
  }
}
