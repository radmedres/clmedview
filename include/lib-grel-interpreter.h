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

#ifndef GREL_INTERPRETER_H
#define GREL_INTERPRETER_H

/**
 * @ingroup grel
 * @{
 *   @defgroup grel_interpreter Interpreter
 *   @{
 *
 * This module provides a command interpreter for GREL. It is the back-end
 * for the @ref grel_shell "GREL::Shell" module. Feel free to attach a
 * different front-end to this module.
 *
 * This module is merely a router invoking callback functions when a command
 * has been received.
 */


#include "lib-common-list.h"


/**
 * An enumeration of the GREL interpreter commands.
 */
typedef enum
{
  GREL_INTERPRETER_TYPE_EMPTY,
  GREL_INTERPRETER_TYPE_LOAD,
  GREL_INTERPRETER_TYPE_SAVE,
  GREL_INTERPRETER_TYPE_OVERRIDE_KEY,
  GREL_INTERPRETER_TYPE_SET_VOXEL,
  GREL_INTERPRETER_TYPE_SET_VIEWPORT
} GRELInterpreterCommandType;


/**
 * A structure to store a command and it's parameters.
 * This is the generic type which should probably not be used.
 */
typedef struct
{
  char type;
  void (*callback) (void);
} GRELInterpreterCommand;


/**
 * A structure to store a command for a "load" function.
 */
typedef struct
{
  char type;
  void (*callback) (char *);
  char *filename;
} GRELInterpreterCommandLoad;


/**
 * A structure to store a command for a "save" function.
 */
typedef struct
{
  char type;
  void (*callback) (char *, char *);
  char *layer;
  char *filename;
} GRELInterpreterCommandSave;


/**
 * A structure to store a command for an "override key" function.
 */
typedef struct
{
  char type;
  void (*callback) (char *, char);
  char *name;
  char key;
} GRELInterpreterCommandOverrideKey;


/**
 * A structure to store a command for a "set voxel" function.
 */
typedef struct
{
  char type;
  void (*callback) (int, int, int, int);
  int x;
  int y;
  int z;
  int value;
} GRELInterpreterCommandSetVoxel;


/**
 * A structure to store a command for a "set" function.
 */
typedef struct
{
  char type;
  void (*callback) (int);
  int value;
} GRELInterpreterCommandSet;


/**
 * A bunch of macros
 */
#define GREL_INTERPRETER_COMMAND_EMPTY(x) ((GRELInterpreterCommand *)x)
#define GREL_INTERPRETER_COMMAND_LOAD(x) ((GRELInterpreterCommandLoad *)x)
#define GREL_INTERPRETER_COMMAND_SAVE(x) ((GRELInterpreterCommandSave *)x)
#define GREL_INTERPRETER_COMMAND_OVERRIDE_KEY(x) ((GRELInterpreterCommandOverrideKey *)x)
#define GREL_INTERPRETER_COMMAND_SET_VOXEL(x) ((GRELInterpreterCommandSetVoxel *)x)
#define GREL_INTERPRETER_COMMAND_SET(x) ((GRELInterpreterCommandSet *)x)
#define GREL_INTERPRETER_COMMAND_QUEUE(x) (x->command_queue)


/**
 * The GREL interpreter object. 
 */
typedef struct
{
  /*--------------------------------------------------------------------------.
   | PROPERTIES                                                               |
   | For each property there should be a getter and a setter.                 |
   '--------------------------------------------------------------------------*/

  char *name; /*< The name, used as file name for the socket.  */

  /*--------------------------------------------------------------------------.
   | MEMBERS                                                                  |
   | Member variables are private fields that should only be used internally. |
   | If access to a variable here is useful for outsiders, please use a macro |
   | definition to provide access.                                            |
   '--------------------------------------------------------------------------*/

  int connection; /*< A socket to create a connection. */
  int shutdown; /*< Internal variable used to shut down the connection. */
  char in_background; /*< Control variable for background processing. */
  List *command_queue; /*< A buffer with unprocessed commands. */

  char buffer[512]; /*< A buffer that can be shared across threads. */

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/

  /**
   * A callback function to call on a "quit" event.
   */
  void (*on_quit)(void);

  /**
   * A callback function to call on a "file load" event.
   * @param filename  The file to load.
   */
  void (*on_file_load)(char *filename);

  /**
   * A callback function to call on a "overlay load" event.
   * @param filename  The file to load.
   */
  void (*on_overlay_load)(char *filename);

  /**
   * A callback function to call on a "file save" event.
   * @param layer_name  The name of the layer to save.
   * @param filename    The path to save the layer to.
   */
  void (*on_file_save)(char *layer, char *filename);

  /**
   * A callback function to call on a "override key" event.
   * @param filename  The file to load.
   */
  void (*on_override_key)(char *name, char key);

  /**
   * A callback function to call on a "set viewport" event.
   * @param viewport  The file to load.
   */
  void (*on_set_viewport)(int viewport);

} GRELInterpreter;


/**
 * This function creates a new GREL interpreter.
 *
 * @param name  The name of the interpreter. The name will also be used for the
 *              socket file name.
 * 
 * @return A newly allocated GRELInterpreter.
 */
GRELInterpreter* grel_interpreter_new (const char *name);


/**
 * Once you've set every callback you need, you can run the interpreter using
 * this function. This function can be executed as a callback of 
 * pthread_create() or executed directly.
 *
 * @param interpreter A pointer to a GRELInterpreter.
 */
void grel_interpreter_run (void *interpreter);


/**
 * This function runs the interpreter in the background (as a separate thread).
 * Use grel_interpreter_get_next_command() to process a command it has
 * received and queued.
 *
 * @param interpreter  The GRELInterpreter to get the next command of.
 */
void grel_interpreter_run_in_background (void *interpreter);


/**
 * This function can be used in combination with
 * grel_interpreter_run_in_background() to get a command from the command 
 * queue. This effectively provides a polling-based alternative interface to
 * the interpreter back-end.
 *
 * The command that is returned is removed from the queue on this call, so
 * the next call to this function will return a different command. You have to
 * free the GRELInterpreterCommand yourself after using it.
 *
 * @param interpreter  The GRELInterpreter to get the next command of.
 */
GRELInterpreterCommand* grel_interpreter_get_next_command (GRELInterpreter *interpreter);


/**
 * This function executes a command stored in a GRELInterpreterCommand.
 */
void grel_interpreter_call_back (GRELInterpreterCommand *command);


/**
 * This function provides a way to set a callback for a specified interpreter.
 * 
 * @param interpreter  The GRELInterpreter to set a callback for.
 * @param name         The name of the callback event to set.
 * @param callback     The callback function.
 */
void grel_interpreter_set_callback (GRELInterpreter *interpreter, char *name,
				    void *callback);


/**
 * This function provides a way to reset a callback for a specified
 * interpreter. After resetting, the callback function set earlier for the 
 * event won't be called anymore.
 *
 * @param interpreter  The GRELInterpreter to reset a callback for.
 * @param name         The name of the callback event to reset.
 */
void grel_interpreter_reset_callback (GRELInterpreter *interpreter, char *name);


/**
 * This function destroys a GREL interpreter. After calling this function
 * the interpreter will be shut down and removed from memory.
 *
 * @param interpreter  The GRELInterpreter to shut down and destroy.
 */
void grel_interpreter_destroy (GRELInterpreter* interpreter);


/**
 *   @}
 * @}
 */

#endif//GREL_INTERPRETER_H
