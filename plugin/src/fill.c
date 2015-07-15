#include "plugin-interface.h"
#include "plugin-interface-brush.h"

#include <stdlib.h>
#include <string.h>

void
plugin_get_metadata (PluginMetaData **metadata)
{
  if (metadata == NULL) return;
  PluginMetaData *meta = *metadata;

  if (meta == NULL)
    meta = calloc (1, sizeof (PluginMetaData));

  meta->name = calloc (5, sizeof (char));
  meta->name = memcpy (meta->name, "fill", 5);

  meta->version = 2;

  meta->properties = calloc (1, sizeof (PluginBrushProperties));
  if (meta->properties != NULL)
  {
    PluginBrushProperties *properties = meta->properties;
    properties->value = 1;
    properties->action = 1;
  }

  *metadata = meta;
}


void
plugin_apply (PluginMetaData *metadata, PixelData *original,
	      PixelData *mask, PixelData *selection,
	      Coordinate point)
{
  if (metadata == NULL || original == NULL || mask == NULL) return;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return;

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (mask);
  if (mask_slice == NULL) return;

  if (point.x > mask_slice->matrix.i16_x || point.y > mask_slice->matrix.i16_y) return;

  int i32_CntArray;
  int i32_CntKernel;

  short int i16_Value;

  int i32_lengthArrayToRead = 1;

  Coordinate *p_ArrayToReadFrom = calloc (1, PIXELDATA_ACTIVE_SLICE (original)->matrix.i16_x *
                                             PIXELDATA_ACTIVE_SLICE (original)->matrix.i16_y *
                                             sizeof (Coordinate));
  Coordinate *p_PointInArray = NULL;

  Coordinate *ps_Kernel = calloc (1, 4 * sizeof (Coordinate));

  Coordinate ts_PixelPoint;

  p_ArrayToReadFrom[0].x = point.x;
  p_ArrayToReadFrom[0].y = point.y;


  i32_lengthArrayToRead=1;
  for(i32_CntArray=0;i32_CntArray<i32_lengthArrayToRead; i32_CntArray++)
  {
    p_PointInArray = &p_ArrayToReadFrom[i32_CntArray];

    ps_Kernel[0].x = p_PointInArray->x;
    ps_Kernel[0].y = p_PointInArray->y + 1;

    ps_Kernel[1].x = p_PointInArray->x - 1;
    ps_Kernel[1].y = p_PointInArray->y;

    ps_Kernel[2].x = p_PointInArray->x + 1;
    ps_Kernel[2].y = p_PointInArray->y;

    ps_Kernel[3].x = p_PointInArray->x;
    ps_Kernel[3].y = p_PointInArray->y - 1;

    for(i32_CntKernel=0; i32_CntKernel<4; i32_CntKernel++)
    {
      ts_PixelPoint = ps_Kernel[i32_CntKernel];

      if ((ts_PixelPoint.x >= 0) && (ts_PixelPoint.x < PIXELDATA_ACTIVE_SLICE (mask)->matrix.i16_x) &&
          (ts_PixelPoint.y >= 0) && (ts_PixelPoint.y < PIXELDATA_ACTIVE_SLICE (mask)->matrix.i16_y))
      {
        if (plugin_get_voxel_at_point (mask, ts_PixelPoint, &i16_Value))
        {
	  if ((properties->action == ACTION_SET && i16_Value == 0) ||
	      (properties->action == ACTION_ERASE && i16_Value) != 0)
	  {
	    plugin_set_voxel_at_point (mask, selection, ts_PixelPoint , properties->value, properties->action);
	    p_ArrayToReadFrom[i32_lengthArrayToRead] = ts_PixelPoint;
	    i32_lengthArrayToRead++;
	  }
        }
      }
    }
  }

  free (p_ArrayToReadFrom);
  p_ArrayToReadFrom = NULL;

  free (ps_Kernel);
  ps_Kernel = NULL;
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
