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

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file include/libcommon-list.h
 * @brief A linked-list type that is compatible with the GList interface. It is
 *        by far not as complete as the GList interface, but the functions it
 *        implements are compatible.
 * @author Roel Janssen
 */


/**
 * @ingroup common
 * @{
 * 
 *   @defgroup common_list List
 *   @{
 *
 * A linked list is a commonly used concept, but there's no such facility in 
 * most C standards. Even though an optimized solution exists inside the GLib
 * library, the cost of requiring GLib can be higher than the cost of having
 * our own implementation.
 *
 * This independent linked list type found its home in the "common" tree. It
 * is not a namespaced interface.
 *
 * @note This implementation provides similar functions to the GLib 
 * implementation. It does not provide all functionality of GLib.
 */


/**
 * The base structure for a list element.
 */
typedef struct _List
{
  void* data; /*< A generic data container for the list. */
  struct _List* previous; /*< A pointer to the previous element. */
  struct _List* next; /*< A pointer to the next element. */
} List;


/**
 * Using this function one can append an item to a list.
 *
 * @param list  The list to append to.
 * @param data  The data of the element to append.
 *
 * @return The head of the new list.
 */
List* list_append (List* list, void* data);


/**
 * Using this function one can prepend an item to a list.
 *
 * @param list  The list to prepend to.
 * @param data  The data of the element to prepend.
 *
 * @return The head of the new list.
 */
List* list_prepend (List* list, void* data);


/**
 * Using this function one can remove an item from a list.
 *
 * @param list  The list element to remove.
 *
 * @return The list after the removed element.
 */
List* list_remove (List* list);


/**
 * Using this function one can clean up the memory associated with a list.
 *
 * @param list  The list to clean up.
 */
void list_free (List* list);


/**
 * Using this function one can clean up the memory associated with a list.
 * The 'data' field is cleaned up by calling 'callback'
 *
 * @param list      The list to clean up.
 * @param callback  The callback to clean up the 'data' element.
 */
void list_free_all (List* list, void (*callback) (void *));


/**
 * Using this function one can get the number of items in a list.
 *
 * @param list  The list to count the elements of.
 *
 * @return The number of elements in the list.
 */
int list_length (List* list);


/**
 * Using this function one can get the next element in a list.
 *
 * @param list  The list to get the next element of.
 *
 * @return The next element in the list or NULL when there is
 *         no next element.
 */
List* list_next (List* list);


/**
 * Using this function one can get the next element in a list.
 * The difference with list_next() is that this function 
 * won't return the next element if the next element is NULL.
 * Instead it will return the current element.
 *
 * @param list  The list to get the next element of.
 *
 * @return The next element in the list or the current element 
 *         when there is no next element.
 */
List* list_safe_next (List* list);


/**
 * Using this function one can get the previous element in a list.
 *
 * @param list  The list to get the previous element of.
 *
 * @return The previous element in the list or NULL when there is
 *         no previous element.
 */
List* list_previous (List* list);


/**
 * Using this function one can get the previous element in a list.
 * The difference with list_previous() is that this function 
 * won't return the previous element if the previous element is NULL.
 * Instead it will return the current element.
 *
 * @param list  The list to get the previous element of.
 *
 * @return The previous element in the list or the current element
 *         when there is no previous element.
 */
List* list_safe_previous (List* list);


/**
 * This function returns the nth element in the list.
 *
 * @param list  The list to get the nth element of.
 * @param nth   The number of the element to get.
 *
 * @return The nth element in the list.
 */
List* list_nth (List* list, unsigned int nth);


/**
 * Using this function one can get last element in a list.
 *
 * @param list  The list to get the last element of.
 *
 * @return The last element in the list.
 */
List* list_last (List* list);


/**
 *   @} 
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//COMMON_LIST_H
