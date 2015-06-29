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

#ifndef COMMON_ALGEBRA_H
#define COMMON_ALGEBRA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include "lib-common.h"


/**
 * @file   include/lib-common-algebra.h
 * @brief  The interface to algebra-specific functionality.
 * @author Jos Slenter
 */


/**
 * @ingroup memory
 * @{
 *
 *   @defgroup memory_quaternion Quaternion
 *   @{
 *
 * This module provides Quaternion-specific functionality.
 */


/**
 * Macro to load a 4x4 matrix
 */
#define LOAD_MAT44(AA,a11,a12,a13,a14 ,a21,a22,a23,a24 ,a31,a32,a33,a34, a41,a42,a43,a44)    \
( AA.af_Matrix[0][0]=a11, AA.af_Matrix[1][0]=a12, AA.af_Matrix[2][0]=a13, AA.af_Matrix[3][0]=a14 ,   \
  AA.af_Matrix[0][1]=a21, AA.af_Matrix[1][1]=a22, AA.af_Matrix[2][1]=a23, AA.af_Matrix[3][1]=a24  ,   \
  AA.af_Matrix[0][2]=a31, AA.af_Matrix[1][2]=a32, AA.af_Matrix[2][2]=a33, AA.af_Matrix[3][2]=a34 , \
  AA.af_Matrix[0][3]=a41, AA.af_Matrix[1][3]=a42, AA.af_Matrix[2][3]=a43, AA.af_Matrix[3][3]=a44 )

/**
 * Quaternion definition.
 */
typedef struct
{
  double W;
  double I;
  double J;
  double K;
} ts_Quaternion;

/**
 * Matrix definition (4x4).
 */
typedef struct
{
  float af_Matrix[4][4];
} ts_Matrix4x4;

/**
 * Matrix definition (3x3).
 */
typedef struct
{
  double af_Matrix[3][3];
} ts_Matrix3x3;




/**
 * Function that normalize a vector
 * @param[in]  ps_Vector  Input vector.
 * @param[out] vector     Normalized vector.
 */
Vector3D s_algebra_vector_normalize(Vector3D *ps_InputVector);

/**
 * Function that normalize a vector
 * @param[in]  ps_Vector    Input vector.
 * @param[in]  ps_UpVector  perpendicular vector.
 * @param[out] vector       Normalized vector.
 */
Vector3D s_algebra_vector_perpendicular(Vector3D *ps_InputVector, Vector3D *ps_UpVector);

/**
 * Function that normalize a vector
 * @param[in]  ps_Vector  Input vector.
 * @param[in]  ps_Vector  perpendicular vector.
 * @param[out] vector     Normalized vector.
 */
Vector3D s_algebra_vector_crossproduct(Vector3D *ps_InputVector,Vector3D *pts_perpendicularVector);

/**
 * Function that transforms a vector according to a rotation matrix.
 * @param[in]  ps_Matrix  Translation matrix.
 * @param[in]  ps_Vector  Vector to transform.
 * @param[out] vector     Transformed vector.
 */
Vector3D ts_algebra_vector_translate(ts_Matrix4x4 *ps_Matrix, Vector3D *ps_Vector);

/**
 * Function that calculates the maximum of a vector.
 * @param[in]  ps_InputVector  Vector to calculate max of.
 * @param[out] float           Maximum value.
 */
float f_algebra_vector_MaximumValue(Vector3D *ps_InputVector);

/**
 * Function that calculates the minimum of a vector.
 * @param[in]  ps_InputVector  Vector to calculate max of.
 * @param[out] float           Minimum value.
 */
float f_algebra_vector_MinimumValue(Vector3D *ps_InputVector);

/**
 * Function that calculates the maximum of a vector.
 * @param[in]  ps_InputVector  Vector to calculate max of.
 * @param[out] short int       Maximum value.
 */;
short int i16_algebra_vector_MaximumValue(ts_Vector3DInt *ps_InputVector);

/**
 * Function that calculates the minimum of a vector.
 * @param[in]  ps_InputVector  Vector to calculate max of.
 * @param[out] short int       Minimum value.
 */
short int  i16_algebra_vector_MinimumValue(ts_Vector3DInt *ps_InputVector);

/**
 * Function that rotates a vector around the X a axis with a certain angle
 * @param[in]  ps_Vector       Vector to calculate max of.
 * @param[in]  f_angle         Angle of rotation.
 * @param[out] Vectro          Rotated Vector.
 */
Vector3D ts_algebra_vector_Rotation_around_X_Axis (Vector3D *ps_Vector, float f_angle);

/**
 * Function that rotates a vector around the Y a axis with a certain angle
 * @param[in]  ps_Vector       Vector to calculate max of.
 * @param[in]  f_angle         Angle of rotation.
 * @param[out] Vectro          Rotated Vector.
 */
Vector3D ts_algebra_vector_Rotation_around_Y_Axis (Vector3D *ps_Vector, float f_angle);

/**
 * Function that rotates a vector around the Z a axis with a certain angle
 * @param[in]  ps_Vector       Vector to calculate max of.
 * @param[in]  f_angle         Angle of rotation.
 * @param[out] Vectro          Rotated Vector.
 */
Vector3D ts_algebra_vector_Rotation_around_Z_Axis (Vector3D *ps_Vector, float f_angle);



/**
 * Function that convert a quaternion to a translation matrix.
 * @param[in]  ps_Source            Quaternion that should be converted.
 * @param[in]  ps_SourceOffset      Offset of quaternion in [i,j,k,w].
 * @param[in]  d_Qfac               Factor which defines stride Z-axis.
 * @param[out] matrix               A rotation matrix according to quaternion
 */
ts_Matrix4x4 tda_algebra_matrix_4x4_QuaternionToMatrix(ts_Quaternion *ps_Source, ts_Quaternion *ps_SourceOffset, double d_Qfac);

/**
 * Function that calculates a inverse of a 4x4 matrix.
 * @param[in]  ps_Matrix  Input matrix.
 * @param[out] matrix     Output matrix.
 */
ts_Matrix4x4 tda_algebra_matrix_4x4_inverse(ts_Matrix4x4 *ps_Matrix);

/**
 * Function that multiplys a 4x4 matrix.
 * @param[in]  ps_MatrixA  Input matrix.
 * @param[in]  ps_MatrixB  Input matrix.
 * @param[out] matrix      Output matrix.
 */
ts_Matrix4x4 tda_algebra_matrix_4x4_multiply(ts_Matrix4x4 *ps_MatrixA , ts_Matrix4x4 *ps_MatrixB);

/**
 * Function that convert a matrix to a quaternion
 * @param[in]  pt_Matrix            Quaternion that should be converted.
 * @param[in]  d_Qfac               Factor which defines stride Z-axis.
 * @param[out] quaternion        A rotation matrix according to quaternion
 */
ts_Quaternion ts_algebra_quaternion_MatrixToQuaternion(ts_Matrix4x4 *pt_Matrix, double *pd_Qfac);

#ifdef __cplusplus
}
#endif

#endif//COMMON_ALGEBRA_H
