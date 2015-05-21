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

#ifndef GUI_H
#define GUI_H

#include "lib-viewer.h"
#include "lib-memory-io.h"
#include <gtk/gtk.h>


/**
 * @file   source/gui/mainwindow.h
 * @brief  The graphical user interface for clmedview.
 * @author Roel Janssen
 */

/**
 * @defgroup gui GUI
 * @{
 *   @defgroup gui_mainwindow Mainwindow
 *   @{
 *
 * This module provides a graphical user interface to visually interact with
 * medical image data.
 *
 * The GUI can be roughly split into four parts:
 * - Gtk window stuff
 * - Sidebar
 *   - Tree view
 *   - Layer manager
 * - Top bar
 * - Plugin bar
 */


/**
 * This function creates a graphical user interface and starts
 * the GTK main loop.
 *
 * @param file The filename passed on the command-line (if any).
 *
 * @return 0 on succes, not 0 on failure (compatible with UNIX shell standards).
 */
void gui_mainwindow_new (char* file);


/**
 * This function should be triggered when the window has been resized.
 *
 * @param widget The widget that triggers this function.
 * @param data   Unused.
 */
gboolean gui_mainwindow_resize (GtkWidget *widget, void *data);


/**
 * This function should be triggered when all Viewer objects should be redrawn.
 *
 * @param viewer   The viewer providing the update.
 * @param data     Unused.
 */
void gui_mainwindow_refresh_viewers (Viewer *viewer, void *data);


/**
 * This function should be called by a viewer object to notify GUI that
 * the mouse position inside the widget has changed.
 *
 * @param viewer   The viewer providing the update.
 * @param data     Should contain a pointer to a Coordinate with the
 *                 current position.
 */
void gui_mainwindow_update_viewer_positions (Viewer *viewer, void *data);


/**
 * This function should be called by a viewer object to notify GUI that
 * the normal vector inside the widget has changed.
 *
 * @param viewer   The viewer providing the update.
 * @param data     Data containing the ratio of DX / DY (float).
 */
void gui_mainwindow_update_handle_position (Viewer *viewer, void *data);


/**
 * This function is called to quit the graphical user interface.
 */
void gui_mainwindow_destroy ();


/**
 * This function should be triggered when the user wants to load a file.
 *
 * @param data   A filename, or NULL to open a file dialog.
 */
void gui_mainwindow_file_load (void* data);


/**
 * This function allows a user to display a specific serie.
 *
 * @param pll_Study The study to display.
 */
void
gui_mainwindow_load_serie (Tree *pll_Serie);


/**
 * This function should be triggered when the user wants to save a file.
 */
gboolean gui_mainwindow_file_export ();


/**
 * This function loads plugins from a given directory and adds them to the
 * specified container.
 *
 * @param path  The directory to the plugins.
 * @param box   The container to add the plugin buttons to.
 */
void gui_mainwindow_load_plugins_from_directory (const char* path, GtkWidget *box);


/**
 *   @}
 * @}
 */

#endif//GUI_H
