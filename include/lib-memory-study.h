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

#ifndef MEMORY_STUDY_H
#define MEMORY_STUDY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib-memory.h"


/**
 * @file   include/lib-memory-study.h
 * @brief  The interface to Study-specific functionality.
 * @author Roel Janssen
 */


/**
 * @ingroup memory
 * @{
 * 
 *   @defgroup memory_study Study
 *   @{
 *
 * This module provides Study-specific functionality.
 */


/**
 * This structure is the base element to store patient information.
 */
typedef struct
{
  /**
   * A unique identifier for a Study.
   */
  unsigned long long id;

  /**
   * The name for a Study.
   */
  char name[100];  

} Study;


/**
 * This function creates a new instance of a Study object.
 * @note An identifier is automatically assigned to a Study.
 *
 * @param name  The name of the study.
 *
 * @return A pointer to a newly created Study object. 
 */
Study* memory_study_new (const char *name);


/**
 * This function clean up a Study object.
 * @param data  The Study to clean up.
 */
void memory_study_destroy (void *data);


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//MEMORY_STUDY_H
