#include "plugin-interface.h"
#include <stddef.h>

bool
plugin_set_voxel_at_point (PixelData *mask,
			   PixelData *selection,
			   Coordinate point,
			   unsigned int value,
			   PixelAction action)
{
  if (mask == NULL) return 0;

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (mask);
  if (mask_slice == NULL) return 0;

  if (point.x < 0 || point.y < 0) return 0;
  if (point.x + 1 > mask_slice->matrix.i16_x || point.y >= mask_slice->matrix.i16_y) return 0;

  void **ppv_SelectionDataCounter = NULL;
  if (selection != NULL)
  {
    ppv_SelectionDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (selection);
    ppv_SelectionDataCounter += (unsigned int)(point.y * mask_slice->matrix.i16_x + point.x);
  }

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (mask);
  ppv_ImageDataCounter += (unsigned int)(point.y * mask_slice->matrix.i16_x + point.x);

  switch (mask->serie->data_type)
  {
  case MEMORY_TYPE_INT16:
    {
      if (action == ACTION_ERASE)
      {
        if (*(short int *)*ppv_ImageDataCounter == (short int)value
            && (selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
          *(short int *)*ppv_ImageDataCounter = 0;
      }
      else
      {
        if (*(short int *)*ppv_ImageDataCounter == 0
            && (selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
          *(short int *)*ppv_ImageDataCounter = (short int)value;
      }
    }
    break;
  case MEMORY_TYPE_INT32:
    {
      if (*(int *)*ppv_ImageDataCounter == 0
          && (selection == NULL || (*((int *)*ppv_SelectionDataCounter))))
        *(int *)*ppv_ImageDataCounter = (int)value;
    }
    break;
  case MEMORY_TYPE_UINT16 :
    {
      if (*(short unsigned int *)*ppv_ImageDataCounter == 0
          && (selection == NULL || (*((short unsigned int *)*ppv_SelectionDataCounter))))
        *(short unsigned int *)*ppv_ImageDataCounter = (short unsigned int)value;
    }
    break;
  case MEMORY_TYPE_UINT32:
    {
      if (*(unsigned int *)*ppv_ImageDataCounter == 0
          && (selection == NULL || (*((unsigned int *)*ppv_SelectionDataCounter))))
        *(unsigned int *)*ppv_ImageDataCounter = value;
    }
    break;
  case MEMORY_TYPE_FLOAT32:
    {
      if (*(float *)*ppv_ImageDataCounter == 0
          && (selection == NULL || (*((float *)*ppv_SelectionDataCounter))))
        *(float *)*ppv_ImageDataCounter = value;
    }
    break;
  case MEMORY_TYPE_FLOAT64:
    {
      if (*(double *)*ppv_ImageDataCounter == 0
          && (selection == NULL || (*((double *)*ppv_SelectionDataCounter))))
        *(double *)*ppv_ImageDataCounter = value;
    }
    break;
  default:
    return 0;
    break;
  }

  return 1;
}


bool
plugin_get_voxel_at_point (PixelData *layer, Coordinate point, void *pixel_value)
{
  if (layer == NULL) return 0;

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (layer);
  if (mask_slice == NULL) return 0;

  if (point.x < 0 || point.y < 0) return 0;
  if (point.x >= mask_slice->matrix.i16_x || point.y >= mask_slice->matrix.i16_y) return 0;

  short int i16_Y = (short int)point.y;
  short int i16_X = (short int)point.x;

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (layer);
  ppv_ImageDataCounter += (unsigned int)(i16_Y * mask_slice->matrix.i16_x + i16_X);

  switch (layer->serie->data_type)
  {
    case MEMORY_TYPE_UINT8   : *(unsigned char *)pixel_value      = *(unsigned char *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_INT16   : *(short int *)pixel_value          = *(short int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_INT32   : *(int *)pixel_value                = *(int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT16  : *(short unsigned int *)pixel_value = *(short unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT32  : *(unsigned int *)pixel_value       = *(unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT32 : *(float *)pixel_value              = *(float *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT64 : *(double *)pixel_value             = *(double *)*ppv_ImageDataCounter; break;
    default                  : return 0; break;
  }

  return 1;
}
