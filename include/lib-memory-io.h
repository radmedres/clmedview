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

#ifndef MEMORY_IO_H
#define MEMORY_IO_H

#include "lib-common-tree.h"
#include "lib-memory.h"
#include "lib-io-nifti.h"

/**
 * @file   include/lib-memory-io.h
 * @brief  The interface to file input/output.
 * @author Roel Janssen
 */


/**
 * @ingroup memory
 * @{
 * 
 *   @defgroup memory_io IO
 *   @{
 *
 * This module provides functions to interact with files. It is currently
 * dependent on the @ref memory_tree "Memory::Tree" module.
 */


/**
 * This function loads a file and extracts information of that file onto a
 * memory tree.
 *
 * @param patient_tree  A pointer to the tree to load the file's content to.
 * @param path          A valid filename.
 *
 * @return 1 on success, 0 on failure.
 */
short int memory_io_load_file (Tree **patient_tree, const char *path);

/**
 * This function saves a serie to a file.
 *
 * @param serie  The serie to save to disk.
 * @param path   A valid filename.
 *
 * @return 1 on success, 0 on failure.
 */
short int memory_io_save_file (Serie *serie, const char *path);


/**
 *   @} 
 * @}
 */


#endif//MEMORY_IO_H
