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

#ifndef COMMON_TREE_H
#define COMMON_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file include/lib-common-tree.h
 * @brief A generic tree type.
 * @author Roel Janssen
 */


/**
 * @ingroup common
 * @{
 * 
 *   @defgroup common_tree Tree
 *   @{
 *
 * This module provides a two-dimensional list type (up-down and left-right).
 * It is tailored to provide a general interface to a tree-like structure.
 *
 * The tree type has a few limitations: Each tree element can have
 * one parent and one child only. Each element is a list element
 * with a previous and a next element.
 */


/**
 * The base structure for a tree element.
 */
typedef struct _Tree
{
  /**
   * A single tree can have multiple types of elements. Often each level in
   * the tree has its own type. For this reason, the type can be specified.
   */
  unsigned char type;

  /**
   * A generic pointer to attach type-specific data to a tree element.
   */
  void* data;

  struct _Tree* parent; /*< A pointer to the parent. */
  struct _Tree* child; /*< A pointer to the child. */
  struct _Tree* next; /*< A pointer to the next element. */
  struct _Tree* previous; /*< A pointer to the previous element. */

} Tree;


/**
 * Using this function one can append an item to a tree.
 *
 * @param tree  The tree to append to.
 * @param data  The data of the element to append.
 * @param type  The type of the current level.
 *
 * @return The head of the new tree.
 */
Tree* tree_append (Tree* tree, void* data, unsigned char type);


/**
 * Using this function one can prepend an item to a tree.
 *
 * @param tree  The tree to prepend to.
 * @param data  The data of the element to prepend.
 * @param type  The type of the current level.
 *
 * @return The head of the new tree.
 */
Tree* tree_prepend (Tree* tree, void* data, unsigned char type);


/**
 * Using this function one can remove an item from a tree.
 * @note It doesn't free the data of the element.
 *
 * @param tree  The tree element to remove.
 *
 * @return The tree after the removed element.
 */
Tree* tree_remove (Tree* tree);


/**
 * Using this function one can clean up the memory associated with a tree.
 *
 * @param tree  The tree to clean up.
 */
void tree_free (Tree* tree);


/**
 * Using this function one can clean up the memory associated with a tree.
 * The 'data' field is cleaned up by calling 'callback'
 *
 * @param tree      The list to clean up.
 * @param callback  The callback to clean up the 'data' element.
 */
void tree_free_all (Tree* tree, void (*callback) (void *));


/**
 * Using this function one can get the next element in a tree.
 *
 * @param tree  The tree to get the next element of.
 *
 * @return The next element in the tree.
 */
Tree* tree_next (Tree* tree);


/**
 * Using this function one can get the previous element in a tree.
 *
 * @param tree  The tree to get the next element of.
 *
 * @return The next element in the tree.
 */
Tree* tree_previous (Tree* tree);


/**
 * Using this function one can get the nth element in a tree.
 *
 * @param tree  The tree to get the nth element of.
 * @param nth   The number of the element to get.
 *
 * @return The nth element in the tree, counted from the first
 *         element on the same level of the tree.
 */
Tree* tree_nth (Tree* tree, unsigned int nth);


/**
 * Using this function one can get last element in a tree.
 *
 * @param tree  The tree to get the last element of.
 *
 * @return The last element in the tree.
 */
Tree* tree_last (Tree* tree);

  
/**
 * This function returns the top of the tree.
 * @param tree  The tree to get the top element of.
 *
 * @return The top of the tree.
 */
Tree* tree_top (Tree* tree);


/**
 * This function returns the parent of the given tree element.
 * @param tree The tree to get the parent element of.
 *
 * @return The parent of the given tree element.
 */
Tree* tree_parent (Tree* tree);


/**
 * This function returns the child of the given tree element.
 * @param tree The tree to get the child element of.
 *
 * @return The child of the given tree element.
 */
Tree* tree_child (Tree* tree);


/**
 * This function sets the child of 'parent' to 'child'.
 *
 * @param tree  The tree to append as a child.
 * @param data  The data of the element to append as child.
 * @param type  The type of the child level.
 *
 * @return The pointer of the newly appended child.
 */
  Tree* tree_append_child (Tree* tree, void *data, unsigned char type);


/**
 * This function returns the length of the current level of the tree.
 */
unsigned int tree_length (Tree* tree);
  
/**
 * This function returns the type of the given tree element.
 * @param tree  The tree to get the type of.
 *
 * @return The type of the given element.
 */
unsigned char tree_type (Tree* tree);


/**
 *   @}
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif//COMMON_TREE_H
