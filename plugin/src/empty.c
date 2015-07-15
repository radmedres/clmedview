#include "plugin-interface.h"
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

  meta->name = calloc (6, sizeof (char));
  meta->name = memcpy (meta->name, "empty", 6);

  meta->version = 2;

  *metadata = meta;
}


void
plugin_apply (UNUSED PluginMetaData *metadata, UNUSED PixelData *original,
	      UNUSED PixelData *mask, UNUSED PixelData *selection,
	      UNUSED Coordinate point)
{
  return;
}

void *
plugin_get_property (PluginMetaData *metadata, const char *property)
{
  if (metadata == NULL)
    return NULL;

  if (!strcmp ("name", property))
    return &(metadata->name);

  return NULL;
}

bool
plugin_set_property (UNUSED PluginMetaData *metadata,
		     UNUSED const char *property,
		     UNUSED void *value)
{
  return false;
}

void
plugin_destroy (PluginMetaData *metadata)
{
  if (metadata == NULL) return;

  free (metadata->name);
  metadata->name = NULL;

  free (metadata);
}
