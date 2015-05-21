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

#ifndef DICOM_DICOM_H
#define DICOM_DICOM_H

/**
 * @defgroup dicom DICOM
 * @{
 *
 * This module take care off reading and interpreting dicom files to a common file type.
 */
#include <stdio.h>
#include <stdlib.h>

#include "lib-memory.h"
#include "lib-memory-patient.h"
#include "lib-memory-study.h"
#include "lib-memory-serie.h"


typedef enum
{
  DCM_CIC_MAGNITUDE,
  DCM_CIC_PHASE,
  DCM_CIC_REAL,
  DCM_CIC_IMAGINARY,
  DCM_CIC_MIXED
} te_DCM_ComplexImageComponent;


/**
 * Load a dicom file from disk to the selected memory.
 *
 * @param ps_reference    Reference coordinate, to calculate relative distance from (in z axis)
 * @param ps_serie        Serie
 *
 * @return relative slice number
 */
short int i16_memory_io_dicom_relativePosition(Coordinate3D *ps_referencePosition,
                                               Coordinate3D *ps_position,
                                               td_Matrix4x4 *pt_rotationMatrix,
                                               Coordinate3D *ps_pixel_dimension);

/**
 * Load a single dicom file from disk to the selected memory.
 *
 * @param ps_patient      patient struct, to store study related information
 * @param ps_study        study struct, to store study related information
 * @param ps_serie        serie struct, to store study related information
 * @param pc_dicom        Filename/path of the header file
 *
 * @return 0 or FALSE if function executes wrong, 1 or TRUE if execution is correct
 */
short int i16_memory_io_dicom_loadMetaData(Patient *ps_patient,
                                           Study *ps_study,
                                           Serie *ps_serie,
                                           Coordinate3D *ps_SlicePosition,
                                           short int *pi16_TemporalPositionIdentifier,
                                           short int *pi16_StackPositionIdentifier,
                                           te_DCM_ComplexImageComponent *pe_DCM_CIC,
                                           const char *pc_dicom);


/**
 * Load a single dicom file from disk to the selected memory.
 *
 * @param serie           The selected memory to store all needed parameters in
 * @param pc_dicom        Filename/path of the header file
 * @param i16_SliceNumber Input to know slice position
 *
 * @return 0 or FALSE if function executes wrong, 1 or TRUE if execution is correct
 */
short int i16_memory_io_dicom_loadSingleSlice(Serie *ps_serie,
                                              const char *pc_dicom,
                                              short int i16_SliceNumber,
                                              short int i16_timeFrameNumber);

/**
 * Load a dicom file from disk to the selected memory.
 *
 * @param serie    The selected memory to store all needed parameters in
 * @param pc_file Filename/path of the header file
 *
 * @return 0 or FALSE if function executes wrong, 1 or TRUE if execution is correct
 */
short int i16_memory_io_dicom_load(Patient  *ps_patient, Study *ps_study, Serie *ps_serie, const char *pc_dicom);

#endif//NIFTII_NIFTII_H
