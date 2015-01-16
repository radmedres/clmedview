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

#ifndef PIXELDATA_H
#define PIXELDATA_H

#include "lib-memory.h"
#include "lib-memory-slice.h"
#include "lib-common.h"


/**
 * @file   include/lib-pixeldata.h
 * @brief  The glue between Memory and Viewer.
 * @author Marc Geerlings, Jos Slenter, Roel Janssen
 */


/**
 * @defgroup pixeldata Pixeldata
 * @{
 *
 * This module provides functions to convert the internal memory format to a
 * displayable format. Making it the essential glue between Memory and Viewer.
 */


/*----------------------------------------------------------------------------.
 | TYPES                                                                      |
 '----------------------------------------------------------------------------*/


/**
 * An enumeration of actions that can be applied.
 *
 * @note ACTION_ERASE has to be 0 because that is the only guaranteed
 * transparent value.
 */
typedef enum
{
  ACTION_ERASE = 0,
  ACTION_SET
} PixelAction;


/**
 * This structure can be used to store a lookup table.
 */
typedef struct
{
  int index; /*< An index number for sorting items. */
  char *name; /*< The name of the lookup table. */
  unsigned int *table; /*< The lookup table's values. */
  unsigned int table_len; /*< The allocated number of bytes for 'table'. */
} PixelDataLookupTable;


/**
 * This structure contains the values needed to generate RGB buffers
 * effectively and efficiently.
 */
typedef struct
{
  Range WWWL; /*< Window width and window level values packed as a range. */

  Slice *slice; /*< The current slice. */
  Serie *serie; /*< The Serie it belongs to. */

  unsigned int *color_lookup_table; /*< The color lookup table. */
  unsigned int *display_lookup_table; /*< The display lookup table. */
  unsigned int *rgb; /*< The RGB data. */

  unsigned int display_lookup_table_len; /*< The allocated size of the
                                             display LUT. */

  unsigned char alpha; /*< The alpha channel value. */

  PixelDataLookupTable *color_lookup_table_ptr; /*< A pointer to the active
						    color lookup table. */
  
} PixelData;


/*----------------------------------------------------------------------------.
 | MACROS                                                                     |
 '----------------------------------------------------------------------------*/

/**
 * A macro to provide access to the active Slice.
 */
#define PIXELDATA_ACTIVE_SLICE(x) (x->slice)


/**
 * A macro to provide access to the active Serie.
 */
#define PIXELDATA_ACTIVE_SERIE(x) (x->serie)


/**
 * A macro to provide access to the Slice's data.
 */
#define PIXELDATA_ACTIVE_SLICE_DATA(x) ((void **)((Slice *)x->slice)->data)


/**
 * A macro to provide access to the PixelData's RGB buffer.
 */
#define PIXELDATA_RGB(x) (x->rgb)

/**
 * A macro to provide access to the PixelData's active color lookup table.
 */
#define PIXELDATA_LOOKUP_TABLE(x) (x->color_lookup_table_ptr)


/*----------------------------------------------------------------------------.
 | FUNCTIONS                                                                  |
 '----------------------------------------------------------------------------*/


/**
 * This function allocates the resources to be able to generate RGB buffers.
 *
 * @return A pointer to an allocated PixelData on success or
 *         NULL on failure.
 */
PixelData* pixeldata_new ();


/**
 * This function cleans up the resources that were initialized by
 * pixeldata_new() or pixeldata_new_with_lookup_table().
 *
 * @param data  The PixelData instance.
 */
void pixeldata_destroy (void *data);


/**
 * This function destroys a single lookup table list item.
 *
 * @param data  A pointer to a PixelDataLookupTable structure.
 */
void pixeldata_lookup_table_destroy_item (void *data);


/**
 * This function creates a look-up table according to a dynamically loaded
 * lookup table.
 *
 * @param pixeldata  A pointer to the PixelData to set the LUT for.
 * @param name       The name of a dynamically loaded LUT.
 *
 * @return 1 on success, 0 on failure.
 */
short pixeldata_set_color_lookup_table (PixelData *pixeldata, const char *name);


/**
 * This function queries the loaded lookup tables to find the one by 'name'.
 *
 * @return The color lookup table with the specified name or NULL.
 */
PixelDataLookupTable* pixeldata_lookup_table_get_by_name (const char *name);


/**
 * This function queries the loaded lookup tables to find the default.
 *
 * @return The default color lookup table.
 */
PixelDataLookupTable* pixeldata_lookup_table_get_default ();


/**
 * This function queries the loaded lookup tables to find the default mask
 * lookup table.
 *
 * @return The default color lookup table for masks.
 */
PixelDataLookupTable* pixeldata_lookup_table_get_default_mask ();


/**
 * This function queries the loaded lookup tables to find the default overlay
 * lookup table.
 *
 * @return The default color lookup table for overlays.
 */
PixelDataLookupTable* pixeldata_lookup_table_get_default_overlay ();


/**
 * This function recalculates the lookup table. Without running this function,
 * your newly selected lookup table is not used (yet). This function is used
 * by pixeldata_set_color_lookup_table(), so you probably don't have to call it
 * yourself.
 * 
 * @param pixeldata  A pointer to the PixelData to apply the LUT of.
 */
void pixeldata_apply_lookup_table (PixelData *pixeldata);


/**
 * This function creates a look-up table defining the window level.
 *
 * @return 1 on success, 0 on failure.
 */
short pixeldata_set_window_width_window_level (PixelData *pixeldata,
                                               int minimum,
                                               int maximum);


/**
 * This function creates an RGB buffer for a given PixelData. This function
 * assumes a succesful call to pixeldata_set_color_lookup_table() and
 * pixeldata_set_window_width_window_level().
 *
 * @param data  The PixelData to create a RGB buffer for.
 *
 * @return The RGB buffer.
 */
unsigned int* pixeldata_create_rgb_pixbuf (PixelData *data);


/**
 * This function destroys an RGB buffer created by 
 * pixeldata_create_rgb_pixbuf().
 *
 * @param pixbuf  The pixbuf returned by pixeldata_create_rgb_pixbuf().
 */
void pixeldata_destroy_rgb_pixbuf (unsigned int* pixbuf);


/**
 * This function returns the value of a pixel at a given point as a string.
 *
 * @param data  The PixelData.
 * @param ts_Point   The pixel's position.
 *
 * @return A dynamically allocated string. You must free() this string yourself.
 */
char* pixeldata_get_pixel_value_as_string (PixelData *data, Coordinate ts_Point);


/**
 * This function sets the active displayable slice to a given slice.
 *
 * @param pixeldata  The PixelData to set the active slice for.
 * @param slice      A valid pointer to a Slice.
 *
 * @return 1 on success or 0 on failure.
 */
short int pixeldata_set_slice (PixelData *pixeldata, Slice *slice);


/**
 * This function is a convenience wrapper to set up a display conversion in
 * one call. It calls other functions in the library to set everything up.
 *
 * @param lut              The look-up table to use.
 * @param i32_MinPixValue  The window-level minimum value.
 * @param i32_MaxPixValue  The window-level maximum value.
 * @param slice            A pointer to a Slice.
 * @param serie            The serie to create pixeldata for.
 *
 * @return A pointer to allocated PixelData on succes or NULL on failure.
 */
PixelData * pixeldata_new_with_lookup_table (PixelDataLookupTable *lut,
                                             int i32_MinPixValue,
                                             int i32_MaxPixValue,
                                             Slice *slice,
                                             Serie *serie);


/**
 * This function sets the alpha, or transparency, for a PixelData object.
 *
 * @param pixeldata        The PixelData object to set the alpha for.
 * @param alpha            The alpha value: 0x00 to 0xFF.
 */
void pixeldata_set_alpha (PixelData *pixeldata, unsigned char alpha);


/**
 * This function sets the alpha, or transparency, for a PixelData object.
 *
 * @param pixeldata        The PixelData object to get the alpha of.
 */
unsigned char pixeldata_get_alpha (PixelData *pixeldata);


/**
 * This function sets the mask to active on a line defined by two coordinates.
 *
 * @param ps_Original       The PixelData of the original image.
 * @param ps_Mask           The PixelData of the mask to draw onto.
 * @param ps_Selection      The PixelData of the selection mask.
 * @param ts_Start          The start coordinate.
 * @param ts_End            The end coordinate.
 * @param ui32_BrushScale   The scaling factor for the brush.
 * @param ui32_BrushValue   The value for the brush.
 * @param te_Action         The action to apply.
 * @param fp_BrushCallback  The brush callback function.
 */
void pixeldata_apply_brush (PixelData *ps_Original, PixelData *ps_Mask,
			    PixelData *ps_Selection, Coordinate ts_Start,
			    Coordinate ts_End, unsigned int ui32_BrushScale,
			    unsigned int ui32_BrushValue, PixelAction te_Action,
			    void (*fp_BrushCallback)(PixelData*, PixelData*, PixelData*,
						     Coordinate, unsigned int, unsigned int, PixelAction));


/**
 * This function loads a LUT file. The format for the file should be:
 * Index, Red, Green, Blue
 *
 * @param filename  The filename to load.
 *
 * @return 1 on success, 0 on failure.
 */
short int pixeldata_lookup_table_load_from_file (const char *filename);


/**
 * This function loads all LUT files in a given directory.
 *
 * @param filename  The filename to load.
 *
 * @return 1 on success, 0 on failure.
 */
short int pixeldata_lookup_table_load_from_directory (const char *filename);


/**
 * For the sake of abstracting the complexities of changing the value of a
 * voxel this function should be used to perform this task.
 *
 * @param mask        The mask to use for drawing.
 * @param selection   The selection mask to use for whether the value may be set.
 * @param point       The coordinate to determine the pixel.
 * @param value       The value to set the pixel to.
 * @param action      The action to apply (draw or erase)
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
short int pixeldata_set_voxel (PixelData *mask, PixelData *selection,
			       Coordinate point, unsigned int value,
			       PixelAction action);


/**
 * For the sake of abstracting the complexities of getting the value of a voxel
 * this function should be used to perform this task.
 *
 * @param mask    The mask to use for drawing.
 * @param point   The coordinate to get the pixel value for.
 * @param value   A pointer to the value of the pixel. The type of the value 
 *                depends on the image type. 
 *
 * @return 1 when the request is valid, 0 when the request is invalid.
 */
short int pixeldata_get_voxel (PixelData *mask, Coordinate point, void *value);


/**
 * @}
 */

#endif//PIXELDATA_H
