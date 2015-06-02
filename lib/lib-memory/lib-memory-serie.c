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
#include <libgen.h>

/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
unsigned long long
memory_serie_next_id ()
{
  debug_functions ();

  static unsigned long long id = 0;
  id++;
  return id;
}

Serie*
memory_serie_new (const char *name, const char *pc_filename)
{
  debug_functions ();

  Serie *serie;
  serie = calloc (1, sizeof (Serie));
  assert (serie != NULL);

  if (name != NULL && strlen (name) > 0)
    memcpy (serie->name, name, strlen (name));

  if ((pc_filename != NULL)&&(strlen(pc_filename)>0))
  {
    serie->pc_filename = calloc(1,strlen(pc_filename));
    memcpy (serie->pc_filename, pc_filename, strlen (pc_filename));
  }

  serie->id = memory_serie_next_id ();
  serie->group_id = memory_serie_next_id ();

  return serie;
}



void
memory_serie_destroy (void *data)
{
  debug_functions ();

  if (data == NULL) return;
  Serie *serie = (Serie *)data;

  free (serie->pc_filename), serie->pc_filename=NULL;
  free (serie->data), serie->data = NULL;
  free (serie->pv_OutOfBlobValue), serie->pv_OutOfBlobValue = NULL;
  free (serie->ps_Quaternion), serie->ps_Quaternion = NULL;
  free (serie->ps_QuaternationOffset), serie->ps_QuaternationOffset = NULL;
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

  unsigned int i32_memory_size = serie->matrix.i16_x * serie->matrix.i16_y * serie->matrix.i16_z * serie->num_time_series;
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

Serie *
memory_serie_create_mask_from_serie (Serie *serie)
{
  debug_functions ();

  Serie* mask = memory_serie_new (NULL,NULL);
  assert (mask != NULL);
  mask->e_SerieType = SERIE_MASK;
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
    mask->matrix.i16_x * mask->matrix.i16_y * mask->matrix.i16_z *
    memory_serie_get_memory_space (mask) * mask->num_time_series;

  debug_extra ("About to allocate: ~ %.2f megabytes.", data_size / 1000000.0);

  mask->data = calloc (1, data_size);
  assert (mask->data != NULL);

//  memset (mask->data, 0, data_size);

  mask->pv_OutOfBlobValue = calloc (1, memory_serie_get_memory_space (mask));

  mask->ps_Quaternion =calloc (1, sizeof (ts_Quaternion));
  mask->ps_QuaternationOffset = calloc (1, sizeof (ts_Quaternion));

  mask->i16_QuaternionCode = serie->i16_QuaternionCode;
  mask->d_Qfac = serie->d_Qfac;

  mask->ps_Quaternion->I = serie->ps_Quaternion->I;
  mask->ps_Quaternion->J = serie->ps_Quaternion->J;
  mask->ps_Quaternion->K = serie->ps_Quaternion->K;
  mask->ps_Quaternion->W = serie->ps_Quaternion->W;

  mask->ps_QuaternationOffset->I = serie->ps_QuaternationOffset->I;
  mask->ps_QuaternationOffset->J = serie->ps_QuaternationOffset->J;
  mask->ps_QuaternationOffset->K = serie->ps_QuaternationOffset->K;
  mask->ps_QuaternationOffset->W = serie->ps_QuaternationOffset->W;

  mask->t_ScannerSpaceIJKtoXYZ = serie->t_ScannerSpaceIJKtoXYZ;
  mask->t_ScannerSpaceXYZtoIJK = serie->t_ScannerSpaceXYZtoIJK;

  mask->i16_StandardSpaceCode = serie->i16_StandardSpaceCode;
  mask->t_StandardSpaceIJKtoXYZ = serie->t_StandardSpaceIJKtoXYZ;
  mask->t_StandardSpaceXYZtoIJK = serie->t_StandardSpaceXYZtoIJK;

  mask->i32_MinimumValue = 0;
  mask->i32_MaximumValue = 255;
  mask->u8_AxisUnits = serie->u8_AxisUnits;

  if((mask->i16_StandardSpaceCode == NIFTI_XFORM_UNKNOWN) &&
         (mask->i16_QuaternionCode == NIFTI_XFORM_UNKNOWN))
  {
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[0][0]=1;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[0][1]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[0][2]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[0][3]=0;

    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[1][0]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[1][1]=1;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[1][2]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[1][3]=0;

    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[2][0]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[2][1]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[2][2]=1;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[2][3]=0;

    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[3][0]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[3][1]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[3][2]=0;
    mask->t_StandardSpaceIJKtoXYZ.d_Matrix[3][3]=1;

    mask->t_StandardSpaceXYZtoIJK = tda_memory_quaternion_inverse_matrix(&mask->t_StandardSpaceIJKtoXYZ);

    mask->pt_RotationMatrix = &mask->t_StandardSpaceIJKtoXYZ;
    mask->pt_InverseMatrix = &mask->t_StandardSpaceXYZtoIJK;
  }
  else if((mask->i16_StandardSpaceCode == NIFTI_XFORM_ALIGNED_ANAT) ||
          (mask->i16_StandardSpaceCode == NIFTI_XFORM_TALAIRACH) ||
          (mask->i16_StandardSpaceCode == NIFTI_XFORM_MNI_152))
  {
    mask->t_StandardSpaceXYZtoIJK = tda_memory_quaternion_inverse_matrix(&mask->t_StandardSpaceIJKtoXYZ);
    mask->pt_RotationMatrix = &mask->t_StandardSpaceIJKtoXYZ;
    mask->pt_InverseMatrix = &mask->t_StandardSpaceXYZtoIJK;
  }
  else if(mask->i16_QuaternionCode == NIFTI_XFORM_SCANNER_ANAT)
  {
    //v_memory_io_handleSpace (mask);
    mask->pt_RotationMatrix = &mask->t_ScannerSpaceIJKtoXYZ;
    mask->pt_InverseMatrix = &mask->t_ScannerSpaceXYZtoIJK;
  }



  char *extension = strstr (serie->name, ".nii");
  if (extension != NULL)
  {
    char *pc_pathToOriginalSerie = dirname (serie->pc_filename);
    char *pc_maskFileName = NULL;

    char serie_name[strlen (serie->name) - 3];
    memset (serie_name, 0, sizeof (serie_name));
    strncpy (serie_name, serie->name, strlen (serie->name) - 4);
    snprintf (mask->name, 100, "%s_mask_%02d.nii", serie_name, (int)(mask->id));

    pc_maskFileName = calloc(1, strlen(pc_pathToOriginalSerie)+2+strlen(mask->name));

    strcpy(pc_maskFileName,pc_pathToOriginalSerie);
    strcpy(&pc_maskFileName[strlen(pc_maskFileName)],"/");
    strcpy(&pc_maskFileName[strlen(pc_maskFileName)],mask->name);

    mask->pc_filename=pc_maskFileName;
  }
  else
  {
    char *pc_pathToOriginalSerie = dirname (serie->pc_filename);
    char *pc_maskFileName = NULL;

    char serie_name[100];
    memset (serie_name, 0, sizeof (serie_name));
    strncpy (serie_name, serie->name, 90);
    snprintf (mask->name, 100, "%s_mask_%02d", serie_name, (int)(mask->id));

    pc_maskFileName = calloc(1, strlen(pc_pathToOriginalSerie)+2+strlen(mask->name));

    strcpy(pc_maskFileName,pc_pathToOriginalSerie);
    strcpy(&pc_maskFileName[strlen(pc_maskFileName)],"/");
    strcpy(&pc_maskFileName[strlen(pc_maskFileName)],mask->name);

    mask->pc_filename=pc_maskFileName;
  }

  return mask;
}

Vector3D
memory_serie_GetPivotpoint(Serie *serie)
{
  Vector3D ts_PivotPoint;
  Vector3D ts_TmpBlobVector;

  ts_PivotPoint.x = 0;
  ts_PivotPoint.y = 0;
  ts_PivotPoint.z = 0;
  if (serie != NULL)
  {
    ts_TmpBlobVector.x=0;
    ts_TmpBlobVector.y=0;
    ts_TmpBlobVector.z=0;
    ts_TmpBlobVector.x = (float)((serie->matrix.i16_x-1)/2);
    ts_TmpBlobVector.y = (float)((serie->matrix.i16_y-1)/2);
    ts_TmpBlobVector.z = (float)((serie->matrix.i16_z-1)/2);

    ts_PivotPoint=ts_memory_matrix_multiply4x4(serie->pt_RotationMatrix, &ts_TmpBlobVector);
  }

  return ts_PivotPoint;
}

void
v_memory_io_handleSpace (Serie *serie)
{
  double d_Rotation=0;

  if (serie->i16_QuaternionCode == NIFTI_XFORM_UNKNOWN)
  {
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][0] = serie->pixel_dimension.x;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][1] = serie->pixel_dimension.y;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][2] = serie->pixel_dimension.z;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][3] = 1;

    serie->t_ScannerSpaceXYZtoIJK = tda_memory_quaternion_inverse_matrix(&serie->t_ScannerSpaceIJKtoXYZ);
  }
  else if (serie->i16_QuaternionCode == NIFTI_XFORM_SCANNER_ANAT)
  {
    d_Rotation = 1.0 - (serie->ps_Quaternion->I * serie->ps_Quaternion->I +
                        serie->ps_Quaternion->J * serie->ps_Quaternion->J +
                        serie->ps_Quaternion->K * serie->ps_Quaternion->K);

    if (d_Rotation == 0 )
    {
      /* Special case according to niftii standard !?!*/
      /* There should be a 180 degree rotation        */
      d_Rotation = 1 / sqrt((serie->ps_Quaternion->I * serie->ps_Quaternion->I +
                                          serie->ps_Quaternion->J * serie->ps_Quaternion->J +
                                          serie->ps_Quaternion->K * serie->ps_Quaternion->K));
      serie->ps_Quaternion->I *= d_Rotation;
      serie->ps_Quaternion->J *= d_Rotation;
      serie->ps_Quaternion->K *= d_Rotation;
      serie->ps_Quaternion->W = 0;
    }
    else
    {
      serie->ps_Quaternion->W = sqrt(d_Rotation);
    }

    serie->t_ScannerSpaceIJKtoXYZ = tda_memory_quaternion_to_matrix(serie->ps_Quaternion, serie->ps_QuaternationOffset, &serie->pixel_dimension, serie->d_Qfac);
    serie->t_ScannerSpaceXYZtoIJK = tda_memory_quaternion_inverse_matrix(&serie->t_ScannerSpaceIJKtoXYZ);
  }

}
