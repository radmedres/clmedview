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

#include "mainwindow.h"
#include "libviewer.h"
#include "libpixeldata-plugin.h"
#include "libconfiguration.h"
//#include "libhistogram.h"

#include "libcommon-list.h"
#include "libcommon-history.h"
#include "libcommon-tree.h"
#include "libcommon-debug.h"
#include "libcommon-unused.h"

#include "libmemory.h"
#include "libmemory-patient.h"
#include "libmemory-study.h"
#include "libmemory-serie.h"
#include "libmemory-slice.h"
#include "libmemory-tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <libgen.h>


/******************************************************************************
 * STATIC SETTINGS
 ******************************************************************************/


#define LABEL_FOLLOW_MODE     "Follow mode"
#define LABEL_AUTO_CLOSE      "Auto close"

#define LABEL_AXIAL_VIEW      "Axial view"
#define LABEL_SAGITAL_VIEW    "Sagital view"
#define LABEL_CORONAL_VIEW    "Coronal view"
#define LABEL_SPLIT_VIEW      "Split view"

#define ICON_CHECKBOX         "checkbox-symbolic"
#define ICON_CHECKBOX_ENABLED "checkbox-checked-symbolic"
#define ICON_RADIO            "radio-symbolic"
#define ICON_RADIO_ENABLED    "radio-checked-symbolic"

#define ICON_DOCUMENT_OPEN    "document-open-symbolic"
#define ICON_DOCUMENT_SAVE    "document-save-as-symbolic"
#define ICON_ADD              "list-add-symbolic"
#define ICON_REMOVE           "list-remove-symbolic"
#define ICON_RESET            "view-refresh-symbolic"
#define ICON_SIDEBAR_HIDE     "go-previous-symbolic"
#define ICON_SIDEBAR_SHOW     "go-next-symbolic"

#ifdef WIN32
#define PLUGIN_PATH           "plugin\\"
#define PLUGIN_ICON_PATH      "icons\\"
#define LOOKUP_TABLES_PATH    "luts\\"
#else
#define PLUGIN_PATH           "plugin/"
#define PLUGIN_ICON_PATH      "icons/"
#define LOOKUP_TABLES_PATH    "luts/"
#endif


/******************************************************************************
 * OBJECT INTERNAL TYPES
 ******************************************************************************/


typedef enum
{
  SIDEBAR_ID,
  SIDEBAR_NAME,
  SIDEBAR_NAME_EDIT,
  SIDEBAR_OUTER,
  SIDEBAR_INNER
} GuiSidebarColumn;


typedef enum
{
  VIEWPORT_TYPE_AXIAL   = ORIENTATION_AXIAL,
  VIEWPORT_TYPE_SAGITAL = ORIENTATION_SAGITAL,
  VIEWPORT_TYPE_CORONAL = ORIENTATION_CORONAL,
  VIEWPORT_TYPE_THREEWAY,
  VIEWPORT_TYPE_UNDEFINED
} GuiViewportType;


typedef enum
{
  GUI_DO_UNDEFINED,
  GUI_DO_RESIZE,
  GUI_DO_REDRAW
} GuiActionType;


typedef enum
{
  GUI_FILE_OPEN,
  GUI_FILE_SAVE_MASK,
  GUI_FILE_CLOSE
} GuiFileAction;


/******************************************************************************
 * OBJECT INTERNAL FUNCTION DEFINITIONS
 ******************************************************************************/


// Event handlers
gboolean gui_mainwindow_menu_file_activate (GtkWidget *widget, void *data);
gboolean gui_mainwindow_views_activate (GtkWidget *widget, void *data);
gboolean gui_mainwindow_on_key_press (GtkWidget *widget, GdkEventKey *event, void *data);
gboolean gui_mainwindow_on_key_release (GtkWidget *widget, GdkEventKey *event, void *data);
gboolean gui_mainwindow_save_undo_step (GtkWidget *widget, void *data);
gboolean gui_mainwindow_select_tool (GtkWidget *widget, void *data);
gboolean gui_mainwindow_sidebar_toggle (GtkWidget *widget, void *data);
gboolean gui_mainwindow_on_timeline_change (GtkWidget *widget, void *data);

gboolean gui_mainwindow_mask_load (unsigned long long ull_serieID, void *data);
gboolean gui_mainwindow_mask_add (unsigned long long ull_serieID);
gboolean gui_mainwindow_mask_remove (unsigned long long ull_serieID);
gboolean gui_mainwindow_mask_set_active (unsigned long long ull_serieID);

gboolean gui_mainwindow_overlay_load (unsigned long long ull_serieID, void *data);
gboolean gui_mainwindow_overlay_remove (unsigned long long ull_serieID);

void gui_mainwindow_update_viewer_wwwl (Viewer *viewer, void *data);

gboolean gui_mainwindow_reset_viewport ();
gboolean gui_mainwindow_toggle_follow_mode ();
gboolean gui_mainwindow_toggle_auto_close ();

void gui_mainwindow_check_extention (char *pc_String);

// Sidebar functions
GtkWidget* gui_mainwindow_sidebar_new ();
void gui_mainwindow_sidebar_populate (Tree *pll_Patients);
void gui_mainwindow_cell_edited (GtkCellRendererText *cell, char *pc_pathstring, char *pc_new_text, void *pv_data);
void gui_mainwindow_sidebar_destroy ();

// Layout properties manager functions
GtkWidget* gui_mainwindow_properties_manager_new ();
void gui_mainwindow_properties_manager_refresh (unsigned long long ull_serieID);

// Other
GtkWidget* gui_mainwindow_toolbar_new ();
char* gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action);
void gui_mainwindow_clear_viewers ();

/******************************************************************************
 * OBJECT INTERNAL VARIABLES
 ******************************************************************************/


// Gtk widgets
GtkWidget *window;
GtkWidget *hbox_viewers;
GtkWidget *hbox_mainmenu;
GtkWidget *views_combo;
GtkWidget *lbl_info;
GtkWidget *chk_follow;
GtkWidget *chk_auto_close;
GtkWidget *inp_brush_size;
GtkWidget *inp_brush_value;
GtkWidget *sidebar;
GtkWidget *layer_manager;
GtkWidget *axial_embed;
GtkWidget *sagital_embed;
GtkWidget *coronal_embed;
GtkWidget *btn_ActiveDrawTool;
GtkWidget *btn_file_save;
GtkWidget *btn_reset_viewport;
GtkWidget *btn_sidebar_toggle;
GtkWidget *properties_opacity_scale;
GtkWidget *properties_lookup_table_combo;
GtkWidget *timeline;
GtkWidget *treeview;
//GtkWidget *histogram_drawarea;
GtkTreeStore *sidebar_TreeStore;

// Application-local stuff
List *pll_Viewers;
List *pll_History;
List *pl_plugins;

Viewer *ps_active_viewer;
Plugin *ps_active_draw_tool;

GuiViewportType te_DisplayType = VIEWPORT_TYPE_UNDEFINED;

Configuration *config;
//Histogram *histogram;

/******************************************************************************
 * FUNCTION IMPLEMENTATIONS
 ******************************************************************************/


void
gui_mainwindow_redisplay_viewers (GuiActionType te_Action, ...)
{
  debug_functions ();

  int width = 0;
  int height = 0;
  List *pll_iter = NULL;
  Viewer *ps_viewer = NULL;

  if (te_Action == GUI_DO_RESIZE)
  {
    va_list arguments;
    va_start (arguments, te_Action);

    width = va_arg (arguments, int);
    height = va_arg (arguments, int);

    if (te_DisplayType == VIEWPORT_TYPE_THREEWAY)
    {
      width = width / 3;
    }
    va_end (arguments);
  }

  pll_iter = list_nth (pll_Viewers, 1);
  while (pll_iter != NULL)
  {
    ps_viewer = pll_iter->data;
    if ((te_DisplayType == VIEWPORT_TYPE_THREEWAY) ||
        ((short)(viewer_get_orientation (ps_viewer)) == (short)(te_DisplayType)))
    {
      switch (te_Action)
      {
        case GUI_DO_RESIZE:
          viewer_resize (ps_viewer, width, height);
          break;
        case GUI_DO_REDRAW:
          viewer_redraw (ps_viewer, REDRAW_ACTIVE);
          break;
        default: break;
      }
    }
    pll_iter = pll_iter->next;
  }
}


void
gui_mainwindow_no_gl_warning ()
{
  debug_functions ();

  GtkWidget* window;
  gtk_init (NULL, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (GTK_WINDOW (window), 500, 75);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (window), "Oops! No GL support detected.");

  GtkWidget *warning = gtk_label_new ("Sorry. Because your graphics card "
                                      "doesn't seem to support GL this "
				      "program cannot run.");

  gtk_container_add (GTK_CONTAINER (window), warning);

  g_signal_connect (G_OBJECT (window), "destroy",
		    G_CALLBACK (gtk_main_quit),
		    NULL);

  gtk_widget_show_all (window);
  gtk_main ();
}


void
gui_mainwindow_new (char *file)
{
  debug_functions ();

  /*--------------------------------------------------------------------------.
   | INITIALIZATION                                                           |
   '--------------------------------------------------------------------------*/
  if (gtk_clutter_init (NULL, NULL) != CLUTTER_INIT_SUCCESS)
  {
    gui_mainwindow_no_gl_warning ();
    return;
  }

  config = configuration_get_default ();

  /*--------------------------------------------------------------------------.
   | LOAD LOOKUP TABLES                                                       |
   '--------------------------------------------------------------------------*/
  pixeldata_lookup_table_load_from_directory (LOOKUP_TABLES_PATH);

  /*--------------------------------------------------------------------------.
   | MAIN WINDOW STUFF                                                        |
   '--------------------------------------------------------------------------*/
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  // Set the window's size to 90% of the active screen size.
  // INFO: On Win32 this generates an error.
  GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (window));
  GdkWindow *gdk_window = gdk_screen_get_root_window (screen);
  int monitor = gdk_screen_get_monitor_at_window (screen, gdk_window);

  GdkRectangle screen_size;
  gdk_screen_get_monitor_geometry (screen, monitor, &screen_size);

  screen_size.width *= 0.9;
  screen_size.height *= 0.9;

  gtk_window_set_default_size (GTK_WINDOW (window),
			       screen_size.width,
			       screen_size.height);

  /*--------------------------------------------------------------------------.
   | HEADER BAR                                                               |
   '--------------------------------------------------------------------------*/
  GtkWidget *header = gtk_header_bar_new ();

  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header), TRUE);
  gtk_header_bar_set_title (GTK_HEADER_BAR (header), "clmedview");
  gtk_window_set_titlebar (GTK_WINDOW (window), header);

  /*--------------------------------------------------------------------------.
   | FILE MENU BUTTONS                                                        |
   '--------------------------------------------------------------------------*/
  // Create the 'Open' button.
  GtkWidget *btn_file_open;
  btn_file_open = gtk_button_new_from_icon_name (ICON_DOCUMENT_OPEN,
						 GTK_ICON_SIZE_BUTTON);

  // Create the 'Save' button.
  btn_file_save = gtk_button_new_from_icon_name (ICON_DOCUMENT_SAVE,
						 GTK_ICON_SIZE_BUTTON);

  // Create the 'Show sidebar' button.
  btn_sidebar_toggle = gtk_button_new_from_icon_name (ICON_SIDEBAR_SHOW, GTK_ICON_SIZE_BUTTON);
  btn_reset_viewport = gtk_button_new_from_icon_name (ICON_RESET, GTK_ICON_SIZE_BUTTON);

  // Make them responsive.
  g_signal_connect (btn_file_open, "clicked",
		    G_CALLBACK (gui_mainwindow_menu_file_activate),
		    (void *)GUI_FILE_OPEN);

  g_signal_connect (btn_file_save, "clicked",
		    G_CALLBACK (gui_mainwindow_menu_file_activate),
		    (void *)GUI_FILE_SAVE_MASK);

  g_signal_connect (btn_sidebar_toggle, "clicked",
		    G_CALLBACK (gui_mainwindow_sidebar_toggle),
		    NULL);

  g_signal_connect (btn_reset_viewport, "clicked",
		    G_CALLBACK (gui_mainwindow_reset_viewport),
		    NULL);

  // Pack the buttons to the header bar.
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), btn_file_open);
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), btn_file_save);
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), btn_sidebar_toggle);
  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), btn_reset_viewport);

  // Disable the 'Save' button by default.
  gtk_widget_set_sensitive (btn_file_save, FALSE);
  gtk_widget_set_sensitive (btn_reset_viewport, FALSE);

  /*--------------------------------------------------------------------------.
   | VIEWPORT VIEW BUTTON                                                     |
   '--------------------------------------------------------------------------*/
  GtkWidget *lbl_views = gtk_label_new ("Views:");
  views_combo = gtk_combo_box_text_new ();

  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (views_combo),
			     LABEL_AXIAL_VIEW, LABEL_AXIAL_VIEW);

  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (views_combo),
			     LABEL_SAGITAL_VIEW, LABEL_SAGITAL_VIEW);

  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (views_combo),
			     LABEL_CORONAL_VIEW, LABEL_CORONAL_VIEW);

  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (views_combo),
			     LABEL_SPLIT_VIEW, LABEL_SPLIT_VIEW);

  gtk_combo_box_set_active (GTK_COMBO_BOX (views_combo), 3);

  g_signal_connect (views_combo, "changed",
		    G_CALLBACK (gui_mainwindow_views_activate), NULL);

  // Add the button box to the header bar.
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), views_combo);
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), lbl_views);

  // Disable the buttons by default.
  gtk_widget_set_sensitive (views_combo, FALSE);

  /*--------------------------------------------------------------------------.
   | TOOL BAR                                                                 |
   '--------------------------------------------------------------------------*/
  GtkWidget *hbox_toolbar = gui_mainwindow_toolbar_new ();

  chk_follow = gtk_check_button_new_with_label (LABEL_FOLLOW_MODE);
  chk_auto_close = gtk_check_button_new_with_label (LABEL_AUTO_CLOSE);
  lbl_info = gtk_label_new ("");

  g_signal_connect (chk_follow, "toggled",
		    G_CALLBACK (gui_mainwindow_toggle_follow_mode),
		    NULL);
  g_signal_connect (chk_auto_close, "toggled",
		    G_CALLBACK (gui_mainwindow_toggle_auto_close),
		    NULL);

  GtkWidget *hbox_optionbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_container_add (GTK_CONTAINER (hbox_optionbar), chk_follow);
  gtk_container_add (GTK_CONTAINER (hbox_optionbar), chk_auto_close);

  /*--------------------------------------------------------------------------.
   | MAIN AREA                                                                |
   '--------------------------------------------------------------------------*/
  hbox_viewers = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /*--------------------------------------------------------------------------.
   | SIDEBAR                                                                  |
   '--------------------------------------------------------------------------*/
  sidebar = gui_mainwindow_sidebar_new (sidebar_TreeStore);
  gtk_widget_set_size_request (sidebar, 150, -1);

  gtk_widget_show_all (sidebar);
  gtk_widget_hide (sidebar);

  /*--------------------------------------------------------------------------.
   | TIMELINE BAR                                                             |
   '--------------------------------------------------------------------------*/

  timeline = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 1);
  gtk_range_set_value (GTK_RANGE (timeline), 0);

  g_signal_connect (G_OBJECT (timeline), "value-changed",
		    G_CALLBACK (gui_mainwindow_on_timeline_change),
		    NULL);

  /*--------------------------------------------------------------------------.
   | HISTOGRAM                                                                |
   '--------------------------------------------------------------------------*/

  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  // Main content packing
  hbox_mainmenu = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_pack_start (GTK_BOX (hbox_mainmenu), hbox_optionbar, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_mainmenu), lbl_info, 1, 1, 0);
  gtk_box_pack_end (GTK_BOX (hbox_mainmenu), hbox_toolbar, 0, 0, 5);
  gtk_widget_set_sensitive (GTK_WIDGET (hbox_mainmenu), FALSE);

  GtkWidget *vbox_mainarea = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox_mainarea), hbox_mainmenu, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_mainarea), hbox_viewers, 1, 1, 0);
  gtk_box_pack_start (GTK_BOX (vbox_mainarea), timeline, 0, 0, 0);

  gtk_widget_set_no_show_all (timeline, TRUE);
  gtk_widget_hide (timeline);

  // Window packing
  GtkWidget* hbox_mainwindow = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack1 (GTK_PANED (hbox_mainwindow), sidebar, TRUE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hbox_mainwindow), vbox_mainarea, TRUE, TRUE);

  gtk_container_add (GTK_CONTAINER (window), hbox_mainwindow);

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect (window, "destroy",
		    G_CALLBACK (gui_mainwindow_destroy),
		    NULL);

  g_signal_connect (window, "key-press-event",
		    G_CALLBACK (gui_mainwindow_on_key_press),
		    NULL);

  g_signal_connect (window, "key-release-event",
		    G_CALLBACK (gui_mainwindow_on_key_release),
		    NULL);

  g_signal_connect (hbox_viewers, "button-release-event",
		    G_CALLBACK (gui_mainwindow_save_undo_step),
		    NULL);

  g_signal_connect (hbox_viewers, "size-allocate",
		    G_CALLBACK (gui_mainwindow_resize),
		    NULL);

  /*--------------------------------------------------------------------------.
   | GTK MAIN LOOP                                                            |
   '--------------------------------------------------------------------------*/
  gtk_widget_set_no_show_all (sidebar, TRUE);
  gtk_widget_show_all (GTK_WIDGET (window));

  /*--------------------------------------------------------------------------.
   | LOAD A FILE WHEN SPECIFIED.                                              |
   '--------------------------------------------------------------------------*/
  if (file)
  {
    gui_mainwindow_file_load (file);
  }

  /*--------------------------------------------------------------------------.
   | GTK MAIN LOOP                                                            |
   '--------------------------------------------------------------------------*/
  gtk_main ();
}


gboolean
gui_mainwindow_save_undo_step (UNUSED GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  Serie *ps_mask;
  unsigned long ul64_SerieSize;

  ps_mask = CONFIGURATION_ACTIVE_MASK(config);
  if (ps_mask == NULL) return FALSE;


  ul64_SerieSize = ps_mask->matrix.i16_x * ps_mask->matrix.i16_y *
    ps_mask->matrix.i16_z * memory_serie_get_memory_space (ps_mask);

  pll_History = common_history_save_state (pll_History, ps_mask->data, ul64_SerieSize);

  return FALSE;
}


gboolean
gui_mainwindow_resize (UNUSED GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  gtk_widget_queue_draw (hbox_viewers);

  int width = gtk_widget_get_allocated_width (hbox_viewers);
  int height = gtk_widget_get_allocated_height (hbox_viewers);

  // Filter out useless configure events.
  if (width < 2 && height < 2) return FALSE;

  gui_mainwindow_redisplay_viewers (GUI_DO_RESIZE, width, height);

  return FALSE;
}


gboolean
gui_mainwindow_menu_file_activate (UNUSED GtkWidget *widget, void *data)
{
  debug_functions ();

  GuiFileAction te_Action = (GuiFileAction)data;
  switch (te_Action)
  {
    case GUI_FILE_OPEN:
      gui_mainwindow_file_load (NULL);
      break;
    case GUI_FILE_SAVE_MASK:
      gui_mainwindow_file_export ();
      break;
    case GUI_FILE_CLOSE:
      break;
    default:
      break;
  }

  return FALSE;
}


gboolean
gui_mainwindow_views_activate (GtkWidget *widget, void *data)
{
  debug_functions ();
  assert (widget != NULL);

  // TODO: This is a nasty hack.. We should find something better..
  if (data != NULL)
  {
    int type = ((long int)data) - 1;
    gtk_combo_box_set_active (GTK_COMBO_BOX (widget), type);
  }

  // GREL could be run before the embed widgets are created.
  // Therefore we check whether that's the case.
  if (axial_embed != NULL && sagital_embed != NULL && coronal_embed != NULL)
  {
    // The strategy is: Hide everything and show whatever we want to see.
    gtk_widget_set_visible (GTK_WIDGET (axial_embed), FALSE);
    gtk_widget_set_visible (GTK_WIDGET (sagital_embed), FALSE);
    gtk_widget_set_visible (GTK_WIDGET (coronal_embed), FALSE);
  }

  char *active;
  active = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (widget));
  if (!strcmp (active, LABEL_AXIAL_VIEW))
  {
    te_DisplayType = VIEWPORT_TYPE_AXIAL;

    if (axial_embed != NULL)
      gtk_widget_set_visible (GTK_WIDGET (axial_embed), TRUE);
  }

  else if (!strcmp (active, LABEL_SAGITAL_VIEW))
  {
    te_DisplayType = VIEWPORT_TYPE_SAGITAL;

    if (sagital_embed != NULL)
      gtk_widget_set_visible (GTK_WIDGET (sagital_embed), TRUE);
  }

  else if (!strcmp (active, LABEL_CORONAL_VIEW))
  {
    te_DisplayType = VIEWPORT_TYPE_CORONAL;

    if (coronal_embed != NULL)
      gtk_widget_set_visible (GTK_WIDGET (coronal_embed), TRUE);
  }

  else if (!strcmp (active, LABEL_SPLIT_VIEW))
  {
    te_DisplayType = VIEWPORT_TYPE_THREEWAY;
    gtk_widget_set_visible (GTK_WIDGET (axial_embed), TRUE);
    gtk_widget_set_visible (GTK_WIDGET (sagital_embed), TRUE);
    gtk_widget_set_visible (GTK_WIDGET (coronal_embed), TRUE);
  }

  return FALSE;
}


void
gui_mainwindow_file_load (void* data)
{
  debug_functions ();
  Tree *pt_study=NULL;
  Tree *pt_serie=NULL;
  Serie *ps_serie = NULL;

  char* filename = NULL;

  // The filename can be passed by 'data'. Otherwise we need to show a
  // dialog to the user to choose a file.
  filename = (data != NULL)
    ? (char *)data
    : gui_mainwindow_file_dialog (window, GTK_FILE_CHOOSER_ACTION_OPEN);

  if (filename != NULL)
  {
    char *window_title = calloc (1, 16 + strlen (filename) + 1);
    sprintf (window_title, "clmedview: %s", filename);
    gtk_window_set_title (GTK_WINDOW (window), window_title);

    free (window_title);

    pt_study=CONFIGURATION_ACTIVE_STUDY_TREE(config);
    pt_serie=pt_memory_io_load_file(&pt_study,filename);

    if (pt_serie != NULL)
    {
      ps_serie = pt_serie->data;
      ps_serie->e_SerieType=SERIE_ORIGINAL;

      gui_mainwindow_load_serie(pt_serie);

      gtk_widget_set_sensitive (btn_file_save, TRUE);
      gtk_widget_set_sensitive (btn_reset_viewport, TRUE);
      gtk_widget_set_sensitive (views_combo, TRUE);
      gtk_widget_set_sensitive (hbox_mainmenu, TRUE);

      gui_mainwindow_views_activate (views_combo, (void *)te_DisplayType);
      gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview));
      /*
      if (histogram == NULL)
	histogram = histogram_new ();

      histogram_set_serie (histogram, ps_serie);
      gtk_widget_queue_draw (histogram_drawarea);
      */
    }
  }
}


void
gui_mainwindow_load_serie (Tree *pt_serie)
{
  Tree *pt_mask = NULL;

  Serie *ps_serie;
  Serie *ps_mask;

  if (pt_serie == NULL) return;
  if (pt_serie->data == NULL) return;
  if (pt_serie->type != TREE_TYPE_SERIE) return;



  CONFIGURATION_ACTIVE_SERIE_TREE(config) = pt_serie;
  CONFIGURATION_ACTIVE_SERIE(config) = (Serie*)(pt_serie->data);
  CONFIGURATION_ACTIVE_STUDY_TREE (config) = tree_parent (pt_serie);

  // initialize the configuration struct.
  if (CONFIGURATION_MEMORY_TREE (config) == NULL)
  {
    CONFIGURATION_MEMORY_TREE(config) = tree_parent (CONFIGURATION_ACTIVE_STUDY_TREE (config));
  }


  ps_serie = CONFIGURATION_ACTIVE_SERIE(config);
  if (memory_tree_serie_has_mask (pt_serie))
  {
    // search for masks in serie tree
    // and grep the first mask with the same groupID

    Tree *p_iter = tree_nth(pt_serie,1);
    while (p_iter != NULL)
    {
      if (p_iter->type == TREE_TYPE_SERIE_MASK)
      {
        ps_mask = p_iter->data;
        if (ps_mask->group_id == ps_serie->group_id)
        {
          pt_mask = p_iter;
          break;
        }
      }
      p_iter = tree_next (p_iter);
    }
  }
  else
  {
    pt_mask = memory_tree_add_mask_for_serie (pt_serie);
  }


  assert (pt_mask != NULL);
  assert (pt_mask->data != NULL);
  CONFIGURATION_ACTIVE_MASK (config) = (Serie*)(pt_mask);

  ps_mask = pt_mask->data;
  if (ps_mask == NULL) return;

  Tree *root_tree = tree_nth (CONFIGURATION_MEMORY_TREE (config), 1);
  gui_mainwindow_sidebar_populate (root_tree);

  gtk_range_set_range (GTK_RANGE (timeline), 1, ps_serie->num_time_series);

  MemoryImageOrientation e_Orientation;
  v_memory_serie_MatrixToOrientation(ps_serie->pt_RotationMatrix, &ps_serie->e_ImageDirection_I, &ps_serie->e_ImageDirection_J, &ps_serie->e_ImageDirection_K);
  e_Orientation = e_memory_serie_ConvertImageDirectionToOrientation(ps_serie->e_ImageDirection_I, ps_serie->e_ImageDirection_J, ps_serie->e_ImageDirection_K);

  char *pc_Axial_Top   = NULL, *pc_Axial_Bottom   = NULL, *pc_Axial_Left   = NULL, *pc_Axial_Right   = NULL;
  char *pc_Sagital_Top = NULL, *pc_Sagital_Bottom = NULL, *pc_Sagital_Left = NULL, *pc_Sagital_Right = NULL;
  char *pc_Coronal_Top = NULL, *pc_Coronal_Bottom = NULL, *pc_Coronal_Left = NULL, *pc_Coronal_Right = NULL;

  Vector3D ts_Normal_Axial;
  Vector3D ts_Normal_Sagital;
  Vector3D ts_Normal_Coronal;

  Vector3D ts_Up_Axial;
  Vector3D ts_Up_Sagital;
  Vector3D ts_Up_Coronal;

  Vector3D ts_Pivot;
  ts_Pivot = memory_serie_GetPivotpoint(ps_serie);


  if (e_Orientation != ORIENTATION_UNKNOWN)
  {
    switch (e_Orientation)
    {
      case ORIENTATION_UNKNOWN :
      case ORIENTATION_AXIAL :
        pc_Axial_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Axial_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);
        pc_Axial_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);
        pc_Axial_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);

        pc_Sagital_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);
        pc_Sagital_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);
        pc_Sagital_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Sagital_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);

        pc_Coronal_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);
        pc_Coronal_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);
        pc_Coronal_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);
        pc_Coronal_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);

        te_DisplayType = VIEWPORT_TYPE_AXIAL;
        ts_Normal_Axial.x   = 0;  ts_Normal_Axial.y   = 0;  ts_Normal_Axial.z   = -1;
        ts_Up_Axial.x       = 0;  ts_Up_Axial.y       = 1;  ts_Up_Axial.z       =  0;

        ts_Normal_Sagital.x = 1;  ts_Normal_Sagital.y = 0;  ts_Normal_Sagital.z =  0;
        ts_Up_Sagital.x     = 0;  ts_Up_Sagital.y     = 0;  ts_Up_Sagital.z     =  1;

        ts_Normal_Coronal.x = 0;  ts_Normal_Coronal.y = 1;  ts_Normal_Coronal.z =  0;
        ts_Up_Coronal.x     = 0;  ts_Up_Coronal.y     = 1;  ts_Up_Coronal.z     =  0;
        break;

      case ORIENTATION_SAGITAL :
        pc_Sagital_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);
        pc_Sagital_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Sagital_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);
        pc_Sagital_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);

        pc_Axial_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);
        pc_Axial_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);
        pc_Axial_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);
        pc_Axial_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);

        pc_Coronal_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);
        pc_Coronal_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Coronal_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);
        pc_Coronal_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);

        te_DisplayType = VIEWPORT_TYPE_SAGITAL;
        ts_Normal_Axial.x   = 0;  ts_Normal_Axial.y   = -1; ts_Normal_Axial.z   =  0;
        ts_Up_Axial.x       = 1;  ts_Up_Axial.y       =  0; ts_Up_Axial.z       =  0;

        ts_Normal_Sagital.x = 0;  ts_Normal_Sagital.y =  0; ts_Normal_Sagital.z = -1;
        ts_Up_Sagital.x     = 0;  ts_Up_Sagital.y     =  1; ts_Up_Sagital.z     =  0;

        ts_Normal_Coronal.x = 1;  ts_Normal_Coronal.y =  0; ts_Normal_Coronal.z =  0;
        ts_Up_Coronal.x     = 0;  ts_Up_Coronal.y     =  1; ts_Up_Coronal.z     =  0;
        break;

      case ORIENTATION_CORONAL :
        pc_Coronal_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Coronal_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);
        pc_Coronal_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);
        pc_Coronal_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);


        pc_Sagital_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_LAST);
        pc_Sagital_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_J,DIRECTION_PART_FIRST);
        pc_Sagital_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);
        pc_Sagital_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);

        pc_Axial_Top    = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_LAST);
        pc_Axial_Bottom = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_K,DIRECTION_PART_FIRST);
        pc_Axial_Left   = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_FIRST);
        pc_Axial_Right  = pc_memory_serie_direction_string(ps_serie->e_ImageDirection_I,DIRECTION_PART_LAST);

        te_DisplayType = VIEWPORT_TYPE_CORONAL;
        ts_Normal_Axial.x   = 0;  ts_Normal_Axial.y   =  1; ts_Normal_Axial.z   =  0;
        ts_Up_Axial.x       = 0;  ts_Up_Axial.y       =  1; ts_Up_Axial.z       =  0;

        ts_Normal_Sagital.x = 1;  ts_Normal_Sagital.y =  0; ts_Normal_Sagital.z =  0;
        ts_Up_Sagital.x     = 1;  ts_Up_Sagital.y     =  0; ts_Up_Sagital.z     =  0;

        ts_Normal_Coronal.x = 0;  ts_Normal_Coronal.y =  0; ts_Normal_Coronal.z = -1;
        ts_Up_Coronal.x     = 0;  ts_Up_Coronal.y     =  1; ts_Up_Coronal.z     =  0;
        break;
      default: break;
    }
  }

  if (pll_Viewers == NULL)
  {
    Viewer *viewer = NULL;


    Plugin *active_plugin = ps_active_draw_tool;

    /*--------------------------------------------------------------------------.
     | AXIAL VIEWER WIDGET                                                      |
     '--------------------------------------------------------------------------*/
    viewer = viewer_new (ps_serie, ps_mask, NULL, ts_Normal_Axial, ts_Pivot, ts_Up_Axial);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_AXIAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      viewer_set_image_orientation_direction(viewer, pc_Axial_Top, pc_Axial_Bottom, pc_Axial_Left, pc_Axial_Right);

      if (active_plugin != NULL)
        viewer_set_active_painter (viewer, active_plugin);

      axial_embed = VIEWER_WIDGET (viewer);
    }

    /*--------------------------------------------------------------------------.
     | SAGITAL VIEWER WIDGET                                                    |
     '--------------------------------------------------------------------------*/
    viewer = viewer_new (ps_serie, ps_mask, NULL, ts_Normal_Sagital, ts_Pivot, ts_Up_Sagital);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_SAGITAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      viewer_set_image_orientation_direction(viewer, pc_Sagital_Top, pc_Sagital_Bottom, pc_Sagital_Left, pc_Sagital_Right);

      if (active_plugin != NULL)
        viewer_set_active_painter (viewer, active_plugin);

      sagital_embed = VIEWER_WIDGET (viewer);
    }

    /*--------------------------------------------------------------------------.
     | CORONAL VIEWER WIDGET                                                    |
     '--------------------------------------------------------------------------*/
    viewer = viewer_new (ps_serie, ps_mask, NULL, ts_Normal_Coronal, ts_Pivot, ts_Up_Coronal);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_CORONAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      viewer_set_image_orientation_direction(viewer, pc_Coronal_Top, pc_Coronal_Bottom, pc_Coronal_Left, pc_Coronal_Right);

      if (active_plugin != NULL)
        viewer_set_active_painter (viewer, active_plugin);

      coronal_embed = VIEWER_WIDGET (viewer);
    }

    /*--------------------------------------------------------------------------.
     | PACKING FOR DISPLAYMENT                                                  |
     '--------------------------------------------------------------------------*/
    if (te_DisplayType == VIEWPORT_TYPE_UNDEFINED)
      te_DisplayType = VIEWPORT_TYPE_THREEWAY;

    gtk_box_pack_start (GTK_BOX (hbox_viewers), axial_embed, 1, 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox_viewers), sagital_embed, 1, 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox_viewers), coronal_embed, 1, 1, 0);

    gtk_widget_show_all (GTK_WIDGET (hbox_viewers));


    /*--------------------------------------------------------------------------.
     | KEEP PREVIOUS VIEWPORT SETTINGS                                          |
     '--------------------------------------------------------------------------*/
    gtk_widget_set_visible (GTK_WIDGET (axial_embed), FALSE);
    gtk_widget_set_visible (GTK_WIDGET (sagital_embed), FALSE);
    gtk_widget_set_visible (GTK_WIDGET (coronal_embed), FALSE);

    switch (te_DisplayType)
    {
    case VIEWPORT_TYPE_AXIAL:
      gtk_widget_set_visible (GTK_WIDGET (axial_embed), TRUE);

      break;
    case VIEWPORT_TYPE_SAGITAL:
      gtk_widget_set_visible (GTK_WIDGET (sagital_embed), TRUE);
      break;
    case VIEWPORT_TYPE_CORONAL:
      gtk_widget_set_visible (GTK_WIDGET (coronal_embed), TRUE);
      break;
    case VIEWPORT_TYPE_THREEWAY:
      gtk_widget_set_visible (GTK_WIDGET (axial_embed), TRUE);
      gtk_widget_set_visible (GTK_WIDGET (sagital_embed), TRUE);
      gtk_widget_set_visible (GTK_WIDGET (coronal_embed), TRUE);
      break;
    default: break;
    }

    gui_mainwindow_save_undo_step (hbox_viewers, NULL);

    // Display a slider for time ps_series if applicable.
    if (ps_serie->num_time_series > 1)
    {
      gtk_scale_clear_marks (GTK_SCALE (timeline));
      gtk_range_set_range (GTK_RANGE (timeline), 1, ps_serie->num_time_series);
      gtk_range_set_value (GTK_RANGE (timeline), 1);
      gtk_widget_show (timeline);
    }
    else
    {
      gtk_widget_hide (timeline);
    }

    // Change the active layer on each Viewer.
    List *viewers = list_nth (pll_Viewers, 1);
    while (viewers != NULL)
    {
      viewer_set_active_layer_serie (viewers->data, ps_serie);
      viewers = list_next (viewers);
    }
  }
  else
  {
    Vector3D ts_Normal;
    Vector3D ts_Up;

    List *temp = list_nth (pll_Viewers, 1);
    while (temp != NULL)
    {
      Viewer* list_viewer = temp->data;

      switch (VIEWER_ORIENTATION (list_viewer))
      {
        case ORIENTATION_AXIAL:
          ts_Up = ts_Up_Axial;
          ts_Normal = ts_Normal_Axial;

          viewer_set_image_orientation_direction(list_viewer, pc_Axial_Top, pc_Axial_Bottom, pc_Axial_Left, pc_Axial_Right);
          break;
        case ORIENTATION_SAGITAL:
          ts_Up = ts_Up_Sagital;
          ts_Normal = ts_Normal_Sagital;
          viewer_set_image_orientation_direction(list_viewer, pc_Sagital_Top, pc_Sagital_Bottom, pc_Sagital_Left, pc_Sagital_Right);
          break;
        case ORIENTATION_CORONAL:
          ts_Up = ts_Up_Coronal;
          ts_Normal = ts_Normal_Coronal;
          viewer_set_image_orientation_direction(list_viewer, pc_Coronal_Top, pc_Coronal_Bottom, pc_Coronal_Left, pc_Coronal_Right);
          break;
        default:
          break;
      }

      viewer_initialize (list_viewer, ps_serie, ps_mask, NULL, ts_Normal, ts_Pivot, ts_Up);

      viewer_refresh_data (list_viewer);
      viewer_redraw (list_viewer, REDRAW_ALL);

      temp = temp->next;
    }
  }
}

gboolean
gui_mainwindow_reset_viewport ()
{
  // Make sure the user cannot reset the viewport while it's resetting.
  gtk_widget_set_sensitive (btn_reset_viewport, FALSE);

  // Reset the viewport.
  gui_mainwindow_load_serie (CONFIGURATION_ACTIVE_SERIE_TREE(config));

  // Release the "lock" for resetting the viewport.
  gtk_widget_set_sensitive (btn_reset_viewport, TRUE);

  return !GDK_EVENT_STOP;
}

char*
gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action)
{
  debug_functions ();

  char *pc_Filename = NULL;
  GtkWidget *dialog = NULL;

  /*--------------------------------------------------------------------------.
   | OPEN A FILE                                                              |
   '--------------------------------------------------------------------------*/
  if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
    dialog = gtk_file_chooser_dialog_new ("Open file",
               GTK_WINDOW (parent), action,
               "Cancel", GTK_RESPONSE_CANCEL,
               "Open", GTK_RESPONSE_ACCEPT, NULL);

  /*--------------------------------------------------------------------------.
   | SAVE A FILE                                                              |
   '--------------------------------------------------------------------------*/
  else if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
    dialog = gtk_file_chooser_dialog_new ("Save file",
	       GTK_WINDOW (parent), action,
               "Cancel", GTK_RESPONSE_CANCEL,
	       "Save", GTK_RESPONSE_ACCEPT, NULL);

  /*--------------------------------------------------------------------------.
   | RUN THE DIALOG WINDOW                                                    |
   '--------------------------------------------------------------------------*/
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    pc_Filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

  gtk_widget_destroy (dialog);
  return pc_Filename;
}


gboolean
gui_mainwindow_file_export ()
{
  debug_functions ();

  Tree *p_iter;
  Serie *ps_serie;

  p_iter = tree_child (CONFIGURATION_ACTIVE_STUDY_TREE(config));
  while (p_iter != NULL)
  {
    ps_serie = p_iter->data;

    // Filter out the series that shouldn't be exported.
    if (ps_serie == NULL || p_iter->type != TREE_TYPE_SERIE_MASK)
    {
      p_iter = tree_next (p_iter);
      continue;
    }

    memory_io_save_file (ps_serie, ps_serie->pc_filename);
    p_iter = tree_next (p_iter);
  }

  gtk_label_set_text (GTK_LABEL (lbl_info), "The files have been saved.");

  return FALSE;
}


gboolean
gui_mainwindow_toggle_follow_mode ()
{
  debug_functions ();

  if (pll_Viewers == NULL) return 0;

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    viewer_set_follow_mode (temp->data, !viewer_get_follow_mode (temp->data));
    temp = list_next (temp);
  }

  return !GDK_EVENT_STOP;
}


gboolean
gui_mainwindow_toggle_auto_close ()
{
  debug_functions ();

  if (pll_Viewers == NULL) return 0;

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    viewer_set_auto_close (temp->data, !viewer_get_auto_close (temp->data));
    temp = list_next (temp);
  }

  return !GDK_EVENT_STOP;
}


void
gui_mainwindow_refresh_viewers (Viewer *viewer, UNUSED void *data)
{
  debug_functions ();

  ps_active_viewer = viewer;

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    Viewer* list_viewer = temp->data;

    // Skip the Viewer that is calling back.
    if (list_viewer != viewer)
      viewer_redraw (list_viewer, REDRAW_MINIMAL);

    temp = temp->next;
  }
}


void
gui_mainwindow_update_viewer_positions (Viewer *viewer, void *data)
{
  debug_functions ();

  ps_active_viewer = viewer;

  if (viewer->b_FollowMode_Enabled)
  {
    Vector3D ts_Pivot;

    Coordinate *ts_Position = (Coordinate *)data;
    Slice *slice = VIEWER_ACTIVE_SLICE (viewer);

    ts_Pivot = memory_serie_GetPivotpoint(slice->serie);

    short int i16_SliceNumber_Axial = 0;
    short int i16_SliceNumber_Sagital = 0;
    short int i16_SliceNumber_Coronal = 0;

    MemoryImageOrientation orientation = viewer_get_orientation (viewer);
    switch (orientation)
    {
      case ORIENTATION_AXIAL:
        i16_SliceNumber_Axial = slice->matrix.i16_z;
        i16_SliceNumber_Sagital = ts_Pivot.x - ts_Position->x;
        i16_SliceNumber_Coronal = ts_Pivot.y - ts_Position->y;
        break;
      case ORIENTATION_SAGITAL:
        i16_SliceNumber_Sagital = slice->matrix.i16_z;
        i16_SliceNumber_Axial =  ts_Pivot.z - ts_Position->y;
        i16_SliceNumber_Coronal =  ts_Pivot.x - ts_Position->x;
        break;
      case ORIENTATION_CORONAL:
        i16_SliceNumber_Coronal = slice->matrix.i16_z;
        i16_SliceNumber_Sagital = ts_Pivot.x - ts_Position->x;
        i16_SliceNumber_Axial = ts_Pivot.z - ts_Position->y;
        break;
      default:
        break;
    }

    List *temp = list_nth (pll_Viewers, 1);
    while (temp != NULL)
    {
      Viewer* list_viewer = temp->data;

      // Skip the Viewer that is calling back.
      if (VIEWER_ORIENTATION (list_viewer) != VIEWER_ORIENTATION (viewer))
      {
        switch (VIEWER_ORIENTATION (list_viewer))
        {
          case ORIENTATION_AXIAL:
            viewer_set_slice (list_viewer, i16_SliceNumber_Axial);
            break;
          case ORIENTATION_SAGITAL:
            viewer_set_slice (list_viewer, i16_SliceNumber_Sagital);
            break;
          case ORIENTATION_CORONAL:
            viewer_set_slice (list_viewer, i16_SliceNumber_Coronal);
            break;
          default:
            break;
        }
      }

      temp = temp->next;
    }
  }
}


void
gui_mainwindow_update_viewer_wwwl (Viewer *viewer, void *data)
{
  debug_functions ();

  Serie  *ps_serie;
  Viewer *list_viewer;

  ps_active_viewer = viewer;

  if (data == NULL) return;

  ps_serie=CONFIGURATION_ACTIVE_MASK(config);
  if (ps_serie == NULL) return;

  WWWL wwwl = *(WWWL *)data;
  List *pl_iter = list_nth (pll_Viewers, 1);
  while (pl_iter != NULL)
  {
    list_viewer = pl_iter->data;

    // Skip the Viewer that is calling back.
    if (VIEWER_ORIENTATION (list_viewer) != VIEWER_ORIENTATION (viewer))
    {
      viewer_set_window_level_for_serie (list_viewer, ps_serie, wwwl.i32_windowWidth, wwwl.i32_windowLevel);
    }

    pl_iter = pl_iter->next;
  }

}




void
gui_mainwindow_update_handle_position (Viewer *viewer, void *data)
{
  float ratio = *(float *)data;

  debug_functions ();

  assert (viewer != NULL);

  Viewer *axial = NULL;
  Viewer *sagital = NULL;
  Viewer *coronal = NULL;
  Vector3D ts_rotationVector;
  Vector3D ts_NormalVector;

  float f_angle = atan (ratio);

  //printf ("Angle in radian: %f\n",f_angle);

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    Viewer* list_viewer = temp->data;

    // Skip the Viewer that is calling back.
    if (VIEWER_ORIENTATION (list_viewer) != VIEWER_ORIENTATION (viewer))
    {
      switch (VIEWER_ORIENTATION (list_viewer))
      {
        case ORIENTATION_AXIAL:
          axial = list_viewer;
          break;
        case ORIENTATION_SAGITAL:
          sagital = list_viewer;
          break;
        case ORIENTATION_CORONAL:
          coronal = list_viewer;
          break;
        default:
          break;
      }
    }

    temp = temp->next;
  }

  switch (VIEWER_ORIENTATION (viewer))
  {
    case ORIENTATION_AXIAL:
    {
      ts_NormalVector=ts_algebra_vector_Rotation_around_X_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_Z_Axis(&ts_NormalVector,f_angle);

      sagital->ts_NormalVector.x = ts_rotationVector.x;
      sagital->ts_NormalVector.y = ts_rotationVector.y;
      sagital->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(sagital);
      viewer_redraw (sagital, REDRAW_MINIMAL);

      ts_NormalVector=ts_algebra_vector_Rotation_around_X_Axis(&viewer->ts_NormalVector,-M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_Z_Axis(&ts_NormalVector,f_angle);

      coronal->ts_NormalVector.x = ts_rotationVector.x;
      coronal->ts_NormalVector.y = ts_rotationVector.y;
      coronal->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(coronal);
      viewer_redraw (coronal, REDRAW_MINIMAL);
    }
    break;
  case ORIENTATION_SAGITAL:
    {
      ts_NormalVector=ts_algebra_vector_Rotation_around_X_Axis(&viewer->ts_NormalVector,-M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      axial->ts_NormalVector.x = ts_rotationVector.x;
      axial->ts_NormalVector.y = ts_rotationVector.y;
      axial->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(axial);
      viewer_redraw (axial, REDRAW_MINIMAL);

      ts_NormalVector=ts_algebra_vector_Rotation_around_Z_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      coronal->ts_NormalVector.x = ts_rotationVector.x;
      coronal->ts_NormalVector.y = ts_rotationVector.y;
      coronal->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(coronal);
      viewer_redraw (coronal, REDRAW_MINIMAL);
    }
    break;
  case ORIENTATION_CORONAL:
    {
      ts_NormalVector=ts_algebra_vector_Rotation_around_X_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      axial->ts_NormalVector.x=ts_rotationVector.x;
      axial->ts_NormalVector.y=ts_rotationVector.y;
      axial->ts_NormalVector.z=ts_rotationVector.z;

      viewer_refresh_data(axial);
      viewer_redraw (axial, REDRAW_MINIMAL);

      ts_NormalVector=ts_algebra_vector_Rotation_around_Z_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_algebra_vector_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      sagital->ts_NormalVector.x=ts_rotationVector.x;
      sagital->ts_NormalVector.y=ts_rotationVector.y;
      sagital->ts_NormalVector.z=ts_rotationVector.z;

      viewer_refresh_data(sagital);
      viewer_redraw (sagital, REDRAW_MINIMAL);
    }
    break;
  default:
    break;
  }
}


gboolean
gui_mainwindow_on_key_press (UNUSED GtkWidget *widget, GdkEventKey *event,
			     UNUSED void *data)
{
  debug_functions ();
  debug_events ();

  Serie *ps_mask;

  /*--------------------------------------------------------------------------.
   | KEY RESPONSES                                                            |
   '--------------------------------------------------------------------------*/

  if (event->keyval == CONFIGURATION_KEY (config, KEY_TOGGLE_FOLLOW))
  {
    gboolean state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chk_follow));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chk_follow), !state);
  }

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_TOGGLE_AUTOCLOSE))
  {
    gboolean state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chk_auto_close));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chk_auto_close), !state);
  }

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_TOGGLE_SIDEBAR)
	   && (event->state & GDK_CONTROL_MASK))
    gui_mainwindow_sidebar_toggle (btn_sidebar_toggle, NULL);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_VIEW_AXIAL))
    gui_mainwindow_views_activate (views_combo, (void *)VIEWPORT_TYPE_AXIAL);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_VIEW_SAGITAL))
    gui_mainwindow_views_activate (views_combo, (void *)VIEWPORT_TYPE_SAGITAL);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_VIEW_CORONAL))
    gui_mainwindow_views_activate (views_combo, (void *)VIEWPORT_TYPE_CORONAL);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_VIEW_SPLIT))
    gui_mainwindow_views_activate (views_combo, (void *)VIEWPORT_TYPE_THREEWAY);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_TOGGLE_RECORDING)
           && ps_active_viewer != NULL)
    viewer_toggle_recording (ps_active_viewer);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_RECORDING)
           && ps_active_viewer != NULL)
    viewer_replay_recording (ps_active_viewer);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_OVER_TIME)
           && ps_active_viewer != NULL)
    viewer_replay_recording_over_time (ps_active_viewer);
  /*
  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_OVER_SLICES)
           && ps_active_viewer != NULL)
    viewer_replay_recording_over_slices (ps_active_viewer);
  */
  else if ((event->keyval == CONFIGURATION_KEY (config, KEY_UNDO)) &&
           (event->state & GDK_CONTROL_MASK))
  {
    ps_mask=CONFIGURATION_ACTIVE_MASK(config);
    if (ps_mask != NULL)
    {
      pll_History = common_history_load_state (pll_History, HISTORY_PREVIOUS, &ps_mask->data);
    }
  }

  else if ((event->keyval == CONFIGURATION_KEY (config, KEY_REDO)) &&
           (event->state & GDK_CONTROL_MASK))
  {
    ps_mask=CONFIGURATION_ACTIVE_MASK(config);
    if (ps_mask != NULL)
    {
      pll_History = common_history_load_state (pll_History, HISTORY_NEXT, &ps_mask->data);
    }
  }

  /*--------------------------------------------------------------------------.
   | OTHER STUFF                                                              |
   '--------------------------------------------------------------------------*/

  if (ps_active_viewer != NULL)
    viewer_on_key_press (ps_active_viewer, event);

  if (pll_History != NULL
      && event->keyval != CONFIGURATION_KEY (config, KEY_TOGGLE_FOLLOW))
  {
    gui_mainwindow_redisplay_viewers (GUI_DO_REDRAW);
  }

  return FALSE;
}


gboolean
gui_mainwindow_on_key_release (UNUSED GtkWidget *widget, GdkEventKey *event,
			       UNUSED void *data)
{
  debug_functions ();
  debug_events ();

  if (ps_active_viewer != NULL)
    viewer_on_key_release (ps_active_viewer, event);

  return FALSE;
}


gboolean
gui_mainwindow_on_timeline_change (GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();
  debug_events ();

  unsigned short int u16_Value = gtk_range_get_value (GTK_RANGE (widget));
  u16_Value--;

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    Viewer* list_viewer = temp->data;
    viewer_set_timepoint (list_viewer,u16_Value);
    viewer_refresh_data (list_viewer);
    viewer_redraw (list_viewer, REDRAW_ALL);

    temp = temp->next;
  }

  return FALSE;
}


gboolean
gui_mainwindow_select_tool (GtkWidget *widget, void *data)
{
  debug_functions ();

  Plugin *active_plugin = (Plugin *)data;
  ps_active_draw_tool = active_plugin;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_active_painter (viewers->data, active_plugin);
    viewers = viewers->next;
  }

  active_plugin->set_property (active_plugin->meta, "reset", NULL);

  unsigned int *size = active_plugin->get_property (active_plugin->meta, "size");
  if (size != NULL)
  {
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (inp_brush_size), *size);
  }

  unsigned int *value = active_plugin->get_property (active_plugin->meta, "value");
  if (value != NULL)
  {
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (inp_brush_value), *value);
  }

  gtk_widget_set_sensitive (inp_brush_size, (size != NULL));
  gtk_widget_set_sensitive (inp_brush_value, (value != NULL));

  /*--------------------------------------------------------------------------.
   | HANDLE "ACTIVE" STATE                                                    |
   '--------------------------------------------------------------------------*/

  if (btn_ActiveDrawTool != NULL)
    gtk_button_set_relief (GTK_BUTTON (btn_ActiveDrawTool), GTK_RELIEF_NONE);

  gtk_button_set_relief (GTK_BUTTON (widget), GTK_RELIEF_NORMAL);
  btn_ActiveDrawTool = widget;

  return FALSE;
}


void
gui_mainwindow_set_brush_size (UNUSED GtkWidget* widget, UNUSED void* data)
{
  debug_functions ();

  Plugin *active_plugin = ps_active_draw_tool;
  if (active_plugin == NULL) return;

  int size = gtk_spin_button_get_value (GTK_SPIN_BUTTON (inp_brush_size));
  if (!active_plugin->set_property (active_plugin->meta, "size", &size))
  {
    unsigned int *plugin_size = active_plugin->get_property (active_plugin->meta, "size");
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (inp_brush_size), *plugin_size);
  }
}


void
gui_mainwindow_set_brush_value (UNUSED GtkWidget* widget, UNUSED void* data)
{
  debug_functions ();

  Plugin *active_plugin = ps_active_draw_tool;
  if (active_plugin == NULL) return;

  int value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (inp_brush_value));

  List *plugins = pl_plugins;
  while (plugins != NULL)
  {
    Plugin *plugin = plugins->data;
    plugin->set_property (plugin->meta, "value", &value);
    plugins = list_next (plugins);
  }
}


void
gui_mainwindow_clear_viewers ()
{
  debug_functions ();

  // Remove Viewer objects.
  list_free_all (pll_Viewers, viewer_destroy);
  pll_Viewers = NULL;

  // Remove undo-history.
  list_free_all (pll_History, common_history_destroy_element);
  pll_History = NULL;
}


void
gui_mainwindow_destroy ()
{
  debug_functions ();

  gui_mainwindow_clear_viewers ();
  gui_mainwindow_sidebar_destroy ();

  memory_tree_destroy (CONFIGURATION_MEMORY_TREE (config));


  list_free_all (pl_pixeldata_lookup_table_get_list(),pixeldata_lookup_table_destroy_item);

  list_free_all (pl_plugins, pixeldata_plugin_destroy);
  list_free_all (pll_Viewers, viewer_destroy);
  list_free_all (pll_History, common_history_destroy_element);

  g_object_ref_sink (window);
  gtk_widget_destroy (window);
  g_object_unref (window);

  gtk_main_quit ();
}


/******************************************************************************
 * TOP BAR
 ******************************************************************************/


void
gui_mainwindow_add_plugin (Plugin *plugin, GtkWidget *box)
{
  debug_functions ();

  char **icon_name;
  char *icon_path;

  icon_name = plugin->get_property (plugin->meta, "name");
  icon_path = calloc (1, strlen (PLUGIN_ICON_PATH) + strlen (*icon_name) + 5);
  sprintf (icon_path, PLUGIN_ICON_PATH "%s.png", *icon_name);

  GtkWidget *btn_plugin;
  if (access (icon_path, F_OK) == 0)
  {
    btn_plugin = gtk_button_new ();

    GtkWidget *img_plugin = gtk_image_new_from_file (icon_path);
    gtk_button_set_image (GTK_BUTTON (btn_plugin), img_plugin);
    gtk_button_set_relief (GTK_BUTTON (btn_plugin), GTK_RELIEF_NONE);
  }
  else
  {
    btn_plugin = gtk_button_new_with_label (*icon_name);
  }

  free (icon_path);

  gtk_box_pack_start (GTK_BOX (box), btn_plugin, 0, 0, 0);
  gtk_widget_show (btn_plugin);

  g_signal_connect (btn_plugin, "clicked",
		    G_CALLBACK (gui_mainwindow_select_tool),
		    plugin);

  gui_mainwindow_select_tool (btn_plugin, plugin);
}


GtkWidget*
gui_mainwindow_toolbar_new ()
{
  debug_functions ();

  GtkWidget *hbox_toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /*--------------------------------------------------------------------------.
   | LOAD PLUGINS                                                             |
   '--------------------------------------------------------------------------*/
  pixeldata_plugin_load_from_directory (PLUGIN_PATH, &pl_plugins);

  /*--------------------------------------------------------------------------.
   | SET STANDARD BRUSH STUFF                                                 |
   '--------------------------------------------------------------------------*/
  GtkWidget* lbl_brush_size = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (lbl_brush_size), "<b>Size:</b>");
  inp_brush_size = gtk_spin_button_new_with_range (1.0, 500.0, 1.0);

  gtk_box_pack_start (GTK_BOX (hbox_toolbar), lbl_brush_size, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_toolbar), inp_brush_size, 0, 0, 5);

  g_signal_connect (inp_brush_size, "value-changed",
		    G_CALLBACK (gui_mainwindow_set_brush_size),
		    NULL);

  GtkWidget* lbl_brush_value = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (lbl_brush_value), "<b>Value:</b>");
  inp_brush_value = gtk_spin_button_new_with_range (1.0, 255.0, 1.0);

  gtk_box_pack_start (GTK_BOX (hbox_toolbar), lbl_brush_value, 0, 0, 5);
  gtk_box_pack_start (GTK_BOX (hbox_toolbar), inp_brush_value, 0, 0, 5);

  g_signal_connect (inp_brush_value, "value-changed",
		    G_CALLBACK (gui_mainwindow_set_brush_value),
		    NULL);

  /*--------------------------------------------------------------------------.
   | ADD PLUGINS TO MENU BAR                                                  |
   '--------------------------------------------------------------------------*/
  List * pll_iter;
  pll_iter = pl_plugins;
  while (pll_iter != NULL)
  {
    Plugin *plugin = pll_iter->data;
    gui_mainwindow_add_plugin (plugin, hbox_toolbar);

    pll_iter = list_next (pll_iter);
  }

  return hbox_toolbar;
}

/*
gboolean
gui_mainwindow_histogram_draw (UNUSED GtkWidget *widget, cairo_t *cr)
{
  if (histogram == NULL) return FALSE;
  histogram_draw (histogram, cr, 200, 300, 0, 0);
  return TRUE;
}
*/


void
gui_mainwindow_sanitize_string (char *pc_String)
{
  debug_functions ();

  if (pc_String == NULL) return;

  unsigned int i = 0;
  while (i < strlen (pc_String))
  {
    #ifdef WIN32
    if (pc_String[i] == '\\') pc_String[i] = '/';
    #endif

    if (pc_String[i] == '\n') pc_String[i] = '.';
    if (pc_String[i] == '\0') break;
    i++;
  }
}

char* stristr(const char* haystack, const char* needle)
{
  do
  {
    const char* h = haystack;
    const char* n = needle;
    while (tolower((unsigned char) *h) == tolower((unsigned char ) *n) && *n)
    {
      h++;
      n++;
    }
    if (*n == 0)
    {
      return (char *) haystack;
    }
  } while (*haystack++);
  return 0;
}

void
gui_mainwindow_check_extention (char *pc_String)
{
  debug_functions ();

  if (pc_String == NULL) return;

  if (stristr (pc_String, ".nii") == 0)
  {
     // add extention
     if (strlen (pc_String) < 96) // max string size of name in serie struct
     {
       strcat(pc_String, ".nii");
     }
     else
     {
       strcat(&pc_String[95], ".nii");
     }
  }
  else
  {
    strncpy(&pc_String[strlen (pc_String)-4],".nii",4);
  }
}

gboolean gui_mainwindow_mask_add (unsigned long long ull_serieID )
{
  debug_functions ();
  Tree *pt_maskSerie = NULL;
  Tree *pt_origSerie = NULL;
  Serie *ps_maskSerie = NULL;

  pt_origSerie = memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);
  pt_maskSerie = memory_tree_add_mask_for_serie (pt_origSerie);

  if (pt_maskSerie == NULL)
  {
    return FALSE;
  }

  CONFIGURATION_ACTIVE_MASK (config) = (Serie*)(pt_maskSerie->data);

  ps_maskSerie = pt_maskSerie->data;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_add_mask_serie (viewers->data, ps_maskSerie);
    viewer_set_active_mask_serie (viewers->data, ps_maskSerie);
    viewers = list_next (viewers);
  }
  return TRUE;
}

gboolean gui_mainwindow_mask_remove (unsigned long long ull_serieID )
{
  debug_functions ();

  debug_functions ();
  Tree *pt_origSerie=NULL;
  Tree *pt_maskSerie=NULL;
  Tree *pt_iter=NULL;

  Serie *ps_origSerie=NULL;
  Serie *ps_maskSerie=NULL;
  Serie *ps_iterSerie=NULL;

  pt_maskSerie = memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);

  if (pt_maskSerie == NULL)
  {
    return FALSE;
  }

  ps_maskSerie=(Serie*)(pt_maskSerie->data);

  // Search Orig Serie of mask
  pt_iter=tree_nth(pt_maskSerie,1);

  while (pt_iter != NULL)
  {
    ps_iterSerie = (Serie*)(pt_iter->data);

    if ((ps_iterSerie->e_SerieType == SERIE_ORIGINAL) && ( ps_iterSerie->group_id == ps_maskSerie->group_id))
    {
      ps_origSerie = ps_iterSerie;
      pt_origSerie = pt_iter;
      break;
    }
    pt_iter=tree_next(pt_iter);
  }
  if ((pt_origSerie == NULL) || (ps_origSerie == NULL))
  {
    return FALSE;
  }

  // CLEAR ALL
  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_remove_mask_serie (viewers->data, pt_maskSerie->data);
    viewers = list_next (viewers);
  }

  memory_serie_destroy (pt_maskSerie->data);
  pt_maskSerie->data = NULL;

  pt_maskSerie = tree_remove (pt_maskSerie);
  pt_maskSerie= NULL;

  // Find first mask in serie tree and set it to active
  pt_iter=tree_nth(pt_origSerie,1);
  while (pt_iter != NULL)
  {
    ps_iterSerie = (Serie*)(pt_iter->data);
    if ((ps_iterSerie->e_SerieType == SERIE_MASK) && ( ps_iterSerie->group_id == ps_origSerie->group_id))
    {
      break;
    }

    pt_iter=tree_next(pt_iter);
  }

  if (pt_iter == NULL)
  {
    // No mask found, create new one
    return gui_mainwindow_mask_add(ps_origSerie->id);
  }


  CONFIGURATION_ACTIVE_MASK (config) = (Serie*)(pt_iter->data);

  return TRUE;
}

gboolean gui_mainwindow_mask_set_active (unsigned long long ull_serieID )
{
  debug_functions ();
  Tree *pt_mask=NULL;

  Serie *ps_serie=NULL;
  Serie *ps_mask=NULL;

  pt_mask = memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);
  if (pt_mask == NULL) return FALSE;


  CONFIGURATION_ACTIVE_MASK(config) = (Serie*)(pt_mask->data);

  ps_serie=CONFIGURATION_ACTIVE_SERIE(config);

  if (ps_mask->group_id != ps_serie->group_id)
  {
    return FALSE;
  }

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_active_mask_serie (viewers->data, ps_mask);
    viewers = list_next (viewers);
  }

  return TRUE;
}

gboolean
gui_mainwindow_overlay_load (unsigned long long ull_serieID, void *data)
{
  char* pc_path = NULL;
  Tree *pt_Study = NULL;
  Tree *pt_serie = NULL;

  Serie *ps_Overlay = NULL;
  Serie *ps_Original = NULL;

  debug_functions ();

  pc_path = (data != NULL)
    ? (char*)(data)
    : gui_mainwindow_file_dialog (window, GTK_FILE_CHOOSER_ACTION_OPEN);

  if (pc_path != NULL)
  {
    pt_serie=memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);

    if(pt_serie == NULL)
    {
      return FALSE;
    }

    pt_Study=pt_serie->parent;

    ps_Original = pt_serie->data;
    pt_serie=pt_memory_io_load_file(&pt_Study,pc_path);

    if (pt_serie == NULL)
    {
      return FALSE;
    }

    pt_serie->type=TREE_TYPE_SERIE_OVERLAY;

    ps_Overlay=(Serie *)(pt_serie->data);
    ps_Overlay->e_SerieType=SERIE_OVERLAY;
    ps_Overlay->group_id = ps_Original->group_id;

    List *viewers = list_nth (pll_Viewers, 1);
    while (viewers != NULL)
    {
      viewer_add_overlay_serie (viewers->data, ps_Overlay);
      viewers = list_next (viewers);
    }

    return TRUE;
  }

  return FALSE;
}

gboolean gui_mainwindow_overlay_remove (unsigned long long ull_serieID)
{
  debug_functions ();
  Tree *pt_serie=NULL;
  Serie *ps_serie=NULL;

  pt_serie = memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);

  if (pt_serie == NULL)
  {
    return FALSE;
  }

  ps_serie = pt_serie->data;
  if (ps_serie == NULL)
  {
    return FALSE;
  }

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_remove_overlay_serie (viewers->data, ps_serie);
    viewers = list_next (viewers);
  }

  memory_serie_destroy (ps_serie);
  ps_serie = NULL;

  pt_serie = tree_remove (pt_serie);
  assert (pt_serie != NULL);

  return TRUE;
}

/******************************************************************************
 * TREE VIEW (SIDE BAR)
 ******************************************************************************/


gboolean
gui_mainwindow_sidebar_toggle (GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  if (gtk_widget_is_visible (sidebar))
  {
    gtk_widget_hide (sidebar);
    GtkWidget *image = gtk_image_new_from_icon_name (ICON_SIDEBAR_SHOW,
                                                     GTK_ICON_SIZE_BUTTON);

    gtk_button_set_image (GTK_BUTTON (widget), image);
  }
  else
  {
    gtk_widget_show (sidebar);
    GtkWidget *image = gtk_image_new_from_icon_name (ICON_SIDEBAR_HIDE,
                                                     GTK_ICON_SIZE_BUTTON);

    gtk_button_set_image (GTK_BUTTON (widget), image);
  }

  return FALSE;
}


void
gui_mainwindow_sidebar_populate (Tree *pll_Patients)
{
  debug_functions ();

  if (sidebar_TreeStore == NULL) return;

  // Avoid duplication.
  gtk_tree_store_clear (sidebar_TreeStore);

  // Populate the store.
  Tree *pll_Studies;
  Tree *pt_series;
  Tree *pt_Masks;
  Tree *pt_Overlays;

  Serie *ps_active_mask;


  GtkTreeIter PatientIterator;
  GtkTreeIter StudyIterator;
  GtkTreeIter SerieIterator;
  GtkTreeIter MaskOverlayIterator;
  GtkTreeIter SubMaskOverlayIterator;

  GdkPixbuf *p_icon=NULL;
  GdkPixbuf *p_radioBtn=NULL;
  GtkIconTheme *p_icon_theme = gtk_icon_theme_get_default();

  /*--------------------------------------------------------------------------.
   | PATIENTS                                                                 |
   '--------------------------------------------------------------------------*/

  while (pll_Patients != NULL)
  {
    Patient *patient = pll_Patients->data;
    if (patient == NULL)
    {
      pll_Patients = tree_next (pll_Patients);
      continue;
    }

    assert (patient->name != NULL);
    assert (patient->id > 0);

    gui_mainwindow_sanitize_string (patient->name);

    gtk_tree_store_append (sidebar_TreeStore, &PatientIterator, NULL);
    gtk_tree_store_set (sidebar_TreeStore, &PatientIterator,
                        SIDEBAR_ID, 40000+patient->id,
                        SIDEBAR_NAME, patient->name,
                        SIDEBAR_NAME_EDIT, FALSE,
                        SIDEBAR_INNER, NULL,
                        SIDEBAR_OUTER, NULL,
			-1);

    /*------------------------------------------------------------------------.
     | STUDIES                                                                |
     '------------------------------------------------------------------------*/

    pll_Studies = tree_child (pll_Patients);

    while (pll_Studies != NULL)
    {
      Study *study = pll_Studies->data;
      if (study == NULL)
      {
        pll_Studies = tree_next (pll_Studies);
        continue;
      }


      gui_mainwindow_sanitize_string (study->name);
      gtk_tree_store_append (sidebar_TreeStore,
			     &StudyIterator,
			     &PatientIterator);

      gtk_tree_store_set  (sidebar_TreeStore, &StudyIterator,
                           SIDEBAR_ID, 50000+study->id,
                           SIDEBAR_NAME, study->name,
                           SIDEBAR_NAME_EDIT, FALSE,
                           SIDEBAR_INNER, NULL,
                           SIDEBAR_OUTER, NULL,
			   -1);

      /*------------------------------------------------------------------------.
      | SERIES                                                                  |
      '------------------------------------------------------------------------*/

      pt_series = tree_child(pll_Studies);

      while (pt_series != NULL)
      {
        Serie *serie = pt_series->data;
        if (serie == NULL)
        {
          pt_series = tree_next (pt_series);
          continue;
        }

        if (serie->e_SerieType == SERIE_ORIGINAL)
        {
          gui_mainwindow_sanitize_string (serie->name);
          gtk_tree_store_append (sidebar_TreeStore, &SerieIterator, &StudyIterator);
          gtk_tree_store_set (sidebar_TreeStore, &SerieIterator,
                              SIDEBAR_ID, serie->id,
                              SIDEBAR_NAME, serie->name,
                              SIDEBAR_NAME_EDIT, FALSE,
                              SIDEBAR_INNER, NULL,
                              SIDEBAR_OUTER, NULL,
			      -1);

          p_icon = gtk_icon_theme_load_icon (p_icon_theme, ICON_DOCUMENT_OPEN, 16, 0,NULL);

          /*--------------------------------------------------------------------.
          | OVERLAYS OF THAT SERIE                                              |
          '--------------------------------------------------------------------*/
          gtk_tree_store_append (sidebar_TreeStore, &MaskOverlayIterator, &SerieIterator);
          gtk_tree_store_set (sidebar_TreeStore, &MaskOverlayIterator,
                              SIDEBAR_ID, 10000+serie->id,
                              SIDEBAR_NAME, "Overlays",
                              SIDEBAR_NAME_EDIT, FALSE,
                              SIDEBAR_INNER, NULL,
                              SIDEBAR_OUTER, p_icon,
			      -1);

          pt_Overlays = tree_child (pll_Studies);
          while (pt_Overlays != NULL)
          {
            Serie *ps_Overlay = pt_Overlays->data;
            if (ps_Overlay == NULL)
            {
              pt_Overlays = tree_next (pt_Overlays);
              continue;
            }

            if ((ps_Overlay->e_SerieType == SERIE_OVERLAY) && (ps_Overlay->group_id == serie->group_id))
            {
              p_icon = gtk_icon_theme_load_icon (p_icon_theme, ICON_REMOVE, 16, 0,NULL);

              gui_mainwindow_sanitize_string (ps_Overlay->name);
              gtk_tree_store_append (sidebar_TreeStore, &SubMaskOverlayIterator, &MaskOverlayIterator);
              gtk_tree_store_set (sidebar_TreeStore, &SubMaskOverlayIterator,
                                  SIDEBAR_ID, 11000 + ps_Overlay->id,
                                  SIDEBAR_NAME, ps_Overlay->name,
                                  SIDEBAR_NAME_EDIT, FALSE,
                                  SIDEBAR_INNER, NULL,
                                  SIDEBAR_OUTER, p_icon,
				  -1);
            }
            pt_Overlays = tree_next(pt_Overlays);
          }


          /*--------------------------------------------------------------------.
          | MASKS OF THAT SERIE                                                 |
          '--------------------------------------------------------------------*/

          p_icon = gtk_icon_theme_load_icon (p_icon_theme, ICON_DOCUMENT_OPEN, 16, 0,NULL);
          p_radioBtn = gtk_icon_theme_load_icon (p_icon_theme, ICON_ADD, 16, 0,NULL);
          gtk_tree_store_append (sidebar_TreeStore, &MaskOverlayIterator, &SerieIterator);
          gtk_tree_store_set (sidebar_TreeStore, &MaskOverlayIterator,
                              SIDEBAR_ID, 12000+serie->id,
                              SIDEBAR_NAME, "Masks",
                              SIDEBAR_NAME_EDIT, FALSE,
                              SIDEBAR_INNER, p_radioBtn,
                              SIDEBAR_OUTER, p_icon,
			      -1);

          ps_active_mask=CONFIGURATION_ACTIVE_MASK(config);
          if (ps_active_mask == NULL) return;

          pt_Masks = tree_child(pll_Studies);
          while (pt_Masks != NULL)
          {


            Serie *ps_Mask = pt_Masks ->data;
            if (ps_Mask == NULL)
            {
              pt_Masks = tree_next (pt_Masks);
              continue;
            }

            if ((ps_Mask->e_SerieType == SERIE_MASK) && (ps_Mask->group_id == serie->group_id))
            {

              p_icon = gtk_icon_theme_load_icon (p_icon_theme, ICON_REMOVE, 16, 0,NULL);

              if (ps_Mask == ps_active_mask)
              {
                p_radioBtn = gtk_icon_theme_load_icon (p_icon_theme, ICON_RADIO_ENABLED, 16, 0,NULL);
              }
              else
              {
                p_radioBtn = gtk_icon_theme_load_icon (p_icon_theme, ICON_RADIO, 16, 0,NULL);
              }


              gui_mainwindow_sanitize_string (ps_Mask->name);

              gtk_tree_store_append (sidebar_TreeStore, &SubMaskOverlayIterator, &MaskOverlayIterator);

              /* Don't show a "remove" button for the active layer. */
              if (ps_Mask == ps_active_mask) p_icon = NULL;

              gtk_tree_store_set (sidebar_TreeStore, &SubMaskOverlayIterator,
                                  SIDEBAR_ID, 13000+ps_Mask->id,
                                  SIDEBAR_NAME, ps_Mask->name,
                                  SIDEBAR_NAME_EDIT, TRUE,
                                  SIDEBAR_INNER, p_icon,
                                  SIDEBAR_OUTER, p_radioBtn, -1);
            }
            pt_Masks = tree_next (pt_Masks);
          }
        }
        pt_series = tree_next (pt_series);
      }
      pll_Studies = tree_next (pll_Studies);
    }
    pll_Patients = tree_next (pll_Patients);
  }
}

void
gui_mainwindow_sidebar_clicked (GtkTreeView       *ps_tree,
                                GtkTreePath       *ps_path,
                                GtkTreeViewColumn *ps_column,
                                UNUSED void       *pv_data)
{
  unsigned long long ull_ID;
  short int b_SuccesfullAction=FALSE;
  const gchar *pc_columnName;

  Tree *pt_serie = NULL;

  // Check which collum is activated
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (sidebar_TreeStore), &iter, ps_path))
  {
    gtk_tree_model_get (GTK_TREE_MODEL (sidebar_TreeStore), &iter, SIDEBAR_ID, &ull_ID, -1);
  }

  pc_columnName = gtk_tree_view_column_get_title (ps_column);

  /*----------------------------------------------------------------------------.
   | Select column by name                                                      |
   '----------------------------------------------------------------------------*/
  if (strcmp (pc_columnName, "Name") == 0)
  {
    if (ull_ID < 10000)
    {
      pt_serie=memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_ID);

      if ((pt_serie != NULL) &&
          (pt_serie->type == TREE_TYPE_SERIE) &&
          (pt_serie != CONFIGURATION_ACTIVE_SERIE_TREE(config)))
      {
        gui_mainwindow_load_serie (pt_serie);
        gtk_tree_view_expand_all (GTK_TREE_VIEW (ps_tree));

        // Look for first mask in row and make it the default layer
      }

      b_SuccesfullAction = TRUE;

      if (pt_serie != NULL)
      {
        CONFIGURATION_ACTIVE_SERIE_TREE(config) = pt_serie;
        CONFIGURATION_ACTIVE_SERIE(config) = (Serie *)(pt_serie->data);
      }

    }

    else if ((ull_ID> 11000) && (ull_ID < 12000)) // Handle add MASK related action
    {
      ull_ID-=11000;
      b_SuccesfullAction=TRUE;
    }
    else if ((ull_ID> 13000) && (ull_ID < 14000)) // Handle add MASK related action
    {
      ull_ID-=13000;
      b_SuccesfullAction=TRUE;
    }
  }
  else if (!strcmp (pc_columnName, "outer"))
  {
    if ((ull_ID > 10000) && (ull_ID < 11000)) // Handle open Overlay
    {
      ull_ID -= 10000;
      b_SuccesfullAction = gui_mainwindow_overlay_load (ull_ID,NULL);
    }
    else if ((ull_ID > 11000) && (ull_ID < 12000)) // Handle open Overlay
    {
      ull_ID -= 11000;
      b_SuccesfullAction = gui_mainwindow_overlay_remove (ull_ID);
    }
    else if ((ull_ID > 13000) && (ull_ID < 14000)) // Handle add MASK
    {
      ull_ID -= 13000;
      b_SuccesfullAction = gui_mainwindow_mask_set_active (ull_ID);
    }
  }
  else if (!strcmp (pc_columnName, "inner"))
  {
    if ((ull_ID> 12000) && (ull_ID < 13000)) // Handle add MASK related action
    {
      ull_ID -= 12000;
      b_SuccesfullAction = gui_mainwindow_mask_add (ull_ID);
    }
    else if ((ull_ID > 13000) && (ull_ID < 14000)) // Handle add MASK related action
    {
      ull_ID -= 13000;
      b_SuccesfullAction = gui_mainwindow_mask_remove (ull_ID);
    }
  }

  if (b_SuccesfullAction)
  {
    Tree *root_tree = tree_nth (CONFIGURATION_MEMORY_TREE (config), 1);
    gui_mainwindow_sidebar_populate (root_tree);
    gtk_tree_view_expand_all (ps_tree);
    gui_mainwindow_properties_manager_refresh (ull_ID);
  }
}


void
gui_mainwindow_sidebar_realized (GtkWidget *widget)
{
  debug_functions ();

  //gtk_tree_view_expand_all (GTK_TREE_VIEW (widget));
  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (widget));
}


void
gui_mainwindow_cell_edited (UNUSED GtkCellRendererText *cell, char *pc_pathstring, char *pc_newtext, UNUSED void *pv_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;

  Tree *pt_serie = NULL;
  Serie *ps_serie = NULL;

  unsigned long long ull_ID;

  path = gtk_tree_path_new_from_string (pc_pathstring);

  if (path == NULL)
  {
    return;
  }

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (sidebar_TreeStore), &iter, path))
  {
    gtk_tree_model_get (GTK_TREE_MODEL (sidebar_TreeStore), &iter, SIDEBAR_ID, &ull_ID, -1);
  }

  if ((ull_ID > 13000) && (ull_ID < 14000)) // Handle add MASK
  {
    ull_ID-=13000;

    pt_serie=memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_ID);
    if (pt_serie != NULL)
    {
      ps_serie=(Serie *)(pt_serie->data);

      gui_mainwindow_sanitize_string(pc_newtext);
      gui_mainwindow_check_extention(pc_newtext);

      //SET new filename in string path
      if (sizeof(pc_newtext) > 100 )
      {
        strncpy(ps_serie->name,pc_newtext,100);
      }
      else
      {
        strcpy(ps_serie->name,pc_newtext);
      }


      char *pc_pathToSerie = dirname (ps_serie->pc_filename);
      char *pc_maskFileName = NULL;

      pc_maskFileName = calloc(1, strlen(pc_pathToSerie)+2+strlen(ps_serie->name));
      strcpy(pc_maskFileName,pc_pathToSerie);
      strcpy(&pc_maskFileName[strlen(pc_maskFileName)],"/");
      strcpy(&pc_maskFileName[strlen(pc_maskFileName)],ps_serie->name);

      ps_serie->pc_filename=pc_maskFileName;
      gtk_tree_store_set(sidebar_TreeStore, &iter, SIDEBAR_NAME, ps_serie->name,-1);
    }
  }
  gtk_tree_path_free (path);

}

GtkWidget*
gui_mainwindow_sidebar_new ()
{
  debug_functions ();

  /*--------------------------------------------------------------------------.
   | CONTAINER STUFFING                                                       |
   '--------------------------------------------------------------------------*/

  GtkWidget *propertiesBox = gui_mainwindow_properties_manager_new ();
  GtkWidget *wrapper = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);

  gtk_box_pack_start (GTK_BOX (wrapper), scrolled, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (wrapper), propertiesBox, FALSE, FALSE, 0);

  /*--------------------------------------------------------------------------.
   | GLOBAL TREE VIEW SETTINGS                                                |
   '--------------------------------------------------------------------------*/

  sidebar_TreeStore = gtk_tree_store_new (5, G_TYPE_UINT64, G_TYPE_STRING,
					  G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
					  GDK_TYPE_PIXBUF);

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (sidebar_TreeStore));

  g_signal_connect (treeview, "realize",
		    G_CALLBACK (gui_mainwindow_sidebar_realized), NULL);

  g_signal_connect (treeview, "row-activated",
		    G_CALLBACK (gui_mainwindow_sidebar_clicked), NULL);

  gtk_widget_set_can_focus (treeview, FALSE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  g_object_set (treeview, "activate-on-single-click", TRUE, NULL);

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /*--------------------------------------------------------------------------.
   | TEXT COLUMN                                                              |
   '--------------------------------------------------------------------------*/

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
						     "text", SIDEBAR_NAME,
						     "editable", SIDEBAR_NAME_EDIT,
						     NULL);

  gtk_tree_view_column_set_expand (column, TRUE);
  g_signal_connect (renderer, "edited",
		    G_CALLBACK (gui_mainwindow_cell_edited), NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  /*--------------------------------------------------------------------------.
   | INNER COLUMN ICON                                                        |
   '--------------------------------------------------------------------------*/

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes ("inner", renderer,
						     "pixbuf", SIDEBAR_INNER,
						     NULL);

  gtk_tree_view_column_set_fixed_width (column, 20);
  gtk_tree_view_column_set_expand (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);

  /*--------------------------------------------------------------------------.
   | OUTER COLUMN ICON                                                        |
   '--------------------------------------------------------------------------*/

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes("outer", renderer,
						    "pixbuf", SIDEBAR_OUTER,
						    NULL);

  gtk_tree_view_column_set_fixed_width (column, 20);
  gtk_tree_view_column_set_expand (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  return wrapper;
}


void
gui_mainwindow_sidebar_destroy ()
{
  debug_functions ();

  gtk_tree_store_clear (sidebar_TreeStore);
}

/******************************************************************************
 * PROPERTIES MANAGER
 ******************************************************************************/

void
gui_mainwindow_set_lookup_table (const char *lut_name)
{
  debug_functions ();

  Serie *ps_serie;

  ps_serie = CONFIGURATION_ACTIVE_MASK(config);
  if (ps_serie == NULL) return;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_lookup_table_for_serie (viewers->data, ps_serie, lut_name);
    viewers = viewers->next;
  }
}


gboolean
gui_mainwindow_lookup_table_changed (GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  char *lut_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (widget));

  // Filter out invalid requests.
  if (lut_name == NULL || !strcmp (lut_name, ""))
    return FALSE;

  gui_mainwindow_set_lookup_table (lut_name);

  return FALSE;
}

gboolean
gui_mainwindow_properties_on_opacity_change (GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  Serie *ps_serie;
  unsigned char value;


  ps_serie = CONFIGURATION_ACTIVE_MASK(config);
  if (ps_serie == NULL) return FALSE;

  value = gtk_range_get_value (GTK_RANGE (widget));

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_opacity_for_serie (viewers->data, ps_serie, value);
    viewers = viewers->next;
  }

  return FALSE;
}


GtkWidget*
gui_mainwindow_properties_manager_new ()
{
  GtkWidget *properties_manager;
  properties_manager = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  GtkWidget *properties_title_lbl = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (properties_title_lbl), "<b>Properties</b>");

  GtkWidget *opacity_lbl = gtk_label_new ("Opacity");
  gtk_widget_set_halign (opacity_lbl, GTK_ALIGN_START);

  properties_opacity_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 255, 1);
  gtk_scale_set_value_pos (GTK_SCALE (properties_opacity_scale), GTK_POS_RIGHT);
  gtk_range_set_value (GTK_RANGE (properties_opacity_scale), 255);

  g_signal_connect (properties_opacity_scale, "value-changed",
		    G_CALLBACK (gui_mainwindow_properties_on_opacity_change),
                    NULL);

  GtkWidget *lookup_table_lbl = gtk_label_new ("Lookup table");
  gtk_widget_set_halign (lookup_table_lbl, GTK_ALIGN_START);

  /*--------------------------------------------------------------------------.
   | LOOKUP TABLE COMBO BOX                                                   |
   '--------------------------------------------------------------------------*/
  properties_lookup_table_combo = gtk_combo_box_text_new ();

  List *lookup_tables = list_nth (pl_pixeldata_lookup_table_get_list(), 1);

  int index_counter = 0;
  while (lookup_tables != NULL)
  {
    PixelDataLookupTable *lut = lookup_tables->data;
    if (lut == NULL)
    {
      lookup_tables = list_next (lookup_tables);
      continue;
    }

    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (properties_lookup_table_combo), lut->name, lut->name);
    lut->index = index_counter;
    index_counter++;
    lookup_tables = list_next (lookup_tables);
  }

  g_signal_connect (properties_lookup_table_combo, "changed",
		    G_CALLBACK (gui_mainwindow_lookup_table_changed), NULL);

  /*--------------------------------------------------------------------------.
   | BOX PACKING                                                              |
   '--------------------------------------------------------------------------*/
  gtk_box_pack_start (GTK_BOX (properties_manager), properties_title_lbl, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (properties_manager), opacity_lbl, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (properties_manager), properties_opacity_scale, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (properties_manager), lookup_table_lbl, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (properties_manager), properties_lookup_table_combo, 0, 0, 0);

  gtk_widget_set_sensitive (properties_opacity_scale, FALSE);
  gtk_widget_set_sensitive (properties_lookup_table_combo, FALSE);

  return properties_manager;
}


void
gui_mainwindow_properties_manager_refresh (unsigned long long ull_serieID)
{
  Tree  *pt_serie=NULL;
  Serie *ps_serie=NULL;
  unsigned char opacity;

  pt_serie = memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), ull_serieID);

  if (pt_serie == NULL)
  {
    return;
  }

  ps_serie = (Serie*)(pt_serie->data);

  opacity = viewer_get_opacity_for_serie (pll_Viewers->data, ps_serie);
  gtk_range_set_value (GTK_RANGE (properties_opacity_scale), opacity);

  if (pt_serie->type == TREE_TYPE_SERIE)
  {
    gtk_widget_set_sensitive (properties_opacity_scale, FALSE);
  }
  else
  {
    gtk_widget_set_sensitive (properties_opacity_scale, TRUE);
  }

  gtk_widget_set_sensitive (properties_lookup_table_combo, TRUE);


  PixelDataLookupTable *lookup_table;
  lookup_table = viewer_get_active_lookup_table_for_serie (pll_Viewers->data, ps_serie);

  // TODO: This leads to undesired GUI malfunctioning from a user's point of
  // view, but prevents a total program crash.
  if (lookup_table == NULL) return;

  // Don't execute the "changed" handler here.
  // To do this we "block" the signal, apply our action, and "unblock" the
  // signal handler.
  g_signal_handlers_block_by_func (properties_lookup_table_combo,
                                   gui_mainwindow_lookup_table_changed, NULL);

  gtk_combo_box_set_active (GTK_COMBO_BOX (properties_lookup_table_combo),
                            lookup_table->index);

  g_signal_handlers_unblock_by_func (properties_lookup_table_combo,
                                     gui_mainwindow_lookup_table_changed, NULL);
  // Disable the opacity option for original Series

}

