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

#include "lib-memory-quaternion.h"
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
/*                                                                                                    */
/*                                                                                                    */
td_Matrix4x4 tda_memory_quaternion_to_matrix(ts_Quaternion *ps_Source, ts_Quaternion *ps_SourceOffset, Coordinate3D *ps_pixel_dimension, double d_Qfac)
{
  td_Matrix4x4 td_Rotation;

  td_Rotation.d_Matrix[3][0] = 0.0;
  td_Rotation.d_Matrix[3][1] = 0.0;
  td_Rotation.d_Matrix[3][2] = 0.0;
  td_Rotation.d_Matrix[3][3] = 1.0;


  td_Rotation.d_Matrix[0][0] =     (ps_Source->W * ps_Source->W + ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K) * ps_pixel_dimension->x;
  td_Rotation.d_Matrix[0][1] = 2 * (ps_Source->I * ps_Source->J - ps_Source->W * ps_Source->K) * ps_pixel_dimension->y;
  td_Rotation.d_Matrix[0][2] = 2 * (ps_Source->I * ps_Source->K - ps_Source->W * ps_Source->J) * d_Qfac * ps_pixel_dimension->z;
  td_Rotation.d_Matrix[0][3] = ps_SourceOffset->I;

  td_Rotation.d_Matrix[1][0] = 2 * (ps_Source->I * ps_Source->J + ps_Source->W * ps_Source->K) * ps_pixel_dimension->x;
  td_Rotation.d_Matrix[1][1] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K) * ps_pixel_dimension->y;
  td_Rotation.d_Matrix[1][2] = 2 * (ps_Source->J * ps_Source->K - ps_Source->W * ps_Source->I) * ps_pixel_dimension->z * d_Qfac;
  td_Rotation.d_Matrix[1][3] = ps_SourceOffset->J;

  td_Rotation.d_Matrix[2][0] = 2 * (ps_Source->I * ps_Source->K - ps_Source->W * ps_Source->J) * ps_pixel_dimension->x;
  td_Rotation.d_Matrix[2][1] = 2 * (ps_Source->J * ps_Source->K + ps_Source->W * ps_Source->I) * ps_pixel_dimension->y;
  td_Rotation.d_Matrix[2][2] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K) * ps_pixel_dimension->z * d_Qfac;
  td_Rotation.d_Matrix[2][3] = ps_SourceOffset->K;

  return td_Rotation;
}

td_Matrix4x4 tda_memory_quaternion_inverse_matrix(td_Matrix4x4 *ps_Matrix)
{
  double d_Determinant;
  double r11,r12,r13,r21,r22,r23,r31,r32,r33,v1,v2,v3;
  td_Matrix4x4 td_Inverse;

  /* [ r11 r12 r13 v1 ] */
  /* [ r21 r22 r23 v2 ] */
  /* [ r31 r32 r33 v3 ] */
  /* [  0   0   0   1 ] */

  r11 = ps_Matrix->d_Matrix[0][0];
  r12 = ps_Matrix->d_Matrix[0][1];
  r13 = ps_Matrix->d_Matrix[0][2];
  v1  = ps_Matrix->d_Matrix[0][3];

  r21 = ps_Matrix->d_Matrix[1][0];
  r22 = ps_Matrix->d_Matrix[1][1];
  r23 = ps_Matrix->d_Matrix[1][2];
  v2  = ps_Matrix->d_Matrix[1][3];

  r31 = ps_Matrix->d_Matrix[2][0];
  r32 = ps_Matrix->d_Matrix[2][1];
  r33 = ps_Matrix->d_Matrix[2][2];
  v3  = ps_Matrix->d_Matrix[2][3];

  d_Determinant = r11*r22*r33 - r11*r32*r23 - r21*r12*r33 + r21*r32*r13 + r31*r12*r23 - r31*r22*r13 ;
  d_Determinant = (d_Determinant == 0 ) ? d_Determinant : 1 / d_Determinant;

  td_Inverse.d_Matrix[0][0] = d_Determinant ;

  td_Inverse.d_Matrix[0][0] = d_Determinant * ( r22*r33-r32*r23);
  td_Inverse.d_Matrix[0][1] = d_Determinant * (-r12*r33+r32*r13);
  td_Inverse.d_Matrix[0][2] = d_Determinant * ( r12*r23-r22*r13);
  td_Inverse.d_Matrix[0][3] = d_Determinant * (-r12*r23*v3 + r12*v2*r33 + r22*r13*v3 - r22*v1*r33 - r32*r13*v2 + r32*v1*r23);

  td_Inverse.d_Matrix[1][0] = d_Determinant * (-r21*r33+r31*r23);
  td_Inverse.d_Matrix[1][1] = d_Determinant * ( r11*r33-r31*r13);
  td_Inverse.d_Matrix[1][2] = d_Determinant * (-r11*r23+r21*r13);
  td_Inverse.d_Matrix[1][3] = d_Determinant * ( r11*r23*v3 - r11*v2*r33 - r21*r13*v3 + r21*v1*r33 + r31*r13*v2 - r31*v1*r23);

  td_Inverse.d_Matrix[2][0] = d_Determinant * ( r21*r32-r31*r22);
  td_Inverse.d_Matrix[2][1] = d_Determinant * (-r11*r32+r31*r12);
  td_Inverse.d_Matrix[2][2] = d_Determinant * ( r11*r22-r21*r12);
  td_Inverse.d_Matrix[2][3] = d_Determinant * (-r11*r22*v3 + r11*r32*v2 + r21*r12*v3 - r21*r32*v1 - r31*r12*v2 + r31*r22*v1);

  td_Inverse.d_Matrix[3][0] = 0;
  td_Inverse.d_Matrix[3][1] = 0;
  td_Inverse.d_Matrix[3][2] = 0;
  td_Inverse.d_Matrix[3][3] = (d_Determinant == 0) ? 0 : 1 ;

  return td_Inverse;
}

Vector3D ts_memory_matrix_multiply4x4(td_Matrix4x4 *ps_Matrix, Vector3D *ps_Vector)
{
  Vector3D ts_MultiplyVector;

  ts_MultiplyVector.x = ps_Vector->x  * ps_Matrix->d_Matrix[0][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[0][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[0][2] +
                        ps_Matrix->d_Matrix[0][3];

  ts_MultiplyVector.y = ps_Vector->x  * ps_Matrix->d_Matrix[1][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[1][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[1][2] +
                        ps_Matrix->d_Matrix[1][3];

  ts_MultiplyVector.z = ps_Vector->x  * ps_Matrix->d_Matrix[2][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[2][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[2][2] +
                        ps_Matrix->d_Matrix[2][3];

  return ts_MultiplyVector;
}





