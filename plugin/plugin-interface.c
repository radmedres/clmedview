#include "plugin-interface.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>


short int
CLM_Plugin_DrawPixelAtPoint (PixelData *ps_Mask, PixelData *ps_Selection,
                             Coordinate ts_Point, unsigned int ui32_DrawValue,
                             PixelAction te_Action)
{
  // We assume the PixelData is allocated.
  assert (ps_Mask != NULL);
  //assert (ps_Selection != NULL);

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (ps_Mask);
  assert (mask_slice != NULL);

  // Boundary checks.
  if (ts_Point.x < 0 || ts_Point.y < 0) return 0;
  if (ts_Point.x + 1 > mask_slice->matrix.x || ts_Point.y >= mask_slice->matrix.y) return 0;

  void **ppv_SelectionDataCounter = NULL;
  if (ps_Selection != NULL)
  {
    ppv_SelectionDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (ps_Selection);
    ppv_SelectionDataCounter += (unsigned int)(ts_Point.y * mask_slice->matrix.x + ts_Point.x);
  }

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (ps_Mask);
  ppv_ImageDataCounter += (unsigned int)(ts_Point.y * mask_slice->matrix.x + ts_Point.x);

  switch (ps_Mask->serie->data_type)
  {
  case MEMORY_TYPE_INT16:
    {
      if (te_Action == ACTION_ERASE)
      {
        if (*(short int *)*ppv_ImageDataCounter == (short int)ui32_DrawValue
            && (ps_Selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
        {
          *(short int *)*ppv_ImageDataCounter = 0;
        }
      }
      else
      {
        if (*(short int *)*ppv_ImageDataCounter == 0
            && (ps_Selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
        {
          *(short int *)*ppv_ImageDataCounter = (short int)ui32_DrawValue;
        }
      }
    }
    break;
  case MEMORY_TYPE_INT32:
    {
      if (*(int *)*ppv_ImageDataCounter == 0
          && (ps_Selection == NULL || (*((int *)*ppv_SelectionDataCounter))))
        *(int *)*ppv_ImageDataCounter = (int)ui32_DrawValue;
    }
    break;
  case MEMORY_TYPE_UINT16 :
    {
      if (*(short unsigned int *)*ppv_ImageDataCounter == 0
          && (ps_Selection == NULL || (*((short unsigned int *)*ppv_SelectionDataCounter))))
        *(short unsigned int *)*ppv_ImageDataCounter = (short unsigned int)ui32_DrawValue;
    }
    break;
  case MEMORY_TYPE_UINT32:
    {
      if (*(unsigned int *)*ppv_ImageDataCounter == 0
          && (ps_Selection == NULL || (*((unsigned int *)*ppv_SelectionDataCounter))))
        *(unsigned int *)*ppv_ImageDataCounter = ui32_DrawValue;
    }
    break;
  case MEMORY_TYPE_FLOAT32:
    {
      if (*(float *)*ppv_ImageDataCounter == 0
          && (ps_Selection == NULL || (*((float *)*ppv_SelectionDataCounter))))
        *(float *)*ppv_ImageDataCounter = ui32_DrawValue;
    }
    break;
  case MEMORY_TYPE_FLOAT64:
    {
      if (*(double *)*ppv_ImageDataCounter == 0
          && (ps_Selection == NULL || (*((double *)*ppv_SelectionDataCounter))))
        *(double *)*ppv_ImageDataCounter = ui32_DrawValue;
    }
    break;
  default:
    return 0;
    break;
  }

  return 1;
}


short int
CLM_Plugin_GetPixelAtPoint (PixelData *ps_Layer, Coordinate ts_Point, void *pv_PixelValue)
{
  // We assume the PixelData is allocated.
  assert (ps_Layer != NULL);

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (ps_Layer);
  assert (mask_slice != NULL);

  // Boundary checks.
  if (ts_Point.x < 0 || ts_Point.y < 0) return 0;
  if (ts_Point.x >= mask_slice->matrix.x || ts_Point.y >= mask_slice->matrix.y) return 0;

  short int i16_Y = (short int)ts_Point.y;
  short int i16_X = (short int)ts_Point.x;

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (ps_Layer);
  ppv_ImageDataCounter += (unsigned int)(i16_Y * mask_slice->matrix.x + i16_X);

  switch (ps_Layer->serie->data_type)
  {
    case MEMORY_TYPE_INT16   : *(short int *)pv_PixelValue          = *(short int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_INT32   : *(int *)pv_PixelValue                = *(int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT16  : *(short unsigned int *)pv_PixelValue = *(short unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT32  : *(unsigned int *)pv_PixelValue       = *(unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT32 : *(float *)pv_PixelValue              = *(float *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT64 : *(double *)pv_PixelValue             = *(double *)*ppv_ImageDataCounter; break;
    default                  : assert (NULL != NULL); break;
  }

  return 1;
}
