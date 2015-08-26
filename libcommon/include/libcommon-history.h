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

#ifndef COMMON_HISTORY_H
#define COMMON_HISTORY_H

#include "libcommon-list.h"

/**
 * @file include/libcommon-history.h
 * @brief A generic interface for history control.
 * @author Roel Janssen
 */


/**
 * @ingroup common
 * @{
 * 
 *   @defgroup common_history History
 *   @{
 *
 * This module provides a way of making restore points and stepping through
 * them. This effectively delivers the common undo functionality found in 
 * programs.
 *
 * To use this module you need to add zlib support to your build system.
 *
 * @note This module depends on the @ref common_list "Common::List" module.
 */


/**
 * This enumeration lists the possible actions that can be performed regarding
 * this History object.
 */
typedef enum
{
  HISTORY_UNDEFINED,
  HISTORY_PREVIOUS,
  HISTORY_NEXT,
  HISTORY_FIRST,
  HISTORY_LAST
} HistoryAction;


/**
 * This type is used to store a history element.
 */
typedef struct
{
  void *data;
  unsigned long data_len;
  unsigned long compressed_len;
} History;


/**
 * This function properly destroys a history list element.
 * @param data  The History element to clean up.
 */
void common_history_destroy_element (void *data);


/**
 * Using this function, a state can be saved so it can be retrieved later.
 * @note The state becomes the active state immediately.
 * @param list      A pointer to the current history list.
 * @param data      The data that belongs to the state that is 
 *                  active after applying the action.
 * @param data_len  The number of bytes of data.
 *
 * @return A pointer to a list element on success, NULL on failure.
 */
List* common_history_save_state (List *list, void *data, unsigned long data_len);


/**
 * Using this function, a previous or next state can be loaded.
 * @param list       A pointer to the current history linked list.
 * @param te_Action  The action to be performed.
 * @param output     The data that belongs to the state that is 
 *                   active after applying the action.
 *
 * @return A pointer to a list element on success, NULL on failure.
 */
List* common_history_load_state (List *list, HistoryAction te_Action, void **output);


/**
 *   @} 
 * @}
 */


#endif//COMMON_HISTORY_H
