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
#include "lib-viewer.h"
#include "lib-pixeldata-plugin.h"

#include "lib-common-list.h"
#include "lib-common-history.h"
#include "lib-common-tree.h"
#include "lib-common-debug.h"
#include "lib-common-unused.h"

#include "lib-memory.h"
#include "lib-memory-patient.h"
#include "lib-memory-study.h"
#include "lib-memory-serie.h"
#include "lib-memory-slice.h"
#include "lib-memory-tree.h"
#include "lib-configuration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>


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
#define ICON_DOCUMENT_OPEN    "document-open-symbolic"
#define ICON_DOCUMENT_SAVE    "document-save-as-symbolic"
#define ICON_ADD              "list-add-symbolic"
#define ICON_REMOVE           "list-remove-symbolic"
#define ICON_RESET            "view-refresh-symbolic"
#define ICON_SIDEBAR_HIDE     "go-previous-symbolic"
#define ICON_SIDEBAR_SHOW     "go-next-symbolic"
#define ICON_TERMINAL_SHOW    "system-run-symbolic"

#ifdef WIN32
#define PLUGIN_PATH_SELECTION "plugin\\selection-tools\\"
#define PLUGIN_PATH_LINE      "plugin\\line-tools\\"
#define PLUGIN_PATH_BRUSHES   "plugin\\brushes\\"
#define LOOKUP_TABLES_PATH    "luts\\"
#else
#define PLUGIN_PATH_SELECTION "plugin/selection-tools/"
#define PLUGIN_PATH_LINE      "plugin/line-tools/"
#define PLUGIN_PATH_BRUSHES   "plugin/brushes/"
#define LOOKUP_TABLES_PATH    "luts/"
#endif


/******************************************************************************
 * OBJECT INTERNAL TYPES
 ******************************************************************************/


typedef enum
{
  SIDEBAR_NAME,
  SIDEBAR_ID,
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
gboolean gui_mainwindow_terminal_toggle (GtkWidget *widget, void *data);
gboolean gui_mainwindow_on_timeline_change (GtkWidget *widget, void *data);
gboolean gui_mainwindow_overlay_add (GtkWidget *widget, void *data);
//void gui_mainwindow_set_active_layer (GtkListBox *box, GtkListBoxRow *row, void *user_data);
void gui_mainwindow_update_viewer_wwwl (Viewer *viewer, void *data);

gboolean gui_mainwindow_reset_viewport ();
gboolean gui_mainwindow_toggle_follow_mode ();
gboolean gui_mainwindow_toggle_auto_close ();

// Sidebar functions
GtkWidget* gui_mainwindow_sidebar_new ();
void gui_mainwindow_sidebar_populate (Tree *pll_Patients);
void gui_mainwindow_sidebar_destroy ();

// Layout manager functions
GtkWidget* gui_mainwindow_layer_manager_new ();
void gui_mainwindow_layer_manager_refresh (Tree *pt_Serie);
GtkWidget* gui_mainwindow_layer_manager_row_label_new (const char *name,
                                                       gboolean (*fp_AddCallback)(GtkWidget *, void *),
                                                       gboolean (*fp_LoadCallback)(GtkWidget *, void *));

// Layout properties manager functions
GtkWidget* gui_mainwindow_properties_manager_new ();
void gui_mainwindow_properties_manager_refresh (Tree *pt_Serie);

// Other
GtkWidget* gui_mainwindow_toolbar_new ();
char* gui_mainwindow_file_dialog (GtkWidget* parent, GtkFileChooserAction action);
void gui_mainwindow_clear_viewers ();

// Matrix rotation
Vector3D ts_Rotation_around_X_Axis (Vector3D *ps_Vector, double f_angle);
Vector3D ts_Rotation_around_Y_Axis (Vector3D *ps_Vector, double f_angle);
Vector3D ts_Rotation_around_Z_Axis (Vector3D *ps_Vector, double f_angle);


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
GtkWidget *sidebar_pane;
GtkWidget *terminal;
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
GtkTreeStore *sidebar_TreeStore;
GdkRGBA header_bg_color;

// Application-local stuff
List *pll_Viewers;
List *pll_History;
Serie *ts_ActiveMask;
Plugin *ts_ActiveDrawTool;
Viewer *ts_ActiveViewer;
GuiViewportType te_DisplayType = VIEWPORT_TYPE_UNDEFINED;

Configuration *config;


/******************************************************************************
 * FUNCTION IMPLEMENTATIONS
 ******************************************************************************/


void
gui_mainwindow_redisplay_viewers (GuiActionType te_Action, ...)
{
  debug_functions ();

  int width = 0;
  int height = 0;
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

  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    Viewer *viewer = temp->data;
    if (te_DisplayType == VIEWPORT_TYPE_THREEWAY
        || (short)viewer_get_orientation (viewer) == (short)te_DisplayType)
    {
      switch (te_Action)
      {
      case GUI_DO_RESIZE:
        viewer_resize (viewer, width, height);
        break;
      case GUI_DO_REDRAW:
        viewer_redraw (viewer, REDRAW_ACTIVE);
        break;
      default: break;
      }
    }
    temp = temp->next;
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

  // Set the default sidebar title header background color.
  gdk_rgba_parse (&header_bg_color, "#cfcfcf");

  /*--------------------------------------------------------------------------.
   | LOAD LOOKUP TABLES                                                       |
   '--------------------------------------------------------------------------*/
  pixeldata_lookup_table_load_from_directory (LOOKUP_TABLES_PATH);

  List *luts = list_nth (CONFIGURATION_LOOKUP_TABLES (config), 1);
  while (luts != NULL)
  {
    PixelDataLookupTable *lut = luts->data;
    if (lut == NULL)
    {
      luts = list_next (luts);
      continue;
    }

    debug_extra ("Loaded '%s'", lut->name);

    luts = list_next (luts);
  }

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
   | SIDEBAR PANE                                                             |
   '--------------------------------------------------------------------------*/
  sidebar_pane = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

  GtkWidget *sidebar = gui_mainwindow_sidebar_new (sidebar_TreeStore);
  gtk_widget_set_size_request (sidebar, 140, 200);

  GtkWidget *vbox_layer_manager = gui_mainwindow_layer_manager_new ();

  gtk_paned_pack1 (GTK_PANED (sidebar_pane), sidebar, FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (sidebar_pane), vbox_layer_manager, TRUE, TRUE);

  gtk_widget_show_all (sidebar_pane);
  gtk_widget_hide (sidebar_pane);

  /*--------------------------------------------------------------------------.
   | TIMELINE BAR                                                             |
   '--------------------------------------------------------------------------*/

  timeline = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 1);
  gtk_range_set_value (GTK_RANGE (timeline), 0);

  g_signal_connect (G_OBJECT (timeline), "value-changed",
		    G_CALLBACK (gui_mainwindow_on_timeline_change),
		    NULL);

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
  gtk_paned_pack1 (GTK_PANED (hbox_mainwindow), sidebar_pane, TRUE, TRUE);

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

  // Prepare for displayment.
  gtk_widget_set_no_show_all (sidebar_pane, TRUE);

  /*--------------------------------------------------------------------------.
   | GTK MAIN LOOP                                                            |
   '--------------------------------------------------------------------------*/
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

  assert (ts_ActiveMask != NULL);

  unsigned long ul64_SerieSize;
  ul64_SerieSize = ts_ActiveMask->matrix.x * ts_ActiveMask->matrix.y *
    ts_ActiveMask->matrix.z * memory_serie_get_memory_space (ts_ActiveMask);

  pll_History = common_history_save_state (pll_History,
					   ts_ActiveMask->data,
					   ul64_SerieSize);

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

  Tree *pt_Serie=NULL;
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

    pt_Serie=pt_memory_io_load_file(&config->active_study,filename);

    if (pt_Serie != NULL)
    {
      ps_serie = pt_Serie->data;
      ps_serie->e_SerieType=SERIE_ORIGINAL;

      gui_mainwindow_load_serie(pt_Serie);

      gtk_widget_set_sensitive (btn_file_save, TRUE);
      gtk_widget_set_sensitive (btn_reset_viewport, TRUE);
      gtk_widget_set_sensitive (views_combo, TRUE);
      gtk_widget_set_sensitive (hbox_mainmenu, TRUE);

      gui_mainwindow_views_activate (views_combo, (void *)te_DisplayType);
      gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview));
    }
  }
}


void
gui_mainwindow_load_serie (Tree *pt_Serie)
{
  Serie *ps_Serie ;
  unsigned long long ull_groupID;
  Tree *p_Iter=NULL;

  if (pt_Serie == NULL) return;
  if (pt_Serie->type != TREE_TYPE_SERIE) return;

  config->active_serie = pt_Serie;
  config->active_layer = pt_Serie->data;
  config->active_study = tree_parent(pt_Serie);

  // initialize the configuration struct.
  if ( config->memory_tree == NULL)
  {
    config->memory_tree = tree_parent(config->active_study);
  }

  if (!memory_tree_serie_has_mask (pt_Serie))
  {
    Tree *pll_MaskSerie = memory_tree_add_mask_for_serie (pt_Serie);
    assert (pll_MaskSerie != NULL);
    assert (pll_MaskSerie->data != NULL);

    ts_ActiveMask = pll_MaskSerie->data;
  }

  if (ts_ActiveMask->group_id != config->active_layer->group_id)
  {
    // search for masks in serie tree
    ps_Serie = pt_Serie->data;
    ull_groupID = ps_Serie->group_id;

    Tree *p_Iter=tree_nth(pt_Serie,1);
    while (p_Iter != NULL)
    {
      if (p_Iter->type == TREE_TYPE_SERIE_MASK)
      {
        ps_Serie = p_Iter->data;
        if (ps_Serie->group_id == ull_groupID)
        {
          ts_ActiveMask = ps_Serie;
          break;
        }
      }
      p_Iter=tree_next(p_Iter);
    }

  }

  Tree *root_tree = tree_nth (CONFIGURATION_MEMORY_TREE (config), 1);
  gui_mainwindow_sidebar_populate (root_tree);
  gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE(config));

  Serie *serie = pt_Serie->data;
  if (serie == NULL || ts_ActiveMask == NULL) return;

  gtk_range_set_range (GTK_RANGE (timeline), 1, serie->num_time_series);

  if (pll_Viewers == NULL)
  {
    Viewer *viewer = NULL;

    /*--------------------------------------------------------------------------.
     | AXIAL VIEWER WIDGET                                                      |
     '--------------------------------------------------------------------------*/
    Vector3D ts_Pivot;
    Vector3D ts_Normal;
    Vector3D ts_Up;

    ts_Pivot = memory_serie_GetPivotpoint(serie);

    ts_Normal.x = 0;
    ts_Normal.y = 0;
    ts_Normal.z = 1;

    ts_Up.x = 0;
    ts_Up.y = 1;
    ts_Up.z = 0;

    viewer = viewer_new (serie, ts_ActiveMask, NULL, ts_Normal, ts_Pivot, ts_Up);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_AXIAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      if (ts_ActiveDrawTool != NULL)
        viewer_set_active_painter (viewer, ts_ActiveDrawTool);

      axial_embed = VIEWER_WIDGET (viewer);
    }

    /*--------------------------------------------------------------------------.
     | SAGITAL VIEWER WIDGET                                                    |
     '--------------------------------------------------------------------------*/

    ts_Normal.x = 1;
    ts_Normal.y = 0;
    ts_Normal.z = 0;

    ts_Up.x = 0;
    ts_Up.y = 0;
    ts_Up.z = 1;

    viewer = viewer_new (serie, ts_ActiveMask, NULL, ts_Normal, ts_Pivot, ts_Up);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_SAGITAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      if (ts_ActiveDrawTool != NULL)
        viewer_set_active_painter (viewer, ts_ActiveDrawTool);

      sagital_embed = VIEWER_WIDGET (viewer);
    }

    /*--------------------------------------------------------------------------.
     | CORONAL VIEWER WIDGET                                                    |
     '--------------------------------------------------------------------------*/

    ts_Normal.x = 0;
    ts_Normal.y = 1;
    ts_Normal.z = 0;

    ts_Up.x = 0;
    ts_Up.y = 1;
    ts_Up.z = 0;

    viewer = viewer_new (serie, ts_ActiveMask, NULL, ts_Normal, ts_Pivot, ts_Up);
    if (viewer != NULL)
    {
      viewer_set_orientation (viewer, ORIENTATION_CORONAL);
      pll_Viewers = list_prepend (pll_Viewers, viewer);

      viewer_set_callback (viewer, "pixel-paint", &gui_mainwindow_refresh_viewers);
      viewer_set_callback (viewer, "focus-change", &gui_mainwindow_update_viewer_positions);
      viewer_set_callback (viewer, "window-level-change", &gui_mainwindow_update_viewer_wwwl);
      viewer_set_callback (viewer, "handle-change", &gui_mainwindow_update_handle_position);

      if (ts_ActiveDrawTool != NULL)
        viewer_set_active_painter (viewer, ts_ActiveDrawTool);

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

    // Display a slider for time series if applicable.
    if (serie->num_time_series > 1)
    {
      gtk_scale_clear_marks (GTK_SCALE (timeline));
      gtk_range_set_range (GTK_RANGE (timeline), 1, serie->num_time_series);
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
      viewer_set_active_layer_serie (viewers->data, serie);
      viewers = list_next (viewers);
    }
  }
  else
  {
    Vector3D ts_Pivot;
    Vector3D ts_Normal;
    Vector3D ts_Up;

    ts_Pivot = memory_serie_GetPivotpoint(serie);

    List *temp = list_nth (pll_Viewers, 1);
    while (temp != NULL)
    {
      Viewer* list_viewer = temp->data;


      switch (VIEWER_ORIENTATION (list_viewer))
      {
        case ORIENTATION_AXIAL:
          ts_Up.x = 0;
          ts_Up.y = 1;
          ts_Up.z = 0;

          ts_Normal.x = 0;
          ts_Normal.y = 0;
          ts_Normal.z = 1;
          break;
        case ORIENTATION_SAGITAL:
          ts_Up.x = 0;
          ts_Up.y = 0;
          ts_Up.z = 1;

          ts_Normal.x = 1;
          ts_Normal.y = 0;
          ts_Normal.z = 0;
          break;
        case ORIENTATION_CORONAL:
          ts_Up.x = 0;
          ts_Up.y = 1;
          ts_Up.z = 0;

          ts_Normal.x = 0;
          ts_Normal.y = 1;
          ts_Normal.z = 0;
          break;
        default:
          break;
      }

      viewer_initialize (list_viewer, serie, ts_ActiveMask, NULL, ts_Normal, ts_Pivot, ts_Up);

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
  gui_mainwindow_load_serie (CONFIGURATION_ACTIVE_SERIE(config));

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

  //char* filename = gui_mainwindow_file_dialog (window, GTK_FILE_CHOOSER_ACTION_SAVE);
  //if (filename == NULL) return FALSE;

  Tree *pt_Series = tree_child (CONFIGURATION_ACTIVE_STUDY (config));
  while (pt_Series != NULL)
  {
    Serie *serie = pt_Series->data;

    // Filter out the series that shouldn't be exported.
    if (serie == NULL || pt_Series->type != TREE_TYPE_SERIE_MASK)
    {
      pt_Series = tree_next (pt_Series);
      continue;
    }

    memory_io_save_file (serie, serie->pc_filename);
    pt_Series = tree_next (pt_Series);
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

  ts_ActiveViewer = viewer;

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

  ts_ActiveViewer = viewer;

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
        i16_SliceNumber_Axial = slice->matrix.z;
        i16_SliceNumber_Sagital = ts_Pivot.x - ts_Position->x;
        i16_SliceNumber_Coronal = ts_Pivot.y - ts_Position->y;
        break;
      case ORIENTATION_SAGITAL:
        i16_SliceNumber_Sagital = slice->matrix.z;
        i16_SliceNumber_Axial =  ts_Pivot.z - ts_Position->y;
        i16_SliceNumber_Coronal =  ts_Pivot.x - ts_Position->x;
        break;
      case ORIENTATION_CORONAL:
        i16_SliceNumber_Coronal = slice->matrix.z;
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

  ts_ActiveViewer = viewer;
  if (data == NULL) return;

  Serie *active_serie = CONFIGURATION_ACTIVE_LAYER (config);
  if (active_serie == NULL) return;

  WWWL wwwl = *(WWWL *)data;
  List *temp = list_nth (pll_Viewers, 1);
  while (temp != NULL)
  {
    Viewer* list_viewer = temp->data;

    // Skip the Viewer that is calling back.
    if (VIEWER_ORIENTATION (list_viewer) != VIEWER_ORIENTATION (viewer))
      viewer_set_window_level_for_serie (list_viewer, active_serie, wwwl.i32_windowWidth, wwwl.i32_windowLevel);

    temp = temp->next;
  }

}


/* Rotation vector around x-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          | 1  0    0  |   | x |
      R = | 0 cos -sin | * | y |
          | 0 sin  cos |   | z |
*/
Vector3D ts_Rotation_around_X_Axis (Vector3D *ps_Vector, double f_angle)
{
  Vector3D ts_rotatedVector;

  ts_rotatedVector.x = 1 * ps_Vector->x -      0        * ps_Vector->y +     0         * ps_Vector->z;
  ts_rotatedVector.y = 0 * ps_Vector->x + cos (f_angle) * ps_Vector->y - sin (f_angle) * ps_Vector->z;
  ts_rotatedVector.z = 0 * ps_Vector->x + sin (f_angle) * ps_Vector->y + cos (f_angle) * ps_Vector->z;

  return ts_rotatedVector;
}

/* Rotation vector around y-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          |  cos  0  sin |   | x |
      R = |   0   1   0  | * | y |
          | -sin  0  cos |   | z |
*/
Vector3D ts_Rotation_around_Y_Axis (Vector3D *ps_Vector, double f_angle)
{
  Vector3D ts_rotatedVector;

  ts_rotatedVector.x =  cos (f_angle) * ps_Vector->x + 0 * ps_Vector->y + sin (f_angle) * ps_Vector->z;
  ts_rotatedVector.y =      0         * ps_Vector->x + 1 * ps_Vector->y + 0             * ps_Vector->z;
  ts_rotatedVector.z = -sin (f_angle) * ps_Vector->x + 0 * ps_Vector->y + cos (f_angle) * ps_Vector->z;

  return ts_rotatedVector;
}

/* Rotation vector around z-axis (http://en.wikipedia.org/wiki/Rotation_matrix)
          | cos -sin 0  |   | x |
      R = | sin  cos 0  | * | y |
          |  0    0  1  |   | z |
*/
Vector3D ts_Rotation_around_Z_Axis (Vector3D *ps_Vector, double f_angle)
{
  Vector3D ts_rotatedVector;

  ts_rotatedVector.x = cos (f_angle) * ps_Vector->x - sin (f_angle) * ps_Vector->y + 0 * ps_Vector->z;
  ts_rotatedVector.y = sin (f_angle) * ps_Vector->x + cos (f_angle) * ps_Vector->y + 0 * ps_Vector->z;
  ts_rotatedVector.z =     0                        -     0                        + 1 * ps_Vector->z;

  return ts_rotatedVector;
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
      ts_NormalVector=ts_Rotation_around_Y_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_Rotation_around_Z_Axis(&ts_NormalVector,f_angle);

      sagital->ts_NormalVector.x = ts_rotationVector.x;
      sagital->ts_NormalVector.y = ts_rotationVector.y;
      sagital->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(sagital);
      viewer_redraw (sagital, REDRAW_MINIMAL);

      ts_NormalVector=ts_Rotation_around_X_Axis(&viewer->ts_NormalVector,-M_PI_2);
      ts_rotationVector=ts_Rotation_around_Z_Axis(&ts_NormalVector,f_angle);

      coronal->ts_NormalVector.x = ts_rotationVector.x;
      coronal->ts_NormalVector.y = ts_rotationVector.y;
      coronal->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(coronal);
      viewer_redraw (coronal, REDRAW_MINIMAL);
    }
    break;
  case ORIENTATION_SAGITAL:
    {
      ts_NormalVector=ts_Rotation_around_Y_Axis(&viewer->ts_NormalVector,-M_PI_2);
      ts_rotationVector=ts_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      axial->ts_NormalVector.x = ts_rotationVector.x;
      axial->ts_NormalVector.y = ts_rotationVector.y;
      axial->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(axial);
      viewer_redraw (axial, REDRAW_MINIMAL);

      ts_NormalVector=ts_Rotation_around_Z_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_Rotation_around_X_Axis(&ts_NormalVector,f_angle);

      coronal->ts_NormalVector.x = ts_rotationVector.x;
      coronal->ts_NormalVector.y = ts_rotationVector.y;
      coronal->ts_NormalVector.z = ts_rotationVector.z;

      viewer_refresh_data(coronal);
      viewer_redraw (coronal, REDRAW_MINIMAL);
    }
    break;
  case ORIENTATION_CORONAL:
    {
      ts_NormalVector=ts_Rotation_around_X_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_Rotation_around_Y_Axis(&ts_NormalVector,f_angle);

      axial->ts_NormalVector.x=ts_rotationVector.x;
      axial->ts_NormalVector.y=ts_rotationVector.y;
      axial->ts_NormalVector.z=ts_rotationVector.z;

      viewer_refresh_data(axial);
      viewer_redraw (axial, REDRAW_MINIMAL);

      ts_NormalVector=ts_Rotation_around_Z_Axis(&viewer->ts_NormalVector,M_PI_2);
      ts_rotationVector=ts_Rotation_around_Y_Axis(&ts_NormalVector,f_angle);

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
           && ts_ActiveViewer != NULL)
    viewer_toggle_recording (ts_ActiveViewer);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_RECORDING)
           && ts_ActiveViewer != NULL)
    viewer_replay_recording (ts_ActiveViewer);

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_OVER_TIME)
           && ts_ActiveViewer != NULL)
    viewer_replay_recording_over_time (ts_ActiveViewer);
  /*
  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REPLAY_OVER_SLICES)
           && ts_ActiveViewer != NULL)
    viewer_replay_recording_over_slices (ts_ActiveViewer);
  */
  else if (event->keyval == CONFIGURATION_KEY (config, KEY_UNDO)
	   && (event->state & GDK_CONTROL_MASK))
  {
        assert (ts_ActiveMask != NULL);
        pll_History = common_history_load_state (pll_History,
						 HISTORY_PREVIOUS,
						 &ts_ActiveMask->data);
  }

  else if (event->keyval == CONFIGURATION_KEY (config, KEY_REDO)
	   && (event->state & GDK_CONTROL_MASK))
  {
    assert (ts_ActiveMask != NULL);
    pll_History = common_history_load_state (pll_History,
					     HISTORY_NEXT,
					     &ts_ActiveMask->data);
  }

  /*--------------------------------------------------------------------------.
   | OTHER STUFF                                                              |
   '--------------------------------------------------------------------------*/

  if (ts_ActiveViewer != NULL)
    viewer_on_key_press (ts_ActiveViewer, event);

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

  if (ts_ActiveViewer != NULL)
    viewer_on_key_release (ts_ActiveViewer, event);

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

  ts_ActiveDrawTool = (Plugin *)data;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_active_painter (viewers->data, ts_ActiveDrawTool);
    viewers = viewers->next;
  }

  if (inp_brush_size != NULL)
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (inp_brush_size),
                               ts_ActiveDrawTool->i32_Size);

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

  if (ts_ActiveDrawTool == NULL) return;

  int size = gtk_spin_button_get_value (GTK_SPIN_BUTTON (inp_brush_size));
  Plugin *plugin = ts_ActiveDrawTool;
  plugin->i32_Size = size;
}


void
gui_mainwindow_set_brush_value (UNUSED GtkWidget* widget, UNUSED void* data)
{
  debug_functions ();

  if (ts_ActiveDrawTool == NULL) return;

  int value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (inp_brush_value));

  List *plugins = CONFIGURATION_DRAW_TOOLS (config);
  while (plugins != NULL)
  {
    Plugin *plugin = plugins->data;
    plugin->i32_Value = value;
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

  list_free_all (CONFIGURATION_LOOKUP_TABLES (config),
                 pixeldata_lookup_table_destroy_item);

  list_free_all (CONFIGURATION_DRAW_TOOLS (config), pixeldata_plugin_destroy);
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

  GtkWidget *btn_selection;
  if (plugin->pc_Icon != NULL)
  {
    btn_selection = gtk_button_new ();
    GdkPixbuf *pix_selection = gdk_pixbuf_new_from_data (plugin->pc_Icon,
							 GDK_COLORSPACE_RGB,
                                                         TRUE,
							 8,
							 16,
							 16,
							 16 * 4,
							 NULL,
							 NULL);

    GtkWidget *img_selection = gtk_image_new_from_pixbuf (pix_selection);
    gtk_button_set_image (GTK_BUTTON (btn_selection), img_selection);
    g_object_unref (pix_selection);

    gtk_button_set_relief (GTK_BUTTON (btn_selection), GTK_RELIEF_NONE);
  }
  else
  {
    btn_selection = gtk_button_new_with_label (plugin->pc_Name);
  }

  gtk_box_pack_start (GTK_BOX (box), btn_selection, 0, 0, 0);
  gtk_widget_show (btn_selection);

  g_signal_connect (btn_selection, "clicked",
		    G_CALLBACK (gui_mainwindow_select_tool),
		    plugin);

  gui_mainwindow_select_tool (btn_selection, plugin);
}


GtkWidget*
gui_mainwindow_toolbar_new ()
{
  debug_functions ();

  GtkWidget *hbox_toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /*--------------------------------------------------------------------------.
   | LOAD PLUGINS                                                             |
   '--------------------------------------------------------------------------*/
  List *pll_PluginList = NULL;

  pixeldata_plugin_load_from_directory (PLUGIN_PATH_LINE, &pll_PluginList);
  pixeldata_plugin_load_from_directory (PLUGIN_PATH_SELECTION, &pll_PluginList);
  pixeldata_plugin_load_from_directory (PLUGIN_PATH_BRUSHES, &pll_PluginList);

  /*--------------------------------------------------------------------------.
   | SET STANDARD BRUSH STUFF                                                 |
   '--------------------------------------------------------------------------*/
  GtkWidget* lbl_brush_size = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (lbl_brush_size), "<b>Brush size:</b>");
  inp_brush_size = gtk_spin_button_new_with_range (1.0, 500.0, 1.0);

  gtk_box_pack_start (GTK_BOX (hbox_toolbar), lbl_brush_size, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_toolbar), inp_brush_size, 0, 0, 10);

  g_signal_connect (inp_brush_size, "value-changed",
		    G_CALLBACK (gui_mainwindow_set_brush_size),
		    NULL);

  GtkWidget* lbl_brush_value = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (lbl_brush_value), "<b>Brush value:</b>");
  inp_brush_value = gtk_spin_button_new_with_range (1.0, 255.0, 1.0);

  gtk_box_pack_start (GTK_BOX (hbox_toolbar), lbl_brush_value, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_toolbar), inp_brush_value, 0, 0, 10);

  g_signal_connect (inp_brush_value, "value-changed",
		    G_CALLBACK (gui_mainwindow_set_brush_value),
		    NULL);

  /*--------------------------------------------------------------------------.
   | ADD PLUGINS TO MENU BAR                                                  |
   '--------------------------------------------------------------------------*/
  CONFIGURATION_DRAW_TOOLS (config) = pll_PluginList;
  pll_PluginList = CONFIGURATION_DRAW_TOOLS (config);
  while (pll_PluginList != NULL)
  {
    Plugin *plugin = pll_PluginList->data;
    gui_mainwindow_add_plugin (plugin, hbox_toolbar);

    pll_PluginList = list_next (pll_PluginList);
  }

  return hbox_toolbar;
}


/******************************************************************************
 * LAYER MANAGER (SIDE BAR)
 ******************************************************************************/


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


void
gui_mainwindow_layer_manager_clear ()
{
  debug_functions ();

  GList *children, *child;

  children = gtk_container_get_children (GTK_CONTAINER (layer_manager));

  for (child = children; child != NULL; child = g_list_next (child))
  {
    g_object_ref_sink (child->data);
    gtk_widget_destroy (GTK_WIDGET (child->data));
    g_object_unref (child->data);
  }

  g_list_free (children);
}


gboolean
gui_mainwindow_mask_add (UNUSED GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  assert (CONFIGURATION_ACTIVE_SERIE (config) != NULL);

  Tree *pll_MaskSerie;
  pll_MaskSerie = memory_tree_add_mask_for_serie (CONFIGURATION_ACTIVE_SERIE (config));

  assert (pll_MaskSerie != NULL);
  assert (pll_MaskSerie->data != NULL);

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_add_mask_serie (viewers->data, pll_MaskSerie->data);
    viewer_set_active_mask_serie (viewers->data, pll_MaskSerie->data);
    viewers = list_next (viewers);
  }

  ts_ActiveMask = pll_MaskSerie->data;

  gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE (config));

  return FALSE;
}


gboolean
gui_mainwindow_overlay_add (UNUSED GtkWidget *widget, void *data)
{
  char* pc_path = NULL;
  Tree *pt_Study = NULL;
  Tree *pt_Serie = NULL;

  Serie *ps_Overlay = NULL;
  Serie *ps_Original = NULL;

  debug_functions ();

  pc_path = (data != NULL)
    ? (char*)(data)
    : gui_mainwindow_file_dialog (window, GTK_FILE_CHOOSER_ACTION_OPEN);

  if (pc_path != NULL)
  {
    pt_Serie = CONFIGURATION_ACTIVE_SERIE(config);
    pt_Study = CONFIGURATION_ACTIVE_STUDY(config);

    ps_Original = pt_Serie->data;

    pt_Serie=pt_memory_io_load_file(&pt_Study,pc_path);

    if (pt_Serie != NULL)
    {
      return FALSE;
    }

    pt_Serie->type=TREE_TYPE_SERIE_OVERLAY;

    ps_Overlay=(Serie *)(pt_Serie->data);
    ps_Overlay->e_SerieType=SERIE_OVERLAY;
    ps_Overlay->group_id = ps_Original->group_id;

    List *viewers = list_nth (pll_Viewers, 1);
    while (viewers != NULL)
    {
      viewer_add_overlay_serie (viewers->data, ps_Overlay);
      viewers = list_next (viewers);
    }

    gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE (config));
  }

  return FALSE;
}


void
gui_mainwindow_mask_remove (UNUSED GtkWidget *widget, void *data)
{
  debug_functions ();

  Tree *pt_Serie = data;
  assert (pt_Serie != NULL);

  Serie *serie = pt_Serie->data;
  assert (serie != NULL);

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_remove_mask_serie (viewers->data, pt_Serie->data);
    viewers = list_next (viewers);
  }

  memory_serie_destroy (serie);
  serie = NULL;

  pt_Serie = tree_remove (pt_Serie);
  assert (pt_Serie != NULL);

  gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE(config));
}

void
gui_mainwindow_overlay_remove (UNUSED GtkWidget *widget, void *data)
{
  debug_functions ();

  Tree *pt_Serie = data;
  assert (pt_Serie != NULL);

  Serie *serie = pt_Serie->data;
  assert (serie != NULL);

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_remove_overlay_serie (viewers->data, pt_Serie->data);
    viewers = list_next (viewers);
  }

  memory_serie_destroy (serie);
  serie = NULL;

  pt_Serie = tree_remove (pt_Serie);
  assert (pt_Serie != NULL);

  gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE (config));
}


gboolean
gui_mainwindow_layer_manager_row_activated (GtkWidget *widget, void *data)
{
  debug_functions ();

  Tree *pt_Series = data;
  if (pt_Series == NULL || pt_Series->type != TREE_TYPE_SERIE_MASK)
  {
    debug_warning ("Bailing early.. pt_Series = %p", pt_Series);
    return FALSE;
  }

  Serie *serie = pt_Series->data;
  if (serie == NULL)
  {
    debug_warning ("Bailing early.. The serie is empty.", NULL);
    return FALSE;
  }

  debug_extra ("Toggling serie %p", serie);

  const char *name = gtk_widget_get_name (widget);

  if (name != NULL && !strcmp (name, "active"))
  {
    gboolean is_active_mask;
    is_active_mask = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    if (is_active_mask && serie != ts_ActiveMask)
    {
      List *viewers = list_nth (pll_Viewers, 1);
      while (viewers != NULL)
      {
        viewer_set_active_mask_serie (viewers->data, serie);
        ts_ActiveMask = serie;
        viewers = list_next (viewers);
      }
    }
  }

  gui_mainwindow_layer_manager_refresh (CONFIGURATION_ACTIVE_SERIE (config));

  return FALSE;
}


GtkWidget*
gui_mainwindow_layer_manager_row_item_new (const char *name,
                                           gboolean is_drawable,
                                           Tree *item)
{
  debug_functions ();

  assert (name != NULL);
  assert (item != NULL);

  // GtkListBoxRow
  GtkWidget *listbox = gtk_list_box_row_new ();

  // Row container
  GtkWidget *hbox_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  // Name field
  GtkWidget *lbl_name = gtk_label_new (name);
  gtk_widget_set_name (lbl_name, "name");

  // Mask checkbox
  if (item->type == TREE_TYPE_SERIE_MASK)
  {
    GtkWidget *chk_drawable = gtk_check_button_new ();

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chk_drawable),
				  is_drawable);

    gtk_widget_set_name (chk_drawable, "active");
    gtk_box_pack_end (GTK_BOX (hbox_row), chk_drawable, 0, 0, 0);

    g_signal_connect (chk_drawable, "clicked",
		      G_CALLBACK (gui_mainwindow_layer_manager_row_activated),
		      item);
  }

  // TODO: Clicking the remove button crashes the application.
  if (item->type != TREE_TYPE_SERIE && ((Serie *)item->data)->id != ts_ActiveMask->id)
  {
    GtkWidget *btn_Remove = gtk_button_new_from_icon_name (ICON_REMOVE,
                                                           GTK_ICON_SIZE_MENU);

    gtk_button_set_relief (GTK_BUTTON (btn_Remove), GTK_RELIEF_NONE);

    gtk_box_pack_end (GTK_BOX (hbox_row), btn_Remove, 0, 0, 0);

    if (item->type == TREE_TYPE_SERIE_OVERLAY)
    {
      g_signal_connect (btn_Remove, "clicked",
                        G_CALLBACK (gui_mainwindow_overlay_remove),
                        item);
    }

    else if (item->type == TREE_TYPE_SERIE_MASK)
    {
      g_signal_connect (btn_Remove, "clicked",
                        G_CALLBACK (gui_mainwindow_mask_remove),
                        item);
    }
  }

  gtk_box_pack_start (GTK_BOX (hbox_row), lbl_name, 0, 0, 0);

  gtk_container_add (GTK_CONTAINER (listbox), hbox_row);
  gtk_widget_show_all (listbox);

  return listbox;
}


void
gui_mainwindow_set_active_layer (UNUSED GtkListBox *box, GtkListBoxRow *row, UNUSED void *data)
{
  // This is an exceptional piece of "this is why you cannot change the
  // layout of the layer manager rows". Feel free to come up with a better idea.

  // Filter out invalid requests.
  if (row == NULL) return;

  // The GtkBox container.
  GList *children = gtk_container_get_children (GTK_CONTAINER (row));

  // The GtkLabel
  GList *label_children = gtk_container_get_children (GTK_CONTAINER (children->data));
  GtkWidget *lbl_name = gtk_widget_get_ancestor (label_children->data, GTK_TYPE_LABEL);

  // Make sure it's a GtkLabel.
  if (G_OBJECT_TYPE (lbl_name) != GTK_TYPE_LABEL)
  {
    debug_error ("%s() has encountered an unexpected container layout.", __func__);
    return;
  }

  // Make sure the label is allocated.
  if (lbl_name == NULL)
  {
    debug_error ("%s() has encountered an unallocated label.", __func__);
    return;
  }

  const char *name = gtk_label_get_text (GTK_LABEL (lbl_name));

  // Get a pointer to the serie and set it as global active layer.
  Tree *pt_Series = tree_nth (tree_child (CONFIGURATION_ACTIVE_STUDY (config)), 1);
  while (pt_Series != NULL)
  {
    Serie *serie = pt_Series->data;
    if (serie == NULL)
    {
      pt_Series = tree_next (pt_Series);
      continue;
    }

    if (!strcmp (serie->name, name))
    {
      CONFIGURATION_ACTIVE_LAYER (config) = serie;
      debug_extra ("GUI has set the active layer to '%s'", serie->name);

      // Change the active layer on each Viewer.
      List *viewers = list_nth (pll_Viewers, 1);
      while (viewers != NULL)
      {
        viewer_set_active_layer_serie (viewers->data, serie);
        viewers = list_next (viewers);
      }

      // Update the properties box.
      gui_mainwindow_properties_manager_refresh (pt_Series);
      break;
    }

    pt_Series = tree_next (pt_Series);
  }
}


GtkWidget*
gui_mainwindow_layer_manager_row_label_new (const char *name,
                                            gboolean (*fp_AddCallback)(GtkWidget *, void *),
                                            gboolean (*fp_LoadCallback)(GtkWidget *, void *))
{
  debug_functions ();

  assert (name != NULL);

  // GtkListBoxRow
  GtkWidget *listbox = gtk_list_box_row_new ();

  // Row container
  GtkWidget *hbox_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  // Name field
  GtkWidget *lbl_name = gtk_label_new ("");

  char *markup = calloc (1, 8 + strlen (name));
  sprintf (markup, "<b>%s</b>", name);

  gtk_label_set_markup (GTK_LABEL (lbl_name), markup);
  gtk_widget_set_name (lbl_name, "name");

  free (markup), markup = NULL;

  // When there's an action specified for a "load" button, create one.
  if (fp_LoadCallback != NULL)
  {
    GtkWidget *btn_Load;
    btn_Load = gtk_button_new_from_icon_name (ICON_DOCUMENT_OPEN, GTK_ICON_SIZE_MENU);

    gtk_button_set_always_show_image (GTK_BUTTON (btn_Load), TRUE);
    gtk_button_set_relief (GTK_BUTTON (btn_Load), GTK_RELIEF_NONE);

    g_signal_connect (btn_Load, "clicked",
		      G_CALLBACK (fp_LoadCallback),
		      NULL);

    gtk_box_pack_end (GTK_BOX (hbox_row), btn_Load, 0, 0, 0);
  }

  // When there's an action specified for a "add" button, create one.
  if (fp_AddCallback != NULL)
  {
    GtkWidget *btn_Add;
    btn_Add = gtk_button_new_from_icon_name (ICON_ADD, GTK_ICON_SIZE_MENU);

    gtk_button_set_always_show_image (GTK_BUTTON (btn_Add), TRUE);
    gtk_button_set_relief (GTK_BUTTON (btn_Add), GTK_RELIEF_NONE);

    g_signal_connect (btn_Add, "clicked",
		      G_CALLBACK (fp_AddCallback),
		      NULL);

    gtk_box_pack_end (GTK_BOX (hbox_row), btn_Add, 0, 0, 0);
  }

  gtk_box_pack_start (GTK_BOX (hbox_row), lbl_name, 0, 1, 0);

  gtk_container_add (GTK_CONTAINER (listbox), hbox_row);

  gtk_widget_show_all (listbox);

  gtk_list_box_row_set_selectable (GTK_LIST_BOX_ROW (listbox), FALSE);
  return listbox;
}


GtkWidget*
gui_mainwindow_layer_manager_row_header_new ()
{
  debug_functions ();

  // GtkListBoxRow
  GtkWidget *listbox = gtk_list_box_row_new ();

  // Row container
  GtkWidget *hbox_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);

  // Name field
  GtkWidget *lbl_name = gtk_label_new ("Name");
  gtk_widget_set_name (lbl_name, "name");

  // Drawable field
  GtkWidget *lbl_active_mask = gtk_label_new ("Active");
  gtk_widget_set_name (lbl_active_mask, "active");

  gtk_box_pack_start (GTK_BOX (hbox_row), lbl_name, 0, 0, 5);
  gtk_box_pack_end (GTK_BOX (hbox_row), lbl_active_mask, 0, 0, 5);

  gtk_container_add (GTK_CONTAINER (listbox), hbox_row);
  gtk_widget_show_all (listbox);

  return listbox;
}


void
gui_mainwindow_layer_manager_refresh (Tree *pt_Series)
{
  debug_functions ();

  assert (pt_Series != NULL);

  gui_mainwindow_layer_manager_clear ();

  Serie *ps_Serie = pt_Series->data;

  Tree *pll_Masks=NULL;
  Tree *pll_Overlays=NULL;

  if (ps_Serie == NULL) return;

  /* NOTE:
   * ---------------------------------------------------------------------------
   * From here you should read from the bottom up to make sense of what will be
   * displayed first because we're prepending items to the list box.
   * ---------------------------------------------------------------------------
   */

  gui_mainwindow_sanitize_string (ps_Serie->name);


  GtkWidget *row;

  /*--------------------------------------------------------------------------.
   | ADD MASK SERIES                                                          |
   '--------------------------------------------------------------------------*/
  pll_Masks=pt_Series;

  while (pll_Masks != NULL)
  {
    Serie *ps_Mask = pll_Masks->data;

    // Filter out the original serie and empty series.
    if ((ps_Mask == NULL) ||
        (pll_Masks == CONFIGURATION_ACTIVE_SERIE (config)) ||
        (pll_Masks->type != TREE_TYPE_SERIE_MASK) ||
        (ps_Serie->group_id != ps_Mask->group_id))
    {
      pll_Masks = tree_next (pll_Masks);
      continue;
    }

    // Figure out whether the serie has a special role.
    short int i16_MaskState = (ps_Mask == ts_ActiveMask) ? 1 : 0;

    char serie_name[100];
    memset (serie_name, 0, 100);
    strcpy (serie_name, ps_Mask->name);

    //gui_mainwindow_pretty_string (serie_name);

    // Add the row to the list box.
    gui_mainwindow_sanitize_string (ps_Mask->name);

    GtkWidget *row = gui_mainwindow_layer_manager_row_item_new (serie_name,
                                                                i16_MaskState,
                                                                pll_Masks);
    gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);

    if (ps_Mask == CONFIGURATION_ACTIVE_LAYER (config))
    {
      gtk_list_box_select_row (GTK_LIST_BOX (layer_manager), GTK_LIST_BOX_ROW (row));
    }

    pll_Masks = tree_next (pll_Masks);
  }

  // Add a "Masks" header.
  row = gui_mainwindow_layer_manager_row_label_new ("Masks",
						    gui_mainwindow_mask_add,
                                                    NULL);

  gtk_list_box_row_set_selectable (GTK_LIST_BOX_ROW (row), FALSE);
  gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);

  /*--------------------------------------------------------------------------.
   | ADD OVERLAY SERIES                                                       |
   '--------------------------------------------------------------------------*/
  pll_Overlays=pt_Series;

  while (pll_Overlays != NULL)
  {
    Serie *ps_Overlay = pll_Overlays->data;

    // Filter out the original serie and empty series.

    if ((ps_Overlay == NULL) ||
        (pll_Overlays == CONFIGURATION_ACTIVE_SERIE (config)) ||
        (pll_Overlays->type != TREE_TYPE_SERIE_OVERLAY) ||
        (ps_Serie->group_id != ps_Overlay->group_id))
    {
      pll_Overlays = tree_next (pll_Overlays);
      continue;
    }

    // Figure out whether the serie has a special role.
    short int i16_MaskState = (ps_Overlay == ts_ActiveMask) ? 1 : 0;

    // Add the row to the list box.
    gui_mainwindow_sanitize_string (ps_Overlay->name);

    GtkWidget *row = gui_mainwindow_layer_manager_row_item_new (ps_Overlay->name,
                                                                i16_MaskState,
                                                                pll_Overlays);
    gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);

    if (ps_Overlay == CONFIGURATION_ACTIVE_LAYER (config))
    {
      gtk_list_box_select_row (GTK_LIST_BOX (layer_manager), GTK_LIST_BOX_ROW (row));
    }

    pll_Overlays = tree_next (pll_Overlays);
  }

  // Add a "Overlays" header.
  row = gui_mainwindow_layer_manager_row_label_new ("Overlays",
                                                    NULL,
						    gui_mainwindow_overlay_add);

  gtk_list_box_row_set_selectable (GTK_LIST_BOX_ROW (row), FALSE);
  gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);

  /*--------------------------------------------------------------------------.
   | ADD ORIGINAL SERIES                                                      |
   '--------------------------------------------------------------------------*/

  assert (CONFIGURATION_ACTIVE_SERIE (config)->type == TREE_TYPE_SERIE);

  if (ps_Serie != NULL)
  {
    GtkWidget *original;
    original = gui_mainwindow_layer_manager_row_item_new (ps_Serie->name, 0,
							  CONFIGURATION_ACTIVE_SERIE (config));

    gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), original);
    if (ps_Serie == CONFIGURATION_ACTIVE_LAYER (config))
    {
      gtk_list_box_select_row (GTK_LIST_BOX (layer_manager), GTK_LIST_BOX_ROW (original));
    }


  }

  // Add a "Original" header.
  row = gui_mainwindow_layer_manager_row_label_new ("Original", NULL, NULL);
  gtk_list_box_row_set_selectable (GTK_LIST_BOX_ROW (row), FALSE);
  gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);

  // Add a header.
  row = gui_mainwindow_layer_manager_row_header_new ();
  gtk_list_box_row_set_selectable (GTK_LIST_BOX_ROW (row), FALSE);
  gtk_list_box_prepend (GTK_LIST_BOX (layer_manager), row);
}


GtkWidget*
gui_mainwindow_layer_manager_new ()
{
  debug_functions ();

  GtkWidget *vbox_layer_manager = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  layer_manager = gtk_list_box_new ();

  GtkWidget *hbox_properties = gui_mainwindow_properties_manager_new ();

  GtkWidget *layers_title_lbl = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (layers_title_lbl), "<b>Layers</b>");
  gtk_widget_override_background_color (layers_title_lbl,
					GTK_STATE_FLAG_NORMAL,
					&header_bg_color);

  gtk_box_pack_start (GTK_BOX (vbox_layer_manager), layers_title_lbl, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_layer_manager), layer_manager, 1, 1, 0);
  gtk_box_pack_start (GTK_BOX (vbox_layer_manager), hbox_properties, 0, 0, 0);

  g_signal_connect (layer_manager, "row-selected", G_CALLBACK (gui_mainwindow_set_active_layer), NULL);

  return vbox_layer_manager;
}


gboolean
gui_mainwindow_terminal_toggle (UNUSED GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  if (gtk_widget_is_visible (terminal))
  {
    gtk_widget_hide (terminal);
    //gtk_button_set_label (GTK_BUTTON (widget), LABEL_TERMINAL_SHOW);
  }
  else
  {
    gtk_widget_show (terminal);
    //gtk_button_set_label (GTK_BUTTON (widget), LABEL_TERMINAL_HIDE);
  }

  return FALSE;
}


/******************************************************************************
 * TREE VIEW (SIDE BAR)
 ******************************************************************************/


gboolean
gui_mainwindow_sidebar_toggle (GtkWidget *widget, UNUSED void *data)
{
  debug_functions ();

  if (gtk_widget_is_visible (sidebar_pane))
  {
    gtk_widget_hide (sidebar_pane);
    GtkWidget *image = gtk_image_new_from_icon_name (ICON_SIDEBAR_SHOW,
                                                     GTK_ICON_SIZE_BUTTON);

    gtk_button_set_image (GTK_BUTTON (widget), image);
  }
  else
  {
    gtk_widget_show (sidebar_pane);
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
  Tree *pt_Series;

  GtkTreeIter PatientIterator;
  GtkTreeIter StudyIterator;
  GtkTreeIter SerieIterator;

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
                        SIDEBAR_NAME, patient->name,
                        SIDEBAR_ID, patient->id, -1);

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

      gtk_tree_store_set (sidebar_TreeStore, &StudyIterator,
                          SIDEBAR_NAME, study->name,
                          SIDEBAR_ID, study->id, -1);


      /*------------------------------------------------------------------------.
      | SERIES                                                                  |
      '------------------------------------------------------------------------*/
      pt_Series = tree_child(pll_Studies);
      while (pt_Series != NULL)
      {
        Serie *serie = pt_Series->data;
        if (serie == NULL)
        {
          pt_Series = tree_next (pt_Series);
          continue;
        }

        if (serie->e_SerieType == SERIE_ORIGINAL)
        {
          gui_mainwindow_sanitize_string (serie->name);
          gtk_tree_store_append (sidebar_TreeStore, &SerieIterator, &StudyIterator);

          gtk_tree_store_set (sidebar_TreeStore, &SerieIterator, SIDEBAR_NAME, serie->name, SIDEBAR_ID, serie->id, -1);
        }

        pt_Series = tree_next(pt_Series);
      }
      pll_Studies = tree_next (pll_Studies);
    }
    pll_Patients = tree_next (pll_Patients);
  }
}


void
gui_mainwindow_sidebar_clicked (GtkWidget *widget)
{
  GtkTreePath *path = NULL;

  unsigned long long serie_id = 0;
  Tree *pt_SerieTree = NULL;

  gtk_tree_view_get_cursor (GTK_TREE_VIEW (widget), &path, NULL);
  if (path != NULL)
  {
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (sidebar_TreeStore), &iter, path))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (sidebar_TreeStore), &iter, SIDEBAR_ID, &serie_id, -1);
      pt_SerieTree=memory_tree_get_serie_by_id (CONFIGURATION_MEMORY_TREE (config), serie_id);
    }
    gtk_tree_path_free (path);
  }

  if ((pt_SerieTree != NULL) &&
      (pt_SerieTree->type == TREE_TYPE_SERIE) &&
      (pt_SerieTree != CONFIGURATION_ACTIVE_SERIE(config)))
  {
    gui_mainwindow_load_serie (pt_SerieTree);
    gtk_tree_view_expand_all(GTK_TREE_VIEW (widget));
  }
}


void
gui_mainwindow_sidebar_realized (GtkWidget *widget)
{
  debug_functions ();

  //gtk_tree_view_expand_all (GTK_TREE_VIEW (widget));
  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (widget));
}


GtkWidget*
gui_mainwindow_sidebar_new ()
{
  debug_functions ();

  GtkWidget *vbox_sidebar = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);

  GtkWidget *patients_title_lbl = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (patients_title_lbl), "<b>Files</b>");
  gtk_widget_override_background_color (patients_title_lbl,
					GTK_STATE_FLAG_NORMAL,
					&header_bg_color);

  gtk_box_pack_start (GTK_BOX (vbox_sidebar), patients_title_lbl, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_sidebar), scrolled, TRUE, TRUE, 0);

  sidebar_TreeStore = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_UINT64, G_TYPE_STRING);

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (sidebar_TreeStore));


  g_signal_connect (treeview, "realize",
		    G_CALLBACK (gui_mainwindow_sidebar_realized),
		    NULL);

  g_signal_connect (treeview, "row-activated",
		    G_CALLBACK (gui_mainwindow_sidebar_clicked),
		    NULL);

  gtk_widget_set_can_focus (treeview, FALSE);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", SIDEBAR_NAME, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("ID", renderer, "text", SIDEBAR_ID, NULL);
  gtk_tree_view_column_set_visible(column,FALSE);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  gtk_cell_renderer_set_alignment (renderer, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);
  return vbox_sidebar;
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

  Serie *active_serie = CONFIGURATION_ACTIVE_LAYER (config);
  if (active_serie == NULL) return;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_lookup_table_for_serie (viewers->data, active_serie, lut_name);
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

  unsigned char value = gtk_range_get_value (GTK_RANGE (widget));

  Serie *active_serie = CONFIGURATION_ACTIVE_LAYER (config);
  if (active_serie == NULL) return FALSE;

  List *viewers = list_nth (pll_Viewers, 1);
  while (viewers != NULL)
  {
    viewer_set_opacity_for_serie (viewers->data, active_serie, value);
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
  gtk_widget_override_background_color (properties_title_lbl,
					GTK_STATE_FLAG_NORMAL,
					&header_bg_color);

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

  List *lookup_tables = list_nth (CONFIGURATION_LOOKUP_TABLES (config), 1);

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
gui_mainwindow_properties_manager_refresh (Tree *serie_tree)
{
  if (pll_Viewers == NULL) return;
  if (serie_tree == NULL) return;

  gtk_widget_set_sensitive (properties_opacity_scale, TRUE);
  gtk_widget_set_sensitive (properties_lookup_table_combo, TRUE);

  unsigned char opacity;
  opacity = viewer_get_opacity_for_serie (pll_Viewers->data, serie_tree->data);
  gtk_range_set_value (GTK_RANGE (properties_opacity_scale), opacity);

  PixelDataLookupTable *lookup_table;
  lookup_table = viewer_get_active_lookup_table_for_serie (pll_Viewers->data,
                                                           serie_tree->data);

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
  if (serie_tree->type == TREE_TYPE_SERIE)
  {
    gtk_widget_set_sensitive (properties_opacity_scale, FALSE);
  }
  else
  {
    gtk_widget_set_sensitive (properties_opacity_scale, TRUE);
  }
}

