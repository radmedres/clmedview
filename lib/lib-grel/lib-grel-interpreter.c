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

#include "lib-grel-interpreter.h"

#include "lib-common-debug.h"
#include "lib-common-unused.h"

#include <unistd.h>
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

#include <pthread.h>

#define MAX_CONNECTIONS 10000
#define MAX_BUFFER_LEN  512

pthread_mutex_t thread_protection;
extern GRELInterpreter *grel;


GRELInterpreter*
grel_interpreter_new (const char *name)
{
  debug_functions ();

  GRELInterpreter *interpreter;
  interpreter = calloc (1, sizeof (GRELInterpreter));
  assert (interpreter != NULL);

  interpreter->name = calloc (1, strlen (name) + 1);
  assert (interpreter->name != NULL);

  strcpy (interpreter->name, name);

  return interpreter;
}


static void
grel_interpreter_command_executer (GRELInterpreter *interpreter, const char *data)
{
  // Avoid parallel execution of commands.
  pthread_mutex_lock (&thread_protection);

  debug_functions ();
  assert (interpreter != NULL);

  if (data == NULL)
  {
    debug_error ("Not attempting to execute an empty command.");
    goto thread_unlock;
  }

  /*--------------------------------------------------------------------------.
   | MESSAGE PARSING                                                          |
   '--------------------------------------------------------------------------*/

  const char *separator = strchr (data, ':');
  if (separator == NULL)
  {
    debug_error ("Received invalid command.");
    goto thread_unlock;
  }

  char *function = calloc (1, (separator - data) + 1);
  assert (function != NULL);

  strncpy (function, data, (separator - data));
  debug_extra ("The command parsed is '%s'", function);

  char *argument = NULL;
  if (strlen (separator) > 0)
  {
    argument = calloc (1, strlen (separator));
    assert (argument != NULL);

    strcpy (argument, separator + 1);
    debug_extra ("The argument parsed is '%s'", argument);
  }

  /*--------------------------------------------------------------------------.
   | COMMAND INTERPRETATION                                                   |
   '--------------------------------------------------------------------------*/

  GRELInterpreterCommand *command = NULL;

  // Event: "quit".
  if (!strcmp (function, "quit"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommand));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_EMPTY;
    GREL_INTERPRETER_COMMAND_EMPTY (command)->callback = interpreter->on_quit;
  }

  // Event: "file-load".
  else if (!strcmp (function, "file-load"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandLoad));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_LOAD;
    GREL_INTERPRETER_COMMAND_LOAD (command)->callback = interpreter->on_file_load;
    GREL_INTERPRETER_COMMAND_LOAD (command)->filename = argument;
  }

  // Event: "overlay-load".
  else if (!strcmp (function, "overlay-load"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandLoad));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_LOAD;
    GREL_INTERPRETER_COMMAND_LOAD (command)->callback = interpreter->on_overlay_load;
    GREL_INTERPRETER_COMMAND_LOAD (command)->filename = argument;
  }

  // Event: "file-save".
  else if (!strcmp (function, "file-save"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandSave));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_SAVE;
    GREL_INTERPRETER_COMMAND_SAVE (command)->callback = interpreter->on_file_save;

    char *delimiter = strchr (argument, ';');
    if (delimiter == NULL) goto thread_unlock;

    // Skip the delimiter.
    size_t first_len = (delimiter - argument) + 1;
    char *first = calloc (1, first_len);
    assert (first != NULL);

    size_t second_len = strlen (delimiter);
    char *second = calloc (1, second_len);
    assert (second != NULL);

    first = strncpy (first, argument, first_len - 1);
    GREL_INTERPRETER_COMMAND_SAVE (command)->layer = first;

    second = strncpy (second, delimiter + 1, second_len - 1);
    GREL_INTERPRETER_COMMAND_SAVE (command)->filename = second;

    free (argument);
    argument = NULL;
  }

  // Event: "override-key".
  else if (!strcmp (function, "override-key"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandOverrideKey));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_OVERRIDE_KEY;
    GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->callback = interpreter->on_override_key;

    char *delimiter = strchr (argument, ';');
    if (delimiter == NULL) goto thread_unlock;

    // Skip the delimiter.
    size_t first_len = (delimiter - argument) + 1;
    char *name = calloc (1, first_len);
    assert (name != NULL);

    name = strncpy (name, argument, first_len - 1);
    GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->name = name;
    GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->key = (unsigned char)*(delimiter + 1);

    free (argument);
    argument = NULL;
  }

  else if (!strcmp (function, "set-active-mask"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandSetVoxel));
    assert (command != NULL);
  }

  else if (!strcmp (function, "set-viewport"))
  {
    command = calloc (1, sizeof (GRELInterpreterCommandSet));
    assert (command != NULL);

    command->type = GREL_INTERPRETER_TYPE_SET_VIEWPORT;
    GREL_INTERPRETER_COMMAND_SET (command)->callback = interpreter->on_set_viewport;
    GREL_INTERPRETER_COMMAND_SET (command)->value = atoi (argument);

    free (argument);
    argument = NULL;
  }

  // Invalid requests.
  else
  {
    debug_warning ("Event '%s' doesn't exist.", function);
  }

  /*--------------------------------------------------------------------------.
   | COMMAND EXECUTION / QUEING                                               |
   '--------------------------------------------------------------------------*/

  // Don't continue when the command is empty.
  if (command == NULL) goto thread_unlock;

  (interpreter->in_background)
    ? interpreter->command_queue = list_append (interpreter->command_queue, command)
    : grel_interpreter_call_back (command);

 thread_unlock:
  pthread_mutex_unlock (&thread_protection);
}


void
grel_interpreter_call_back (GRELInterpreterCommand *command)
{
  debug_functions ();
  assert (command != NULL);

  switch (command->type)
  {
    case GREL_INTERPRETER_TYPE_EMPTY:
      {
        GREL_INTERPRETER_COMMAND_EMPTY (command)->callback ();
      }
      break;
    case GREL_INTERPRETER_TYPE_LOAD:
      {
	char *filename = GREL_INTERPRETER_COMMAND_LOAD (command)->filename;

	GREL_INTERPRETER_COMMAND_LOAD (command)->callback (filename);

	free (GREL_INTERPRETER_COMMAND_LOAD (command)->filename);
	GREL_INTERPRETER_COMMAND_LOAD (command)->filename = NULL;
      }
      break;
    case GREL_INTERPRETER_TYPE_SAVE:
      {
	char *filename = GREL_INTERPRETER_COMMAND_SAVE (command)->filename;
	char *layer = GREL_INTERPRETER_COMMAND_SAVE (command)->layer;

	GREL_INTERPRETER_COMMAND_SAVE (command)->callback (layer, filename);
	free (GREL_INTERPRETER_COMMAND_SAVE (command)->layer);

	GREL_INTERPRETER_COMMAND_SAVE (command)->layer = NULL;
	free (GREL_INTERPRETER_COMMAND_SAVE (command)->filename);

	GREL_INTERPRETER_COMMAND_SAVE (command)->filename = NULL;
      }
      break;
    case GREL_INTERPRETER_TYPE_OVERRIDE_KEY:
      {
	char *name = GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->name;
	char key = GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->key;

	GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->callback (name, key);
	free (GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->name);

	GREL_INTERPRETER_COMMAND_OVERRIDE_KEY (command)->name = NULL;
      }
      break;
    case GREL_INTERPRETER_TYPE_SET_VIEWPORT:
      {
	int viewport = GREL_INTERPRETER_COMMAND_SET (command)->value;
	GREL_INTERPRETER_COMMAND_SET (command)->callback (viewport);
      }
      break;
  }

  free (command), command = NULL;
}


static void
grel_interpreter_command_handler (GRELInterpreter *interpreter, int connection)
{
  debug_functions ();

  // Reset the buffer.
  memset (interpreter->buffer, '\0', MAX_BUFFER_LEN);

  int bytes = 0;
  do
  {
    bytes = recv (connection, interpreter->buffer, MAX_BUFFER_LEN, 0);

    if (bytes < 0)
    {
      debug_warning ("Did not receive any data: %s", strerror (errno));
      break;
    }
  }
  while (bytes < 0);

  grel_interpreter_command_executer (interpreter, interpreter->buffer);
}


void
grel_interpreter_run (void *instance)
{
  debug_functions ();

  GRELInterpreter *interpreter = instance;
  assert (interpreter != NULL);

  // Currently, the interpreter uses a UNIX socket, which is limited to the
  // local computer it's running on. This has been done on purpose to limit
  // the capabilities of the interpreter. You are free to change the parts
  // to make it network-aware.
  interpreter->connection = socket (AF_LOCAL, SOCK_STREAM, 0);

  if (interpreter->connection == -1)
  {
    debug_error ("Couldn't set up a socket on '%s': %s",
		 interpreter->name,
		 strerror (errno));
    return;
  }
  
  // Set the address/path of the connection.
  struct sockaddr_un address;
  address.sun_family = AF_LOCAL;
  strcpy (address.sun_path, interpreter->name);

  // Remove the socket if it already exists.
  unlink (interpreter->name);

  // Bind to the socket.
  int length = strlen (address.sun_path) + sizeof (address.sun_family);
  if (bind (interpreter->connection, (struct sockaddr *)&address, length) == -1)
  {
    debug_error ("Couldn't bind to '%s': %s",
		 interpreter->name,
		 strerror (errno));
    goto cleanup;
  }

  // Listen for new connections.
  if (listen (interpreter->connection, MAX_CONNECTIONS) == -1)
  {
    debug_error ("Couldn't listen on '%s': %s", interpreter->name, strerror (errno));
    goto cleanup;
  }

  debug_extra ("GRELInterpreter is running.");

  int incoming_request;
  while (interpreter->shutdown != 1)
  {
    // Accept the incoming request.
    incoming_request = accept (interpreter->connection, NULL, NULL);
    if (incoming_request != -1)
    {
      // Let someone else handle the connection.
      grel_interpreter_command_handler (interpreter, incoming_request);

      // Clean up the mess.
      close (incoming_request);
    }
  }

 cleanup:
  close (interpreter->connection);
}


void
grel_interpreter_set_callback (GRELInterpreter *interpreter, char *name, void *callback)
{
  debug_functions ();
  assert (interpreter != NULL);
  assert (name != NULL);

  if (!strcmp (name, "quit"))
  {
    interpreter->on_quit = callback;
    debug_extra ("The 'quit' callback has been set to %p.", callback);
  }

  // Event: "file-load".
  else if (!strcmp (name, "file-load"))
  {
    interpreter->on_file_load = callback;
    debug_extra ("The 'file-load' callback has been set to %p.", callback);
  }

  // Event: "overlay-load".
  else if (!strcmp (name, "overlay-load"))
  {
    interpreter->on_overlay_load = callback;
    debug_extra ("The 'overlay-load' callback has been set to %p.", callback);
  }

  // Event: "file-save".
  else if (!strcmp (name, "file-save"))
  {
    interpreter->on_file_save = callback;
    debug_extra ("The 'file-save' callback has been set to %p.", callback);
  }

  // Event: "override-key".
  else if (!strcmp (name, "override-key"))
  {
    interpreter->on_override_key = callback;
    debug_extra ("The 'override-key' callback has been set to %p.", callback);
  }

  // Event: "override-key".
  else if (!strcmp (name, "set-viewport"))
  {
    interpreter->on_set_viewport = callback;
    debug_extra ("The 'set-viewport' callback has been set to %p.", callback);
  }

  // Invalid requests.
  else
  {
    debug_warning ("Event '%s' isn't implemented.", name);
  }
}


void
grel_interpreter_reset_callback (GRELInterpreter *interpreter, char *name)
{
  debug_functions ();
  assert (interpreter != NULL);

  grel_interpreter_set_callback (interpreter, name, NULL);
}


void
grel_interpreter_destroy (GRELInterpreter* interpreter)
{
  debug_functions ();

  if (interpreter == NULL) return;

  interpreter->shutdown = 1;
  close (interpreter->connection);
  unlink (interpreter->name);

  free (interpreter);
  interpreter = NULL;
}


void
grel_interpreter_run_in_background (void *instance)
{
  debug_functions ();

  GRELInterpreter *interpreter = instance;
  assert (interpreter != NULL);

  // Set the 'in_background' flag to enable background processing.
  interpreter->in_background = 1;

  // Start a secondary thread to do the processing in the background.
  pthread_t secondary;
  pthread_create (&secondary,
		  NULL,
		  (void *)&grel_interpreter_run,
		  (void *)interpreter);
}


GRELInterpreterCommand*
grel_interpreter_get_next_command (GRELInterpreter *interpreter)
{
  debug_functions ();
  assert (interpreter != NULL);

  // This function is only useful when we're in background-mode.
  if (!interpreter->in_background) return NULL;

  // When the command queue is empty, there's no point in continuing.
  if (interpreter->command_queue == NULL) return NULL;

  // Get the command stored in the list.
  GRELInterpreterCommand *command = interpreter->command_queue->data;
  if (command == NULL) return NULL;

  // Remove it from the queue.
  interpreter->command_queue = list_remove (interpreter->command_queue);

  return command;
}
