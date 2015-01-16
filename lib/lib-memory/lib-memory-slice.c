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

#include "lib-memory-slice.h"
#include "lib-memory-serie.h"
#include "lib-common-debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define MEMORY_CAST(data_type,source)                                    \
( ((data_type)==(MEMORY_TYPE_INT8))    ? ((signed char *)source) :       \
  (((data_type)==(MEMORY_TYPE_INT16))   ? ((signed short int*)source) :  \
  (((data_type)==(MEMORY_TYPE_INT32))   ? ((signed int*)source) :        \
  (((data_type)==(MEMORY_TYPE_INT64))   ? ((signed long int*)source) :   \
  (((data_type)==(MEMORY_TYPE_UINT8))   ? ((unsigned char*)source) :     \
  (((data_type)==(MEMORY_TYPE_UINT16))  ? ((unsigned short int*)source) :\
  (((data_type)==(MEMORY_TYPE_UINT32))  ? ((unsigned int*)source) :      \
  (((data_type)==(MEMORY_TYPE_UINT64))  ? ((unsigned long*)source) :     \
  (((data_type)==(MEMORY_TYPE_FLOAT32)) ? ((float*)source) :             \
  (((data_type)==(MEMORY_TYPE_FLOAT64)) ? ((double*)source) :            \
  ((void*)source) ))))))))))


float f_angleBetweenVectors(Vector3D *ps_upVector, Vector3D *ps_InputVector)
{
  debug_functions ();

  float f_inProduct;
  float f_upVectorLenght;
  float f_inputVectorLength;
  float f_angle;



  // Calculate the inproduct

  f_inProduct = ps_upVector->x * ps_InputVector->x + ps_upVector->y * ps_InputVector->y + ps_upVector->z * ps_InputVector->z;
  f_upVectorLenght = sqrt(ps_upVector->x * ps_upVector->x + ps_upVector->y * ps_upVector->y + ps_upVector->z * ps_upVector->z);
  f_inputVectorLength = sqrt(ps_InputVector->x * ps_InputVector->x + ps_InputVector->y * ps_InputVector->y + ps_InputVector->z * ps_InputVector->z);
  f_angle = f_inProduct /  (f_upVectorLenght * f_inputVectorLength);


  return acos(f_angle);
}


Vector3D ts_rotateArountAxis(Vector3D *ps_Axis, Vector3D *ps_vectorToRotate, float f_Angle)
{
  debug_functions ();

  Vector3D ts_tmp;

  ts_tmp.x = 0;
  ts_tmp.y = 0;
  ts_tmp.z = 0;

  float costheta,sintheta;

  costheta = cos(f_Angle);
  sintheta = sin(f_Angle);

  ts_tmp.x += (costheta + (1 - costheta) * ps_Axis->x * ps_Axis->x) * ps_vectorToRotate->x;
  ts_tmp.x += ((1 - costheta) * ps_Axis->x * ps_Axis->y - ps_Axis->z * sintheta) * ps_vectorToRotate->y;
  ts_tmp.x += ((1 - costheta) * ps_Axis->x * ps_Axis->z + ps_Axis->y * sintheta) * ps_vectorToRotate->z;

  ts_tmp.y += ((1 - costheta) * ps_Axis->x * ps_Axis->y + ps_Axis->z * sintheta) * ps_vectorToRotate->x;
  ts_tmp.y += (costheta + (1 - costheta) * ps_Axis->y * ps_Axis->y) * ps_vectorToRotate->y;
  ts_tmp.y += ((1 - costheta) * ps_Axis->y * ps_Axis->z - ps_Axis->x * sintheta) * ps_vectorToRotate->z;

  ts_tmp.z += ((1 - costheta) * ps_Axis->x * ps_Axis->z - ps_Axis->y * sintheta) * ps_vectorToRotate->x;
  ts_tmp.z += ((1 - costheta) * ps_Axis->y * ps_Axis->z + ps_Axis->x * sintheta) * ps_vectorToRotate->y;
  ts_tmp.z += (costheta + (1 - costheta) * ps_Axis->z * ps_Axis->z) * ps_vectorToRotate->z;

  return(ts_tmp);
}



Vector3D s_perpendicular(Vector3D *ps_InputVector)
{
  debug_functions ();

  Vector3D s_OutputVector;
  s_OutputVector.x=0;
  s_OutputVector.y=0;
  s_OutputVector.z=0;

  assert((ps_InputVector->x != 0) || (ps_InputVector->y != 0) || (ps_InputVector->z != 0));

  if (ps_InputVector->x == 0)
  {
    s_OutputVector.x=1;
    return s_OutputVector;
  }
  else if (ps_InputVector->y == 0)
  {
    s_OutputVector.y=1;
    return s_OutputVector;
  }
  else if (ps_InputVector->z == 0)
  {
    s_OutputVector.z=1;
    return s_OutputVector;
  }


  s_OutputVector.x=1;
  s_OutputVector.y=1;
  s_OutputVector.z= -1 * ((ps_InputVector->x  + ps_InputVector->y) / ps_InputVector->z);
  return s_OutputVector;
}
Vector3D s_crossproduct(Vector3D *ps_InputVector,Vector3D *pts_perpendicularVector)
{
  debug_functions ();

  Vector3D s_OutputVector;
  s_OutputVector.x = ps_InputVector->y * pts_perpendicularVector->z - ps_InputVector->z * pts_perpendicularVector->y;
  s_OutputVector.y = ps_InputVector->z * pts_perpendicularVector->x - ps_InputVector->x * pts_perpendicularVector->z;
  s_OutputVector.z = ps_InputVector->x * pts_perpendicularVector->y - ps_InputVector->y * pts_perpendicularVector->x;

  return s_OutputVector;
}

float f_MaximumValue(Vector3D *ps_InputVector)
{
  debug_functions ();

  if ((ps_InputVector->x >= ps_InputVector->y) && (ps_InputVector->x >= ps_InputVector->z))
  {
    return ps_InputVector->x;
  }
  else if ((ps_InputVector->y >= ps_InputVector->x) && (ps_InputVector->y >= ps_InputVector->z))
  {
    return ps_InputVector->y;
  }
  else
  {
    return ps_InputVector->z;
  }
}

float f_MinimumValue(Vector3D *ps_InputVector)
{
  debug_functions ();

  if ((ps_InputVector->x < ps_InputVector->y) && (ps_InputVector->x < ps_InputVector->z))
  {
    return ps_InputVector->x;
  }
  else if ((ps_InputVector->y < ps_InputVector->x) && (ps_InputVector->y < ps_InputVector->z))
  {
    return ps_InputVector->y;
  }
  else
  {
    return ps_InputVector->z;
  }
}

Vector3D s_normalize(Vector3D *ps_InputVector)
{
  debug_functions ();

  float f_Max;
  float f_Min;
  Vector3D s_NormalizeVector;

  f_Max=f_MaximumValue(ps_InputVector);
  f_Min=f_MinimumValue(ps_InputVector);

  if (f_Max > fabs(f_Min))
  {
    s_NormalizeVector.x=ps_InputVector->x / f_Max;
    s_NormalizeVector.y=ps_InputVector->y / f_Max;
    s_NormalizeVector.z=ps_InputVector->z / f_Max;
  }
  else
  {
    s_NormalizeVector.x=ps_InputVector->x / fabs(f_Min);
    s_NormalizeVector.y=ps_InputVector->y / fabs(f_Min);
    s_NormalizeVector.z=ps_InputVector->z / fabs(f_Min);
  }
  return s_NormalizeVector;
}



void*
memory_slice_get_data (Slice *slice)
{
  debug_functions ();

  assert (slice != NULL);
  Serie *serie = slice->serie;
  assert (serie != NULL);

  Vector3D ts_perpendicularVector;
  Vector3D ts_crossproductVector;
  Vector3D ts_pointInPlane;

  Vector3D ts_PositionVector;

  short int i16_BytesToRead;

  int i32_MemoryPerSlice;
  int i32_MemoryOffset;
  int i32_MemoryInBlob;
  int i32_MemoryTimeSerieOffset;

  float f_widthCnt;
  float f_heightCnt;

  void *pv_OrigData = NULL;
  void **ppv_Data = NULL;
  void **ppv_CntData = NULL;


  /*------------------------------------------------------------------------------+
  | STEP 1 Determine a perpendicular and crossproduct vector to the normal vactor. |
  +-------------------------------------------------------------------------------*/
  *slice->ps_NormalVector = s_normalize(slice->ps_NormalVector);


  ts_perpendicularVector.x = 0;
  ts_perpendicularVector.y = 0;
  ts_perpendicularVector.z = 0;

  if (slice->ps_upVector->x == 1)
  {
    ts_perpendicularVector.x = 1;

    if (slice->ps_NormalVector->y != 0)
    {
      ts_perpendicularVector.y = 1 * slice->ps_NormalVector->x/slice->ps_NormalVector->y;
    }
    else if (slice->ps_NormalVector->z != 0)
    {
      ts_perpendicularVector.z = 1 * slice->ps_NormalVector->x/slice->ps_NormalVector->z;
    }
  }
  else if (slice->ps_upVector->y == 1)
  {
    //
    ts_perpendicularVector.y = 1;
    if (slice->ps_NormalVector->z != 0)
    {
      ts_perpendicularVector.z = 1 * slice->ps_NormalVector->y/slice->ps_NormalVector->z;
    }
    else if (slice->ps_NormalVector->x != 0)
    {
      ts_perpendicularVector.x = 1 * slice->ps_NormalVector->y/slice->ps_NormalVector->x;
    }
  }
  else if (slice->ps_upVector->z == 1)
  {
    ts_perpendicularVector.z = 1;

    if (slice->ps_NormalVector->y != 0)
    {
      ts_perpendicularVector.y = 1 * slice->ps_NormalVector->z/slice->ps_NormalVector->y;
    }
    else if (slice->ps_NormalVector->x != 0)
    {
      ts_perpendicularVector.x = 1 * slice->ps_NormalVector->z/slice->ps_NormalVector->x;
    }

  }

  ts_perpendicularVector = s_normalize(&ts_perpendicularVector);

  ts_crossproductVector = s_crossproduct(slice->ps_NormalVector,&ts_perpendicularVector);
  ts_crossproductVector = s_normalize(&ts_crossproductVector);


  /*------------------------------------------------------------------------------+
  | STEP 2 Determine maximal width and height of the image to create              |
  +-------------------------------------------------------------------------------*/

  float f_NormalVectorSteps;

  f_NormalVectorSteps=0;
  while (1)
  {
    ts_pointInPlane.x = f_NormalVectorSteps * ts_perpendicularVector.x;
    ts_pointInPlane.y = f_NormalVectorSteps * ts_perpendicularVector.y;
    ts_pointInPlane.z = f_NormalVectorSteps * ts_perpendicularVector.z;

    if ((fabs(ts_pointInPlane.x) >= serie->matrix.x)
	|| (fabs(ts_pointInPlane.y) >= serie->matrix.y)
	|| (fabs(ts_pointInPlane.z) > serie->matrix.z))
    {
      break;
    }

    f_NormalVectorSteps++;
  }

  slice->matrix.x = f_NormalVectorSteps;


  f_NormalVectorSteps = 0;
  while (1)
  {
    ts_pointInPlane.x = f_NormalVectorSteps * ts_crossproductVector.x;
    ts_pointInPlane.y = f_NormalVectorSteps * ts_crossproductVector.y;
    ts_pointInPlane.z = f_NormalVectorSteps * ts_crossproductVector.z;

    if ((fabs(ts_pointInPlane.x) >= serie->matrix.x)
	|| (fabs(ts_pointInPlane.y) >= serie->matrix.y)
	|| (fabs(ts_pointInPlane.z) > serie->matrix.z))
    {
      break;
    }

    f_NormalVectorSteps++;
  }

  slice->matrix.y = f_NormalVectorSteps;

  slice->matrix.x = (int)(slice->matrix.x);
  slice->matrix.y = (int)(slice->matrix.y);
  slice->matrix.z = (int)(slice->matrix.z);

  i16_BytesToRead = memory_serie_get_memory_space (serie);
  i32_MemoryInBlob = serie->matrix.z * serie->matrix.y * serie->matrix.x * i16_BytesToRead;
  i32_MemoryPerSlice = slice->matrix.x * slice->matrix.y * sizeof (void *);
  i32_MemoryTimeSerieOffset = serie->matrix.z * serie->matrix.y *
                              serie->matrix.x * i16_BytesToRead *
                              slice->u16_timePoint;

  ppv_Data = calloc (1, i32_MemoryPerSlice);
  assert (ppv_Data != NULL);

  ts_PositionVector.x = slice->matrix.z * slice->ps_NormalVector->x + slice->ps_PivotPoint->x;
  ts_PositionVector.y = slice->matrix.z * slice->ps_NormalVector->y + slice->ps_PivotPoint->y;
  ts_PositionVector.z = slice->matrix.z * slice->ps_NormalVector->z + slice->ps_PivotPoint->z;

  ppv_CntData = ppv_Data;

  for(f_heightCnt=slice->matrix.y/-2; f_heightCnt < slice->matrix.y/2; f_heightCnt++)
  {
    for(f_widthCnt=slice->matrix.x/-2; f_widthCnt < slice->matrix.x/2; f_widthCnt++)
    {
      ts_pointInPlane.x = ts_PositionVector.x + f_widthCnt*ts_perpendicularVector.x + f_heightCnt* ts_crossproductVector.x;
      ts_pointInPlane.y = ts_PositionVector.y + f_widthCnt*ts_perpendicularVector.y + f_heightCnt* ts_crossproductVector.y;
      ts_pointInPlane.z = ts_PositionVector.z + f_widthCnt*ts_perpendicularVector.z + f_heightCnt* ts_crossproductVector.z;

      if ((ts_pointInPlane.x > serie->matrix.x) || (ts_pointInPlane.y > serie->matrix.y) || (ts_pointInPlane.z > serie->matrix.z) ||
          (ts_pointInPlane.x < 0) || (ts_pointInPlane.y < 0) || (ts_pointInPlane.z < 0))
      {
        pv_OrigData = serie->pv_OutOfBlobValue;
      }
      else
      {
        i32_MemoryOffset = ((int)(ts_pointInPlane.z) * (int)(serie->matrix.y) * (int)(serie->matrix.x) + //depth offset in blob
                            (int)(ts_pointInPlane.y) * (int)(serie->matrix.x) +  // Y offset in blob
                            (int)(ts_pointInPlane.x)) * i16_BytesToRead;
        pv_OrigData = serie->data;
        pv_OrigData += i32_MemoryOffset;
        pv_OrigData += i32_MemoryTimeSerieOffset;

        if ((i32_MemoryOffset <0 ) || (i32_MemoryOffset > i32_MemoryInBlob))
        {
          pv_OrigData = serie->pv_OutOfBlobValue;
        }

      }

      *ppv_CntData=MEMORY_CAST(serie->data_type,pv_OrigData);
      ppv_CntData++;
    }
  }

  slice->f_ScaleFactorX=sqrt( (ts_perpendicularVector.x * serie->pixel_dimension.x)*(ts_perpendicularVector.x * serie->pixel_dimension.x) +
                              (ts_perpendicularVector.y * serie->pixel_dimension.y)*(ts_perpendicularVector.y * serie->pixel_dimension.y) +
                              (ts_perpendicularVector.z * serie->pixel_dimension.z)*(ts_perpendicularVector.z * serie->pixel_dimension.z));

  slice->f_ScaleFactorY=sqrt( (ts_crossproductVector.x * serie->pixel_dimension.x)*(ts_crossproductVector.x * serie->pixel_dimension.x) +
                              (ts_crossproductVector.y * serie->pixel_dimension.y)*(ts_crossproductVector.y * serie->pixel_dimension.y) +
                              (ts_crossproductVector.z * serie->pixel_dimension.z)*(ts_crossproductVector.z * serie->pixel_dimension.z));


  if (slice->f_ScaleFactorX > slice->f_ScaleFactorY)
  {
    slice->f_ScaleFactorX=slice->f_ScaleFactorX/slice->f_ScaleFactorY;
    slice->f_ScaleFactorY=1;
  }
  else
  {
    slice->f_ScaleFactorY=slice->f_ScaleFactorY/slice->f_ScaleFactorX;
    slice->f_ScaleFactorX=1;
  }

  return ppv_Data;
}


void
memory_slice_destroy (void *data)
{
  debug_functions ();

  if (data == NULL) return;

  Slice *slice = (Slice *)data;
  slice->serie = NULL;

  free (slice->data);
  slice->data = NULL;

  free (slice);
}


Slice*
memory_slice_new (Serie *serie)
{
  debug_functions ();

  assert (serie != NULL);

  Slice *slice = calloc (1, sizeof (Slice));
  slice->serie = serie;

  slice->ps_NormalVector =NULL;
  slice->ps_PivotPoint = NULL;

  return slice;
}

Slice*
memory_slice_get_next (Slice *slice)
{
  debug_functions ();

  slice->matrix.z += 1;

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  slice->data = memory_slice_get_data (slice);
  assert (slice->data != NULL);

  return slice;
}


Slice*
memory_slice_get_previous (Slice *slice)
{
  debug_functions ();

  slice->matrix.z -= 1;

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  slice->data = memory_slice_get_data (slice);
  assert (slice->data != NULL);

  return slice;
}


Slice*
memory_slice_get_nth (Slice *slice, int nth)
{
  debug_functions ();

  slice->matrix.z = nth;

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  slice->data = memory_slice_get_data (slice);
  assert (slice->data != NULL);


  return slice;
}

void
memory_slice_set_timepoint (Slice *slice, unsigned short int timepoint)
{
  debug_functions ();

  slice->u16_timePoint = timepoint;
}
