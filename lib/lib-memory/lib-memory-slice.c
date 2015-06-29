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
#include "lib-common-algebra.h"

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
Vector3D s_memory_slice_GetCurrentPosition(float f_Depth, Vector3D *ps_NormalVector, Vector3D *ps_PivotPoint)
{
  Vector3D ts_PositionVector;

  ts_PositionVector.x = f_Depth * ps_NormalVector->x + ps_PivotPoint->x;
  ts_PositionVector.y = f_Depth * ps_NormalVector->y + ps_PivotPoint->y;
  ts_PositionVector.z = f_Depth * ps_NormalVector->z + ps_PivotPoint->z;

  return ts_PositionVector;
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

  ts_Vector3DInt ts_pointInPlane;

  Vector3D ts_floatingPointInPlane;
  Vector3D ts_PositionVector;
  Vector3D ts_TmpPosition;
  Vector3D ts_PivotVectorInBlob;

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
    *slice->ps_NormalVector = s_algebra_vector_normalize(slice->ps_NormalVector);

    p_ViewportProps->ts_normalVector.x=slice->ps_NormalVector->x;
    p_ViewportProps->ts_normalVector.y=slice->ps_NormalVector->y;
    p_ViewportProps->ts_normalVector.z=slice->ps_NormalVector->z;

    p_ViewportProps->ts_perpendicularVector = s_algebra_vector_perpendicular(&p_ViewportProps->ts_normalVector, slice->ps_upVector);
    p_ViewportProps->ts_crossproductVector = s_algebra_vector_crossproduct(&p_ViewportProps->ts_normalVector,&p_ViewportProps->ts_perpendicularVector);

    /*------------------------------------------------------------------------------+
    | STEP 2 Calculate the direction to build the image from                        |
    |        (e.g. left to right, or right to left)                                 |
    +-------------------------------------------------------------------------------*/
    ts_floatingPointInPlane.x = 0;
    ts_floatingPointInPlane.y = 0;
    ts_floatingPointInPlane.z = 0;

    ts_Startpoint = ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_floatingPointInPlane);

    ts_floatingPointInPlane.x = p_ViewportProps->ts_perpendicularVector.x;
    ts_floatingPointInPlane.y = p_ViewportProps->ts_perpendicularVector.y;
    ts_floatingPointInPlane.z = p_ViewportProps->ts_perpendicularVector.z;

    ts_EndPoint = ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_floatingPointInPlane);

    ts_Delta.x = ts_EndPoint.x - ts_Startpoint.x;
    ts_Delta.y = ts_EndPoint.y - ts_Startpoint.y;
    ts_Delta.z = ts_EndPoint.z - ts_Startpoint.z;

    p_ViewportProps->i16_StrideHeight = (f_algebra_vector_MinimumValue(&ts_Delta) < 0) ? -1 : 1;

    ts_floatingPointInPlane.x = p_ViewportProps->ts_crossproductVector.x;
    ts_floatingPointInPlane.y = p_ViewportProps->ts_crossproductVector.y;
    ts_floatingPointInPlane.z = p_ViewportProps->ts_crossproductVector.z;

    ts_EndPoint = ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_floatingPointInPlane);

    ts_Delta.x = ts_EndPoint.x - ts_Startpoint.x;
    ts_Delta.y = ts_EndPoint.y - ts_Startpoint.y;
    ts_Delta.z = ts_EndPoint.z - ts_Startpoint.z;

    p_ViewportProps->i16_StrideWidth = (f_algebra_vector_MinimumValue(&ts_Delta) < 0) ? -1 : 1;

    ts_floatingPointInPlane.x = p_ViewportProps->ts_normalVector.x;
    ts_floatingPointInPlane.y = p_ViewportProps->ts_normalVector.y;
    ts_floatingPointInPlane.z = p_ViewportProps->ts_normalVector.z;

    ts_EndPoint = ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_floatingPointInPlane);

    ts_Delta.x = ts_Startpoint.x - ts_EndPoint.x;
    ts_Delta.y = ts_Startpoint.y - ts_EndPoint.y;
    ts_Delta.z = ts_Startpoint.z - ts_EndPoint.z;

    p_ViewportProps->i16_StrideDepth = (f_algebra_vector_MinimumValue(&ts_Delta) < 0) ? -1 : 1;

    /*------------------------------------------------------------------------------+
    | STEP 3 Translate viewport vectors to a plane, get actual width and height     |
    |        of the image                                                           |
    +-------------------------------------------------------------------------------*/
    ts_pointInPlane.i16_x = (fabs(p_ViewportProps->ts_perpendicularVector.x) * serie->matrix.i16_x);
    ts_pointInPlane.i16_y = (fabs(p_ViewportProps->ts_perpendicularVector.y) * serie->matrix.i16_y);
    ts_pointInPlane.i16_z = (fabs(p_ViewportProps->ts_perpendicularVector.z) * serie->matrix.i16_z);

    slice->matrix.i16_y = i16_algebra_vector_MaximumValue(&ts_pointInPlane);

    ts_pointInPlane.i16_x = (fabs(p_ViewportProps->ts_crossproductVector.x) * serie->matrix.i16_x);
    ts_pointInPlane.i16_y = (fabs(p_ViewportProps->ts_crossproductVector.y) * serie->matrix.i16_y);
    ts_pointInPlane.i16_z = (fabs(p_ViewportProps->ts_crossproductVector.z) * serie->matrix.i16_z);

    slice->matrix.i16_x = i16_algebra_vector_MaximumValue(&ts_pointInPlane);

    /*------------------------------------------------------------------------------+
    | STEP 4 Calculate the starting and ending point of the width and height        |
    +-------------------------------------------------------------------------------*/

    //if the strides are negative, the vector should change direction
    p_ViewportProps->ts_perpendicularVector.x *= p_ViewportProps->i16_StrideHeight;
    p_ViewportProps->ts_perpendicularVector.y *= p_ViewportProps->i16_StrideHeight;
    p_ViewportProps->ts_perpendicularVector.z *= p_ViewportProps->i16_StrideHeight;

    p_ViewportProps->ts_crossproductVector.x *= p_ViewportProps->i16_StrideWidth;
    p_ViewportProps->ts_crossproductVector.y *= p_ViewportProps->i16_StrideWidth;
    p_ViewportProps->ts_crossproductVector.z *= p_ViewportProps->i16_StrideWidth;

    ts_PivotVectorInBlob = ts_algebra_vector_translate(serie->pt_InverseMatrix, slice->ps_PivotPoint);
    ts_PositionVector = s_memory_slice_GetCurrentPosition(slice->matrix.i16_z,&p_ViewportProps->ts_normalVector, &ts_PivotVectorInBlob);

    p_ViewportProps->i16_StartHeight = 0;
    p_ViewportProps->i16_StopHeight = 0;

    for(i16_heightCnt=0; i16_heightCnt < slice->matrix.i16_y; i16_heightCnt++)
    {
      ts_PositionVector.x += p_ViewportProps->ts_perpendicularVector.x;
      ts_PositionVector.y += p_ViewportProps->ts_perpendicularVector.y;
      ts_PositionVector.z += p_ViewportProps->ts_perpendicularVector.z;

      if ((ts_PositionVector.x > serie->matrix.i16_x) || (ts_PositionVector.y > serie->matrix.i16_y) || (ts_PositionVector.z > serie->matrix.i16_z))
      {
        p_ViewportProps->i16_StopHeight = i16_heightCnt;
        p_ViewportProps->i16_StartHeight = p_ViewportProps->i16_StopHeight - slice->matrix.i16_y;
        break;
      }

      if ((ts_PositionVector.x < 0) || (ts_PositionVector.y < 0) || (ts_PositionVector.z < 0))
      {
        p_ViewportProps->i16_StartHeight = i16_heightCnt;
        p_ViewportProps->i16_StopHeight = p_ViewportProps->i16_StartHeight- slice->matrix.i16_y;
        break;
      }
    }

    ts_PivotVectorInBlob = ts_algebra_vector_translate(serie->pt_InverseMatrix, slice->ps_PivotPoint);
    ts_PositionVector = s_memory_slice_GetCurrentPosition(slice->matrix.i16_z,&p_ViewportProps->ts_normalVector, &ts_PivotVectorInBlob);


    p_ViewportProps->i16_StartWidth = 0;
    p_ViewportProps->i16_StopWidth = 0;

    for(i16_widthCnt=0; i16_widthCnt < slice->matrix.i16_x; i16_widthCnt++)
    {
      ts_PositionVector.x += p_ViewportProps->ts_crossproductVector.x;
      ts_PositionVector.y += p_ViewportProps->ts_crossproductVector.y;
      ts_PositionVector.z += p_ViewportProps->ts_crossproductVector.z;

      if ((ts_PositionVector.x > serie->matrix.i16_x) || (ts_PositionVector.y > serie->matrix.i16_y) || (ts_PositionVector.z > serie->matrix.i16_z))
      {
        p_ViewportProps->i16_StopWidth = i16_widthCnt;
        p_ViewportProps->i16_StartWidth = p_ViewportProps->i16_StopWidth - slice->matrix.i16_x;
        break;
      }

      if ((ts_PositionVector.x < 0) || (ts_PositionVector.y < 0) || (ts_PositionVector.z < 0))
      {
        p_ViewportProps->i16_StartWidth = i16_widthCnt;
        p_ViewportProps->i16_StopWidth = p_ViewportProps->i16_StartWidth - slice->matrix.i16_x;
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

    slice->i16_ViewportChange = 0;
  }

  /*------------------------------------------------------------------------------+
  | STEP 6 Calculate the scaling parameters                                       |
  +-------------------------------------------------------------------------------*/


  ts_PivotVectorInBlob = ts_algebra_vector_translate(serie->pt_InverseMatrix, slice->ps_PivotPoint);
  ts_PositionVector = s_memory_slice_GetCurrentPosition(slice->matrix.i16_z,&p_ViewportProps->ts_normalVector, &ts_PivotVectorInBlob);



  i16_BytesToRead = memory_serie_get_memory_space (serie);
  i32_MemoryInBlob = serie->matrix.i16_z * serie->matrix.i16_y * serie->matrix.i16_x * i16_BytesToRead;

  i32_MemoryPerSlice = slice->matrix.i16_x * slice->matrix.i16_y * sizeof (void *);
  i32_MemoryTimeSerieOffset = serie->matrix.i16_z * serie->matrix.i16_y *
                              serie->matrix.i16_x * i16_BytesToRead *
                              slice->u16_timePoint;

  ppv_Data = calloc (1, i32_MemoryPerSlice);
  assert (ppv_Data != NULL);

  ppv_CntData = ppv_Data;


  short int i16_positionX;
  short int i16_positionY;
  short int i16_positionZ;

  /*

  clock_t begin, end;
  double time_spent;
  begin = clock();
  */

  short int i16_strideY = ((p_ViewportProps->i16_StopHeight - p_ViewportProps->i16_StartHeight) < 0) ? -1 : 1;
  short int i16_strideX = ((p_ViewportProps->i16_StopWidth - p_ViewportProps->i16_StartWidth) < 0) ? -1 : 1;

  i16_heightCnt = p_ViewportProps->i16_StartHeight;
  while (i16_heightCnt != p_ViewportProps->i16_StopHeight)
  {
    ts_TmpPosition.x = ts_PositionVector.x + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.x + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.x;
    ts_TmpPosition.y = ts_PositionVector.y + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.y + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.y;
    ts_TmpPosition.z = ts_PositionVector.z + i16_heightCnt*p_ViewportProps->ts_perpendicularVector.z + p_ViewportProps->i16_StartWidth * p_ViewportProps->ts_crossproductVector.z;

    i16_widthCnt=p_ViewportProps->i16_StartWidth;
    while (i16_widthCnt != p_ViewportProps->i16_StopWidth)
    {
      i16_positionX=(short int)(floor(ts_TmpPosition.x));
      i16_positionY=(short int)(floor(ts_TmpPosition.y));
      i16_positionZ=(short int)(floor(ts_TmpPosition.z));

      ts_PivotVectorInBlob = ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_TmpPosition);

      if ((i16_positionX > serie->matrix.i16_x) ||
          (i16_positionY > serie->matrix.i16_y) ||
          (i16_positionZ > serie->matrix.i16_z) ||
          (i16_positionX < 0) ||
          (i16_positionY < 0) ||
          (i16_positionZ < 0))
      {
        pv_OrigData = serie->pv_OutOfBlobValue;
      }
      else
      {
        // Flip Y
        if (i16_strideY == 1)
        {
          i16_positionY = serie->matrix.i16_y - i16_positionY;
        }

        if (i16_strideX == 1)
        {
          i16_positionX = serie->matrix.i16_x - i16_positionX;
        }

        i32_MemoryOffset  = ((int)(i16_positionZ * serie->matrix.i16_x * serie->matrix.i16_y) +
                             (int)(i16_positionY * serie->matrix.i16_x) +
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



      if (i16_strideX > 0)
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

      i16_widthCnt += i16_strideX;//p_ViewportProps->i16_StrideWidth;
    }
    i16_heightCnt += i16_strideY;//p_ViewportProps->i16_StrideHeight;
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

  slice->matrix.i16_z += 1;

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

  slice->matrix.i16_z -= 1;

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

  slice->matrix.i16_z = nth;

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
