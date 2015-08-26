#include "plugin-interface.h"
#include "plugin-interface-brush.h"
#include "libcommon-unused.h"

#include <stdlib.h>
#include <string.h>

static void plugin_bresenham_line (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate point);
static void plugin_draw (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate point);

void
plugin_get_metadata (PluginMetaData **metadata)
{
  if (metadata == NULL) return;
  PluginMetaData *meta = *metadata;

  if (meta == NULL)
    meta = calloc (1, sizeof (PluginMetaData));

  meta->name = calloc (7, sizeof (char));
  meta->name = memcpy (meta->name, "pencil", 7);

  meta->version = 2;

  meta->properties = calloc (1, sizeof (PluginBrushProperties));
  if (meta->properties != NULL)
  {
    PluginBrushProperties *properties = meta->properties;
    properties->size = 1;
    properties->value = 1;
    properties->action = 1;
  }

  *metadata = meta;
}

void
plugin_apply (PluginMetaData *metadata, UNUSED PixelData *original,
	      PixelData *mask, PixelData *selection, Coordinate point)
{
  if (metadata == NULL) return;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return;

  (properties->previous.x == 0 && properties->previous.y == 0)
    ? plugin_draw (metadata, mask, selection, point)
    : plugin_bresenham_line (metadata, mask, selection, point);
}

void *
plugin_get_property (PluginMetaData *metadata, const char *property)
{
  if (metadata == NULL) return NULL;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return NULL;

  if (!strcmp ("name", property))
    return &(metadata->name);

  else if (!strcmp ("size", property))
    return &(properties->size);

  else if (!strcmp ("value", property))
    return &(properties->value);

  else if (!strcmp ("action", property))
    return &(properties->action);

  else if (!strcmp ("previous-coordinate", property))
    return &(properties->previous);

  return NULL;
}

bool
plugin_set_property (PluginMetaData *metadata, const char *property, void *value)
{
  if (metadata == NULL) return false;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return false;

  if (!strcmp ("size", property))
    properties->size = *(unsigned int *)value;

  else if (!strcmp ("value", property))
    properties->value = *(unsigned int *)value;

  else if (!strcmp ("action", property))
    properties->action = *(PixelAction *)value;

  else if (!strcmp ("previous-coordinate", property))
    properties->previous = *(Coordinate *)value;
  
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
plugin_bresenham_line (PluginMetaData *metadata, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginBrushProperties *properties = metadata->properties;
  Coordinate ts_Start, ts_End;

  ts_Start = properties->previous;
  ts_End = point;

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
  PluginBrushProperties *properties = metadata->properties;

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
