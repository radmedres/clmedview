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

#include "lib-memory-study.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

unsigned long
memory_study_next_id ()
{
  debug_functions ();

  static unsigned long long id = 0;
  id++;
  return id;
}


Study*
memory_study_new (const char *name)
{
  debug_functions ();

  Study *study;
  study = calloc (1, sizeof (Study));
  assert (study != NULL);

  study->id = memory_study_next_id ();

  assert (strlen (name) > 0);
  memcpy (study->name, name, strlen (name));

  return study;
}


void
memory_study_destroy (void *data)
{
  debug_functions ();

  if (data == NULL) return;
  Study *study = (Study *)data;

  free (study);
}
