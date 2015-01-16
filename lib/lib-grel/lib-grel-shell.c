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

#include "lib-grel-shell.h"

#include "lib-common-debug.h"
#include "lib-common-unused.h"
#include "lib-configuration.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define SOCKET_NAME "clmedview.socket"


// A static GREL structure for use by the GREL shell.
GRELShell grel_config;


static SCM
grel_shell_send_command (const char *function, const char *params)
{
  debug_functions ();
  assert (function != NULL);
  assert (params != NULL);

  SCM state = SCM_BOOL_T;

  int connection = socket (AF_LOCAL, SOCK_STREAM, 0);
  if (connection == -1)
  {
    debug_error ("Couldn't set up client socket: %s", strerror (errno));

    state = SCM_BOOL_F;
    goto cleanup;
  }

  struct sockaddr_un address;
  address.sun_family = AF_LOCAL;
  strcpy (address.sun_path, SOCKET_NAME);

  int length = strlen (address.sun_path) + sizeof (address.sun_family);
  if (connect (connection, (struct sockaddr *)&address, length) == -1)
  {
    debug_error ("Couldn't connect on '%s': %s", SOCKET_NAME, strerror (errno));

    state = SCM_BOOL_F;
    goto cleanup;
  }

  char *message = calloc (1, strlen (function) + strlen (params) + 2);
  assert (message != NULL);

  sprintf (message, "%s:%s", function, params);
  if (send (connection, message, strlen (message), 0) == -1)
  {
    debug_error ("Couldn't send data to '%s': %s", SOCKET_NAME, strerror (errno));

    state = SCM_BOOL_F;
    goto cleanup;
  }

 cleanup:
  close (connection);

  return state;
}


static SCM
grel_shell_file_load (SCM filename)
{
  SCM state = SCM_BOOL_F;

  char *path = scm_to_locale_string (filename);
  if (path != NULL)
  {
    state = grel_shell_send_command ("file-load", path);

    free (path);
    path = NULL;
  }

  return state;
}


static SCM
grel_shell_overlay_load (SCM filename)
{
  SCM state = SCM_BOOL_F;

  char *path = scm_to_locale_string (filename);
  if (path != NULL)
  {
    state = grel_shell_send_command ("overlay-load", path);

    free (path);
    path = NULL;
  }

  return state;
}


/*
static SCM
grel_shell_plot_in_time (UNUSED SCM scm_x, UNUSED SCM scm_y)
{
  //double x = scm_to_double (scm_x);
  //double y = scm_to_double (scm_y);

  return SCM_BOOL_T;
}
*/


static SCM
grel_shell_file_save (SCM layer, SCM location)
{
  SCM state = SCM_BOOL_T;

  char *layer_str = scm_to_locale_string (layer);

  char *path = NULL;
  path = (location == SCM_UNDEFINED)
    ? scm_to_locale_string (layer)
    : scm_to_locale_string (location);
  
  if (path != NULL)
  {
    if (location == SCM_UNDEFINED)
    {
      state = grel_shell_send_command ("file-save", path);
    }
    else
    {
      char *total = calloc (1, strlen (layer_str) + strlen (path) + 2);
      assert (total != NULL);
      sprintf (total, "%s;%s", layer_str, path);

      state = grel_shell_send_command ("file-save", total);

      free (total);
      total = NULL;
    }
 
    free (layer_str);
    layer_str = NULL;

    free (path);
    path = NULL;
  }

  return state;
}


static SCM
grel_shell_override_key (SCM scm_name, SCM scm_key)
{
  SCM state = SCM_BOOL_T;

  char *name = scm_to_locale_string (scm_name);
  char *key = scm_to_locale_string (scm_key);

  char *total_command = calloc (1, strlen (name) + 3);
  assert (total_command != NULL);

  sprintf (total_command, "%s;%c", name, key[0]);
  state = grel_shell_send_command ("override-key", total_command);

  free (total_command);
  total_command = NULL;

  free (name);
  name = NULL;

  free (key);
  key = NULL;

  return state;
}


/*
static SCM
grel_shell_get_voxel_value (SCM scm_x, SCM scm_y)
{
  int x = scm_to_int (scm_x);
  int y = scm_to_int (scm_y);

  return scm_from_int (x * y);
}


static SCM
grel_shell_set_voxel_value (SCM scm_layer, SCM scm_x, SCM scm_y, SCM scm_value)
{
  SCM state = SCM_BOOL_F;

  char *layer = scm_to_locale_string (scm_layer);
  int x = scm_to_int (scm_x);
  int y = scm_to_int (scm_y);
  int value = scm_to_int (scm_value);

  return state;
}

*/


/*
static SCM
grel_shell_get_active_mask ()
{
  Configuration *config = configuration_get_default ();

  Tree *active_mask = CONFIGURATION_ACTIVE_MASK (config);
  if (active_mask == NULL)
  {
    debug_error ("No active mask set.");
    return SCM_BOOL_F;
  }

  Serie *active_mask_serie = active_mask->data;
  if (active_mask_serie == NULL)
  {
    debug_error ("No active mask Serie set.");
    return SCM_BOOL_F;
  }

  char *active_mask_name = active_mask_serie->name;
  if (active_mask_name == NULL)
  {
    debug_error ("Active mask name is empty.");
    return SCM_BOOL_F;
  }

  return scm_from_utf8_string (active_mask_name);
}
*/


static SCM
grel_shell_set_voxel_value (SCM scm_x, SCM scm_y, SCM scm_z, SCM scm_value)
{
  SCM state = SCM_BOOL_F;

  int x = scm_to_int (scm_x);
  int y = scm_to_int (scm_y);
  int z = scm_to_int (scm_z);
  int value = scm_to_int (scm_value);

  char *parameters = calloc (1, 255);
  assert (parameters != NULL);

  sprintf (parameters, "%d;%d;%d;%d", x, y, z, value);
  state = grel_shell_send_command ("set-voxel-value", parameters);

  free (parameters);
  parameters = NULL;

  return state;
}

static SCM
grel_shell_set_viewport (SCM scm_viewport)
{
  SCM state = SCM_BOOL_F;
  char *viewport = scm_to_locale_string (scm_viewport);

  short int viewport_type = 0;

  if (!strcmp (viewport, "axial"))
    viewport_type = 0;

  else if (!strcmp (viewport, "sagital"))
    viewport_type = 1;

  else if (!strcmp (viewport, "coronal"))
    viewport_type = 2;

  else if (!strcmp (viewport, "split"))
    viewport_type = 3;

  char viewport_type_str[5];
  memset (viewport_type_str, 0, 5);
  sprintf (viewport_type_str, "%d", viewport_type);
  state = grel_shell_send_command ("set-viewport", viewport_type_str);

  free (viewport);
  viewport = NULL;

  return state;
}


static SCM
grel_shell_quit (void)
{
  return grel_shell_send_command ("quit", "");
}


void
grel_shell_prepare ()
{
  scm_c_define_gsubr ("grel-file-load", 1, 0, 0, grel_shell_file_load);
  scm_c_define_gsubr ("grel-overlay-load", 1, 0, 0, grel_shell_overlay_load);
  scm_c_define_gsubr ("grel-file-save", 1, 1, 0, grel_shell_file_save);
  scm_c_define_gsubr ("grel-override-key", 2, 0, 0, grel_shell_override_key);

  //scm_c_define_gsubr ("grel-get-active-mask", 0, 0, 0, grel_shell_get_active_mask);
  //scm_c_define_gsubr ("grel-set-active-mask", 1, 0, 0, grel_shell_set_active_mask);

  //scm_c_define_gsubr ("grel-get-voxel-value", 2, 0, 0, grel_shell_get_voxel_value);
  scm_c_define_gsubr ("grel-set-voxel-value", 4, 0, 0, grel_shell_set_voxel_value);
  scm_c_define_gsubr ("grel-set-viewport", 1, 0, 0, grel_shell_set_viewport);
  
  scm_c_define_gsubr ("grel-quit", 0, 0, 0, grel_shell_quit);
  
  //scm_c_define_gsubr ("grel-active-mask", 0, 0, 0, grel_shell_get_active_mask);
}


void
grel_shell_boot (UNUSED void *closure, int argc, char **argv)
{
  grel_shell_prepare ();
  scm_shell (argc, argv);
}


void
grel_shell_start ()
{
  char *arg1 = "--no-auto-compile";

  if (!access ("grel-scripts/init.scm", R_OK))
  {
    char *arg2 = "-l";
    char *arg3 = "grel-scripts/init.scm";
    char *argv[3] = { arg1, arg2, arg3 };

    scm_boot_guile (3, argv, grel_shell_boot, 0);
  }
  else
  {
    char *argv[1] = { arg1 };
    scm_boot_guile (1, argv, grel_shell_boot, 0);
  }
}
