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

#include "libcommon-list.h"
#include "libcommon-debug.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*----------------------------------------------------------------------------.
 | TS_LIST_APPEND                                                             |
 | This function appends an element to the list.                              |
 '----------------------------------------------------------------------------*/
List*
list_append (List* list, void* data)
{
  debug_functions ();

  /* Create the first element if the list is empty. */
  if (list == NULL)
  {
    list = calloc (1, sizeof (List));
    assert (list != NULL);
    
    list->data = data;
  }

  /* Traverse to the last element and add a new one to it. */
  else
  {
    List* last = list_last (list);
    List *next = calloc (1, sizeof (List));
    assert (next != NULL);

    next->previous = last;
    next->data = data;
    last->next = next;
    list = next;
  }

  return list;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_PREPEND                                                            |
 | This function prepends an element to the list.                             |
 '----------------------------------------------------------------------------*/
List*
list_prepend (List* list, void* data)
{
  debug_functions ();

  List* new_list = calloc (1, sizeof (List));
  assert (new_list != NULL);

  list = list_nth (list, 1);

  new_list->next = list;
  new_list->data = data;

  if (list != NULL)
    list->previous = new_list;
  
  return new_list;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_REMOVE                                                             |
 | This function removes an element to the list.                              |
 '----------------------------------------------------------------------------*/
List*
list_remove (List* list)
{
  debug_functions ();

  if (list == NULL) return NULL;

  List *next = list->next;
  List *prev = list->previous;

  free (list), list = NULL;

  if (next != NULL) next->previous = prev;
  if (prev != NULL) prev->next = next;

  return (next != NULL) ? next : prev;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_FREE                                                               |
 | This function frees a list.                                                |
 '----------------------------------------------------------------------------*/
void
list_free (List* list)
{
  debug_functions ();

  if (list == NULL) return;

  // Rewind the list.
  list = list_nth (list, 1);

  // Clean up.
  while (list != NULL)
    list = list_remove (list);

}


/*----------------------------------------------------------------------------.
 | TS_LIST_FREE_ALL                                                           |
 | This function frees a list.                                                |
 '----------------------------------------------------------------------------*/
void
list_free_all (List* list, void (*callback) (void *))
{
  debug_functions ();

  if (list == NULL) return;

  // Rewind the list.
  list = list_nth (list, 1);

  // Clean up.
  while (list != NULL)
  {
    if (callback != NULL) callback (list->data);
    list = list_remove (list);
  }
}


/*----------------------------------------------------------------------------.
 | TS_LIST_LENGTH                                                             |
 | This function returns the length of a list.                                |
 '----------------------------------------------------------------------------*/
int
list_length (List* list)
{
  debug_functions ();

  // Return zero when the list is unallocated.
  if (list == NULL) return 0;

  list = list_nth (list, 1);
  
  int length = 1;

  while (list->next != NULL)
  {
    length++;
    list = list->next;
  }

  return length;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_NEXT                                                               |
 | This function returns the next element in the list.                        |
 '----------------------------------------------------------------------------*/
List*
list_next (List* list)
{
  debug_functions ();

  if (list == NULL) return NULL;
  return list->next;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_SAFE_NEXT                                                          |
 | This function returns the next safe element in the list.                   |
 '----------------------------------------------------------------------------*/
List*
list_safe_next (List* list)
{
  debug_functions ();

  if (list == NULL)
    return NULL;

  else
    return (list->next != NULL)
      ? list->next
      : list;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_PREVIOUS                                                           |
 | This function returns the previous element in the list.                    |
 '----------------------------------------------------------------------------*/
List*
list_previous (List* list)
{
  debug_functions ();

  if (list == NULL) return NULL;
  return list->previous;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_SAFE_PREVIOUS                                                      |
 | This function returns the previous safe element in the list.               |
 '----------------------------------------------------------------------------*/
List*
list_safe_previous (List* list)
{
  debug_functions ();

  if (list == NULL)
    return NULL;

  else
    return (list->previous != NULL)
      ? list->previous
      : list;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_NTH                                                                |
 | This function returns the nth element in the list.                         |
 '----------------------------------------------------------------------------*/
List*
list_nth (List* list, unsigned int nth)
{
  debug_functions ();

  if (list == NULL || nth < 1) return NULL;

  // Rewind the list.
  while (list->previous != NULL)
    list = list->previous;

  unsigned int counter;
  for (counter = 1; counter < nth; counter++)
  {
    if (list->next == NULL) break;
    list = list->next;
  }

  return list;
}


/*----------------------------------------------------------------------------.
 | TS_LIST_LAST                                                               |
 | This function returns the last element in the list.                        |
 '----------------------------------------------------------------------------*/
List*
list_last (List* list)
{
  debug_functions ();

  if (list == NULL) return NULL;

  while (list->next != NULL)
    list = list->next;

  return list;
}
