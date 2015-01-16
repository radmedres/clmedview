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

#include "lib-memory.h"
#include "lib-memory-serie.h"
#include "lib-memory-slice.h"
#include "lib-memory-tree.h"
#include "lib-memory-io.h"
#include "lib-common-debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>


unsigned long long
memory_serie_next_id ()
{
  debug_functions ();

  static unsigned long long id = 0;
  id++;
  return id;
}


Serie*
memory_serie_new (const char *name)
{
  debug_functions ();

  Serie *serie;
  serie = calloc (1, sizeof (Serie));
  assert (serie != NULL);

  if (name != NULL && strlen (name) > 0)
    memcpy (serie->name, name, strlen (name));

  serie->id = memory_serie_next_id ();
  return serie;
}


void
memory_serie_destroy (void *data)
{
  debug_functions ();

  if (data == NULL) return;
  Serie *serie = (Serie *)data;

  free (serie->data), serie->data = NULL;
  free (serie->pv_OutOfBlobValue), serie->pv_OutOfBlobValue = NULL;
  free (serie->ps_Quaternion), serie->ps_Quaternion = NULL;
  free (serie->ps_QuternationOffset), serie->ps_QuternationOffset = NULL;
  free (serie->pv_Header), serie->pv_Header = NULL;

  free (serie), serie = NULL;
}


MemoryDataType
memory_serie_get_memory_type (Serie *serie)
{
  debug_functions ();

  if (serie == NULL) return MEMORY_TYPE_NONE;
  return serie->data_type;
}


short int
memory_serie_get_memory_space (Serie *serie)
{
  debug_functions ();

  if (serie == NULL) return 0;

  short int num_bytes = 0;
  switch (serie->data_type)
  {
    case MEMORY_TYPE_INT8       : num_bytes = 1; break;
    case MEMORY_TYPE_INT16      : num_bytes = 2; break;
    case MEMORY_TYPE_INT32      : num_bytes = 4; break;
    case MEMORY_TYPE_INT64      : num_bytes = 8; break;

    case MEMORY_TYPE_UINT8      : num_bytes = 1; break;
    case MEMORY_TYPE_UINT16     : num_bytes = 2; break;
    case MEMORY_TYPE_UINT32     : num_bytes = 4; break;
    case MEMORY_TYPE_UINT64     : num_bytes = 8; break;

    case MEMORY_TYPE_FLOAT32    : num_bytes = 4; break;
    case MEMORY_TYPE_FLOAT64    : num_bytes = 8; break;
    case MEMORY_TYPE_FLOAT128   : num_bytes = 16; break;

    case MEMORY_TYPE_COMPLEX64  : num_bytes = 8; break;
    case MEMORY_TYPE_COMPLEX128 : num_bytes = 16; break;
    case MEMORY_TYPE_COMPLEX256 : num_bytes = 32; break;

    case MEMORY_TYPE_RGB24      : num_bytes = 3; break;
    case MEMORY_TYPE_RGBA32     : num_bytes = 4; break;
    default                     : break;
  }

  return num_bytes;
}


void
memory_serie_set_upper_and_lower_borders_from_data (Serie *serie)
{
  short int num_bytes = memory_serie_get_memory_space(serie);

  unsigned int i32_memory_size = serie->matrix.x * serie->matrix.y * serie->matrix.z * serie->num_time_series;
  unsigned int i32_blobCnt;

  int i32_Value = 0, i32_minimum, i32_maximum;

  void *pv_Data = serie->data;

  i32_minimum = INT_MAX;
  i32_maximum = INT_MIN;
  switch (serie->data_type)
  {
    case MEMORY_TYPE_INT8    :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(signed char*)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_INT16   :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(signed short int *)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_INT32   :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(signed int *)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_INT64   :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(signed long long *)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_UINT8   :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(unsigned char*)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_UINT16  :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(unsigned short int *)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_UINT32  :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(unsigned int*)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_UINT64  :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)(*(unsigned long long*)(pv_Data));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_FLOAT32 :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)roundf((*(float*)(pv_Data)));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    case MEMORY_TYPE_FLOAT64 :
      for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
      {
        i32_Value = (signed int)roundf((*(double*)(pv_Data)));
        i32_minimum = (i32_Value < i32_minimum) ? i32_Value : i32_minimum;
        i32_maximum = (i32_Value > i32_maximum) ? i32_Value : i32_maximum;
        pv_Data+=num_bytes;
      }
      break;
    default : break;
  }

  serie->i32_MaximumValue = i32_maximum;
  serie->i32_MinimumValue = i32_minimum;
}

void
memory_serie_convert_data_big_to_little_endian (Serie *serie)
{
  short int num_bytes = memory_serie_get_memory_space(serie);

  unsigned int i32_memory_size = serie->matrix.x * serie->matrix.y * serie->matrix.z * serie->num_time_series;
  unsigned int i32_blobCnt;

  void *pv_Data=serie->data;
  switch (serie->data_type)
  {
    case MEMORY_TYPE_INT8    :
    case MEMORY_TYPE_UINT8   :
      break;
    case MEMORY_TYPE_INT16   :
    case MEMORY_TYPE_UINT16  :
      {
        unsigned short int x16_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x16_Value = (unsigned short int)(*(unsigned short int *)(pv_Data));
          (*(unsigned short int *)(pv_Data)) = __bswap_16(x16_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    case MEMORY_TYPE_INT32   :
    case MEMORY_TYPE_UINT32  :
    case MEMORY_TYPE_FLOAT32 :
      {
        unsigned int x32_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x32_Value = (unsigned int)(*(unsigned int *)(pv_Data));
          (*(unsigned int *)(pv_Data)) = __bswap_32(x32_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    case MEMORY_TYPE_INT64   :
    case MEMORY_TYPE_UINT64  :
    case MEMORY_TYPE_FLOAT64 :
      {
        unsigned long long x64_Value;

        for (i32_blobCnt=0; i32_blobCnt<i32_memory_size; i32_blobCnt++)
        {
          x64_Value = (unsigned long long)(*(unsigned long long *)(pv_Data));
          (*(unsigned long long *)(pv_Data)) = __bswap_64(x64_Value);

          pv_Data+=num_bytes;
        }
      }
      break;
    default : break;
  }
}

Serie *
memory_serie_create_mask_from_serie (Serie *serie)
{
  debug_functions ();

  Serie* mask = memory_serie_new (NULL);
  assert (mask != NULL);

  mask->group_id = serie->group_id;
  mask->matrix = serie->matrix;
  mask->pixel_dimension = serie->pixel_dimension;
  mask->slope = serie->slope;
  mask->offset = serie->offset;
  mask->raw_data_type = 4;
  mask->data_type = MEMORY_TYPE_INT16;
  mask->input_type = serie->input_type;
  mask->num_time_series = serie->num_time_series;

  double data_size =
    mask->matrix.x * mask->matrix.y * mask->matrix.z *
    memory_serie_get_memory_space (mask) * mask->num_time_series;

  debug_extra ("About to allocate: ~ %.2f megabytes.", data_size / 1000000.0);

  mask->data = calloc (1, data_size);
  assert (mask->data != NULL);
  
  mask->pv_OutOfBlobValue = calloc (1, memory_serie_get_memory_space (mask));

  mask->pv_Header = calloc (1, MIN_HEADER_SIZE);
  mask->pv_Header = memcpy (mask->pv_Header, serie->pv_Header, MIN_HEADER_SIZE);

  mask->ps_Quaternion = memory_quaternion_new ();
  mask->ps_QuternationOffset = NULL;

  mask->i32_MinimumValue = 0;
  mask->i32_MaximumValue = 255;

  char *extension = strstr (serie->name, ".nii");
  if (extension != NULL)
  {
    char serie_name[strlen (serie->name) - 3];
    memset (serie_name, 0, sizeof (serie_name));
    strncpy (serie_name, serie->name, strlen (serie->name) - 4);
    snprintf (mask->name, 100, "%s_clone_%d.nii", serie_name, (int)mask->id);
  }
  else
  {
    snprintf (mask->name, 100, "%s_clone_%d.nii", serie->name, (int)mask->id);
  }

  return mask;
}
