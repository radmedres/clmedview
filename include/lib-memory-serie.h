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

#ifndef MEMORY_SERIE_H
#define MEMORY_SERIE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib-common-tree.h"
#include "lib-common.h"
#include "lib-common-list.h"
#include "lib-memory.h"
#include "lib-memory-quaternion.h"


/**
 * @file   include/lib-memory-serie.h
 * @brief  The interface to Serie-specific functionality.
 * @author Roel Janssen
 */


/**
 * @ingroup memory
 * @{
 *
 *   @defgroup memory_serie Serie
 *   @{
 *
 * This module provides Serie-specific functionality.
 */

/**
 * This structure is the base element to store serie information.
 */
typedef struct
{
  /**
   * A unique identifier for a Serie.
   */
  unsigned long long id;

  /**
   * The name for a Serie.
   */
  char name[100];

  /**
   * The filename of Serie.
   */
  char *pc_filename;


  /**
   * The group identifier to which this Serie belongs.
   */
  unsigned long long group_id;

  /**
   * The 3D size matrix of the Serie volume.
   */
  Coordinate3D matrix;

  /**
   * The pixel dimension matrix of the Serie.
   */
  Coordinate3D pixel_dimension;

  /**
   * The slope setting of the Serie.
   */
  float slope;

  /**
   * The offset setting of the Serie.
   */
  float offset;

  /**
   * The raw data type definition for the Serie.
   * It is used to save as a NIFTII file later on.
   */
  short int raw_data_type;

  /**
   * The enumerated data type for the Serie.
   */
  MemoryDataType data_type;

  /**
   * The enumerated input type from the original file the Serie.
   */
  te_ImageIOFiletype input_type;

  /**
   * The number of time series in the Serie.
   */
  short unsigned int num_time_series;

  /**
   * A field to store the Serie volume data. The allocated size of this blob
   * can be determined by multiplying the dimensions of the 'matrix' field
   * times the size of the data_type, which can be determined with
   * memory_serie_get_memory_space().
   */
  void *data;

  /**
   * A pointer to the value that is pointed to for all values that are not
   * inside the volume of the Serie.
   *
   * When we take a slice that is half outside of the volume, the value of this
   * pointer is pointed to by this variable is read for all places outside of
   * the volume.
   */
  void *pv_OutOfBlobValue;

  /**
   * A variable containing all units of the dimensions
   */
  unsigned char u8_AxisUnits;

  /**
  * The direction of the z ax
  */
  double d_Qfac;

  /**
  * A value which indicates the stride in a positive direction according to
  * the right handed coordinate system
  */
  Vector3D ts_Stride;

  /**
  * Quaternion code code which defined the type of space
  */
  short int i16_QuaternionCode;

  /**
  * Quaternion which describes the scanner space
  */
  ts_Quaternion *ps_Quaternion;

  /**
  * Offset at [0,0,0]
  */
  ts_Quaternion *ps_QuaternationOffset;

  /**
  * Rotation matrix of scanner space.
  * It defines the translation from IJK space to XYZ
  */
  td_Matrix4x4 t_ScannerSpaceIJKtoXYZ;

  /**
  * Inverse rotation matrix of scanner space.
  * It defines the translation from XYZ space to IJK
  */
  td_Matrix4x4 t_ScannerSpaceXYZtoIJK;

  /**
  * Standard space code which defined the type of space
  */
  short int i16_StandardSpaceCode;

  /**
  * Rotation matrix of scanner space.
  * It defines the translation from IJK space to XYZ
  */
  td_Matrix4x4 t_StandardSpaceIJKtoXYZ;

  /**
  * Inverse rotation matrix of scanner space.
  * It defines the translation from XYZ space to IJK
  */
  td_Matrix4x4 t_StandardSpaceXYZtoIJK;


  /**
  * Pointer to selected rotation matrices
  */
  td_Matrix4x4 *pt_RotationMatrix;

  /**
  * Pointer to selected inverse rotation matrices
  */
  td_Matrix4x4 *pt_InverseMatrix;

  /**
  * Minimum value in serie.
  */
  int i32_MinimumValue;

  /**
  * Maximum value in serie
  */
  int i32_MaximumValue;
} Serie;


/**
 * This function returns a unique identifier for a Serie.
 * @return A unique identifier for a Serie.
 */
unsigned long long memory_serie_next_id ();


/**
 * This function returns a newly created Serie object.
 *
 * @param name  A name for the serie.
 * @param fileName  The filename of the original file.
 *
 * @return A pointer to a newly created Serie object.
 */
Serie*
memory_serie_new (const char *name, const char *pc_filename);


/**
 * This function clean up a Serie object.
 * @param data  The Serie to clean up.
 */
void memory_serie_destroy (void *data);


/**
 * This function returns the memory type of a Serie.
 * @param serie  The serie to get the memory type of.
 *
 * @return The MemoryDataType for the serie.
 */
MemoryDataType memory_serie_get_memory_type (Serie *serie);


/**
 * This function returns the number of bytes (space) for the
 * memory type of a Serie.
 *
 * @param serie  The Serie to get the memory space for.
 *
 * @return The number of bytes per voxel for serie.
 */
short int memory_serie_get_memory_space (Serie *serie);


/**
 * This function set the minimum and maximum value of a Serie.
 *
 * @param serie  The Serie to get the minimum value for.
 */
void memory_serie_set_upper_and_lower_borders_from_data(Serie *serie);

/**
 * This function creates a new (mask)serie which is cloned from the original serie.
 *
 * @param serie The serie to clone from.
 *
 * @return A (mask)serie of the original serie
 */
Serie *memory_serie_create_mask_from_serie (Serie *serie);

/**
 * This function calculates the pivot point of a blob.
 *
 * @param serie The serie to calculate the point from.
 *
 * @return A [x,y,z] coordinate to the pivot point
 */
Coordinate3D memory_serie_GetPivotpoint(Serie *serie);


/**
 * This function calculates the pivot point of a blob.
 *
 * @param serie The serie to calculate the point from.
 *
 * @return A [x,y,z] coordinate to the pivot point
 */
void v_memory_io_handleSpace(Serie *serie);

/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//MEMORY_SERIE_H
