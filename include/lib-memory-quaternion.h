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

#ifndef MEMORY_QUATERNION_H
#define MEMORY_QUATERNION_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lib-common.h"


/**
 * @file   include/lib-memory-quaternion.h
 * @brief  The interface to Quaternion-specific functionality.
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
  double d_Matrix[4][4];
} td_Matrix4x4;

/**
 * Function that convert a quaternion with its offset to a rotation matrix.
 * @param[in]  ps_Source            Quaternion that should be converted.
 * @param[in]  ps_SourceOffset      Offset of quaternion in [i,j,k,w].
 * @param[in]  ps_pixel_dimension   Physical dimensions of [i,j,k].
 * @param[in]  d_Qfac               Factor which defines stride Z-axis.
 * @param[out] matrix               A rotation matrix according to quaternion
 */
td_Matrix4x4 tda_memory_quaternion_to_matrix(ts_Quaternion *ps_Source, ts_Quaternion *ps_SourceOffset, Coordinate3D *ps_pixel_dimension, double d_Qfac);

/**
 * Function that calculates a inverse of a 4x4 matrix.
 * @param[in]  ps_Matrix  Input matrix.
 * @param[out] matrix     Output matrix.
 */
 td_Matrix4x4 tda_memory_quaternion_inverse_matrix(td_Matrix4x4 *ps_Matrix);


/**
 * Function that transforms a vector according to a rotation Matrix.
 * @param[in]  ps_Matrix  Input matrix.
 * @param[in]  ps_Vector  Vector to transform.
 * @param[out] vector     Transformed vector.
 */
Vector3D ts_memory_matrix_multiply4x4(td_Matrix4x4 *ps_Matrix, Vector3D *ps_Vector);

/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//MEMORY_QUATERNION_H
