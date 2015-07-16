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

#include "libpixeldata.h"
#include "libmemory-slice.h"
#include "libmemory-serie.h"
#include "libcommon-debug.h"
#include "libconfiguration.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif


/* --------------------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * -------------------------------------------------------------------------- */

PixelData*
pixeldata_new ()
{
  debug_functions ();
  return calloc (1, sizeof (PixelData));
}


PixelData*
pixeldata_new_with_lookup_table (PixelDataLookupTable *lut,
                                 int i32_windowWidth,
                                 int i32_windowLevel,
                                 Slice *slice,
                                 Serie *serie)
{
  debug_functions ();

  assert (lut != NULL);
  assert (slice != NULL);

  PixelData *pixeldata = pixeldata_new ();
  assert (pixeldata != NULL);
  pixeldata->serie = serie;
  pixeldata->alpha = 255;

  pixeldata->ts_WWWL.i32_windowWidth = i32_windowWidth;
  pixeldata->ts_WWWL.i32_windowLevel = i32_windowLevel;

  if (pixeldata_set_slice (pixeldata, slice) == 0)
  {
    pixeldata_destroy (pixeldata);
    return NULL;
  }

  if (pixeldata_set_color_lookup_table (pixeldata, lut->name))
  {
    if (!strcmp (lut->name, "default-mask.lut"))
    {
      pixeldata->display_lookup_table = calloc (1, lut->table_len);
      assert (pixeldata->display_lookup_table != NULL);

      // Make a copy of the color lookup table.
      memcpy (pixeldata->display_lookup_table,
              pixeldata->color_lookup_table,
              lut->table_len);
    }
    else if (!pixeldata_calculate_window_width_level(pixeldata, 0, 0))
    {
      pixeldata_destroy (pixeldata);
      pixeldata = NULL;
    }
  }
  else
  {
    pixeldata_destroy (pixeldata);
    pixeldata = NULL;
  }

  return pixeldata;
}


void
pixeldata_destroy (void *data)
{
  debug_functions ();

  PixelData *pixeldata = data;
  if (pixeldata == NULL) return;

  // Clean up the Display LUT.
  free (pixeldata->display_lookup_table);
  pixeldata->display_lookup_table = NULL;

  // Clean up the RGB buffer.
  free (pixeldata->rgb);
  pixeldata->rgb = NULL;

  // Clean up the slice.
  memory_slice_destroy (pixeldata->slice);
  pixeldata->slice = NULL;

  // Clean up the buffer itself.
  free (pixeldata);
}


short int
pixeldata_set_color_lookup_table (PixelData *pixeldata, const char *name)
{
  debug_functions ();

  if (pixeldata == NULL) return 0;

  Configuration *config = configuration_get_default ();
  List *tables = list_nth (CONFIGURATION_LOOKUP_TABLES (config), 1);

  while (tables != NULL)
  {
    PixelDataLookupTable *lut = tables->data;
    if (lut == NULL)
    {
      tables = list_next (tables);
      continue;
    }

    if (!strcmp (lut->name, name))
    {
      pixeldata->color_lookup_table = lut->table;
      pixeldata->color_lookup_table_ptr = lut;
      break;
    }

    tables = list_next (tables);
  }

  if (tables == NULL)
  {
    debug_error ("Could not find lookup table '%s'.", name);
    return 0;
  }

  // Make the change visible.
  pixeldata_apply_lookup_table (pixeldata);

  return 1;
}


PixelDataLookupTable*
pixeldata_lookup_table_get_by_name (const char *name)
{
  if (name == NULL) return NULL;

  Configuration *config = configuration_get_default ();
  List *tables = list_nth (CONFIGURATION_LOOKUP_TABLES (config), 1);

  while (tables != NULL)
  {
    PixelDataLookupTable *lut = tables->data;
    if (lut == NULL)
    {
      tables = list_next (tables);
      continue;
    }

    if (!strcmp (lut->name, name))
    {
      return lut;
    }

    tables = list_next (tables);
  }

  return NULL;
}


PixelDataLookupTable*
pixeldata_lookup_table_get_default ()
{
  return pixeldata_lookup_table_get_by_name ("default-grey.lut");
}


PixelDataLookupTable*
pixeldata_lookup_table_get_default_mask ()
{
  return pixeldata_lookup_table_get_by_name ("default-mask.lut");
}


PixelDataLookupTable*
pixeldata_lookup_table_get_default_overlay ()
{
  return pixeldata_lookup_table_get_by_name ("default-hot256.lut");
}


void
pixeldata_apply_lookup_table (PixelData *pixeldata)
{
  debug_functions ();
  assert (pixeldata != NULL);

  Serie *serie = pixeldata->serie;
  assert (serie != NULL);

  // Get the active lookup table.
  PixelDataLookupTable *lut = pixeldata->color_lookup_table_ptr;
  assert (lut != NULL);

  // Determine the range of the values in the Serie.
  int range = serie->i32_MaximumValue - serie->i32_MinimumValue;

  // Make sure the display lookup table is allocated.
  if (pixeldata->display_lookup_table != NULL)
  {
    free (pixeldata->display_lookup_table);
  }

  pixeldata->display_lookup_table = calloc (sizeof (unsigned int), range + 1);

  assert (pixeldata->display_lookup_table != NULL);

  float f_Slope = 255.0 / (float)(pixeldata->ts_WWWL.i32_windowWidth);
  float f_Offset = 128.0 - ((float)(pixeldata->ts_WWWL.i32_windowLevel)*f_Slope);


  unsigned int *display_lookup_table = pixeldata->display_lookup_table;
  unsigned int *color_lookup_table = lut->table;

  assert (display_lookup_table != NULL);
  assert (color_lookup_table != NULL);

  /*--------------------------------------------------------------------------.
   | TRANSLATE LUT TO MINIMUM AND MAXIMUM VALUES.                             |
   '--------------------------------------------------------------------------*/
  int counter;
  int translated_value;
  unsigned int array_items = lut->table_len / sizeof (unsigned int);

  for (counter = 0; counter <= range; counter++)
  {
    translated_value = counter * f_Slope + f_Offset;

    if (translated_value < 0)
    {
      display_lookup_table[counter] = 0;
    }
    else if (translated_value >= (int)abs(array_items))
    {
      display_lookup_table[counter] = color_lookup_table[array_items - 1];
    }
    else
    {
      display_lookup_table[counter] = color_lookup_table[translated_value];
    }
  }
}

short int
pixeldata_calculate_window_width_level (PixelData *pixeldata, int i32_deltaWidth, int i32_deltaLevel)
{
  debug_functions ();
  if (pixeldata == NULL)
  {
    return 0;
  }

  pixeldata->ts_WWWL.i32_windowWidth += i32_deltaWidth;

  if (pixeldata->ts_WWWL.i32_windowWidth < 1)
  {
    pixeldata->ts_WWWL.i32_windowWidth = 1;
  }

  pixeldata->ts_WWWL.i32_windowLevel += i32_deltaLevel;
  pixeldata_apply_lookup_table (pixeldata);

  return 1;
}


short int
pixeldata_refreshData (PixelData *pixeldata, Slice *slice)
{
  debug_functions ();

  assert (slice != NULL);
  assert (pixeldata != NULL);

  Serie *serie = slice->serie;
  if (serie == NULL) return 0;

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  slice->data = memory_slice_get_data (slice);
  pixeldata->slice = slice;

  return 1;
}


short int
pixeldata_set_slice (PixelData *pixeldata, Slice *slice)
{
  debug_functions ();

  assert (slice != NULL);
  assert (pixeldata != NULL);

  Serie *serie = slice->serie;
  if (serie == NULL) return 0;

  if (pixeldata->slice != NULL)
  {
    memory_slice_destroy (pixeldata->slice);
    pixeldata->slice = NULL;
  }

  if (slice->data != NULL)
    free (slice->data), slice->data = NULL;

  slice->data = memory_slice_get_data (slice);
  pixeldata->slice = slice;

  return 1;
}


unsigned int*
pixeldata_create_rgb_pixbuf (PixelData *pixeldata)
{
  debug_functions ();

  assert (pixeldata != NULL);

  Slice *slice = pixeldata->slice;
  assert (slice != NULL);

  Serie *serie = slice->serie;
  assert (serie != NULL);

  pixeldata->rgb = realloc (pixeldata->rgb, sizeof (unsigned int) * slice->matrix.i16_x * slice->matrix.i16_y);

  unsigned int *rgb = pixeldata->rgb;
  unsigned int *display_lookup_table = pixeldata->display_lookup_table;

  void **data = slice->data;
  assert (data != NULL);

  // TODO: What happens when *data_counter contains a negative value?
  unsigned int counter;
  for (counter = 0; counter < (unsigned int)(slice->matrix.i16_x * slice->matrix.i16_y); counter++)
  {
    switch (serie->data_type)
    {
      case MEMORY_TYPE_INT8    : rgb[counter] = display_lookup_table[ *((unsigned char *)(*data)) ]; break;
      case MEMORY_TYPE_INT16   : rgb[counter] = display_lookup_table[ *((short int *)(*data)) ]; break;
      case MEMORY_TYPE_INT32   : rgb[counter] = display_lookup_table[ *((int *)(*data)) ]; break;
      case MEMORY_TYPE_INT64   : rgb[counter] = display_lookup_table[ *((long *)(*data)) ]; break;
      case MEMORY_TYPE_UINT8   : rgb[counter] = display_lookup_table[ *((unsigned char *)(*data)) ]; break;
      case MEMORY_TYPE_UINT16  : rgb[counter] = display_lookup_table[ *((unsigned short int *)(*data)) ]; break;
      case MEMORY_TYPE_UINT32  : rgb[counter] = display_lookup_table[ *((unsigned int *)(*data)) ]; break;
      case MEMORY_TYPE_UINT64  : rgb[counter] = display_lookup_table[ *((unsigned long *)(*data)) ]; break;

      // TODO: Is roundf() really correct here?
      case MEMORY_TYPE_FLOAT32 : rgb[counter] = display_lookup_table[ (int)(roundf(*((float *)(*data)))) ]; break;
      case MEMORY_TYPE_FLOAT64 : rgb[counter] = display_lookup_table[ (int)(roundf(*((double *)(*data)))) ]; break;
      default : break;

    }

    data++;
  }

  return pixeldata->rgb;
}

void
pixeldata_destroy_rgb_pixbuf (unsigned int* pixbuf)
{
  debug_functions ();

  free (pixbuf);
  pixbuf = NULL;
}


char *
pixeldata_get_pixel_value_as_string (PixelData *pixeldata, Coordinate ts_Point)
{
  debug_functions ();

  assert (pixeldata != NULL);

  char *output = calloc (1, 255);
  assert (output != NULL);

  Slice *slice = PIXELDATA_ACTIVE_SLICE (pixeldata);
  assert (slice != NULL);

  if ((ts_Point.x > slice->matrix.i16_x || ts_Point.y > slice->matrix.i16_y)
      || (ts_Point.x < 0 || ts_Point.y < 0))
  {
    sprintf (output, "-");
    return output;
  }

  void **source = PIXELDATA_ACTIVE_SLICE_DATA (pixeldata);
  assert (source != NULL);

  short int i16_X  = (short int)ts_Point.x;
  short int i16_Y  = (short int)ts_Point.y;

  source += (unsigned int)(i16_Y * slice->matrix.i16_x + i16_X);

  switch (pixeldata->serie->data_type)
  {
    case MEMORY_TYPE_INT8       : sprintf (output, "%c", *((char*)*source)); break;
    case MEMORY_TYPE_INT16      : sprintf (output, "%d", *((short int*)*source)); break;
    case MEMORY_TYPE_INT32      : sprintf (output, "%d", *((int*)*source)); break;
    case MEMORY_TYPE_UINT8      : sprintf (output, "%hhu", *((unsigned char*)*source)); break;
    case MEMORY_TYPE_UINT16     : sprintf (output, "%hu", *((short unsigned int*)*source)); break;
    case MEMORY_TYPE_UINT32     : sprintf (output, "%u", *((unsigned int*)*source)); break;
    case MEMORY_TYPE_FLOAT32    : sprintf (output, "%.2f", *((float*)*source)); break;
    case MEMORY_TYPE_FLOAT64    : sprintf (output, "%.2f", *((double*)*source)); break;
    default                     : sprintf (output, "Unknown"); break;
  }

  return output;
}


short int
pixeldata_lookup_table_load_from_file (const char *filename)
{
  debug_functions ();
  assert (filename != NULL);

  // Open the file read-only in binary mode. This is important for portability
  // with win32. On POSIX systems this shouldn't hurt either.
  FILE* file = fopen (filename, "rb");

  // Don't bother when the file cannot be opened.
  if (file == NULL)
  {
    debug_error ("Cannot open file '%s'\r\n", filename);
    return 0;
  }

  // Allocate memory for the lookup table.
  // Determine the size of the file.
  fseek (file, 0L, SEEK_END);
  size_t data_len = ftell (file);
  fseek (file, 0L, SEEK_SET);

  char *data = calloc (1, data_len + 1);

  unsigned int index = 0;
  unsigned char red = 0;
  unsigned char green = 0;
  unsigned char blue = 0;
  unsigned char alpha = 255;

  unsigned int lookup_table_len = 0;
  unsigned int *lookup_table;

  if (fread (data, 1, data_len, file) != data_len)
  {
    debug_error ("Error loading file: Couldn't load file to memory.");

    free (data);
    data = NULL;

    return 0;
  }

  // Read the first line.
  char line[255];
  memset (line, '\0', 255);

  // Skip the first line.
  char *values = strchr (data, '\n');
  assert (values != NULL);

  // Find the maximum index value.
  // CSV format: index, red, green, blue
  while (values != NULL && sscanf (values, "%u\t%hhu\t%hhu\t%hhu\n", &index, &red, &green, &blue) == 4)
  {
    if (index > lookup_table_len)
      lookup_table_len = index;

    values = strchr (values + 1, '\n');

    if (values != NULL) values += 1;
  }

  // Make sure we have values to parse.
  if (lookup_table_len < 1)
  {
    debug_warning ("The lookup table in '%s' seems to be empty.", filename);

    free (data);
    data = NULL;

    return 0;
  }

  // Allocate the size of the lookup table.
  lookup_table = calloc (sizeof (unsigned int), lookup_table_len + 1);
  assert (lookup_table != NULL);

  values = strchr (data, '\n');
  assert (values != NULL);

  // Assign the values to the lookup table.
  // CSV format: index, red, green, blue
  while (values != NULL && sscanf (values, "%u\t%hhu\t%hhu\t%hhu\n", &index, &red, &green, &blue) == 4)
  {
    alpha = ((index + red + green + blue) == 0) ? 0 : 255;
    lookup_table[index] = red + (green << 8) + (blue << 16) + (alpha << 24);

    values = strchr (values, '\n');
    if (values != NULL) values += 1;
  }

  char *filename_only;
  filename_only = strrchr (filename, PATH_SEPARATOR);

  if (filename_only != NULL)
    filename_only += 1;
  else
    filename_only = (char *)filename;

  // Create a list item to store the lookup table in a list.
  PixelDataLookupTable *list_item = calloc (1, sizeof (PixelDataLookupTable));
  assert (list_item != NULL);

  list_item->name = calloc (1, strlen (filename_only) + 1);
  assert (list_item->name != NULL);

  list_item->name = strcpy (list_item->name, filename_only);

  list_item->table = lookup_table;
  list_item->table_len = (lookup_table_len + 1) * sizeof (unsigned int);

  // Add it to the global configuration.
  Configuration *config = configuration_get_default ();

  List *lookup_tables = CONFIGURATION_LOOKUP_TABLES (config);
  lookup_tables = list_append (lookup_tables, list_item);

  CONFIGURATION_LOOKUP_TABLES (config) = lookup_tables;

  free (data);
  data = NULL;

  return 1;
}


short int
pixeldata_lookup_table_load_from_directory (const char *pc_path)
{
  DIR *ps_directory;
  struct dirent **pps_listOfNames;

  char *pc_extension;
  char *pc_fullpath;

  short int i16_NumberOfFiles;
  short int i16_fileCnt;

  debug_functions ();

  if (pc_path == NULL) return 0;

  if ((ps_directory = opendir (pc_path)) == NULL) return 0;

  i16_NumberOfFiles = scandir(pc_path, &pps_listOfNames, NULL, alphasort);
  if (i16_NumberOfFiles < 0)
  {
    closedir (ps_directory);
    return 0;
  }
  else
  {
    for(i16_fileCnt=0; i16_fileCnt<i16_NumberOfFiles; i16_fileCnt++)
    {
      pc_extension = strrchr (pps_listOfNames[i16_fileCnt]->d_name, '.');

      if ((pps_listOfNames[i16_fileCnt]->d_name[0] != '.') && (strcmp (pc_extension, ".lut") == 0))
      {
        pc_fullpath = calloc (1, strlen (pc_path) + strlen (pps_listOfNames[i16_fileCnt]->d_name) + 2);

        sprintf (pc_fullpath, "%s%c%s", pc_path, PATH_SEPARATOR, pps_listOfNames[i16_fileCnt]->d_name);

        pixeldata_lookup_table_load_from_file (pc_fullpath);

        free (pc_fullpath), pc_fullpath = NULL;
        free(pps_listOfNames[i16_fileCnt]);
      }
    }
    free(pps_listOfNames);
  }

  closedir (ps_directory);

  return 1;
}


void
pixeldata_lookup_table_destroy_item (void *data)
{
  PixelDataLookupTable *item = data;
  if (item == NULL) return;

  free (item->name);
  item->name = NULL;

  free (item->table);
  item->table = NULL;

  free (item);
}


void
pixeldata_set_alpha (PixelData *pixeldata, unsigned char alpha)
{
  assert (pixeldata != NULL);
  pixeldata->alpha = alpha;
}


unsigned char
pixeldata_get_alpha (PixelData *pixeldata)
{
  assert (pixeldata != NULL);
  return pixeldata->alpha;
}


short int
pixeldata_set_voxel (PixelData *mask, PixelData *selection,
                     Coordinate point, unsigned int value,
                     PixelAction action)
{
  // We assume the PixelData is allocated.
  assert (mask != NULL);

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (mask);
  assert (mask_slice != NULL);

  // Boundary checks.
  if (point.x < 0 || point.y < 0) return 0;
  if (point.x >= mask_slice->matrix.i16_x || point.y >= mask_slice->matrix.i16_y) return 0;

  void **ppv_SelectionDataCounter = NULL;
  if (selection != NULL)
  {
    ppv_SelectionDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (selection);
    ppv_SelectionDataCounter += (unsigned int)(point.y * mask_slice->matrix.i16_x + point.x);
  }

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (mask);
  ppv_ImageDataCounter += (unsigned int)(point.y * mask_slice->matrix.i16_x + point.x);

  switch (mask->serie->data_type)
  {
  case MEMORY_TYPE_INT16:
    {
      if (action == ACTION_ERASE)
      {
        if (*(short int *)*ppv_ImageDataCounter == (short int)value
            && (selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
        {
          *(short int *)*ppv_ImageDataCounter = 0;
        }
      }
      else
      {
        if (*(short int *)*ppv_ImageDataCounter == 0
            && (selection == NULL || (*((short int *)*ppv_SelectionDataCounter))))
        {
          *(short int *)*ppv_ImageDataCounter = (short int)value;
        }
      }
    }
    break;
  default:
    return 0;
    break;
  }

  return 1;
}


short int
pixeldata_get_voxel (PixelData *layer, Coordinate point, void *value)
{
  // We assume the PixelData is allocated.
  assert (layer != NULL);

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (layer);
  assert (mask_slice != NULL);

  // Boundary checks.
  if (point.x < 0 || point.y < 0) return 0;
  if (point.x >= mask_slice->matrix.i16_x || point.y >= mask_slice->matrix.i16_y) return 0;

  short int i16_Y = (short int)point.y;
  short int i16_X = (short int)point.x;

  void **ppv_ImageDataCounter = PIXELDATA_ACTIVE_SLICE_DATA (layer);
  ppv_ImageDataCounter += (unsigned int)(i16_Y * mask_slice->matrix.i16_x + i16_X);

  switch (layer->serie->data_type)
  {
    case MEMORY_TYPE_INT8    : *(char *)value               = *(char *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_INT16   : *(short int *)value          = *(short int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_INT32   : *(int *)value                = *(int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT8   : *(unsigned char *)value      = *(unsigned char *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT16  : *(short unsigned int *)value = *(short unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_UINT32  : *(unsigned int *)value       = *(unsigned int *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT32 : *(float *)value              = *(float *)*ppv_ImageDataCounter; break;
    case MEMORY_TYPE_FLOAT64 : *(double *)value             = *(double *)*ppv_ImageDataCounter; break;
    default                  : assert (NULL != NULL); break;
  }

  return 1;
}
