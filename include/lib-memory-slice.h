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

#ifndef MEMORY_SLICE_H
#define MEMORY_SLICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib-memory.h"
#include "lib-memory-serie.h"
#include "lib-common.h"

/**
 * @file   include/lib-memory-slice.h
 * @brief  The functionality provided by Memory specifically for slices.
 * @author Roel Janssen
 */

/**
 * @ingroup memory
 * @{
 *
 *   @defgroup memory_slice Slice
 *   @{
 *
 * This module provides Slice-specific functionality.
 */

/**
 * This structure groups some viewport properties which only change if the
 * normal vector or pivot point changes.
 *
 */
typedef struct
{
  Vector3D ts_normalVector;
  Vector3D ts_perpendicularVector;
  Vector3D ts_crossproductVector;


  short int i16_StrideHeight;
  short int i16_StrideWidth;
  short int i16_StrideDepth;

  short int i16_StartHeight;
  short int i16_StartWidth;

  short int i16_StopHeight;
  short int i16_StopWidth;
} ViewportProperties;

/**
 * This structure is the base element to store slice information.
 */
typedef struct
{
  /**
   * A unique identifier for a Slice.
   */
  unsigned long long id;

  /**
   * The size matrix of the Slice.
   */
  ts_Coordinate3DInt matrix;

  /**
   * The point in time for time series.
   */
  unsigned short int u16_timePoint;

  /**
   * The normal vector to calculate the plane in a Serie volume.
   */
  Vector3D *ps_NormalVector;

  /**
   * The position vector to calculate the plane in a Serie volume.
   */
  Vector3D *ps_PivotPoint;

  /**
   * The up vector to define what the 'upside' is.
   */
  Vector3D *ps_upVector;

  /**
   * The scale factor of the slice in the horizontal direction.
   */
  float f_ScaleFactorX;

  /**
   * The scale factor of the slice in the vertical direction.
   */
  float f_ScaleFactorY;

  /**
   * The serie this slice's data is pointing to.
   */
  Serie *serie;

  /**
   * A field to allocate a group of pointers to point to the actual volume data.
   */
  void *data;

  /**
   * Viewport Change (widht, height, strides etc)
   */
  short int i16_ViewportChange;

  /**
   * Viewport properties (widht, height, strides etc)
   */
  ViewportProperties viewportProperties;

  /**
  * Image Directions of image loaded
  */
  te_MemoryImageDirection e_ImageDirection_I;
  te_MemoryImageDirection e_ImageDirection_J;
  te_MemoryImageDirection e_ImageDirection_K;


} Slice;

/**
 * This function gets the data for a slice. The actual data for the slice is
 * loaded into memory by calling this function.
 *
 * @param slice  The slice to get data for.
 *
 * @return The data for a slice.
 */
void* memory_slice_get_data (Slice* slice);

/**
 * This function creates a new slice from a serie.
 *
 * @param serie  The Serie to get a slice of.
 *
 * @return A newly created slice.
 */
Slice* memory_slice_new (Serie* serie);

/**
 * This function cleans up the memory for a given slice.
 * @param data  The Slice to clean up.
 */
void memory_slice_destroy (void *data);

/**
 * This function returns the next slice.
 * @param slice  The current slice.
 *
 * @return The next slice or the current slice when there is no next slice.
 */
Slice* memory_slice_get_next (Slice *slice);

/**
 * This function returns the previous slice.
 * @param slice  The current slice.
 *
 * @return The previous slice or the current slice in lack of a previous.
 */
Slice* memory_slice_get_previous (Slice *slice);

/**
 * This function returns the nth slice.
 * @param slice  The current slice.
 * @param nth    The number of the slice to set.
 *
 * @return The nth slice.
 */
Slice* memory_slice_get_nth (Slice *slice, int nth);

/**
 * This function sets the nth timepoint.
 * @param slice      The current slice.
 * @param timepoint  The number of the slice to set.
 */
void memory_slice_set_timepoint (Slice *slice, unsigned short int timepoint);

/**
 * This function returns the depth of the slice with the orientation of the
 * slice taken into account.
 *
 * @param slice  The slice to get the depth of.
 *
 * @return The number of slices in the Z-axis.
 */
//float memory_slice_get_depth (Slice *slice);

/**
 * This function sets the normal vector.
 * @param slice            The current slice.
 * @param ps_NormalVector  Pointer to external normal vector.
 */
void memory_slice_set_NormalVector (Slice *slice, Vector3D *ps_NormalVector);

/**
 * This function sets the pivot point.
 * @param slice            The current slice.
 * @param ps_NormalVector  Pointer to external pivot point.
 */
void memory_slice_set_PivotPoint (Slice *slice, Vector3D *ps_PivotPoint);

/**
 * This function sets the up vector.
 * @param slice            The current slice.
 * @param ps_NormalVector  Pointer to external up vector.
 */
void memory_slice_set_UpVector (Slice *slice, Vector3D *ps_upVector);


/**
 *    @}
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif//MEMORY_SLICE_H
