#include "libhistogram.h"
#include "libmemory-serie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Histogram *
histogram_new (void)
{
  return calloc (1, sizeof (Histogram));
}

void
histogram_set_serie (Histogram *histogram, Serie *serie)
{
  if (histogram == NULL || serie == NULL) return;

  /* Obtain the minimum and maximum value of the dataset. */
  histogram->minimum.x = serie->i32_MinimumValue;
  histogram->maximum.x = serie->i32_MaximumValue;

  /* Allocate memory for the histogram data. */
  int shift = abs(histogram->minimum.x);
  histogram->data_len = histogram->maximum.x + shift;
  histogram->data = calloc (sizeof (int), histogram->data_len);

  void **data = serie->data;

   if (data == NULL) return;

  /* Set the data points.  */
  unsigned int blob_size = serie->matrix.i16_x * serie->matrix.i16_y * serie->matrix.i16_z;
  unsigned int index;
  int data_index;
  for (index = 0; index < (unsigned int)blob_size; index++)
  {
    data_index = 0;

    switch (serie->data_type)
    {
      case MEMORY_TYPE_INT8    : data_index = *((unsigned char *)data) + shift; break;
      case MEMORY_TYPE_INT16   : data_index = *((short int *)data) + shift; break;
      case MEMORY_TYPE_INT32   : data_index = *((int *)data) + shift; break;
      case MEMORY_TYPE_INT64   : data_index = *((long *)data) + shift; break;
      case MEMORY_TYPE_UINT8   : data_index = *((unsigned char *)data) + shift; break;
      case MEMORY_TYPE_UINT16  : data_index = *((unsigned short int *)data) + shift; break;
      case MEMORY_TYPE_UINT32  : data_index = *((unsigned int *)data) + shift; break;
      case MEMORY_TYPE_UINT64  : data_index = *((unsigned long *)data) + shift; break;

      // TODO: Is roundf() really correct here?
      case MEMORY_TYPE_FLOAT32 : data_index = (int)(roundf(*((float *)data))) + shift; break;
      case MEMORY_TYPE_FLOAT64 : data_index = (int)(roundf(*((double *)data))) + shift; break;
      default : break;
    }    

    /* Update histogram column. */
    histogram->data[data_index] += 1;

    /* Check for maximum value. */
    if (histogram->data[data_index] > histogram->maximum.y)
      histogram->maximum.y = histogram->data[data_index];

    /* Check for minimum value. */
    if (histogram->data[data_index] < histogram->minimum.y)
      histogram->minimum.y = histogram->data[data_index];
  }

  histogram->title = calloc (1, strlen (serie->name) + 1);
  histogram->title = strcpy (histogram->title, serie->name);
}

void
histogram_draw (Histogram *histogram, cairo_t *cr, int width, int height, int x, int y)
{
  if (histogram == NULL || cr == NULL) return;

  const int padding = 10;

  /*--------------------------------------------------------------------------.
   | DRAW BACKGROUND                                                          |
   '--------------------------------------------------------------------------*/
  cairo_rectangle (cr,
		   x + padding,
		   y + padding,
		   x + width - padding * 2,
		   height - padding * 2);

  cairo_set_source_rgba (cr, 0.97, 0.97, 0.97, 1);
  cairo_fill (cr);

  cairo_stroke (cr);

  /*--------------------------------------------------------------------------.
   | DRAW MARKERS                                                             |
   '--------------------------------------------------------------------------*/
  cairo_set_source_rgba (cr, 0, 0, 0, 1);
  cairo_set_line_width (cr, 1);

  unsigned int rows;
  unsigned int num_rows = 10;
  unsigned int row_height = height / num_rows;
  for (rows = 0; rows < num_rows; rows++)
  {
    cairo_move_to (cr, 0 + padding, rows * row_height + padding);
    cairo_line_to (cr, 10 + padding, rows * row_height + padding);
  }

  cairo_stroke (cr);

  /*--------------------------------------------------------------------------.
   | DRAW DATA                                                                |
   '--------------------------------------------------------------------------*/

  /* ... work in progress. */
}

void
histogram_destroy (Histogram *histogram)
{
  if (histogram == NULL) return;

  free (histogram->title);
  free (histogram->data);
  free (histogram);
}
