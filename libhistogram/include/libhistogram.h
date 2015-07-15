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
#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include "libcommon.h"
#include "libmemory-serie.h"
#include <cairo.h>

/**
 * @file include/libhistogram.h
 * @brief A widget to display the histogram of a dataset.
 * @author Roel Janssen
 */


/**
 * @defgroup histogram Histogram
 * @{
 *
 * This module provides a histogram widget for a dataset. It can only draw
 * itself on a Cairo surface.
 */

/**
 * A type that contains the necesarry data for a histogram.
 */
typedef struct
{
  char *title;
  int *data;

  Coordinate minimum;
  Coordinate maximum;

} Histogram;

/**
 * Constructor for a histogram widget.
 *
 * @return A newly allocated Histogram instance.
 */
Histogram * histogram_new (void);

/**
 * Function to set the data for the histogram widget.
 *
 * @param histogram  The Histogram to set the data for.
 * @param serie      The Serie that contains the data.
 */
void histogram_set_serie (Histogram *histogram, Serie *serie);

/**
 * Function to draw the histogram on a Cairo surface.
 *
 * @param histogram  The Histogram to draw.
 * @param cr         The cairo surface to draw to.
 * @param width      The width of the drawing.
 * @param height     The height of the drawing.
 * @param x          The x position on the surface to start drawing.
 * @param y          The y position on the surface to start drawing.
 */
void histogram_draw (Histogram *histogram, cairo_t *cr, int width, int height, int x, int y);

/**
 * Destructor for a histogram instance.
 *
 * @param histogram  The Histogram instance to destroy. 
 */
void histogram_destroy (Histogram *histogram);

/**
 * @}
 */


#endif
