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

#include "libconfiguration.h"

#include <string.h>

Configuration*
configuration_get_default ()
{
  return &configuration_state;
}

void
configuration_init ()
{
  // Reset all options.
  memset (&configuration_state, 0, sizeof (Configuration));

  // Set the default key bindings.
  memcpy (configuration_state.c_key_bindings, "fazy1234gsrptv", 14);
}
