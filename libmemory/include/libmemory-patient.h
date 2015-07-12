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

#ifndef MEMORY_PATIENT_H
#define MEMORY_PATIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libmemory.h"

/**
 * @file   include/lib-memory-patient.h
 * @brief  The interface to Patient-specific functionality.
 * @author Roel Janssen
 */

/**
 * @ingroup memory
 * @{
 *
 *   @defgroup memory_patient Patient
 *   @{
 *
 * This module provides Patient-specific functionality.
 */


/**
 * This structure is the base element to store patient information.
 */
typedef struct
{
  /**
   * A unique identifier for a patient.
   */
  unsigned long long id;

  /**
   * A unique identifier for a patient used in dicom files.
   */
  char c_patientID[64];

  /**
   * The name of a patient.
   */
  char name[100];
} Patient;


/**
 * This function creates a new instance of a Patient object.
 * @note An identifier is automatically assigned to a Patient.
 *
 * @param name  The name of the patient.
 *
 * @return A pointer to a newly created Patient object.
 */
Patient* memory_patient_new (const char *name);


/**
 * This function clean up a Patient object.
 * @param data  The Patient to clean up.
 */
void memory_patient_destroy (void *data);


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//MEMORY_PATIENT_H
