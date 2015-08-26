#include "plugin-interface.h"
#include "plugin-interface-brush.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>


void plugin_handle_uint8 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point);
void plugin_handle_uint16 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point);
void plugin_handle_int16 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point);
void plugin_handle_float32 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point);


void
plugin_get_metadata (PluginMetaData **metadata)
{
  if (metadata == NULL) return;
  PluginMetaData *meta = *metadata;

  if (meta == NULL)
    meta = calloc (1, sizeof (PluginMetaData));

  meta->name = calloc (6, sizeof (char));
  meta->name = memcpy (meta->name, "sobel", 6);

  meta->version = 2;

  meta->properties = calloc (1, sizeof (PluginBrushProperties));
  if (meta->properties != NULL)
  {
    PluginBrushProperties *properties = meta->properties;
    properties->size = 10;
    properties->value = 1;
    properties->action = 1;
  }

  *metadata = meta;
}


void
plugin_apply (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point)
{
  if (metadata == NULL || original == NULL || mask == NULL) return;

  PluginBrushProperties *properties = metadata->properties;
  if (properties == NULL) return;

  Slice *slice = PIXELDATA_ACTIVE_SLICE (original);
  if (slice == NULL) return; 

  unsigned int boundary = properties->size / 2;
  
  if (point.x > (slice->matrix.i16_x - boundary)
      || point.x < boundary
      || point.y > (slice->matrix.i16_y - boundary)
      || point.y < boundary)
  {
    return;
  }

  switch (original->serie->data_type)
  {
    case MEMORY_TYPE_UINT8  : plugin_handle_uint8 (metadata, original, mask, selection, point); break;
    case MEMORY_TYPE_INT16  : plugin_handle_int16 (metadata, original, mask, selection, point); break;
    case MEMORY_TYPE_UINT16 : plugin_handle_uint16 (metadata, original, mask, selection, point); break;
    case MEMORY_TYPE_INT32  : break;
    case MEMORY_TYPE_UINT32 : break;
    case MEMORY_TYPE_FLOAT32: plugin_handle_float32 (metadata, original, mask, selection, point); break;
    case MEMORY_TYPE_FLOAT64: break;
    default: break;
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
    {
      if (*(unsigned int *)value < 10) return false;
      properties->size = *(unsigned int *)value;
    }

  else if (!strcmp ("value", property))
    properties->value = *(unsigned int *)value;

  else if (!strcmp ("action", property))
    properties->action = *(PixelAction *)value;

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


void
plugin_handle_uint8 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginBrushProperties *properties = metadata->properties;

  Slice *slice = PIXELDATA_ACTIVE_SLICE (original);
  assert (slice != NULL);

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;
  int i32_tempValue;

  /* DECLARATIONS FOR INT16 */
  unsigned char u8_tempImage[10][10];
  unsigned char u8_tempImageSmooth[10][10];

  unsigned char u8_GradientX[10][10];
  unsigned char u8_GradientY[10][10];

  unsigned char u8_Gradient[10][10];

  unsigned int u32_AvarageValue = 0;



  // Initialize the data to zero.
  memset (&u8_tempImage,       0, 100 * sizeof (unsigned char));
  memset (&u8_tempImageSmooth, 0, 100 * sizeof (unsigned char));
  memset (&u8_GradientX,       0, 100 * sizeof (unsigned char));
  memset (&u8_GradientY,       0, 100 * sizeof (unsigned char));
  memset (&u8_Gradient,        0, 100 * sizeof (unsigned char));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  u32_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      plugin_get_voxel_at_point (original, ts_BlobCount, &(u8_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      u32_AvarageValue += u8_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  u32_AvarageValue = u32_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      i32_tempValue =
        // first row kernel
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        u8_tempImageSmooth[Y_Cnt][X_Cnt] = (short int)(i32_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  unsigned char u8_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      u8_GradientX[Y_Cnt][X_Cnt] =
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u8_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u8_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u8_GradientY[Y_Cnt][X_Cnt] =
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * u8_tempImageSmooth[Y_Cnt - 1][X_Cnt] + u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (u8_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * u8_tempImageSmooth[Y_Cnt + 1][X_Cnt] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u8_Gradient[Y_Cnt][X_Cnt] = sqrt (u8_GradientX[Y_Cnt][X_Cnt] * u8_GradientX[Y_Cnt][X_Cnt] +
                                     u8_GradientY[Y_Cnt][X_Cnt] * u8_GradientY[Y_Cnt][X_Cnt]);

      if (u8_Gradient[Y_Cnt][X_Cnt] > u8_MaxValue)
      {
        u8_MaxValue = u8_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (u8_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      u8_Gradient[Y_Cnt][X_Cnt] = (u8_Gradient[Y_Cnt][X_Cnt] * 255) / u8_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      if (u8_tempImage[Y_Cnt+5][X_Cnt+5] > u32_AvarageValue)
      {
        plugin_set_voxel_at_point (mask, selection, ts_BlobCount, properties->value, properties->action);
      }
    }
  }
}

void
plugin_handle_uint16 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginBrushProperties *properties = metadata->properties;

  Slice *slice = PIXELDATA_ACTIVE_SLICE (original);
  assert (slice != NULL);

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  unsigned int u32_tempValue;

  /* DECLARATIONS FOR SHORT UNSIGNED INT16 */
  unsigned short int u16_tempImage[10][10];
  unsigned short int u16_tempImageSmooth[10][10];

  unsigned short int u16_GradientX[10][10];
  unsigned short int u16_GradientY[10][10];
  unsigned short int u16_Gradient[10][10];

  unsigned short int u16_MaxValue = 0;

  unsigned int u32_AvarageValue = 0;

  // Initialize the data to zero.
  memset (&u16_tempImage,       0, 100 * sizeof (unsigned char));
  memset (&u16_tempImageSmooth, 0, 100 * sizeof (unsigned char));
  memset (&u16_GradientX,       0, 100 * sizeof (unsigned char));
  memset (&u16_GradientY,       0, 100 * sizeof (unsigned char));
  memset (&u16_Gradient,        0, 100 * sizeof (unsigned char));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  u32_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      plugin_get_voxel_at_point (original, ts_BlobCount, &(u16_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      u32_AvarageValue += u16_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  u32_AvarageValue = u32_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      u32_tempValue =
        // first row kernel
        (unsigned int)(u16_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (unsigned int)(u16_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (unsigned int)(u16_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (unsigned int)(u16_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +
        (unsigned int)(u16_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +

        // second row kernel
        (unsigned int)(u16_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (unsigned int)(u16_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (unsigned int)(u16_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (unsigned int)(u16_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (unsigned int)(u16_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (unsigned int)(u16_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (unsigned int)(u16_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (unsigned int)(u16_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (unsigned int)(u16_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (unsigned int)(u16_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (unsigned int)(u16_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (unsigned int)(u16_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (unsigned int)(u16_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (unsigned int)(u16_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (unsigned int)(u16_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (unsigned int)(u16_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (unsigned int)(u16_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (unsigned int)(u16_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (unsigned int)(u16_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (unsigned int)(u16_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        u16_tempImageSmooth[Y_Cnt][X_Cnt] = (short unsigned int)(u32_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/



  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      u16_GradientX[Y_Cnt][X_Cnt] =
        (u16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (u16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u16_GradientY[Y_Cnt][X_Cnt] =
        (u16_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * u16_tempImageSmooth[Y_Cnt - 1][X_Cnt] + u16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (u16_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * u16_tempImageSmooth[Y_Cnt + 1][X_Cnt] + u16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u16_Gradient[Y_Cnt][X_Cnt] = sqrt (u16_GradientX[Y_Cnt][X_Cnt] * u16_GradientX[Y_Cnt][X_Cnt] +
                                     u16_GradientY[Y_Cnt][X_Cnt] * u16_GradientY[Y_Cnt][X_Cnt]);

      if (u16_Gradient[Y_Cnt][X_Cnt] > u16_MaxValue)
      {
        u16_MaxValue = u16_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (u16_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      u16_Gradient[Y_Cnt][X_Cnt] = (u16_Gradient[Y_Cnt][X_Cnt] * 255) / u16_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      if (u16_tempImage[Y_Cnt+5][X_Cnt+5] > u32_AvarageValue)
      {
        plugin_set_voxel_at_point (mask, selection, ts_BlobCount, properties->value, properties->action);
      }
    }
  }
}


void
plugin_handle_int16 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginBrushProperties *properties = metadata->properties;

  Slice *slice = PIXELDATA_ACTIVE_SLICE (original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((point.x > (slice->matrix.i16_x - 5)) || (point.x < 5)
      || (point.y > (slice->matrix.i16_y - 5)) || (point.y < 5))
  {
    return;
  }

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  int i32_tempValue;

  /* DECLARATIONS FOR INT16 */
  short int i16_tempImage[10][10];
  short int i16_tempImageSmooth[10][10];

  short int i16_GradientX[10][10];
  short int i16_GradientY[10][10];

  short int i16_Gradient[10][10];

  int i32_AvarageValue = 0;



  // Initialize the data to zero.
  memset (&i16_tempImage,       0, 100 * sizeof (short int));
  memset (&i16_tempImageSmooth, 0, 100 * sizeof (short int));
  memset (&i16_GradientX,       0, 100 * sizeof (short int));
  memset (&i16_GradientY,       0, 100 * sizeof (short int));
  memset (&i16_Gradient,        0, 100 * sizeof (short int));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  i32_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      plugin_get_voxel_at_point (original, ts_BlobCount, &(i16_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      i32_AvarageValue += i16_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  i32_AvarageValue = i32_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      i32_tempValue =
        // first row kernel
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        i16_tempImageSmooth[Y_Cnt][X_Cnt] = (short int)(i32_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  short int i16_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      i16_GradientX[Y_Cnt][X_Cnt] =
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * i16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * i16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      i16_GradientY[Y_Cnt][X_Cnt] =
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * i16_tempImageSmooth[Y_Cnt - 1][X_Cnt] + i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (i16_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * i16_tempImageSmooth[Y_Cnt + 1][X_Cnt] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      i16_Gradient[Y_Cnt][X_Cnt] = sqrt (i16_GradientX[Y_Cnt][X_Cnt] * i16_GradientX[Y_Cnt][X_Cnt] +
                                     i16_GradientY[Y_Cnt][X_Cnt] * i16_GradientY[Y_Cnt][X_Cnt]);

      if (i16_Gradient[Y_Cnt][X_Cnt] > i16_MaxValue)
      {
        i16_MaxValue = i16_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (i16_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      i16_Gradient[Y_Cnt][X_Cnt] = (i16_Gradient[Y_Cnt][X_Cnt] * 255) / i16_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      if (i16_tempImage[Y_Cnt+5][X_Cnt+5] > i32_AvarageValue)
      {
        plugin_set_voxel_at_point (mask, selection, ts_BlobCount, properties->value, properties->action);
      }
    }
  }
}

void
plugin_handle_float32 (PluginMetaData *metadata, PixelData *original, PixelData *mask, PixelData *selection, Coordinate point)
{
  PluginBrushProperties *properties = metadata->properties;

  Slice *slice = PIXELDATA_ACTIVE_SLICE (original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((point.x > (slice->matrix.i16_x - 5)) || (point.x < 5)
      || (point.y > (slice->matrix.i16_y - 5)) || (point.y < 5))
  {
    return;
  }

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  double d_tempValue;

  /* DECLARATIONS FOR INT16 */
  float f_tempImage[10][10];
  float f_tempImageSmooth[10][10];

  float f_GradientX[10][10];
  float f_GradientY[10][10];

  float f_Gradient[10][10];

  float f_AvarageValue = 0;

  // Initialize the data to zero.
  memset (&f_tempImage,       0, 100 * sizeof (float));
  memset (&f_tempImageSmooth, 0, 100 * sizeof (float));
  memset (&f_GradientX,       0, 100 * sizeof (float));
  memset (&f_GradientY,       0, 100 * sizeof (float));
  memset (&f_Gradient,        0, 100 * sizeof (float));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  f_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      plugin_get_voxel_at_point (original, ts_BlobCount, &(f_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      f_AvarageValue += f_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  f_AvarageValue = f_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      d_tempValue =
        // first row kernel
        (int)(f_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(f_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(f_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(f_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(f_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        f_tempImageSmooth[Y_Cnt][X_Cnt] = (float)(d_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  float f_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      f_GradientX[Y_Cnt][X_Cnt] =
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * f_tempImageSmooth[Y_Cnt][X_Cnt + 1] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * f_tempImageSmooth[Y_Cnt][X_Cnt + 1] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      f_GradientY[Y_Cnt][X_Cnt] =
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * f_tempImageSmooth[Y_Cnt - 1][X_Cnt] + f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (f_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * f_tempImageSmooth[Y_Cnt + 1][X_Cnt] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      f_Gradient[Y_Cnt][X_Cnt] = sqrt (f_GradientX[Y_Cnt][X_Cnt] * f_GradientX[Y_Cnt][X_Cnt] +
                                       f_GradientY[Y_Cnt][X_Cnt] * f_GradientY[Y_Cnt][X_Cnt]);

      if (f_Gradient[Y_Cnt][X_Cnt] > f_MaxValue)
      {
        f_MaxValue = f_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (f_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      f_Gradient[Y_Cnt][X_Cnt] = (f_Gradient[Y_Cnt][X_Cnt] * 255) / f_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = point.x + X_Cnt;

      if (f_tempImage[Y_Cnt+5][X_Cnt+5] > f_AvarageValue)
      {
        plugin_set_voxel_at_point (mask, selection, ts_BlobCount, properties->value, properties->action);
      }
    }
  }
}
