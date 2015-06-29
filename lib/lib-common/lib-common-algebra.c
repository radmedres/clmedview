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

#include "lib-common-algebra.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <math.h>

/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/* Functions are ordered in Vector-, Matrix, Quaternion related functionality                         */
/*                                                                                                    */
/*                                                                                                    */
Vector3D s_algebra_vector_normalize(Vector3D *ps_InputVector)
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

Vector3D s_algebra_vector_perpendicular(Vector3D *ps_InputVector, Vector3D *ps_UpVector)
{
  debug_functions ();

  Vector3D ts_perpendicularVector;

  ts_perpendicularVector.x = 0;
  ts_perpendicularVector.y = 0;
  ts_perpendicularVector.z = 0;

  float f_UpProjection;
  float f_UpMagnitude;

  f_UpProjection = ps_InputVector->x * ps_UpVector->x + ps_InputVector->y * ps_UpVector->y + ps_InputVector->z * ps_UpVector->z;

  // first try at making a View Up vector: use World Up
  ts_perpendicularVector.x = ps_UpVector->x - f_UpProjection * ps_InputVector->x;
  ts_perpendicularVector.y = ps_UpVector->y - f_UpProjection * ps_InputVector->y;
  ts_perpendicularVector.z = ps_UpVector->z - f_UpProjection * ps_InputVector->z;

  // Check for validity:
  f_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;

  if (f_UpMagnitude < 0.0000001)
  {
    //Second try at making a View Up vector: Use Y axis default  (0,1,0)
    ts_perpendicularVector.x = -ps_InputVector->y * ps_InputVector->x;
    ts_perpendicularVector.y = 1-ps_InputVector->y * ps_InputVector->y;
    ts_perpendicularVector.z = -ps_InputVector->y * ps_InputVector->z;

    // Check for validity:
    f_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;

    if (f_UpMagnitude < 0.0000001)
    {
          //Final try at making a View Up vector: Use Z axis default  (0,0,1)
      ts_perpendicularVector.x = -ps_InputVector->z * ps_InputVector->x;
      ts_perpendicularVector.y = -ps_InputVector->z * ps_InputVector->y;
      ts_perpendicularVector.z = 1-ps_InputVector->z * ps_InputVector->z;

      // Check for validity:
      f_UpMagnitude = ts_perpendicularVector.x * ts_perpendicularVector.x + ts_perpendicularVector.y * ts_perpendicularVector.y + ts_perpendicularVector.z * ts_perpendicularVector.z;
      if (f_UpMagnitude < 0.0000001)
      {
        assert(f_UpMagnitude < 0.0000001);
      }

    }
  }

  // normalize the Up Vector
  ts_perpendicularVector = s_algebra_vector_normalize(&ts_perpendicularVector);

  return ts_perpendicularVector;
}

Vector3D s_algebra_vector_crossproduct(Vector3D *ps_InputVector,Vector3D *pts_perpendicularVector)
{
  debug_functions ();

  Vector3D s_OutputVector;

  s_OutputVector.x = ps_InputVector->y * pts_perpendicularVector->z - ps_InputVector->z * pts_perpendicularVector->y;
  s_OutputVector.y = ps_InputVector->z * pts_perpendicularVector->x - ps_InputVector->x * pts_perpendicularVector->z;
  s_OutputVector.z = ps_InputVector->x * pts_perpendicularVector->y - ps_InputVector->y * pts_perpendicularVector->x;

  s_OutputVector = s_algebra_vector_normalize(&s_OutputVector);

  return s_OutputVector;
}

Vector3D ts_algebra_vector_translate(ts_Matrix4x4 *ps_Matrix, Vector3D *ps_Vector)
{
  Vector3D ts_MultiplyVector;

  ts_MultiplyVector.x = ps_Vector->x  * ps_Matrix->af_Matrix[0][0] +
                        ps_Vector->y  * ps_Matrix->af_Matrix[1][0] +
                        ps_Vector->z  * ps_Matrix->af_Matrix[2][0] +
                        ps_Matrix->af_Matrix[3][0];

  ts_MultiplyVector.y = ps_Vector->x  * ps_Matrix->af_Matrix[0][1] +
                        ps_Vector->y  * ps_Matrix->af_Matrix[1][1] +
                        ps_Vector->z  * ps_Matrix->af_Matrix[2][1] +
                        ps_Matrix->af_Matrix[3][1];

  ts_MultiplyVector.z = ps_Vector->x  * ps_Matrix->af_Matrix[0][2] +
                        ps_Vector->y  * ps_Matrix->af_Matrix[1][2] +
                        ps_Vector->z  * ps_Matrix->af_Matrix[2][2] +
                        ps_Matrix->af_Matrix[3][2];

  return ts_MultiplyVector;
}

Vector3D ts_algebra_vector_Rotation_around_X_Axis (Vector3D *ps_Vector, float f_angle)
{
  /* Rotation vector around x-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          | 1  0    0  |   | x |
      R = | 0 cos -sin | * | y |
          | 0 sin  cos |   | z |
  */

  Vector3D ts_rotatedVector;

  ts_rotatedVector.x = 1 * ps_Vector->x -      0        * ps_Vector->y +     0         * ps_Vector->z;
  ts_rotatedVector.y = 0 * ps_Vector->x + cos (f_angle) * ps_Vector->y - sin (f_angle) * ps_Vector->z;
  ts_rotatedVector.z = 0 * ps_Vector->x + sin (f_angle) * ps_Vector->y + cos (f_angle) * ps_Vector->z;

  return ts_rotatedVector;
}

Vector3D ts_algebra_vector_Rotation_around_Y_Axis (Vector3D *ps_Vector, float f_angle)
{
  /* Rotation vector around y-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          |  cos  0  sin |   | x |
      R = |   0   1   0  | * | y |
          | -sin  0  cos |   | z |
  */
  Vector3D ts_rotatedVector;

  ts_rotatedVector.x =  cos (f_angle) * ps_Vector->x + 0 * ps_Vector->y + sin (f_angle) * ps_Vector->z;
  ts_rotatedVector.y =      0         * ps_Vector->x + 1 * ps_Vector->y + 0             * ps_Vector->z;
  ts_rotatedVector.z = -sin (f_angle) * ps_Vector->x + 0 * ps_Vector->y + cos (f_angle) * ps_Vector->z;

  return ts_rotatedVector;
}

Vector3D ts_algebra_vector_Rotation_around_Z_Axis (Vector3D *ps_Vector, float f_angle)
{
  /* Rotation vector around z-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          | cos -sin 0  |   | x |
      R = | sin  cos 0  | * | y |
          |  0    0  1  |   | z |
  */
  Vector3D ts_rotatedVector;

  ts_rotatedVector.x = cos (f_angle) * ps_Vector->x - sin (f_angle) * ps_Vector->y + 0 * ps_Vector->z;
  ts_rotatedVector.y = sin (f_angle) * ps_Vector->x + cos (f_angle) * ps_Vector->y + 0 * ps_Vector->z;
  ts_rotatedVector.z =     0                        -     0                        + 1 * ps_Vector->z;

  return ts_rotatedVector;
}

float f_algebra_vector_MaximumValue(Vector3D *ps_InputVector)
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

float f_algebra_vector_MinimumValue(Vector3D *ps_InputVector)
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

short int i16_algebra_vector_MaximumValue(ts_Vector3DInt *ps_InputVector)
{
  debug_functions ();

  if ((ps_InputVector->i16_x >= ps_InputVector->i16_y) && (ps_InputVector->i16_x >= ps_InputVector->i16_z))
  {
    return ps_InputVector->i16_x;
  }
  else if ((ps_InputVector->i16_y >= ps_InputVector->i16_x) && (ps_InputVector->i16_y >= ps_InputVector->i16_z))
  {
    return ps_InputVector->i16_y;
  }
  else
  {
    return ps_InputVector->i16_z;
  }
}

short int  i16_algebra_vector_MinimumValue(ts_Vector3DInt *ps_InputVector)
{
  debug_functions ();

  if ((ps_InputVector->i16_x < ps_InputVector->i16_y) && (ps_InputVector->i16_x < ps_InputVector->i16_z))
  {
    return ps_InputVector->i16_x;
  }
  else if ((ps_InputVector->i16_y < ps_InputVector->i16_x) && (ps_InputVector->i16_y < ps_InputVector->i16_z))
  {
    return ps_InputVector->i16_y;
  }
  else
  {
    return ps_InputVector->i16_z;
  }
}




ts_Matrix4x4 tda_algebra_matrix_4x4_QuaternionToMatrix(ts_Quaternion *ps_Source, ts_Quaternion *ps_SourceOffset, double d_Qfac)
{
  ts_Matrix4x4 td_Rotation;
  double d_Rotation;

  d_Rotation = 1.0 - (ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K);

  if (d_Rotation == 0 )
  {
    /* Special case according to niftii standard !?!*/
    /* There should be a 180 degree rotation        */
    d_Rotation = 1 / sqrt((ps_Source->I * ps_Source->I +  ps_Source->J * ps_Source->J +  ps_Source->K * ps_Source->K));
    ps_Source->I *= d_Rotation;
    ps_Source->J *= d_Rotation;
    ps_Source->K *= d_Rotation;
    ps_Source->W = 0;
  }
  else
  {
    ps_Source->W = sqrt(d_Rotation);
  }

  td_Rotation.af_Matrix[0][0] =     (ps_Source->W * ps_Source->W + ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.af_Matrix[0][1] = 2 * (ps_Source->I * ps_Source->J + ps_Source->W * ps_Source->K);
  td_Rotation.af_Matrix[0][2] = 2 * (ps_Source->I * ps_Source->K - ps_Source->W * ps_Source->J) * d_Qfac;

  td_Rotation.af_Matrix[1][0] = 2 * (ps_Source->I * ps_Source->J - ps_Source->W * ps_Source->K);
  td_Rotation.af_Matrix[1][1] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.af_Matrix[1][2] = 2 * (ps_Source->J * ps_Source->K + ps_Source->W * ps_Source->I)* d_Qfac;

  td_Rotation.af_Matrix[2][0] = 2 * (ps_Source->I * ps_Source->K + ps_Source->W * ps_Source->J);
  td_Rotation.af_Matrix[2][1] = 2 * (ps_Source->J * ps_Source->K - ps_Source->W * ps_Source->I);
  td_Rotation.af_Matrix[2][2] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K) * d_Qfac;

  td_Rotation.af_Matrix[3][0] = ps_SourceOffset->I;
  td_Rotation.af_Matrix[3][1] = ps_SourceOffset->J;
  td_Rotation.af_Matrix[3][2] = ps_SourceOffset->K;

  td_Rotation.af_Matrix[0][3] = 0;
  td_Rotation.af_Matrix[1][3] = 0;
  td_Rotation.af_Matrix[2][3] = 0;
  td_Rotation.af_Matrix[3][3] = 1;


  return td_Rotation;
}

ts_Matrix4x4 tda_algebra_matrix_4x4_inverse(ts_Matrix4x4 *ps_Matrix)
{
  double d_Determinant;
  double r11,r12,r13,r21,r22,r23,r31,r32,r33,v1,v2,v3;
  ts_Matrix4x4 td_Inverse;

  /* [ r11 r12 r13 v1 ] */
  /* [ r21 r22 r23 v2 ] */
  /* [ r31 r32 r33 v3 ] */
  /* [  0   0   0   1 ] */

  r11 = ps_Matrix->af_Matrix[0][0];
  r12 = ps_Matrix->af_Matrix[1][0];
  r13 = ps_Matrix->af_Matrix[2][0];
  v1  = ps_Matrix->af_Matrix[3][0];

  r21 = ps_Matrix->af_Matrix[0][1];
  r22 = ps_Matrix->af_Matrix[1][1];
  r23 = ps_Matrix->af_Matrix[2][1];
  v2  = ps_Matrix->af_Matrix[3][1];

  r31 = ps_Matrix->af_Matrix[0][2];
  r32 = ps_Matrix->af_Matrix[1][2];
  r33 = ps_Matrix->af_Matrix[2][2];
  v3  = ps_Matrix->af_Matrix[3][2];

  d_Determinant = r11*r22*r33 - r11*r32*r23 - r21*r12*r33 + r21*r32*r13 + r31*r12*r23 - r31*r22*r13 ;
  d_Determinant = (d_Determinant == 0 ) ? d_Determinant : 1 / d_Determinant;


  td_Inverse.af_Matrix[0][0] = d_Determinant * ( r22*r33-r32*r23);
  td_Inverse.af_Matrix[1][0] = d_Determinant * (-r12*r33+r32*r13);
  td_Inverse.af_Matrix[2][0] = d_Determinant * ( r12*r23-r22*r13);
  td_Inverse.af_Matrix[3][0] = d_Determinant * (-r12*r23*v3 + r12*v2*r33 + r22*r13*v3 - r22*v1*r33 - r32*r13*v2 + r32*v1*r23);

  td_Inverse.af_Matrix[0][1] = d_Determinant * (-r21*r33+r31*r23);
  td_Inverse.af_Matrix[1][1] = d_Determinant * ( r11*r33-r31*r13);
  td_Inverse.af_Matrix[2][1] = d_Determinant * (-r11*r23+r21*r13);
  td_Inverse.af_Matrix[3][1] = d_Determinant * ( r11*r23*v3 - r11*v2*r33 - r21*r13*v3 + r21*v1*r33 + r31*r13*v2 - r31*v1*r23);

  td_Inverse.af_Matrix[0][2] = d_Determinant * ( r21*r32-r31*r22);
  td_Inverse.af_Matrix[1][2] = d_Determinant * (-r11*r32+r31*r12);
  td_Inverse.af_Matrix[2][2] = d_Determinant * ( r11*r22-r21*r12);
  td_Inverse.af_Matrix[3][2] = d_Determinant * (-r11*r22*v3 + r11*r32*v2 + r21*r12*v3 - r21*r32*v1 - r31*r12*v2 + r31*r22*v1);

  td_Inverse.af_Matrix[0][3] = 0;
  td_Inverse.af_Matrix[1][3] = 0;
  td_Inverse.af_Matrix[2][3] = 0;
  td_Inverse.af_Matrix[3][3] = (d_Determinant == 0) ? 0 : 1 ;

  return td_Inverse;
}

ts_Matrix4x4 tda_algebra_matrix_4x4_multiply(ts_Matrix4x4 *ps_MatrixA , ts_Matrix4x4 *ps_MatrixB)
{
    ts_Matrix4x4 t_Result;
    short int i16_ColumnCnt;
    short int i16_RowCnt;
    short int i16_SumCnt;

    for( i16_ColumnCnt=0; i16_ColumnCnt < 4; i16_ColumnCnt++)
    {
      for( i16_RowCnt=0; i16_RowCnt < 4; i16_RowCnt++)
      {
        t_Result.af_Matrix[i16_ColumnCnt][i16_RowCnt] = 0;
        for (i16_SumCnt=0; i16_SumCnt < 4; i16_SumCnt++)
        {
          t_Result.af_Matrix[i16_ColumnCnt][i16_RowCnt] += ps_MatrixA->af_Matrix[i16_SumCnt][i16_RowCnt] * ps_MatrixB->af_Matrix[i16_ColumnCnt][i16_SumCnt];
        }
      }
    }
    return t_Result;
}


ts_Quaternion ts_algebra_quaternion_MatrixToQuaternion(ts_Matrix4x4 *pt_Matrix, double *pd_Qfac)
{
  ts_Quaternion ts_Quat;
  double r11,r12,r13 , r21,r22,r23 , r31,r32,r33 ;

  double xd,yd,zd , a,b,c,d ;

  ts_Matrix3x3 P,Q ;

  /* [ r11 r12 r13 v1 ] */
  /* [ r21 r22 r23 v2 ] */
  /* [ r31 r32 r33 v3 ] */
  /* [  0   0   0   1 ] */

  /* load 3x3 matrix into local variables */

  r11 = pt_Matrix->af_Matrix[0][0] ; r12 = pt_Matrix->af_Matrix[1][0] ; r13 = pt_Matrix->af_Matrix[2][0] ;
  r21 = pt_Matrix->af_Matrix[0][1] ; r22 = pt_Matrix->af_Matrix[1][1] ; r23 = pt_Matrix->af_Matrix[2][1] ;
  r31 = pt_Matrix->af_Matrix[0][2] ; r32 = pt_Matrix->af_Matrix[1][2] ; r33 = pt_Matrix->af_Matrix[2][2] ;

  /* compute lengths of each column; these determine grid spacings  */

  xd = sqrt( r11*r11 + r21*r21 + r31*r31 ) ;
  yd = sqrt( r12*r12 + r22*r22 + r32*r32 ) ;
  zd = sqrt( r13*r13 + r23*r23 + r33*r33 ) ;

  /* if a column length is zero, patch the trouble */

  if( xd == 0.0l ){ r11 = 1.0l ; r21 = r31 = 0.0l ; xd = 1.0l ; }
  if( yd == 0.0l ){ r22 = 1.0l ; r12 = r32 = 0.0l ; yd = 1.0l ; }
  if( zd == 0.0l ){ r33 = 1.0l ; r13 = r23 = 0.0l ; zd = 1.0l ; }

   /* normalize the columns */

   r11 /= xd ; r21 /= xd ; r31 /= xd ;
   r12 /= yd ; r22 /= yd ; r32 /= yd ;
   r13 /= zd ; r23 /= zd ; r33 /= zd ;

   /* At this point, the matrix has normal columns, but we have to allow
      for the fact that the hideous user may not have given us a matrix
      with orthogonal columns.

      So, now find the orthogonal matrix closest to the current matrix.

      One reason for using the polar decomposition to get this
      orthogonal matrix, rather than just directly orthogonalizing
      the columns, is so that inputting the inverse matrix to R
      will result in the inverse orthogonal matrix at this point.
      If we just orthogonalized the columns, this wouldn't necessarily hold. */

//   Q.af_Matrix[0][0] = r11 ; Q.af_Matrix[1][0] = r12 ; Q.af_Matrix[2][0] = r13 ; /* load Q */
//   Q.af_Matrix[0][1] = r21 ; Q.af_Matrix[1][1] = r22 ; Q.af_Matrix[2][1] = r23 ;
//   Q.af_Matrix[0][2] = r31 ; Q.af_Matrix[1][2] = r32 ; Q.af_Matrix[2][2] = r33 ;

//   P = nifti_mat33_polar(Q) ;  /* P is orthog matrix closest to Q */

//   r11 = P.af_Matrix[0][0] ; r12 = P.af_Matrix[1][0] ; r13 = P.af_Matrix[2][0] ; /* unload */
//   r21 = P.af_Matrix[0][1] ; r22 = P.af_Matrix[1][1] ; r23 = P.af_Matrix[2][1] ;
//   r31 = P.af_Matrix[0][2] ; r32 = P.af_Matrix[1][2] ; r33 = P.af_Matrix[2][2] ;

   /*                            [ r11 r12 r13 ]               */
   /* at this point, the matrix  [ r21 r22 r23 ] is orthogonal */
   /*                            [ r31 r32 r33 ]               */

   /* compute the determinant to determine if it is proper */

   zd = r11*r22*r33-r11*r32*r23-r21*r12*r33
       +r21*r32*r13+r31*r12*r23-r31*r22*r13 ;  /* should be -1 or 1 */

   if( zd <= 0 )
    {                  /* improper ==> flip 3rd column */
      *pd_Qfac = -1;
  //     ASSIF(qfac,-1.0) ;
     r13 = -r13 ; r23 = -r23 ; r33 = -r33 ;
   }
   else
   {
     *pd_Qfac = 1;
   }


   /* now, compute quaternion parameters */

  a = r11 + r22 + r33 + 1.0l ;

  if( a > 0.5l )
  {                /* simplest case */
    a = 0.5l * sqrt(a) ;
    b = 0.25l * (r32-r23) / a ;
    c = 0.25l * (r13-r31) / a ;
    d = 0.25l * (r21-r12) / a ;
  }
  else
  {                       /* trickier case */
    xd = 1.0 + r11 - (r22+r33) ;  /* 4*b*b */
    yd = 1.0 + r22 - (r11+r33) ;  /* 4*c*c */
    zd = 1.0 + r33 - (r11+r22) ;  /* 4*d*d */

    if( xd > 1.0 )
    {
      b = 0.5l * sqrt(xd) ;
      c = 0.25l* (r12+r21) / b ;
      d = 0.25l* (r13+r31) / b ;
      a = 0.25l* (r32-r23) / b ;
    }
    else if( yd > 1.0 )
    {
      c = 0.5l * sqrt(yd) ;
      b = 0.25l* (r12+r21) / c ;
      d = 0.25l* (r23+r32) / c ;
      a = 0.25l* (r13-r31) / c ;
    }
    else
    {
      d = 0.5l * sqrt(zd) ;
      b = 0.25l* (r13+r31) / d ;
      c = 0.25l* (r23+r32) / d ;
      a = 0.25l* (r21-r12) / d ;
    }
    if( a < 0.0l )
    {
      b=-b;
      c=-c;
      d=-d;
      a=-a;
    }
  }

  //   ASSIF(qb,b) ; ASSIF(qc,c) ; ASSIF(qd,d) ;
  ts_Quat.W = a;
  ts_Quat.I = b;
  ts_Quat.J = c;
  ts_Quat.K = d;

  return ts_Quat;
}

