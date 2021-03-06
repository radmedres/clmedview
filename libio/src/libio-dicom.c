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

#include "libcommon-debug.h"
#include "libio-dicom.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <byteswap.h>
#include <math.h>

#include "dicom/include/znzlib.h"
#include "dicom/include/zz.h"
#include "dicom/include/zz_priv.h"

/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
short int i16_memory_io_dicom_relativePosition(Coordinate3D *ps_referencePosition,
                                               Coordinate3D *ps_position,
                                               Vector3D     *ps_Zvector,
                                               Coordinate3D *ps_pixel_dimension)
{
  Coordinate3D s_Delta;
  Vector3D relativeVector;

  s_Delta.z = ps_position->z - ps_referencePosition->z;

  relativeVector.z = s_Delta.z / (ps_Zvector->z * ps_pixel_dimension->z);

  return round(relativeVector.z);
}


short int i16_memory_io_dicom_loadMetaData(Patient *ps_patient,
                                           Study *ps_study,
                                           Serie *ps_serie,
                                           Coordinate3D *ps_SlicePosition,
                                           Vector3D  *ps_XVector,
                                           Vector3D  *ps_YVector,
                                           short int *pi16_TemporalPositionIdentifier,
                                           short int *pi16_StackPositionIdentifier,
                                           te_DCM_ComplexImageComponent *pe_DCM_CIC,
                                           const char *pc_dicom)
{
  struct zzfile szz, *zz;
  char value[MAX_LEN_LO];
  double imageposvector[3];
  double imageorientation[9];

  double tmpd[2];
  uint16_t group, element;
  long len;

  zz = zzopen(pc_dicom, "r", &szz);
  if (!zz)
  {
    return 0;
  }



  char c_currentPatientID[MAX_LEN_LO];
  char c_currentStudyInstanceUID[MAX_LEN_LO];
  char c_currentSerieInstanceUID[MAX_LEN_LO];
  char c_ComplexImageComponent[MAX_LEN_LO];

  memset(c_currentPatientID,'\0',MAX_LEN_LO);
  memset(c_currentStudyInstanceUID,'\0',MAX_LEN_LO);
  memset(c_currentSerieInstanceUID,'\0',MAX_LEN_LO);
  memset(c_ComplexImageComponent,'\0',MAX_LEN_LO);


  zziterinit(zz);
  while (zziternext(zz, &group, &element, &len))
  {
    switch (ZZ_KEY(group, element))
    {
      case DCM_PatientID:
        zzgetstring(zz, c_currentPatientID, sizeof(c_currentPatientID)-1);
        break;
      case DCM_StudyInstanceUID:
        zzgetstring(zz, c_currentStudyInstanceUID, sizeof(c_currentStudyInstanceUID)-1);
        break;
      case DCM_SeriesInstanceUID:
        zzgetstring(zz, c_currentSerieInstanceUID, sizeof(c_currentSerieInstanceUID)-1);
        break;
      default : break;
    }
    if ((c_currentPatientID[0] != '\0') && (c_currentStudyInstanceUID[0] != '\0') &&  (c_currentSerieInstanceUID[0] != '\0'))
    {
      break;
    }
  }

  if ((ps_patient->c_patientID[0] == '\0') &&
      (ps_study->c_studyInstanceUID[0] == '\0') &&
      (ps_serie->c_serieInstanceUID[0] == '\0'))
  {
    // first time, set params.
    memcpy(ps_patient->c_patientID,c_currentPatientID,strlen(c_currentPatientID));
    memcpy(ps_study->c_studyInstanceUID,c_currentStudyInstanceUID,strlen(c_currentStudyInstanceUID));
    memcpy(ps_serie->c_serieInstanceUID,c_currentSerieInstanceUID,strlen(c_currentSerieInstanceUID));
    ps_serie->pc_filename = calloc(1,strlen(pc_dicom));
    memcpy(ps_serie->pc_filename,pc_dicom, strlen(pc_dicom));

  }

  if ((memcmp(ps_patient->c_patientID, c_currentPatientID,sizeof(ps_patient->c_patientID))==0) &&
           (memcmp(ps_study->c_studyInstanceUID, c_currentStudyInstanceUID,sizeof(ps_study->c_studyInstanceUID))==0) &&
           (memcmp(ps_serie->c_serieInstanceUID, c_currentSerieInstanceUID,sizeof(ps_serie->c_serieInstanceUID))==0))
  {
    ps_serie->pc_filename = calloc(1, strlen(pc_dicom));
    strcpy(ps_serie->pc_filename,pc_dicom);

    ps_serie->input_type = MUMC_FILETYPE_DICOM;

    ps_serie->i16_QuaternionCode = COORDINATES_SCANNER_ANAT;
    ps_serie->num_time_series = 1;

    zzclose(zz);
    zz = zzopen(pc_dicom, "r", &szz);
    zziterinit(zz);
    while (zziternext(zz, &group, &element, &len))
    {
      switch (ZZ_KEY(group, element))
      {
        case DCM_BitsAllocated:
          ps_serie->data_type = ( zzgetuint16(zz, 0) == 16) ? MEMORY_TYPE_UINT16 : MEMORY_TYPE_ERROR;
          if (ps_serie->data_type == MEMORY_TYPE_ERROR)
          {
            return 0;
          }
          break;

        case DCM_NumberOfFrames:
          zzgetstring(zz, value, sizeof(value) - 1);
          ps_serie->matrix.i16_z = (float)(atoi(value));
          break;
        case DCM_Rows:
          ps_serie->matrix.i16_y = (float)(zzgetuint16(zz, 0));
          break;
        case DCM_Columns:
          ps_serie->matrix.i16_x = (float)(zzgetuint16(zz, 0));
          break;
        case DCM_PatientsName:
          zzgetstring(zz, ps_patient->name, sizeof(ps_patient->name) - 1);
          break;
        case DCM_StudyDescription:
          zzgetstring(zz, ps_study->name, sizeof(ps_study->name) - 1);
          break;
        case DCM_SeriesDescription:
          zzgetstring(zz, ps_serie->name, sizeof(ps_serie->name) - 1);
          break;
        case DCM_ImagePositionPatient:		// DS, 3 values
          zzrDS(zz, 3, imageposvector);
          ps_SlicePosition->x = imageposvector[0];
          ps_SlicePosition->y = imageposvector[1];
          ps_SlicePosition->z = imageposvector[2];
          break;
        case DCM_ImageOrientationPatient:	// DS, 6 values
          zzrDS(zz, 6, imageorientation);
          ps_XVector->x = imageorientation[0];
          ps_XVector->y = imageorientation[1];
          ps_XVector->z = imageorientation[2];
          ps_YVector->x = imageorientation[3];
          ps_YVector->y = imageorientation[4];
          ps_YVector->z = imageorientation[5];
          break;
        case DCM_RescaleIntercept:	// DS, the b in m*SV + b
          zzgetstring(zz, value, sizeof(value) - 1);
          ps_serie->offset = atof(value);
          break;
        case DCM_RescaleSlope:		// DS, the m in m*SV + b
          zzgetstring(zz, value, sizeof(value) - 1);
          ps_serie->slope = atof(value);
          break;
        case DCM_PixelSpacing:
          zzrDS(zz, 2, tmpd);
          ps_serie->pixel_dimension.x = tmpd[0];
          ps_serie->pixel_dimension.y = tmpd[1];
          break;
        case DCM_SliceThickness:
          zzrDS(zz, 1, tmpd);
          ps_serie->pixel_dimension.z = tmpd[0];
          break;
        case DCM_NumberOfTemporalPositions:
          zzrDS(zz, 1, tmpd);
          ps_serie->num_time_series = tmpd[0];
          break;
        case DCM_TemporalPositionIdentifier:
          zzrDS(zz, 1, tmpd);
          *pi16_TemporalPositionIdentifier = tmpd[0];
          break;
        case DCM_InStackPositionNumber:
          zzrDS(zz, 1, tmpd);
          *pi16_StackPositionIdentifier = tmpd[0];
          break;
        case DCM_ComplexImageComponent:
          zzgetstring(zz, c_ComplexImageComponent, sizeof(c_ComplexImageComponent) - 1);

          if (memcmp("MAGNITUDE", c_ComplexImageComponent,9)==0)
          {
            *pe_DCM_CIC = DCM_CIC_MAGNITUDE;
          }
          else if (memcmp("PHASE", c_ComplexImageComponent,5)==0)
          {
            *pe_DCM_CIC = DCM_CIC_PHASE;
          }
          else if (memcmp("REAL", c_ComplexImageComponent,4)==0)
          {
            *pe_DCM_CIC = DCM_CIC_REAL;
          }
          else if (memcmp("IMAGINARY", c_ComplexImageComponent,9)==0)
          {
            *pe_DCM_CIC = DCM_CIC_IMAGINARY;
          }
          else if (memcmp("MIXED", c_ComplexImageComponent,5)==0)
          {
            *pe_DCM_CIC = DCM_CIC_MIXED;
          }

          break;




        default : break;
      }
    }
    zz = zzclose(zz);

    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][0] = -imageorientation[0];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][1] = -imageorientation[3];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][2] =  -(imageorientation[1] * imageorientation[5] -
                                                        imageorientation[2] * imageorientation[4]);
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[0][3] = 0;

    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][0] = -imageorientation[1];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][1] = -imageorientation[4];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][2] =  -(imageorientation[2] * imageorientation[3] -
                                                        imageorientation[0] * imageorientation[5]);
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[1][3] = 0;

    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][0] = imageorientation[2];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][1] = imageorientation[5];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][2] = (imageorientation[0] * imageorientation[4] -
                                                       imageorientation[1] * imageorientation[3]);
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[2][3] = 0;

    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][0] = -imageposvector[0];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][1] = -imageposvector[1];
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][2] = imageposvector[2];;
    ps_serie->t_ScannerSpaceIJKtoXYZ.af_Matrix[3][3] = 1;

    return 1;
  }
  else
  {
  	zz = zzclose(zz);
    return 0;
  }
  return 0;
}

short int i16_memory_io_dicom_loadSingleSlice(Serie *ps_serie, const char *pc_dicom, short int i16_SliceNumber, short int i16_timeFrameNumber)
{
  struct zzfile szz, *zz;
  uint16_t group, element;
  long len;

  void *pv_data;

  short int i16_BytesToRead;
  int i32_PixelsInSlice, i32_MemoryPerSlice, i32_MemoryOffset, i32_MemoryPerVolume, i32_MemoryInBlob;

  zz = zzopen(pc_dicom, "r", &szz);
  if (!zz)
  {
    return 0;
  }

  zziterinit(zz);
  while (zziternext(zz, &group, &element, &len))
  {
    switch (ZZ_KEY(group, element))
    {
      case DCM_PixelData:
        if (ps_serie->data == NULL)
        {
          if (ps_serie->matrix.i16_z == 0)
          {
            ps_serie->matrix.i16_z = 1;
          }

          i16_BytesToRead = 2;
          i32_PixelsInSlice = ps_serie->matrix.i16_x * ps_serie->matrix.i16_y * ps_serie->matrix.i16_z;
          i32_MemoryPerVolume = i16_BytesToRead * i32_PixelsInSlice;
          i32_MemoryInBlob = i32_MemoryPerVolume * ps_serie->num_time_series;

          ps_serie->data = calloc (1, i32_MemoryInBlob);
          ps_serie->pv_OutOfBlobValue = calloc (1, i16_BytesToRead);

        }

        i16_BytesToRead = 2;
        i32_PixelsInSlice = ps_serie->matrix.i16_x * ps_serie->matrix.i16_y;
        i32_MemoryPerSlice = i16_BytesToRead * i32_PixelsInSlice;

        i32_MemoryOffset= i16_timeFrameNumber * i32_MemoryPerSlice * ps_serie->matrix.i16_z;
        i32_MemoryOffset+=i16_SliceNumber * i32_MemoryPerSlice;


        pv_data=ps_serie->data;
        pv_data+=i32_MemoryOffset;

        void *pv_tmpData=zireadbuf(zz->zi, i32_MemoryPerSlice );
        memcpy(pv_data,pv_tmpData, i32_MemoryPerSlice );

        zifreebuf(zz->zi, pv_tmpData, i32_MemoryPerSlice );

        break;
      default : break;
    }
  }
  zz = zzclose(zz);
  return 1;
}

