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

#include "libcommon-history.h"
#include "libcommon-debug.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <assert.h>


void
common_history_destroy_element (void *data)
{
  debug_functions ();

  History *history = (History *)data;
  if (history == NULL) return;

  free (history->data);
  free (history);
}


List *
common_history_save_state (List *list, void *data, unsigned long data_len)
{
  debug_functions ();

  // Add a new entry to the list.
  List *new_entry = calloc (1, sizeof (List));
  assert (new_entry != NULL);

  // Prevent branching from the list. Remove all elements in the list
  // after the current point.
  if (list != NULL)
  {
    List *next = list_next (list);
    while (next != NULL)
    {
      common_history_destroy_element (next->data);
      next = list_next (next);
    }

    new_entry->previous = list;
    list->next = new_entry;
  }

  // Compress the data.
  History *new_data = calloc (1, sizeof (History));
  assert (new_data != NULL);
  
  unsigned long compressed_len = compressBound (data_len);
  void *compressed = calloc (1, compressed_len);
  assert (compressed != NULL);

  if (compress (compressed, &compressed_len, data, data_len) == Z_OK)
  {
    new_data->data = compressed;
    new_data->compressed_len = compressed_len;
    new_data->data_len = data_len;
    new_entry->data = new_data;
  }
  else
  {
    free (compressed);
    compressed = NULL;
    return list;
  }

  return new_entry;
}


List*
common_history_load_state (List *list, HistoryAction te_Action, void **output)
{
  debug_functions ();

  assert (output != NULL);
  if (list == NULL) return NULL;

  switch (te_Action)
  {
    case HISTORY_PREVIOUS:
      list = list_safe_previous (list);
      break;
    case HISTORY_NEXT:
      list = list_safe_next (list);
      break;
    case HISTORY_LAST:
      list = list_last (list);
      break;
    case HISTORY_FIRST:
      list = list_nth (list, 1);
      break;
    default:
      return NULL;
  }

  History *list_data = list->data;
  if (list_data == NULL) return list;

  if (*output == NULL)
    *output = calloc (1, list_data->data_len);
  
  assert (*output != NULL);

  uncompress (*output, &list_data->data_len, list_data->data, list_data->compressed_len);

  return list;
}
