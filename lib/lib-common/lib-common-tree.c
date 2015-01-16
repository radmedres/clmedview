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

#include "lib-common-tree.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*----------------------------------------------------------------------------.
 | TS_TREE_APPEND                                                             |
 | This function appends an element to the tree.                              |
 '----------------------------------------------------------------------------*/
Tree*
tree_append (Tree* tree, void* data, unsigned char type)
{
  debug_functions ();

  // Create the first element if the tree is empty.
  if (tree == NULL)
  {
    tree = calloc (1, sizeof (Tree));
    assert (tree != NULL);

    tree->data = data;
    tree->type = type;
  }

  // Go to the last element and add a new one there.
  else
  {
    Tree* last = tree_last (tree);

    last->next = calloc (1, sizeof (Tree));
    assert (last->next != NULL);

    last->next->data = data;
    last->next->previous = last;
    last->next->parent = tree->parent;
    last->next->type = type;
    tree = last->next;
  }

  return tree;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_PREPEND                                                            |
 | This function prepends an element to the tree.                             |
 '----------------------------------------------------------------------------*/
Tree*
tree_prepend (Tree* tree, void* data, unsigned char type)
{
  debug_functions ();

  Tree* new_tree = calloc (1, sizeof (Tree));
  assert (new_tree != NULL);

  tree = tree_nth (tree, 1);

  new_tree->next = tree;
  new_tree->data = data;
  new_tree->parent = tree->parent;
  new_tree->type = type;

  tree->previous = new_tree;

  return new_tree;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_REMOVE                                                             |
 | This function removes an element to the tree.                              |
 '----------------------------------------------------------------------------*/
Tree*
tree_remove (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  Tree* next = NULL;
  if (tree->next != NULL)
    next = tree->next;
  else
    next = tree->previous;
  
  if (tree->next != NULL)
    tree->next->previous = tree->previous;

  if (tree->previous != NULL)
    tree->previous->next = tree->next;
  
  free (tree);
  return next;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_FREE                                                               |
 | This function cleans up a tree.                                            |
 '----------------------------------------------------------------------------*/
void
tree_free (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return;

  // Rewind the tree.
  tree_nth (tree, 1);

  // Clean up.
  while (tree != NULL)
    tree = tree_remove (tree);
}


/*----------------------------------------------------------------------------.
 | TS_TREE_FREE_ALL                                                           |
 | This function frees a tree.                                                |
 '----------------------------------------------------------------------------*/
void
tree_free_all (Tree* tree, void (*callback) (void *))
{
  debug_functions ();

  if (tree == NULL) return;

  // Rewind the tree.
  tree = tree_nth (tree, 1);

  // Clean up.
  while (tree != NULL)
  {
    callback (tree->data);
    tree = tree_remove (tree);
  }
}


/*----------------------------------------------------------------------------.
 | TS_TREE_NEXT                                                               |
 | This function returns the next element in the tree.                        |
 '----------------------------------------------------------------------------*/
Tree*
tree_next (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;
  return tree->next;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_PREVIOUS                                                           |
 | This function returns the previous element in the tree.                    |
 '----------------------------------------------------------------------------*/
Tree*
tree_previous (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;
  return tree->previous;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_NTH                                                                |
 | This function returns the nth element in the tree.                         |
 '----------------------------------------------------------------------------*/
Tree*
tree_nth (Tree* tree, unsigned int nth)
{
  debug_functions ();

  if (tree == NULL || nth < 1) return NULL;

  // Rewind the tree.
  if (tree->parent != NULL)
    tree = tree->parent->child;
  else
  {
    while (tree->previous != NULL)
      tree = tree->previous;
  }

  unsigned int counter;
  for (counter = 1; counter < nth; counter++)
  {
    if (tree->next == NULL) break;
    tree = tree->next;
  }

  return tree;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_LAST                                                               |
 | This function returns the last element in the tree.                        |
 '----------------------------------------------------------------------------*/
Tree*
tree_last (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  while (tree->next != NULL)
    tree = tree->next;

  return tree;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_TOP                                                                |
 | This function returns the top of the tree.                                 |
 '----------------------------------------------------------------------------*/
Tree*
tree_top (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  while (tree->parent != NULL)
    tree = tree->parent;

  return tree;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_PARENT                                                             |
 | This function returns the parent of the current tree element.              |
 '----------------------------------------------------------------------------*/
Tree*
tree_parent (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;
  return tree->parent;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_CHILD                                                              |
 | This function returns the child of the current tree element.               |
 '----------------------------------------------------------------------------*/
Tree*
tree_child (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return NULL;
  return tree->child;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_APPEND_CHILD                                                       |
 | This function appends an element as a child to the tree.                   |
 '----------------------------------------------------------------------------*/
Tree*
tree_append_child (Tree* tree, void *data, unsigned char type)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  Tree *child = tree->child;

  // Create the first element if the tree is empty.
  if (child == NULL)
  {
    child = calloc (1, sizeof (Tree));
    assert (child != NULL);

    child->data = data;
    tree->child = child;
    child->parent = tree;
    child->type = type;
  }

  // Go to the last element and add a new one there.
  else
  {
    Tree* last = child;
    while (last->next != NULL)
      last = last->next;

    last->next = calloc (1, sizeof (Tree));
    assert (last->next != NULL);

    last->next->data = data;
    last->next->previous = last;
    last->next->parent = child->parent;
    last->next->type = type;

    child = last->next;
  }

  return child;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_LENGTH                                                             |
 | This function returns the length of the current tree level.                |
 '----------------------------------------------------------------------------*/
unsigned int
tree_length (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return 0;

  tree = tree_nth (tree, 1);
  
  unsigned int length = 1;
  while (tree->next != NULL)
  {
    length++;
    tree = tree->next;
  }

  return length;
}


/*----------------------------------------------------------------------------.
 | TS_TREE_TYPE                                                               |
 | This function returns the type of the current tree element.                |
 '----------------------------------------------------------------------------*/
unsigned char
tree_type (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return 0;
  return tree->type;
}
