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

#ifndef MEMORY_TREE_H
#define MEMORY_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libcommon-tree.h"
#include "libcommon.h"
#include "libcommon-list.h"

#include "libmemory.h"
#include "libmemory-patient.h"
#include "libmemory-study.h"
#include "libmemory-serie.h"

/**
 * @file   include/lib-memory-tree.h
 * @brief  A memory model based on a generic tree type.
 * @author Roel Janssen
 */

/**
 * @ingroup memory
 * @{
 *
 *   @defgroup memory_tree Tree
 *   @{
 *
 * This module defines an application-dependent implementation of the
 * @ref common_tree "Common::Tree" module. It's application-dependent because the tree
 * structure is defined as follows:
 * - ts_Tree of Patient
 *   - ts_Tree of Study
 *     - ts_Tree of Serie
 *       - ts_List of Slice
 */


/**
 * This enumeration contains all tree types known to memory.
 */
typedef enum
{
  TREE_TYPE_UKNOWN,
  TREE_TYPE_PATIENT,
  TREE_TYPE_STUDY,
  TREE_TYPE_SERIE,
  TREE_TYPE_SERIE_MASK,
  TREE_TYPE_SERIE_OVERLAY,
  TREE_TYPE_SLICE
} MemoryTreeType;


/**
 * This functions sets the active tree to use. It can be obtained later
 * by calling memory_tree_get_active().
 *
 * @param tree  The tree to set as the active tree.
 */
void memory_tree_set_active (Tree* tree);


/**
 * This functions returns the current active tree.
 *
 * @return The current active tree set by memory_tree_set_active().
 */
Tree* memory_tree_get_active ();


/**
 * This function returns the study at the nth position. When a patient
 * tree is given, it will use the patient child tree to search for studies.
 *
 * @param tree  The tree to search in.
 * @param nth   The number of the element to get.
 *
 * @return The nth study.
 */
Tree* memory_tree_get_nth_study (Tree* tree, unsigned int nth);


/**
 * This function returns the study tree element that contains
 * Study data with a specific id.
 *
 * @param tree  The patient tree to search in.
 * @param id    The id to look for.
 *
 * @return A pointer to the study tree element.
 */
Tree* memory_tree_get_study_by_id (Tree* tree, unsigned long long id);


/**
 * This function returns the serie tree element that contains
 * serie data with a specific id.
 *
 * @param tree  The patient tree to search in.
 * @param id    The id to look for.
 *
 * @return A pointer to the study tree element.
 */
Tree*
memory_tree_get_serie_by_id (Tree* tree, unsigned long long id);

/**
 * This function returns the serie at the nth position. When a patient
 * or a study tree is given, it will use the tree's child to search for
 * series.
 *
 * @param tree  The tree to search in.
 * @param nth   The number of the element to get.
 *
 * @return The nth serie.
 */
Tree* memory_tree_get_nth_serie (Tree* tree, unsigned int nth);


/**
 * This function clones a serie and zeros the data so it can be used
 * as a mask layer. Both series will carry the same group_id.
 *
 * @param tree  The serie tree to use for cloning.
 *
 * @return The cloned serie tree.
 */
Tree* memory_tree_add_mask_for_serie (Tree* tree);


/**
 * This function clones a serie and zeros the data so it can be used
 * as an overlay layer. Both series will carry the same group_id.
 *
 * @param tree  The serie tree to use for cloning.
 *
 * @return The cloned serie tree.
 */
Tree* memory_tree_add_overlay_for_serie (Tree* tree);


/**
 * This function tells whether a given serie in the tree has a mask.
 * It is tested by looking for the same group ID.
 *
 * @return 1 when the given serie tree element has a mask, 0 when not.
 */
short int memory_tree_serie_has_mask (Tree *tree);


/**
 * This function destroys an entire memory tree (from the Patient level to the
 * SliceLists).
 *
 * @param tree  The patient tree to clean up.
 */
void memory_tree_destroy (Tree* tree);


/**
 *   @}
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif//MEMORY_TREE_H
