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

  void *pv_LibHandle;
  void (*fp_GetMetaData)(char **, unsigned char **, int*, PluginType*);
  char *pc_LoadError;

  char *pc_PluginName;
  unsigned char *pc_PluginIcon;
  int i32_PluginVersion;
  PluginType te_PluginType;
  
  pv_LibHandle = dlopen (pc_Filename, RTLD_LAZY);
  if (!pv_LibHandle)
  {
    fprintf (stderr, "%s\n", dlerror());
  }

  fp_GetMetaData = dlsym (pv_LibHandle, "CLM_Plugin_Metadata");
  if ((pc_LoadError = dlerror()) != NULL)
  {
    fprintf (stderr, "%s\n", pc_LoadError);
    return NULL;
  }

  Plugin *plugin = calloc (1, sizeof (Plugin));
  if (plugin == NULL)
  {
    dlclose (pv_LibHandle);
    return NULL;
  }
  
  fp_GetMetaData (&pc_PluginName, &pc_PluginIcon, &i32_PluginVersion, &te_PluginType);
  if (te_PluginType != PLUGIN_TYPE_BRUSH
      && te_PluginType != PLUGIN_TYPE_SELECTION
      && te_PluginType != PLUGIN_TYPE_LINE)
  {
    free (plugin);
    dlclose (pv_LibHandle);
    return NULL;
  }

  plugin->te_Type = te_PluginType;
  plugin->pc_Name = pc_PluginName;
  plugin->pc_Icon = pc_PluginIcon;
  plugin->i32_Size = 1;
  plugin->i32_Value = 1;

  pll_PluginList = list_prepend (pll_PluginList, plugin);

  char *pc_CallbackSymbolName = NULL;
  switch (te_PluginType)
  {
    case PLUGIN_TYPE_BRUSH:
      pc_CallbackSymbolName = "CLM_Plugin_Brush_Apply";
    break;
    case PLUGIN_TYPE_SELECTION:
      pc_CallbackSymbolName = "CLM_Plugin_Selection_Apply";
    break;
    case PLUGIN_TYPE_LINE:
      pc_CallbackSymbolName = "CLM_Plugin_Line_Apply";
    break;
    default:
    break;
  }

  plugin->fp_Callback = dlsym (pv_LibHandle, pc_CallbackSymbolName);
  if ((pc_LoadError = dlerror()) != NULL)
  {
    fprintf (stderr, "%s\n", pc_LoadError);
  }
  
  plugin->v_LibraryHandler = pv_LibHandle;

  return pll_PluginList;
}


void
pixeldata_plugin_destroy (void *data)
{
  debug_functions ();

  Plugin *plugin = data;
  dlclose (plugin->v_LibraryHandler);

  free (plugin->pc_Name);
  plugin->pc_Name = NULL;

  free (plugin->pc_Icon);
  plugin->pc_Icon = NULL;

  plugin->fp_Callback = NULL;
  plugin->v_LibraryHandler = NULL;

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

      #ifdef WIN32
      if (strcmp (extension, ".dll")) continue;
      #else
      if (strcmp (extension, ".so")) continue;
      #endif

      // Load the plug-in.
      char *full_path = calloc (1, strlen (path) + strlen (entry->d_name) + 2);

      #ifdef WIN32
      sprintf (full_path, "%s\\%s", path, entry->d_name);
      #else
      sprintf (full_path, "%s/%s", path, entry->d_name);
      #endif

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

