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


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
short int
memory_io_load_file (Tree **patient_tree, const char *path)
{
  debug_functions ();

  if (patient_tree == NULL || path == NULL) return 0;

  const char *filename = strrchr (path, PATH_SEPARATOR);

  if (filename == NULL) filename = path;

  Patient *patient = NULL;
  Study *study = NULL;
  Serie *serie = NULL;

  switch (memory_io_niftii_file_type ((char *)path))
  {
    case MUMC_FILETYPE_NIFTII_SF:
      {
        patient = (filename[0] == PATH_SEPARATOR)
          ? memory_patient_new (filename + 1)
          : memory_patient_new (filename);

        *patient_tree = tree_append (*patient_tree, patient, TREE_TYPE_PATIENT);

        study = (filename[0] == PATH_SEPARATOR)
          ? memory_study_new (filename + 1)
          : memory_study_new (filename);

        Tree *study_tree = tree_append_child (*patient_tree, study, TREE_TYPE_STUDY);

        serie = (filename[0] == PATH_SEPARATOR)
          ? memory_serie_new (filename + 1, path )
          : memory_serie_new (filename, path );

        tree_append_child (study_tree, serie, TREE_TYPE_SERIE);

        if (!memory_io_niftii_load (serie, path, NULL)) return 0;
      }
      break;
    case MUMC_FILETYPE_NIFTII_TF:
      {
        //Check weather the hdr or img file is passed
        char* pc_Extension = strrchr (path, '.');

        char c_ImageFile[1024];
        char c_HeaderFile[1024];

        strcpy(&c_ImageFile[0],path);
        strcpy(&c_HeaderFile[0],path);

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

        printf("Header: %s\n",c_HeaderFile);
        printf("Image: %s\n",c_ImageFile);

        Patient *patient = memory_patient_new (filename);
        *patient_tree = tree_append (*patient_tree, patient, TREE_TYPE_PATIENT);

        Study *study = memory_study_new (filename);
        Tree *study_tree = tree_append_child (*patient_tree, study, TREE_TYPE_STUDY);

        Serie *serie = memory_serie_new (filename,filename);
        tree_append_child (study_tree, serie, TREE_TYPE_SERIE);

        if (!memory_io_niftii_load (serie, &c_HeaderFile[0], &c_ImageFile[0])) return 0;
      }
      break;
    //case MUMC_FILETYPE_ANALYZE75:
    //case MUMC_FILETYPE_DICOM:
    //case MUMC_FILETYPE_NIFTII_TF:
    //case MUMC_FILETYPE_NOT_KNOWN:
    default:
      return 0;
  }

  return 1;
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
      break;
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


