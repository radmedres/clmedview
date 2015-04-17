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

#ifndef NIFTII_NIFTII_H
#define NIFTII_NIFTII_H

#include <stdio.h>
#include <stdlib.h>

#include "lib/lib-io-nifti/include/nifti1.h"
#include "lib-memory.h"
#include "lib-memory-serie.h"
#include "lib-memory-quaternion.h"


#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

/**
 * Determine the type of the file.
 *
 * @param path The path to the selected file.
 *
 * @return a known file type.
 */
te_ImageIOFiletype memory_io_niftii_file_type (const char *path);

/**
 * Load a niftii file from disk to the selected memory.
 *
 * @param serie  The selected memory to store all needed parameters in
 * @param header Filename/path of the header file
 * @param image  Filename/path of the image file
 *
 * @return 0 or FALSE if function executes wrong, 1 or TRUE if execution is correct
 */
short int memory_io_niftii_load (Serie *serie, const char *header, const char *image);

/**
 * Save a niftii file from memory to disk
 *
 * @param serie  The selected memory to store all needed parameters in
 * @param header Filename/path of the header file
 * @param image  Filename/path of the image file
 *
 * @return 0 or FALSE if function executes wrong, 1 or TRUE if execution is correct
 */
short int memory_io_niftii_save (Serie *serie, const char *pc_File, const char *pc_ImageFile);

#endif//NIFTII_NIFTII_H
