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

typedef struct
{
  double W;
  double I;
  double J;
  double K;
} ts_Quaternion;

ts_Quaternion *memory_quaternion_new (void);
ts_Quaternion *memory_quaternion_set (double W, double A, double B, double C);
ts_Quaternion *memory_quaternion_copy (ts_Quaternion *ps_source);
ts_Quaternion *memory_quaternion_negative (ts_Quaternion *ps_source);
ts_Quaternion *memory_quaternion_conjungate (ts_Quaternion *ps_source);
ts_Quaternion *memory_quaternion_add_value (ts_Quaternion *ps_source, double d_Value);
ts_Quaternion *memory_quaternion_add (ts_Quaternion *ps_first, ts_Quaternion *ps_second);
ts_Quaternion *memory_quaternion_multiply_double (ts_Quaternion *ps_source, double d_Value);
ts_Quaternion *memory_quaternion_multiply (ts_Quaternion *ps_first, ts_Quaternion *ps_second);

double memory_quaternion_normalize (ts_Quaternion *ts_quaternion);
short int memory_quaternion_equal (ts_Quaternion *ps_first, ts_Quaternion *ps_second);


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//MEMORY_QUATERNION_H
