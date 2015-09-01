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

#include <stdlib.h>
#include <getopt.h>

#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif

#include "libconfiguration.h"
#include "gui/mainwindow.h"

// VERSION should be provided by the build system, otherwise define it here.
#ifndef VERSION
#define VERSION 0.1
#endif

static void
show_help ()
{
  puts ("\nAvailable options:\n"
        " --file, -f          A valid path to a niftii file.\n"
        " --version, -v       Show versioning information.\n"
        " --help, -h          Show this message.\n");
}

static void
sanity_check ()
{
  // Some files are required on startup.
  #ifdef WIN32
  #define LUTS_PATH(file) "luts\\" file
  #else
  #define LUTS_PATH(file) "luts/" file
  #endif

  if (access (LUTS_PATH ("default-grey.lut"), R_OK))
  {
    puts ("default-grey.lut is missing");
    exit (1);
  }

  if (access (LUTS_PATH ("default-mask.lut"), R_OK))
  {
    puts ("default-mask.lut is missing");
    exit (1);
  }

  if (access (LUTS_PATH ("default-hot256.lut"), R_OK))
  {
    puts ("default-hot256.lut is missing");
    exit (1);
  }
}

int
main (int argc, char** argv)
{
  #ifdef ENABLE_MTRACE
  mtrace ();
  #endif

  sanity_check ();
  configuration_init ();

  char* file_path = NULL;
  int arg = 0;
  int index = 0;
  char start_gui = 1;

  /*----------------------------------------------------------------------.
   | OPTIONS                                                              |
   | An array of structs that list all possible arguments that can be     |
   | provided by the user.                                                |
   '----------------------------------------------------------------------*/
  static struct option options[] =
  {
    { "file",              required_argument, 0, 'f' },
    { "help",              no_argument,       0, 'h' },
    { "version",           no_argument,       0, 'v' },
    { 0,                   0,                 0, 0 }
  };

  while (arg != -1)
  {
    // Make sure to list all short options in the string below.
    arg = getopt_long (argc, argv, "f:vh", options, &index);
    switch (arg)
    {
    case 'f':
      file_path = optarg;
      break;
    case 'v':
      printf ("Version: %s\n", VERSION);
      start_gui = 0;
      break;
    case 'h':
      show_help ();
      start_gui = 0;
      break;
    }
  }

  if (start_gui)
    gui_mainwindow_new (file_path);

  #ifdef ENABLE_MTRACE
  muntrace ();
  #endif
}
