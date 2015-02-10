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
#include "lib-memory-quaternion.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <time.h>

#define MEMORY_CAST(data_type,source)                                    \
(  ((data_type)==(MEMORY_TYPE_INT16))   ? ((signed short int*)source) :  \
  (((data_type)==(MEMORY_TYPE_FLOAT32)) ? ((float*)source) :             \
  (((data_type)==(MEMORY_TYPE_INT8))    ? ((signed char *)source) :      \
  (((data_type)==(MEMORY_TYPE_INT32))   ? ((signed int*)source) :        \
  (((data_type)==(MEMORY_TYPE_INT64))   ? ((signed long int*)source) :   \
  (((data_type)==(MEMORY_TYPE_UINT8))   ? ((unsigned char*)source) :     \
  (((data_type)==(MEMORY_TYPE_UINT16))  ? ((unsigned short int*)source) :\
  (((data_type)==(MEMORY_TYPE_UINT32))  ? ((unsigned int*)source) :      \
  (((data_type)==(MEMORY_TYPE_UINT64))  ? ((unsigned long*)source) :     \
  (((data_type)==(MEMORY_TYPE_FLOAT64)) ? ((double*)source) :            \
  ((void*)source) ))))))))))

/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */
float f_MinimumValue(Vector3D *ps_InputVector);

Coordinate3D s_GetCurrentPositionInBlob(float f_Depth, Vector3D *ps_NormalVector, Vector3D *ps_PivotPoint)
{
  Coordinate3D ts_PositionVector;

  ts_PositionVector.x = f_Depth * ps_NormalVector->x + ps_PivotPoint->x;
  ts_PositionVector.y = f_Depth * ps_NormalVector->y + ps_PivotPoint->y;
  ts_PositionVector.z = f_Depth * ps_NormalVector->z + ps_PivotPoint->z;

  return ts_PositionVector;
}

Vector3D s_normalize(Vector3D *ps_InputVector)
{
  debug_functions ();

  float f_magnitude=0;
  Vector3D s_NormalizeVector;

  f_magnitude = sqrt(ps_InputVector->x*ps_InputVector->x+ps_InputVector->y*ps_InputVector->y+ps_InputVector->z*ps_InputVector->z);

  s_NormalizeVector.x=ps_InputVector->x / f_magnitude;
  s_NormalizeVector.y=ps_InputVector->y / f_magnitude;
  s_NormalizeVector.z=ps_InputVector->z / f_magnitude;

  return s_NormalizeVector;
}

Vector3D s_perpendicular(Vector3D *ps_InputVector, Vector3D *ps_UpVector)
{
  debug_functions ();

  Vector3D ts_perpendicularVector;

  ts_perpendicularVector.x = 0;
  ts_perpendicularVector.y = 0;
  ts_perpendicularVector.z = 0;

  double d_UpProjection;
  double d_UpMagnitude;

  d_UpProjection = ps_InputVector->x * ps_UpVector->x + ps_InputVector->y * ps_UpVector->y + ps_InputVector->z * ps_UpVector->z;

  // first try at making a View Up vector: use World Up
  ts_perpendicularVector.x = ps_UpVector->x - d_UpProjection * ps_InputVector->x;
  ts_perpendicularVector.y = ps_UpVector->y - d_UpProjection * ps_InputVector->y;
  ts_perpendicularVector.z = ps_UpVector->z - d_UpProjection * ps_InputVector->z;

  // Check for validity:
  d_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;

  if (d_UpMagnitude < 0.0000001)
  {
    //Second try at making a View Up vector: Use Y axis default  (0,1,0)
    ts_perpendicularVector.x = -ps_InputVector->y * ps_InputVector->x;
    ts_perpendicularVector.y = 1-ps_InputVector->y * ps_InputVector->y;
    ts_perpendicularVector.z = -ps_InputVector->y * ps_InputVector->z;

    // Check for validity:
    d_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;

    if (d_UpMagnitude < 0.0000001)
    {
          //Final try at making a View Up vector: Use Z axis default  (0,0,1)
      ts_perpendicularVector.x = -ps_InputVector->z * ps_InputVector->x;
      ts_perpendicularVector.y = -ps_InputVector->z * ps_InputVector->y;
      ts_perpendicularVector.z = 1-ps_InputVector->z * ps_InputVector->z;

      // Check for validity:
      d_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;
      if (d_UpMagnitude < 0.0000001)
      {
        assert(d_UpMagnitude < 0.0000001);
      }

    }
  }

  // normalize the Up Vector
  ts_perpendicularVector = s_normalize(&ts_perpendicularVector);

  return ts_perpendicularVector;
}

Vector3D s_crossproduct(Vector3D *ps_InputVector,Vector3D *pts_perpendicularVector)
{
  debug_functions ();

  Vector3D s_OutputVector;

  s_OutputVector.x = ps_InputVector->y * pts_perpendicularVector->z - ps_InputVector->z * pts_perpendicularVector->y;
  s_OutputVector.y = ps_InputVector->z * pts_perpendicularVector->x - ps_InputVector->x * pts_perpendicularVector->z;
  s_OutputVector.z = ps_InputVector->x * pts_perpendicularVector->y - ps_InputVector->y * pts_perpendicularVector->x;

  if (f_MinimumValue(&s_OutputVector) < 0)
  {
    s_OutputVector.x *= -1;
    s_OutputVector.y *= -1;
    s_OutputVector.z *= -1;
  }

  s_OutputVector = s_normalize(&s_OutputVector);

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


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
void*
memory_slice_get_data (Slice *slice)
{
  debug_functions ();

  assert (slice != NULL);
  Serie *serie = slice->serie;
  assert (serie != NULL);

  Vector3D ts_pointInPlane;
  Vector3D ts_PositionVector;
  Vector3D ts_TmpPosition;

  Vector3D ts_Startpoint;
  Vector3D ts_EndPoint;
  Vector3D ts_Delta;


  ViewportProperties *p_ViewportProps = &slice->viewportProperties;

  short int i16_BytesToRead;

  int i32_MemoryPerSlice;
  int i32_MemoryOffset;
  int i32_MemoryInBlob;
  int i32_MemoryTimeSerieOffset;

  short int i16_widthCnt;
  short int i16_heightCnt;
  float f_CurrentDepth;

  void *pv_OrigData = NULL;
  void **ppv_Data = NULL;
  void **ppv_CntData = NULL;


  slice->i16_ViewportChange=1;

  if (slice->i16_ViewportChange)
  {
    /*------------------------------------------------------------------------------+
    | STEP 1 Determine the defined plane of the normalvector. The application uses  |
    | a right handed axis system!                                                   |
    +-------------------------------------------------------------------------------*/
    *slice->ps_NormalVector = s_normalize(slice->ps_NormalVector);

    slice->ps_NormalVector->x = slice->ps_NormalVector->x;
    slice->ps_NormalVector->y = slice->ps_NormalVector->y;
    slice->ps_NormalVector->z = slice->ps_NormalVector->z;

    p_ViewportProps->ts_perpendicularVector = s_perpendicular(slice->ps_NormalVector, slice->ps_upVector);
    p_ViewportProps->ts_crossproductVector = s_crossproduct(slice->ps_NormalVector,&p_ViewportProps->ts_perpendicularVector);

    /*------------------------------------------------------------------------------+
    | STEP 2 Translate viewport vectors to a plane, get actual width and height     |
    |        of the image                                                           |
    +-------------------------------------------------------------------------------*/
    ts_pointInPlane.x = (fabs(p_ViewportProps->ts_perpendicularVector.x) * serie->matrix.x);
    ts_pointInPlane.y = (fabs(p_ViewportProps->ts_perpendicularVector.y) * serie->matrix.y);
    ts_pointInPlane.z = (fabs(p_ViewportProps->ts_perpendicularVector.z) * serie->matrix.z);

    slice->matrix.y = f_MaximumValue(&ts_pointInPlane);

    ts_pointInPlane.x = (fabs(p_ViewportProps->ts_crossproductVector.x) * serie->matrix.x);
    ts_pointInPlane.y = (fabs(p_ViewportProps->ts_crossproductVector.y) * serie->matrix.y);
    ts_pointInPlane.z = (fabs(p_ViewportProps->ts_crossproductVector.z) * serie->matrix.z);

    slice->matrix.x = f_MaximumValue(&ts_pointInPlane);
    /*------------------------------------------------------------------------------+
    | STEP 3 Calculate the direction to build the image from                        |
    |        (e.g. left to right, or right to left)                                 |
    +-------------------------------------------------------------------------------*/
    ts_pointInPlane.x = 0;
    ts_pointInPlane.y = 0;
    ts_pointInPlane.z = 0;

    ts_Startpoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

    ts_pointInPlane.x = p_ViewportProps->ts_perpendicularVector.x;
    ts_pointInPlane.y = p_ViewportProps->ts_perpendicularVector.y;
    ts_pointInPlane.z = p_ViewportProps->ts_perpendicularVector.z;

    ts_EndPoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

    ts_Delta.x = ts_Startpoint.x - ts_EndPoint.x;
    ts_Delta.y = ts_Startpoint.y - ts_EndPoint.y;
    ts_Delta.z = ts_Startpoint.z - ts_EndPoint.z;

    p_ViewportProps->i32_StrideHeight = (f_MinimumValue(&ts_Delta) < 0) ? -1 : 1;


    ts_pointInPlane.x = p_ViewportProps->ts_crossproductVector.x;
    ts_pointInPlane.y = p_ViewportProps->ts_crossproductVector.y;
    ts_pointInPlane.z = p_ViewportProps->ts_crossproductVector.z;

    ts_EndPoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

    ts_Delta.x = ts_Startpoint.x - ts_EndPoint.x;
    ts_Delta.y = ts_Startpoint.y - ts_EndPoint.y;
    ts_Delta.z = ts_Startpoint.z - ts_EndPoint.z;

    p_ViewportProps->i32_StrideWidth = (f_MinimumValue(&ts_Delta) < 0) ? -1 : 1;

    ts_pointInPlane.x = slice->ps_NormalVector->x;
    ts_pointInPlane.y = slice->ps_NormalVector->y;
    ts_pointInPlane.z = slice->ps_NormalVector->z;

    ts_EndPoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

    ts_Delta.x = ts_Startpoint.x - ts_EndPoint.x;
    ts_Delta.y = ts_Startpoint.y - ts_EndPoint.y;
    ts_Delta.z = ts_Startpoint.z - ts_EndPoint.z;

    f_CurrentDepth = (f_MinimumValue(&ts_Delta) < 0) ? slice->matrix.z : -slice->matrix.z;


    /*------------------------------------------------------------------------------+
    | STEP 4 Calculate the starting and ending point of the width and height        |
    +-------------------------------------------------------------------------------*/
    ts_PositionVector = s_GetCurrentPositionInBlob(f_CurrentDepth, slice->ps_NormalVector, slice->ps_PivotPoint);

    p_ViewportProps->i16_StartHeight = 0;
    p_ViewportProps->i16_StopHeight = 1;

    for(i16_heightCnt=0; i16_heightCnt < slice->matrix.y; i16_heightCnt++)
    {
      ts_PositionVector.x += p_ViewportProps->ts_perpendicularVector.x;
      ts_PositionVector.y += p_ViewportProps->ts_perpendicularVector.y;
      ts_PositionVector.z += p_ViewportProps->ts_perpendicularVector.z;

      if ((ts_PositionVector.x > serie->matrix.x) || (ts_PositionVector.y > serie->matrix.y) || (ts_PositionVector.z > serie->matrix.z))
      {
        p_ViewportProps->i16_StopHeight = i16_heightCnt;
        p_ViewportProps->i16_StartHeight = p_ViewportProps->i16_StopHeight - slice->matrix.y;
        break;
      }

      if ((ts_PositionVector.x < 0) || (ts_PositionVector.y < 0) || (ts_PositionVector.z < 0))
      {
        p_ViewportProps->i16_StartHeight = -i16_heightCnt;
        p_ViewportProps->i16_StopHeight = p_ViewportProps->i16_StartHeight + slice->matrix.y;
        break;
      }
    }

    ts_PositionVector = s_GetCurrentPositionInBlob(f_CurrentDepth, slice->ps_NormalVector, slice->ps_PivotPoint);

    p_ViewportProps->i16_StartWidth = 0;
    p_ViewportProps->i16_StopWidth = 1;

    for(i16_widthCnt=0; i16_widthCnt < slice->matrix.x; i16_widthCnt++)
    {
      ts_PositionVector.x += p_ViewportProps->ts_crossproductVector.x;
      ts_PositionVector.y += p_ViewportProps->ts_crossproductVector.y;
      ts_PositionVector.z += p_ViewportProps->ts_crossproductVector.z;

      if ((ts_PositionVector.x > serie->matrix.x) || (ts_PositionVector.y > serie->matrix.y) || (ts_PositionVector.z > serie->matrix.z))
      {
        p_ViewportProps->i16_StopWidth = i16_widthCnt;
        p_ViewportProps->i16_StartWidth = p_ViewportProps->i16_StopWidth - slice->matrix.x;
        break;
      }

      if ((ts_PositionVector.x < 0) || (ts_PositionVector.y < 0) || (ts_PositionVector.z < 0))
      {
        p_ViewportProps->i16_StartWidth = -i16_widthCnt;
        p_ViewportProps->i16_StopWidth = p_ViewportProps->i16_StartWidth + slice->matrix.x;
        break;
      }
    }

    /*------------------------------------------------------------------------------+
    | STEP 5 Calculate the scaling parameters                                       |
    +-------------------------------------------------------------------------------*/

    /* Calculate the scaling params */
    slice->f_ScaleFactorY=sqrt( (p_ViewportProps->ts_perpendicularVector.x * serie->pixel_dimension.x)*(p_ViewportProps->ts_perpendicularVector.x * serie->pixel_dimension.x) +
                                (p_ViewportProps->ts_perpendicularVector.y * serie->pixel_dimension.y)*(p_ViewportProps->ts_perpendicularVector.y * serie->pixel_dimension.y) +
                                (p_ViewportProps->ts_perpendicularVector.z * serie->pixel_dimension.z)*(p_ViewportProps->ts_perpendicularVector.z * serie->pixel_dimension.z));

    slice->f_ScaleFactorX=sqrt( (p_ViewportProps->ts_crossproductVector.x * serie->pixel_dimension.x)*(p_ViewportProps->ts_crossproductVector.x * serie->pixel_dimension.x) +
                                (p_ViewportProps->ts_crossproductVector.y * serie->pixel_dimension.y)*(p_ViewportProps->ts_crossproductVector.y * serie->pixel_dimension.y) +
                                (p_ViewportProps->ts_crossproductVector.z * serie->pixel_dimension.z)*(p_ViewportProps->ts_crossproductVector.z * serie->pixel_dimension.z));

    // normalise the scaleFactor
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
    p_ViewportProps->i16_StartHeight /= p_ViewportProps->i32_StrideHeight;
    p_ViewportProps->i16_StopHeight /= p_ViewportProps->i32_StrideHeight;

    p_ViewportProps->i16_StartWidth /= p_ViewportProps->i32_StrideWidth;
    p_ViewportProps->i16_StopWidth /= p_ViewportProps->i32_StrideWidth;

    slice->i16_ViewportChange = 0;
  }

  ts_pointInPlane.x = 0;
  ts_pointInPlane.y = 0;
  ts_pointInPlane.z = 0;

  ts_Startpoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

  ts_pointInPlane.x = slice->ps_NormalVector->x;
  ts_pointInPlane.y = slice->ps_NormalVector->y;
  ts_pointInPlane.z = slice->ps_NormalVector->z;

  ts_EndPoint = ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_pointInPlane);

  ts_Delta.x = ts_Startpoint.x - ts_EndPoint.x;
  ts_Delta.y = ts_Startpoint.y - ts_EndPoint.y;
  ts_Delta.z = ts_Startpoint.z - ts_EndPoint.z;

  f_CurrentDepth = (f_MinimumValue(&ts_Delta) < 0) ? slice->matrix.z : -slice->matrix.z;



  /*------------------------------------------------------------------------------+
  | STEP 6 Calculate the scaling parameters                                       |
  +-------------------------------------------------------------------------------*/

  ts_PositionVector = s_GetCurrentPositionInBlob(f_CurrentDepth, slice->ps_NormalVector, slice->ps_PivotPoint);

  i16_BytesToRead = memory_serie_get_memory_space (serie);
  i32_MemoryInBlob = serie->matrix.z * serie->matrix.y * serie->matrix.x * i16_BytesToRead;

  i32_MemoryPerSlice = slice->matrix.x * slice->matrix.y * sizeof (void *);
  i32_MemoryTimeSerieOffset = serie->matrix.z * serie->matrix.y *
                              serie->matrix.x * i16_BytesToRead *
                              slice->u16_timePoint;

  ppv_Data = calloc (1, i32_MemoryPerSlice);
  assert (ppv_Data != NULL);

  ppv_CntData = ppv_Data;


  short int i16_positionX;
  short int i16_positionY;
  short int i16_positionZ;

  short int i16_MatrixX=(short int)(serie->matrix.x);
  short int i16_MatrixY=(short int)(serie->matrix.y);
  short int i16_MatrixZ=(short int)(serie->matrix.z);

  /*

  clock_t begin, end;
  double time_spent;
  begin = clock();
  */

  i16_heightCnt = p_ViewportProps->i16_StartHeight;
  while (i16_heightCnt != p_ViewportProps->i16_StopHeight)
  {
    ts_TmpPosition.x = ts_PositionVector.x + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.x + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.x;
    ts_TmpPosition.y = ts_PositionVector.y + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.y + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.y;
    ts_TmpPosition.z = ts_PositionVector.z + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.z + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.z;

    i16_widthCnt=p_ViewportProps->i16_StartWidth;
    while (i16_widthCnt != p_ViewportProps->i16_StopWidth)
    {
      if (p_ViewportProps->i32_StrideWidth > 0)
      {
        ts_TmpPosition.x +=  p_ViewportProps->ts_crossproductVector.x;
        ts_TmpPosition.y +=  p_ViewportProps->ts_crossproductVector.y;
        ts_TmpPosition.z +=  p_ViewportProps->ts_crossproductVector.z;
      }
      else
      {
        ts_TmpPosition.x -=  p_ViewportProps->ts_crossproductVector.x;
        ts_TmpPosition.y -=  p_ViewportProps->ts_crossproductVector.y;
        ts_TmpPosition.z -=  p_ViewportProps->ts_crossproductVector.z;
      }


      i16_positionX=(short int)(floor(ts_TmpPosition.x));
      i16_positionY=(short int)(floor(ts_TmpPosition.y));
      i16_positionZ=(short int)(floor(ts_TmpPosition.z));

      if ((i16_positionX > i16_MatrixX) ||
          (i16_positionY > i16_MatrixY) ||
          (i16_positionZ > i16_MatrixZ) ||
          (i16_positionX < 0) ||
          (i16_positionY < 0) ||
          (i16_positionZ < 0))
      {
        pv_OrigData = serie->pv_OutOfBlobValue;
      }
      else
      {
        i32_MemoryOffset  = ((int)(i16_positionZ * i16_MatrixX * i16_MatrixY) +
                             (int)(i16_positionY * i16_MatrixX) +
                             (int)(i16_positionX)) * i16_BytesToRead;


        if ((i32_MemoryOffset < 0 ) || (i32_MemoryOffset >= i32_MemoryInBlob))
        {
          pv_OrigData = serie->pv_OutOfBlobValue;
        }
        else
        {
          pv_OrigData = serie->data;
          pv_OrigData += i32_MemoryOffset;
          pv_OrigData += i32_MemoryTimeSerieOffset;
        }
      }

      *ppv_CntData=MEMORY_CAST(serie->data_type,pv_OrigData);
      ppv_CntData++;


      i16_widthCnt += p_ViewportProps->i32_StrideWidth;
    }
    i16_heightCnt += p_ViewportProps->i32_StrideHeight;
  }


  /*
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("timespent %10.5f sec\n",time_spent);
  */

  return ppv_Data;
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

void memory_slice_set_NormalVector (Slice *slice, Vector3D *ps_NormalVector)
{
  slice->ps_NormalVector = ps_NormalVector;
  slice->i16_ViewportChange = 1;
}

void memory_slice_set_PivotPoint (Slice *slice, Vector3D *ps_PivotPoint)
{
  slice->ps_PivotPoint = ps_PivotPoint;
  slice->i16_ViewportChange = 1;
}

void memory_slice_set_UpVector (Slice *slice, Vector3D *ps_upVector)
{
  slice->ps_upVector = ps_upVector;
  slice->i16_ViewportChange = 1;
}
