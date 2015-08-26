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

#include "libpixeldata-plugin.h"
#include "libcommon-debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>

List*
pixeldata_plugin_load_file (List *pll_PluginList, const char *pc_Filename)
{
  debug_functions ();

  Plugin *plugin = calloc (1, sizeof (Plugin));
  if (plugin == NULL) return NULL;

  char *error_message = NULL;
  
  plugin->handle = dlopen (pc_Filename, RTLD_LAZY);
  if (!plugin->handle) goto preload_error;

  plugin->get_metadata = dlsym (plugin->handle, "plugin_get_metadata");
  if ((error_message = dlerror()) != NULL) goto load_error;

  plugin->apply = dlsym (plugin->handle, "plugin_apply");
  if ((error_message = dlerror()) != NULL) goto load_error;

  plugin->set_property = dlsym (plugin->handle, "plugin_set_property");
  if ((error_message = dlerror()) != NULL) goto load_error;

  plugin->get_property = dlsym (plugin->handle, "plugin_get_property");
  if ((error_message = dlerror()) != NULL) goto load_error;

  plugin->destroy = dlsym (plugin->handle, "plugin_destroy");
  if ((error_message = dlerror()) != NULL) goto load_error;

  plugin->get_metadata (&(plugin->meta));
  if (plugin->meta == NULL)
  {
    dlclose (plugin->handle);
    free (plugin);
    return NULL;
  }

  pll_PluginList = list_prepend (pll_PluginList, plugin);
  return pll_PluginList;

 preload_error:
  error_message = dlerror();
 load_error:
  debug_error ("%s", error_message);
  return NULL;
}


void
pixeldata_plugin_destroy (void *data)
{
  debug_functions ();

  Plugin *plugin = data;
  plugin->destroy (plugin->meta);
  dlclose (plugin->handle);
  free (plugin);
}


short int
pixeldata_plugin_load_from_directory (const char *path, List **ppll_PluginList)
{
  debug_functions ();

  assert (ppll_PluginList != NULL);

  if (path == NULL) return 0;
  
  DIR* directory;
  if ((directory = opendir (path)) == NULL) return 0;

  struct dirent* entry;
  while ((entry = readdir (directory)) != NULL)
  {
    // Don't process files starting with a dot, '.' and '..'.
    if (entry->d_name[0] == '.') continue;

    DIR* test = opendir (entry->d_name);
    if (test != NULL)
    {
      closedir (test);
      continue;
    }
    else
    {
      // Only process .so or .dll files. Anything else cannot be a plug-in.
      const char *extension = strrchr (entry->d_name, '.');
      if (extension == NULL) continue;
      
      #ifdef WIN32
      if (strcmp (extension, ".dll")) continue;
      #else
      if (strcmp (extension, ".so")) continue;
      #endif

      // Load the plug-in.
      char *full_path = calloc (1, strlen (path) + strlen (entry->d_name) + 2);
      sprintf (full_path, "%s%s", path, entry->d_name);

      *ppll_PluginList = pixeldata_plugin_load_file (*ppll_PluginList, full_path);
      free (full_path), full_path = NULL;

      // When loading the file fails, move on to the next file.
      if (*ppll_PluginList == NULL) continue;

      // TODO: Sort the plugins..
    }
  }

  closedir (directory);

  return 1;
}

