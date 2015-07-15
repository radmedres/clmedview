#include "plugin-interface.h"
#include "plugin-interface-brush.h"
#include "libcommon-unused.h"

#include <stdlib.h>
#include <string.h>

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
	      PixelData *mask, PixelData *selection,
	      Coordinate point)
{
  if (metadata == NULL) return;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return;

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

void *
plugin_get_property (PluginMetaData *metadata, const char *property)
{
  if (metadata == NULL)
    return NULL;

  PluginBrushProperties *properties = metadata->properties;
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

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL)
    return false;

  if (!strcmp ("size", property))
    properties->size = *(unsigned int *)value;

  else if (!strcmp ("value", property))
    properties->value = *(unsigned int *)value;

  else if (!strcmp ("action", property))
    properties->action = *(PixelAction*)value;

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
