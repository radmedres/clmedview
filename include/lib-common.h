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

#ifndef COMMON_H
#define COMMON_H

/**
 * @defgroup common Common
 * @{
 *
 * This module provides several types for grouping related variables.
 * It also provides several generic structured types (List, Tree and History).
 */


/**
 * In the program there's a common need for storing x/y data.
 * This struct provides just that.
 */
typedef struct
{
  float x;
  float y;
} Coordinate;

typedef Coordinate Vector;

/**
 * In the program there's a common need for storing x/y/z data.
 * This struct provides just that.
 */
typedef struct
{
  float x;
  float y;
  float z;
} Coordinate3D;

typedef Coordinate3D Vector3D;

/**
 * In the program there's a common need for storing x/y/z as short int data.
 * This struct provides just that.
 */
typedef struct
{
  short int i16_x;
  short int i16_y;
  short int i16_z;
} ts_Coordinate3DInt;

typedef ts_Coordinate3DInt ts_Vector3DInt;


/**
 * In the program there's a common need for storing width/height data.
 * This struct provides just that.
 */
typedef struct
{
  float width;
  float height;
} Plane;

/**
 * @}
 */

#endif//COMMON_H
