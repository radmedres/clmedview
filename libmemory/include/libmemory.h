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

#ifndef MEMORY_H
#define MEMORY_H

/**
 * @file   include/lib-memory.h
 * @brief  The internal representation of data in memory.
 * @author Jos Slenter, Roel Janssen
 */


/**
 * @defgroup memory Memory
 * @{
 *
 * This module provides an internal image format that allows other layers
 * to interact with all supported image formats in a standardized way.
 *
 * The module has been separated into several submodules which all-together
 * provide an image format abstraction layer.
 */


/**
 * This enumeration contains all known image orientations.
 */
typedef enum
{
  ORIENTATION_UNKNOWN,
  ORIENTATION_AXIAL,
  ORIENTATION_SAGITAL,
  ORIENTATION_CORONAL
} MemoryImageOrientation;

/**
 * This enumeration contains all orientation directions
 */
typedef enum
{
  DIRECTION_ERROR,
  DIRECTION_L2R,
  DIRECTION_R2L,
  DIRECTION_P2A,
  DIRECTION_A2P,
  DIRECTION_I2S,
  DIRECTION_S2I
} te_MemoryImageDirection;


typedef enum
{
  DIRECTION_PART_ALL,
  DIRECTION_PART_FIRST,
  DIRECTION_PART_LAST
} te_MemoryImageDirectionPart;

/**
 * This enumeration contains all known memory types.
 */
typedef enum
{
  MEMORY_TYPE_ERROR,
  MEMORY_TYPE_NONE,

  MEMORY_TYPE_UINT8,
  MEMORY_TYPE_UINT16,
  MEMORY_TYPE_UINT32,
  MEMORY_TYPE_UINT64,

  MEMORY_TYPE_INT8,
  MEMORY_TYPE_INT16,
  MEMORY_TYPE_INT32,
  MEMORY_TYPE_INT64,

  MEMORY_TYPE_FLOAT32,
  MEMORY_TYPE_FLOAT64,
  MEMORY_TYPE_FLOAT128,

  MEMORY_TYPE_COMPLEX64,
  MEMORY_TYPE_COMPLEX128,
  MEMORY_TYPE_COMPLEX256,

  MEMORY_TYPE_RGB24,
  MEMORY_TYPE_RGBA32
} MemoryDataType;


/**
 * This enumeration contains all known file types.
 */
typedef enum ImageIOFiletype
{
  MUMC_FILETYPE_ERROR,
  MUMC_FILETYPE_NOT_KNOWN,
  MUMC_FILETYPE_ANALYZE75,
  MUMC_FILETYPE_NIFTII_TF,
  MUMC_FILETYPE_NIFTII_SF,
  MUMC_FILETYPE_DICOM
} te_ImageIOFiletype;


/**
 * @}
 */

#endif//MEMORY_H
