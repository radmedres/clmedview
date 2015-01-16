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

#include "lib-common-debug.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

void
common_debug (DebugMode mode, const char *format, ...)
{
  char tag[14];
  memset (tag, 0, 14);

  switch (mode)
  {
    case DEBUG_MODE_FUNCTIONS: strcpy (tag, "[ FUNCTION ]"); break;
    case DEBUG_MODE_EVENTS:    strcpy (tag, "[ EVENT ]   "); break;
    case DEBUG_MODE_EXTRA:     strcpy (tag, "[ EXTRA ]   "); break;
    case DEBUG_MODE_WARNING:   strcpy (tag, "[ WARNING ] "); break;
    case DEBUG_MODE_ERROR:     strcpy (tag, "[ ERROR ]   "); break;
    default:                   strcpy (tag, "[ UNKNOWN ] "); break;
  }

  va_list args;
  va_start (args, format);

  char message[DEBUG_MAX_MESSAGE_LENGTH];
  memset (message, 0, DEBUG_MAX_MESSAGE_LENGTH);

  time_t rawtime = time (NULL);
  char time_str[9];
  memset (time_str, 0, 9);

  strftime (time_str, 9, "%H:%M:%S", localtime (&rawtime));

  if (vsnprintf (message, DEBUG_MAX_MESSAGE_LENGTH, format, args) > 0)
  {
    (mode == DEBUG_MODE_ERROR)
      ? fprintf (stderr, "%s %s :: %s\n", tag, time_str, message)
      : fprintf (stdout, "%s %s :: %s\n", tag, time_str, message);
  }

  
  va_end (args);
}
