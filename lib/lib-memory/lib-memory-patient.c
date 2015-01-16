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

#include "lib-memory-patient.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

unsigned long long
memory_patient_next_id ()
{
  debug_functions ();

  static unsigned long long id = 0;
  id++;
  return id;
}

Patient*
memory_patient_new (const char *name)
{
  debug_functions ();

  Patient *patient;
  patient = calloc (1, sizeof (Patient));
  assert (patient != NULL);

  patient->id = memory_patient_next_id ();

  assert (strlen (name) > 0);
  memcpy (patient->name, name, strlen (name));

  return patient;
}


void
memory_patient_destroy (void *data)
{
  debug_functions ();

  if (data == NULL) return;
  Patient *patient = (Patient *)data;

  free (patient);
}
