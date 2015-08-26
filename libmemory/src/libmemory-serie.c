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

#include "libmemory.h"
#include "libmemory-serie.h"
#include "libmemory-slice.h"
#include "libmemory-tree.h"
#include "libmemory-io.h"
#include "libcommon-debug.h"


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
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[0][0]=1;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[1][0]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[2][0]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[3][0]=0;

    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[0][1]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[1][1]=1;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[2][1]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[3][1]=0;

    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[0][2]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[1][2]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[2][2]=1;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[3][2]=0;

    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[0][3]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[1][3]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[2][3]=0;
    mask->t_StandardSpaceIJKtoXYZ.af_Matrix[3][3]=1;

    mask->t_StandardSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&mask->t_StandardSpaceIJKtoXYZ);

    mask->pt_RotationMatrix = &mask->t_StandardSpaceIJKtoXYZ;
    mask->pt_InverseMatrix = &mask->t_StandardSpaceXYZtoIJK;
  }
  else if((mask->i16_StandardSpaceCode == NIFTI_XFORM_ALIGNED_ANAT) ||
          (mask->i16_StandardSpaceCode == NIFTI_XFORM_TALAIRACH) ||
          (mask->i16_StandardSpaceCode == NIFTI_XFORM_MNI_152))
  {
    mask->t_StandardSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&mask->t_StandardSpaceIJKtoXYZ);
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

    ts_PivotPoint=ts_algebra_vector_translate(serie->pt_RotationMatrix, &ts_TmpBlobVector);
  }

  return ts_PivotPoint;
}

void
v_memory_io_handleSpace (Serie *serie)
{
  if (serie->i16_QuaternionCode == NIFTI_XFORM_UNKNOWN)
  {
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][0] = 1;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][1] = 1;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][2] = 1;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][3] = 0;

    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][0] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][1] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][2] = 0;
    serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][3] = 1;

    serie->t_ScannerSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&serie->t_ScannerSpaceIJKtoXYZ);
  }
  else if (serie->i16_QuaternionCode == NIFTI_XFORM_SCANNER_ANAT)
  {
    serie->t_ScannerSpaceIJKtoXYZ = tda_algebra_matrix_4x4_QuaternionToMatrix(serie->ps_Quaternion, serie->ps_QuaternationOffset, serie->d_Qfac);
    serie->t_ScannerSpaceXYZtoIJK = tda_algebra_matrix_4x4_inverse(&serie->t_ScannerSpaceIJKtoXYZ);
  }

}

void v_memory_serie_MatrixToOrientation(ts_Matrix4x4 *pt_R , te_MemoryImageDirection *pe_I, te_MemoryImageDirection *pe_J, te_MemoryImageDirection *pe_K)
{
  Vector3D ts_I, ts_J, ts_K;
  float    f_DotProduct;
   float val,detQ,detP ;
   ts_Matrix3x3 P , Q , M ;
   int i,j,k=0,p,q,r , ibest,jbest,kbest,pbest,qbest,rbest ;
   float vbest ;


   if( pe_I == NULL || pe_J == NULL || pe_K == NULL )
   {
     return;
   }

   *pe_I = 0;
   *pe_J = 0;
   *pe_K = 0;

   /* load column vectors for each (i,j,k) direction from matrix */
  ts_I.x = pt_R->af_Matrix[0][0];
  ts_I.y = pt_R->af_Matrix[0][1];
  ts_I.z = pt_R->af_Matrix[0][2];

  ts_J.x = pt_R->af_Matrix[1][0];
  ts_J.y = pt_R->af_Matrix[1][1];
  ts_J.z = pt_R->af_Matrix[1][2];

  ts_K.x = pt_R->af_Matrix[2][0] ;
  ts_K.y = pt_R->af_Matrix[2][1] ;
  ts_K.z = pt_R->af_Matrix[2][2] ;

   /* normalize i,j ats_I.xs */
  ts_I = s_algebra_vector_normalize(&ts_I);
  ts_J = s_algebra_vector_normalize(&ts_J);
  ts_K = s_algebra_vector_normalize(&ts_K);


  /* orthogonalize j ats_I.xs to i ats_I.xs, if needed */
  f_DotProduct = f_algebra_vector_dotproduct(&ts_I, &ts_J);
  if( fabs(f_DotProduct) > 1.e-4 )
  {
    ts_J.x -= f_DotProduct*ts_I.x ;
    ts_J.y -= f_DotProduct*ts_I.y ;
    ts_J.z -= f_DotProduct*ts_I.z ;

    ts_J = s_algebra_vector_normalize(&ts_J);
   }

   /* orthogonalize k to i */
  f_DotProduct = f_algebra_vector_dotproduct(&ts_I, &ts_K);
  if( fabs(f_DotProduct) > 1.e-4 )
  {
   ts_K.x -= f_DotProduct*ts_I.x;
   ts_K.y -= f_DotProduct*ts_I.y;
   ts_K.z -= f_DotProduct*ts_I.z;

   ts_K = s_algebra_vector_normalize(&ts_K);
  }

  /* orthogonalize k to j */
  f_DotProduct = f_algebra_vector_dotproduct(&ts_J, &ts_K);
  if( fabs(f_DotProduct) > 1.e-4 )
  {
   ts_K.x -= f_DotProduct*ts_J.x;
   ts_K.y -= f_DotProduct*ts_J.y;
   ts_K.z -= f_DotProduct*ts_J.z;

   ts_K = s_algebra_vector_normalize(&ts_K);
  }

  Q.af_Matrix[0][0] = ts_I.x ; Q.af_Matrix[1][0] = ts_J.x ; Q.af_Matrix[2][0] = ts_K.x ;
  Q.af_Matrix[0][1] = ts_I.y ; Q.af_Matrix[1][1] = ts_J.y ; Q.af_Matrix[2][1] = ts_K.y ;
  Q.af_Matrix[0][2] = ts_I.z ; Q.af_Matrix[1][2] = ts_J.z ; Q.af_Matrix[2][2] = ts_K.z ;

  /* at this point, Q is the rotation matrix from the (i,j,k) to (x,y,z) axes */

  detQ = f_algebra_matrix_3x3_Determinant( &Q ) ;
  if( detQ == 0.0 ) return ; /* shouldn't happen unless user is a DUFIS */

  /* Build and test all possible +1/-1 coordinate permutation matrices P;
    then find the P such that the rotation matrix M=PQ is closest to the
    identity, in the sense of M having the smallest total rotation angle. */

  /* Despite the formidable looking 6 nested loops, there are
    only 3*3*3*2*2*2 = 216 passes, which will run very quickly. */

  vbest = -666.0;
  pbest=1;
  qbest=1;
  rbest=1;

  ibest=1;
  jbest=2;
  kbest=3;

  for( i=1 ; i <= 3 ; i++ ) /* i = column number to use for row #1 */
  {
    for( j=1 ; j <= 3 ; j++ ) /* j = column number to use for row #2 */
    {
      if( i == j )
      {
        continue;
      }

      for( k=1 ; k <= 3 ; k++ ) /* k = column number to use for row #3 */
      {
        if( i == k || j == k )
        {
          continue;
        }

        LOAD_MAT33(P, 0,0,0, 0,0,0, 0,0,0);

        for( p=-1 ; p <= 1 ; p+=2 )  /* p,q,r are -1 or +1      */
        {
          for( q=-1 ; q <= 1 ; q+=2 ) /* and go into rows #1,2,3 */
          {
            for( r=-1 ; r <= 1 ; r+=2 )
            {
              P.af_Matrix[i-1][0] = p;
              P.af_Matrix[j-1][1] = q;
              P.af_Matrix[k-1][2] = r;

              detP = f_algebra_matrix_3x3_Determinant(&P); /* sign of permutation */

              if( detP * detQ <= 0.0 )
              {
                continue ;  /* doesn't match sign of Q */
              }

              M = tda_algebra_matrix_3x3_multiply(&P,&Q) ;

              /* angle of M rotation = 2.0*acos(0.5*sqrt(1.0+trace(M)))       */
              /* we want largest trace(M) == smallest angle == M nearest to I */

              val = M.af_Matrix[0][0] + M.af_Matrix[1][1] + M.af_Matrix[2][2] ; /* trace */
              if( val > vbest )
              {
                vbest = val ;
                ibest = i;
                jbest = j;
                kbest = k;

                pbest = p;
                qbest = q;
                rbest = r;
              }
            }
          }
        }
      }
    }
  }

  /* At this point ibest is 1 or 2 or 3; pbest is -1 or +1; etc.

    The matrix P that corresponds is the best permutation approts_I.xmation
    to Q-inverse; that is, P (approts_I.xmately) takes (x,y,z) coordinates
    to the (i,j,k) axes.

    For example, the first row of P (which contains pbest in column ibest)
    determines the way the i ats_I.xs points relative to the anatomical
    (x,y,z) axes.  If ibest is 2, then the i ats_I.xs is along the y ats_I.xs,
    which is direction P2A (if pbest > 0) or A2P (if pbest < 0).

    So, using ibest and pbest, we can assign the output code for
    the i ats_I.xs.  Mutatis mutandis for the j and k axes, of course.
  */

  switch( ibest*pbest )
  {
   case  1: *pe_I = DIRECTION_L2R ; break ;
   case -1: *pe_I = DIRECTION_R2L ; break ;
   case  2: *pe_I = DIRECTION_P2A ; break ;
   case -2: *pe_I = DIRECTION_A2P ; break ;
   case  3: *pe_I = DIRECTION_I2S ; break ;
   case -3: *pe_I = DIRECTION_S2I ; break ;
  }

  switch( jbest*qbest )
  {
   case  1: *pe_J = DIRECTION_L2R ; break ;
   case -1: *pe_J = DIRECTION_R2L ; break ;
   case  2: *pe_J = DIRECTION_P2A ; break ;
   case -2: *pe_J = DIRECTION_A2P ; break ;
   case  3: *pe_J = DIRECTION_I2S ; break ;
   case -3: *pe_J = DIRECTION_S2I ; break ;
  }

  switch( kbest*rbest )
  {
   case  1: *pe_K = DIRECTION_L2R ; break ;
   case -1: *pe_K = DIRECTION_R2L ; break ;
   case  2: *pe_K = DIRECTION_P2A ; break ;
   case -2: *pe_K = DIRECTION_A2P ; break ;
   case  3: *pe_K = DIRECTION_I2S ; break ;
   case -3: *pe_K = DIRECTION_S2I ; break ;
  }
}

MemoryImageOrientation e_memory_serie_ConvertImageDirectionToOrientation(te_MemoryImageDirection e_ImageDirection_I, te_MemoryImageDirection e_ImageDirection_J, te_MemoryImageDirection e_ImageDirection_K)
{
  if ((e_ImageDirection_I == DIRECTION_ERROR) || ( e_ImageDirection_J == DIRECTION_ERROR) || (e_ImageDirection_K == DIRECTION_ERROR))
  {
    return ORIENTATION_UNKNOWN;
  }


  if ((e_ImageDirection_I == DIRECTION_L2R) || (e_ImageDirection_I == DIRECTION_R2L))
  {
    if ((e_ImageDirection_J == DIRECTION_P2A) || (e_ImageDirection_J == DIRECTION_A2P))
    {
      return ORIENTATION_AXIAL;
    }
    else if ((e_ImageDirection_J == DIRECTION_I2S) || (e_ImageDirection_J == DIRECTION_S2I))
    {
      return ORIENTATION_CORONAL;
    }
  }
  else if ((e_ImageDirection_I == DIRECTION_P2A) || (e_ImageDirection_I ==DIRECTION_A2P))
  {
    if ((e_ImageDirection_J == DIRECTION_I2S) || (e_ImageDirection_J == DIRECTION_S2I))
    {
      return ORIENTATION_SAGITAL;
    }
    else if ((e_ImageDirection_J == DIRECTION_L2R) || (e_ImageDirection_J == DIRECTION_R2L))
    {
      return ORIENTATION_AXIAL;
    }
  }
  else if ((e_ImageDirection_I == DIRECTION_S2I) || (e_ImageDirection_I == DIRECTION_I2S))
  {
    if ((e_ImageDirection_J == DIRECTION_L2R) || (e_ImageDirection_J == DIRECTION_R2L))
    {
      return ORIENTATION_CORONAL;
    }
    else if ((e_ImageDirection_J == DIRECTION_A2P) || (e_ImageDirection_J == DIRECTION_P2A))
    {
      return ORIENTATION_SAGITAL;
    }
  }

  return ORIENTATION_UNKNOWN;
}

char *pc_memory_serie_orientation_string( MemoryImageOrientation e_ImageOrientation)
{
  switch(e_ImageOrientation)
  {
   case ORIENTATION_AXIAL   : return "Axial" ;
   case ORIENTATION_SAGITAL : return "Sagital";
   case ORIENTATION_CORONAL : return "Coronal";
   case ORIENTATION_UNKNOWN : return "Unknown";
  }
  return "Unknown";
}

char *pc_memory_serie_direction_string( te_MemoryImageDirection e_ImageDirection, te_MemoryImageDirectionPart e_IDP)
{

  switch (e_IDP)
  {
    case DIRECTION_PART_ALL   :
      switch(e_ImageDirection)
      {
        case DIRECTION_L2R :  return "Left-to-Right";
        case DIRECTION_R2L :  return "Right-to-Left";
        case DIRECTION_P2A :  return "Posterior-to-Anterior";
        case DIRECTION_A2P :  return "Anterior-to-Posterior";
        case DIRECTION_I2S :  return "Inferior-to-Superior";
        case DIRECTION_S2I :  return "Superior-to-Inferior";
        case DIRECTION_ERROR:  return "Unknown-to-Unknown";
      }

    case DIRECTION_PART_FIRST :
      switch(e_ImageDirection)
      {
        case DIRECTION_L2R :  return "L";
        case DIRECTION_R2L :  return "R";
        case DIRECTION_P2A :  return "P";
        case DIRECTION_A2P :  return "A";
        case DIRECTION_I2S :  return "I";
        case DIRECTION_S2I :  return "S";
        case DIRECTION_ERROR:  return "Unknown-to-Unknown";
      }

    case DIRECTION_PART_LAST  :
      switch(e_ImageDirection)
      {
        case DIRECTION_L2R :  return "R";
        case DIRECTION_R2L :  return "L";
        case DIRECTION_P2A :  return "A";
        case DIRECTION_A2P :  return "P";
        case DIRECTION_I2S :  return "S";
        case DIRECTION_S2I :  return "I";
        case DIRECTION_ERROR:  return "Unknown-to-Unknown";
      }
  }
  return "Unknown";
}

