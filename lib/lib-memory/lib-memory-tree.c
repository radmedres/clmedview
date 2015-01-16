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

#include "lib-memory-tree.h"
#include "lib-memory-serie.h"
#include "lib-memory-slice.h"
#include "lib-common-debug.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


static Tree* ts_ActiveTree;


void
memory_tree_set_active (Tree* tree)
{
  debug_functions ();

  ts_ActiveTree = tree;
}


Tree*
memory_tree_get_nth_study (Tree* tree, unsigned int nth)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  if (tree->type == TREE_TYPE_PATIENT)
    tree = tree->child;

  else if (tree->type != TREE_TYPE_STUDY)
    return NULL;

  return tree_nth (tree, nth);
}


Tree*
memory_tree_get_study_by_id (Tree* tree, unsigned long long id)
{
  debug_functions ();

  if (tree == NULL) return NULL;
  if (tree->type != TREE_TYPE_PATIENT) return NULL;

  Tree *root = tree_nth (tree, 1);
  while (root != NULL)
  {
    Tree *studies = tree_child (root);
    while (studies != NULL)
    {
      Study *study = studies->data;
      if (study == NULL)
      {
        studies = tree_next (studies);
        continue;
      }

      if (study->id == id)
        return studies;

      studies = tree_next (studies);
    }

    root = tree_next (root);
  }

  return 0;
}


Tree*
memory_tree_get_nth_serie (Tree* tree, unsigned int nth)
{
  debug_functions ();

  if (tree == NULL) return NULL;

  if (tree->type == TREE_TYPE_PATIENT)
    tree = tree->child;

  else if (tree->type == TREE_TYPE_STUDY)
  {
    tree = tree->child;
    if (tree == NULL) return NULL;
    tree = tree->child;
  }

  else if (tree->type != TREE_TYPE_SERIE
           && tree->type != TREE_TYPE_SERIE_MASK
           && tree->type != TREE_TYPE_SERIE_OVERLAY)
    return NULL;

  return tree_nth (tree, nth);
}


short int
memory_tree_serie_has_mask (Tree *tree)
{
  assert (tree != NULL);

  Serie *current_serie = tree->data;
  assert (current_serie != NULL);

  unsigned long long group_id = current_serie->group_id;

  tree = tree_nth (tree, 1);
  while (tree != NULL)
  {
    Serie *serie = tree->data;
    if (serie != current_serie && group_id == serie->group_id) return 1;

    tree = tree_next (tree);
  }

  return 0;
}

Tree*
memory_tree_add_mask_for_serie (Tree* tree)
{
  debug_functions ();

  if (tree == NULL || tree->type != TREE_TYPE_SERIE) return NULL;

  Serie* serie = tree->data;
  assert (serie != NULL);

  Serie* mask = memory_serie_create_mask_from_serie (serie);
  assert (mask != NULL);

  tree = tree_append (tree, mask, TREE_TYPE_SERIE_MASK);

  return tree;
}


Tree*
memory_tree_add_overlay_for_serie (Tree* tree)
{
  debug_functions ();

  if (tree == NULL || tree->type != TREE_TYPE_SERIE) return NULL;

  Serie* serie = tree->data;
  assert (serie != NULL);

  Serie* overlay = memory_serie_create_mask_from_serie (serie);
  assert (overlay != NULL);

  tree = tree_append (tree, overlay, TREE_TYPE_SERIE_OVERLAY);

  return tree;
}


void
memory_tree_destroy (Tree* tree)
{
  debug_functions ();

  if (tree == NULL) return;

  // Make sure we are destroying from the top tree.
  while (tree->type != TREE_TYPE_PATIENT)
    tree = tree_parent (tree);

  Tree *pll_Patients = tree;
  assert (pll_Patients->type == TREE_TYPE_PATIENT);

  while (pll_Patients != NULL)
  {
    Tree *pll_Studies = tree_child (pll_Patients);
    assert (pll_Studies->type == TREE_TYPE_STUDY);

    while (pll_Studies != NULL)
    {
      Tree *pll_Series = tree_child (pll_Studies);
      assert (pll_Series->type == TREE_TYPE_SERIE
              || pll_Series->type == TREE_TYPE_SERIE_MASK
              || pll_Series->type == TREE_TYPE_SERIE_OVERLAY);

      tree_free_all (pll_Series, memory_serie_destroy);
      pll_Series = NULL;

      free (pll_Studies->data);
      pll_Studies = tree_remove (pll_Studies);
    }

    free (pll_Patients->data);
    pll_Patients = tree_remove (pll_Patients);
  }
}
