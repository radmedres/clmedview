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

#include "lib-memory-io.h"
#include "lib-memory.h"
#include "lib-memory-tree.h"
#include "lib-memory-patient.h"
#include "lib-memory-study.h"
#include "lib-memory-serie.h"
#include "lib-common-debug.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

#include <string.h>

// On Microsoft Windows we should search for backslashes instead of forward
// slashes.
#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif


/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */

/**
 * This enumeration contains all known image orientations.
 */
short int i16_memory_io_isDirectory(const char *path);
short int i16_memory_io_isFile(const char *path);
short int i16_memory_io_load_file_nifti (Tree **patient_tree, char *path);
short int i16_memory_io_load_file_dicom (Tree **patient_tree, char *path);


typedef struct s_dicom_FileProperties
{
  char                          *pc_Filename;
  Coordinate3D                  ts_Position;
  short int                     i16_relativeOrderNumber;
  short int                     i16_TemporalPositionIdentifier;
  short int                     i16_StackPositionIdentifier;

  te_DCM_ComplexImageComponent  e_DCM_CIC;

} ts_dicom_FileProperties;



short int
i16_memory_io_isDirectory(const char *path)
{
  struct stat statbuf;

  if (stat(path, &statbuf) != 0)
  {
    return 0;
  }

  return S_ISDIR(statbuf.st_mode);
}


short int
i16_memory_io_isFile(const char *path)
{
  struct stat statbuf;

  if (stat(path, &statbuf) != 0)
  {
    return 0;
  }

  return S_ISREG(statbuf.st_mode);
}

Tree *pt_memory_io_load_file_nifti (Tree **ppt_study, char *pc_path)
{
  char *pc_filename=NULL;

  Tree *pt_patient=NULL;
  Tree *pt_serie=NULL;

  Patient *ps_patient = NULL;
  Study *ps_study = NULL;
  Serie *ps_serie = NULL;

  pc_filename = basename(pc_path);

  if (*ppt_study==NULL)
  {
    ps_patient=memory_patient_new("unknown");
    pt_patient=tree_append(NULL,ps_patient,TREE_TYPE_PATIENT);

    ps_study=memory_study_new ("unknown");
    *ppt_study=tree_append_child(pt_patient, ps_study, TREE_TYPE_STUDY);
  }

  ps_serie = memory_serie_new (pc_filename, pc_path);
  pt_serie = tree_append_child (*ppt_study, ps_serie, TREE_TYPE_SERIE);

  switch (memory_io_niftii_file_type (pc_path))
  {
    case MUMC_FILETYPE_NIFTII_SF: return (memory_io_niftii_load (ps_serie, pc_path, NULL) == 1) ? pt_serie : NULL; break;
    case MUMC_FILETYPE_NIFTII_TF:
      {
        //Check weather the hdr or img file is passed
        char* pc_Extension = strrchr (pc_path, '.');

        char c_ImageFile[1024];
        char c_HeaderFile[1024];

        strcpy(&c_ImageFile[0],pc_path);
        strcpy(&c_HeaderFile[0],pc_path);

        if (pc_Extension == NULL)
        {
          pc_Extension = strrchr (&c_ImageFile[0], '.');
          strcat(pc_Extension, ".img");

          pc_Extension = strrchr (&c_HeaderFile[0], '.');
          strcat(pc_Extension, ".hdr");
        }
        else
        {
          pc_Extension = strrchr (&c_ImageFile[0], '.');
          strcpy(pc_Extension, ".img");
          pc_Extension = strrchr (&c_HeaderFile[0], '.');
          strcpy(pc_Extension, ".hdr");
        }

        return (memory_io_niftii_load (ps_serie, c_ImageFile, c_HeaderFile) == 1) ? pt_serie : NULL; break;
      }
      break;
    case MUMC_FILETYPE_ANALYZE75:
    case MUMC_FILETYPE_DICOM:
    case MUMC_FILETYPE_NOT_KNOWN:
    default:
      return NULL;
  }

}

Tree *pt_memory_io_load_file_dicom (Tree **patient_tree, char *pc_path)
{
  Patient *ps_patient = NULL;
  Patient *patient=NULL;

  Study *ps_study = NULL;
  Study *study=NULL;
  Serie *ps_serie=NULL;
  Serie *serie=NULL;

  Tree *patientTreeIterator=NULL;
  Tree *studyTreeIterator=NULL;
  Tree *serieTreeIterator=NULL;
  Tree *pt_serie=NULL;

  DIR *p_dicomDirectory;
  struct dirent *p_dirEntry;

  char *pc_dirName = NULL;
  char *pc_fullPath = NULL;

  ts_dicom_FileProperties *ps_ReferenceFileProps = NULL;
  ts_dicom_FileProperties *ps_dicomFile;

  List *pll_dicomFiles = NULL;
  List *pll_dicomFilesIter = NULL;

  short int i16_NumberOfSlices=0;
  short int i16_MinimumReferenceOrderValue=0;
  short int i16_MaximumReferenceOrderValue=0;
  short int i16_TemporalPositionIdentifier=0;
  short int i16_StackPositionIdentifier=0;
  short int i16_NumberOfReconstructions=0;
  short int b_StudyExists=0;
  short int b_SerieExists=0;
  short int b_PatientExists=0;
  short int i16_Cnt=0;
  short int i16_timeFrameCnt=0;

  te_DCM_ComplexImageComponent e_DCM_CIC;

  // Build list of all files

  // Check weather path is a path or a directory
  pc_dirName = (i16_memory_io_isFile(pc_path)) ? dirname(pc_path) : pc_path;
  p_dicomDirectory = opendir (pc_dirName);

  if (p_dicomDirectory == NULL)
  {
    return 0;
  }

  ps_patient = memory_patient_new ("TEST");
  ps_study = memory_study_new("TEST");

  ps_serie = memory_serie_new("TEST",NULL);

  p_dirEntry = readdir (p_dicomDirectory);
  pll_dicomFilesIter = pll_dicomFiles;

  while (p_dirEntry!=NULL)
  {
    // create Full path name
    pc_fullPath = calloc(1, strlen(pc_path)+2+strlen(p_dirEntry->d_name));
    strcpy(pc_fullPath,pc_dirName);
    strcpy(&pc_fullPath[strlen(pc_fullPath)],"/");
    strcpy(&pc_fullPath[strlen(pc_fullPath)],p_dirEntry->d_name);

    Coordinate3D ts_slicePosition;

    if (i16_memory_io_dicom_loadMetaData(ps_patient,ps_study,ps_serie, &ts_slicePosition, &i16_TemporalPositionIdentifier, &i16_StackPositionIdentifier, &e_DCM_CIC,  pc_fullPath))
    {
      ts_dicom_FileProperties *ps_dicomFile=calloc(1,sizeof(ts_dicom_FileProperties));

      if ( ps_ReferenceFileProps == NULL)
      {
        //first time passing this loop, refer to first read file
        ps_ReferenceFileProps = ps_dicomFile;
      }

      ps_dicomFile->pc_Filename = pc_fullPath;
      ps_dicomFile->ts_Position=ts_slicePosition;
      ps_dicomFile->i16_relativeOrderNumber = i16_memory_io_dicom_relativePosition(&ps_ReferenceFileProps->ts_Position,
                                                                                   &ps_dicomFile->ts_Position,
                                                                                   &ps_serie->t_ScannerSpaceIJKtoXYZ,
                                                                                   &ps_serie->pixel_dimension);

      ps_dicomFile->i16_TemporalPositionIdentifier = i16_TemporalPositionIdentifier;
      ps_dicomFile->i16_StackPositionIdentifier = i16_StackPositionIdentifier;
      ps_dicomFile->e_DCM_CIC = e_DCM_CIC;






      if (ps_dicomFile->i16_relativeOrderNumber < i16_MinimumReferenceOrderValue)
      {
        i16_MinimumReferenceOrderValue = ps_dicomFile->i16_relativeOrderNumber;
      }

      if (ps_dicomFile->i16_relativeOrderNumber > i16_MaximumReferenceOrderValue)
      {
        i16_MaximumReferenceOrderValue = ps_dicomFile->i16_relativeOrderNumber;
      }

      i16_NumberOfSlices++;

      pll_dicomFilesIter = list_append(pll_dicomFilesIter, ps_dicomFile);
    }
    p_dirEntry = readdir (p_dicomDirectory);
  }

  closedir (p_dicomDirectory);




  //Check if patient exists
  patientTreeIterator=tree_nth(*patient_tree,1);
  while (patientTreeIterator != NULL)
  {
    patient=(Patient * )(patientTreeIterator->data);

    if (strcmp(patient->c_patientID,ps_patient->c_patientID) == 0)
    {
      b_PatientExists=1;
      *patient_tree = patientTreeIterator;
      // Patient exists
      break;
    }
    patientTreeIterator=tree_next(patientTreeIterator);
  }

  if (!b_PatientExists)
  {
    *patient_tree = tree_append (*patient_tree, ps_patient, TREE_TYPE_PATIENT);
  }

  studyTreeIterator=tree_nth((*patient_tree)->child,1);
  while (studyTreeIterator != NULL)
  {
    study=(Study * )(studyTreeIterator->data);

    if (strcmp(study->c_studyInstanceUID,ps_study->c_studyInstanceUID) == 0)
    {
      b_StudyExists=1;
      studyTreeIterator = (*patient_tree)->child;
      break;
    }
    studyTreeIterator=tree_next(studyTreeIterator);
  }

  if (!b_StudyExists)
  {
    studyTreeIterator = tree_append_child (*patient_tree, ps_study, TREE_TYPE_STUDY);
  }

  serieTreeIterator=tree_nth(studyTreeIterator->child,1);
  while (serieTreeIterator != NULL)
  {
    serie=(Serie *)(studyTreeIterator->data);

    if (strcmp(serie->c_serieInstanceUID,ps_serie->c_serieInstanceUID) == 0)
    {
      b_SerieExists=1;
      pt_serie=serieTreeIterator;
      break;
    }
    serieTreeIterator=tree_next(serieTreeIterator);
  }

  if (!b_SerieExists)
  {
     pt_serie=tree_append_child (studyTreeIterator, ps_serie, TREE_TYPE_SERIE);
  }
  else
  {
    pll_dicomFiles = pll_dicomFilesIter;
    // clean up all mess
    ts_dicom_FileProperties *ps_dicomFile;
    pll_dicomFilesIter = list_nth(pll_dicomFiles,1);
    while (pll_dicomFilesIter->next != NULL)
    {
      ps_dicomFile=(ts_dicom_FileProperties*)(pll_dicomFilesIter->data);

      free(ps_dicomFile->pc_Filename), ps_dicomFile->pc_Filename = NULL;
      free(ps_dicomFile), ps_dicomFile = NULL;
      pll_dicomFilesIter = list_next(pll_dicomFilesIter);
    }
    list_free(pll_dicomFiles);

    return NULL;
  }

  ps_serie->matrix.i16_z=(ps_serie->matrix.i16_z==0) ? i16_NumberOfSlices/ps_serie->num_time_series : 1;
  i16_NumberOfReconstructions=(short int)(ps_serie->matrix.i16_z)/(i16_MaximumReferenceOrderValue - i16_MinimumReferenceOrderValue + 1);

  if (i16_NumberOfReconstructions <= 1)
  {
    i16_NumberOfReconstructions =1;
    e_DCM_CIC = DCM_CIC_REAL;
  }
  else
  {
    ps_serie->matrix.i16_z= ps_serie->matrix.i16_z/i16_NumberOfReconstructions;
    ps_serie->num_time_series*=i16_NumberOfReconstructions;
  }

  pll_dicomFiles = list_nth(pll_dicomFilesIter,1);

  short int i16_RecoCnt;
  short int b_RecoDoesntMatter=0;
  for (i16_RecoCnt=0; i16_RecoCnt<i16_NumberOfReconstructions; i16_RecoCnt++)
  {
    if (i16_NumberOfReconstructions > 1)
    {
      switch (i16_RecoCnt)
      {
        case 0: e_DCM_CIC = DCM_CIC_MAGNITUDE; break;
        case 1: e_DCM_CIC = DCM_CIC_PHASE; break;
      }
    }
    else
    {
      b_RecoDoesntMatter=1;
    }


    for (i16_timeFrameCnt=1; i16_timeFrameCnt<=ps_serie->num_time_series/i16_NumberOfReconstructions; i16_timeFrameCnt++)
    {
      i16_NumberOfSlices=0;
      for (i16_Cnt=i16_MinimumReferenceOrderValue; i16_Cnt<=i16_MaximumReferenceOrderValue; i16_Cnt++ )
      {
        pll_dicomFilesIter = pll_dicomFiles;
        while (pll_dicomFilesIter != NULL)
        {
          ps_dicomFile=(ts_dicom_FileProperties*)(pll_dicomFilesIter->data);

          if ((ps_dicomFile->i16_relativeOrderNumber == i16_Cnt) &&
              (ps_dicomFile->i16_TemporalPositionIdentifier == i16_timeFrameCnt) &&
              ((ps_dicomFile->e_DCM_CIC == e_DCM_CIC) || b_RecoDoesntMatter))
          {
            i16_memory_io_dicom_loadSingleSlice(ps_serie, ps_dicomFile->pc_Filename, i16_NumberOfSlices, i16_timeFrameCnt-1 + i16_RecoCnt * ps_serie->num_time_series/i16_NumberOfReconstructions);
            i16_NumberOfSlices++;

            if (pll_dicomFilesIter != pll_dicomFiles)
            {
              pll_dicomFilesIter=list_remove(pll_dicomFilesIter);
              break;
            }

          }
          pll_dicomFilesIter = list_next(pll_dicomFilesIter);
        }
      }
    }
  }

  ps_serie->d_Qfac=1;
  ps_serie->i16_QuaternionCode=1; //NIFTI_XFORM_SCANNER_ANAT
  ps_serie->t_ScannerSpaceXYZtoIJK = tda_memory_quaternion_inverse_matrix(&ps_serie->t_ScannerSpaceIJKtoXYZ);

  ps_serie->pt_RotationMatrix = &ps_serie->t_ScannerSpaceIJKtoXYZ;
  ps_serie->pt_InverseMatrix = &ps_serie->t_ScannerSpaceXYZtoIJK;

  ps_serie->i16_StandardSpaceCode=0;
  ps_serie->raw_data_type=MEMORY_TYPE_UINT16;
  ps_serie->u8_AxisUnits=0;

  ps_serie->ps_Quaternion = calloc (1, sizeof (ts_Quaternion));

  td_Matrix4x4 td_Temp;
  td_Temp.d_Matrix[0][0] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][0] * ps_serie->pixel_dimension.x;
  td_Temp.d_Matrix[0][1] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][1] * ps_serie->pixel_dimension.x;
  td_Temp.d_Matrix[0][2] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][2] * ps_serie->pixel_dimension.x;

  td_Temp.d_Matrix[1][0] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][0] * ps_serie->pixel_dimension.y;
  td_Temp.d_Matrix[1][1] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][1] * ps_serie->pixel_dimension.y;
  td_Temp.d_Matrix[1][2] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][2] * ps_serie->pixel_dimension.y;

  td_Temp.d_Matrix[2][0] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][0] * ps_serie->pixel_dimension.z;
  td_Temp.d_Matrix[2][1] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][1] * ps_serie->pixel_dimension.z;
  td_Temp.d_Matrix[2][2] = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][2] * ps_serie->pixel_dimension.z;

  *ps_serie->ps_Quaternion = ts_memory_matrix_to_quaternion(&td_Temp, &ps_serie->d_Qfac);

  ps_serie->ps_QuaternationOffset = calloc (1, sizeof (ts_Quaternion));
  ps_serie->ps_QuaternationOffset->I = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][0];
  ps_serie->ps_QuaternationOffset->J = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][1];
  ps_serie->ps_QuaternationOffset->K = ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][2];


  td_Matrix4x4 ts_Recalculated;

  ts_Recalculated = tda_memory_quaternion_to_matrix(ps_serie->ps_Quaternion,ps_serie->ps_QuaternationOffset,ps_serie->d_Qfac);

  printf("Translation matrix in scannerspace:\n");
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][0], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][0], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][0], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][0]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][1], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][1], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][1], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][1]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][2], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][2], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][2], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][2]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[0][3], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[1][3], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[2][3], ps_serie->t_ScannerSpaceIJKtoXYZ.d_Matrix[3][3]);
  printf("\n");

  printf("Recalculated translation matrix in scannerspace:\n");
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ts_Recalculated.d_Matrix[0][0], ts_Recalculated.d_Matrix[1][0], ts_Recalculated.d_Matrix[2][0], ts_Recalculated.d_Matrix[3][0]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ts_Recalculated.d_Matrix[0][1], ts_Recalculated.d_Matrix[1][1], ts_Recalculated.d_Matrix[2][1], ts_Recalculated.d_Matrix[3][1]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ts_Recalculated.d_Matrix[0][2], ts_Recalculated.d_Matrix[1][2], ts_Recalculated.d_Matrix[2][2], ts_Recalculated.d_Matrix[3][2]);
  printf("%10.2f, %10.2f, %10.2f, %10.2f \n", ts_Recalculated.d_Matrix[0][3], ts_Recalculated.d_Matrix[1][3], ts_Recalculated.d_Matrix[2][3], ts_Recalculated.d_Matrix[3][3]);
  printf("\n");

  // clear everything
  pll_dicomFilesIter = list_nth(pll_dicomFiles,1);
  while (pll_dicomFilesIter->next != NULL)
  {
    ps_dicomFile=(ts_dicom_FileProperties*)(pll_dicomFilesIter->data);

    free(ps_dicomFile->pc_Filename), ps_dicomFile->pc_Filename = NULL;
    free(ps_dicomFile), ps_dicomFile = NULL;
    pll_dicomFilesIter = list_next(pll_dicomFilesIter);
  }
  list_free(pll_dicomFiles);

  memory_serie_set_upper_and_lower_borders_from_data(ps_serie);

  return pt_serie;
}


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
Tree *pt_memory_io_load_file (Tree **ppt_study, char *pc_path)
{
  Tree *pt_patient=NULL
  debug_functions ();

  if (pc_path == NULL)
  {
    return NULL;
  }

  // check wheater it is a niftii
  if (memory_io_niftii_file_type(pc_path) != MUMC_FILETYPE_NOT_KNOWN)
  {
    return pt_memory_io_load_file_nifti(&(*ppt_study),pc_path);
  }
  else
  {
    // maybe a dicom?
    pt_patient=((*ppt_study) == NULL)?NULL:(*ppt_study)->parent;
    return pt_memory_io_load_file_dicom(&pt_patient, pc_path);
  }
  return NULL;
}

short int memory_io_save_file (Serie *serie, const char *path)
{
  debug_functions ();

  char *pc_Path = calloc (1, strlen (path) + 4);
  strcpy (pc_Path, path);

  char* pc_Extension = strrchr (pc_Path, '.');

  switch (serie->input_type)
  {
    case MUMC_FILETYPE_ANALYZE75:
      break;
    case MUMC_FILETYPE_DICOM:
    case MUMC_FILETYPE_NIFTII_SF:
      if (pc_Extension == NULL)
      {
        strcat (pc_Path, ".nii");
      }

      memory_io_niftii_save (serie, pc_Path, NULL);

      break;
    case MUMC_FILETYPE_NIFTII_TF:
      {
        char c_ImageFile[1024];
        char c_HeaderFile[1024];

        strcpy(&c_ImageFile[0],path);
        strcpy(&c_HeaderFile[0],path);

        if (pc_Extension == NULL)
        {
          strcat(&c_ImageFile[0], ".img");
          strcat(&c_HeaderFile[0], ".hdr");
        }

        memory_io_niftii_save(serie,&c_HeaderFile[0],&c_ImageFile[0]);

      }
      break;
    case MUMC_FILETYPE_NOT_KNOWN :
    default:
      {
        free (pc_Path);
        pc_Path = NULL;
        return 1;
      }
  }

  free (pc_Path);
  pc_Path = NULL;
  return 0;
}


