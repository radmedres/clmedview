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

#include "lib-memory-quaternion.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <math.h>

ts_Quaternion *memory_quaternion_new(void)
{
  debug_functions ();

  return calloc (1, sizeof (ts_Quaternion));
}

ts_Quaternion *memory_quaternion_set(double W, double A, double B, double C)
{
  ts_Quaternion *ts_quaternion = malloc(sizeof(ts_Quaternion));

  if (ts_quaternion != NULL) {
    ts_quaternion->W = W;
    ts_quaternion->I = A;
    ts_quaternion->J = B;
    ts_quaternion->K = C;
  }
  return ts_quaternion;
}

ts_Quaternion *memory_quaternion_copy(ts_Quaternion *ps_source)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();

  if ((ps_source == NULL || ps_destination == NULL)) return NULL;

  ps_destination->W = ps_source->W;
  ps_destination->I = ps_source->I;
  ps_destination->J = ps_source->J;
  ps_destination->K = ps_source->K;
  return ps_destination;
}


double memory_quaternion_normalize(ts_Quaternion *ts_quaternion)
{
  debug_functions ();

  double d_normalisation = 0.0;

  if (ts_quaternion == NULL)
  {
    return 0.0;
  }

  d_normalisation = ts_quaternion->W * ts_quaternion->W +
                    ts_quaternion->I * ts_quaternion->I +
                    ts_quaternion->J * ts_quaternion->J +
                    ts_quaternion->K * ts_quaternion->K;

  return sqrt(d_normalisation);
}


ts_Quaternion *memory_quaternion_negative(ts_Quaternion *ps_source)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();
  if ((ps_source == NULL) || (ps_destination == NULL)) return NULL;

  ps_destination->W = -ps_source->W;
  ps_destination->I = -ps_source->I;
  ps_destination->J = -ps_source->J;
  ps_destination->K = -ps_source->K;

  return ps_destination;
}


ts_Quaternion *memory_quaternion_conjungate(ts_Quaternion *ps_source)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();

  if ((ps_source == NULL || ps_destination == NULL)) return NULL;

  ps_destination->W = ps_source->W;

  ps_destination->I = -ps_source->I;
  ps_destination->J = -ps_source->J;
  ps_destination->K = -ps_source->K;

  return ps_destination;
}


ts_Quaternion *memory_quaternion_add_value(ts_Quaternion *ps_source, double d_Value)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_copy (ps_source);

  if ((ps_source == NULL || ps_destination == NULL)) return NULL;

  ps_destination->W += d_Value;
  return ps_destination;
}


ts_Quaternion *memory_quaternion_add(ts_Quaternion *ps_first, ts_Quaternion *ps_second)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();
  if ((ps_first == NULL) || (ps_second == NULL) || (ps_destination == NULL)) return NULL;

  ps_destination->W = ps_first->W + ps_second->W;
  ps_destination->I = ps_first->I + ps_second->I;
  ps_destination->J = ps_first->J + ps_second->J;
  ps_destination->K = ps_first->K + ps_second->K;

  return ps_destination;
}

short int memory_quaternion_equal(ts_Quaternion *ps_first, ts_Quaternion *ps_second)
{
  debug_functions ();

  if ((ps_first->W == ps_second->W ) ||
      (ps_first->I == ps_second->I ) ||
      (ps_first->J == ps_second->J ) ||
      (ps_first->K == ps_second->K ))
  {
    return 1;
  }
  return 0;
}

ts_Quaternion *memory_quaternion_multiply_double(ts_Quaternion *ps_source, double d_Value)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();
  if ((ps_source == NULL || ps_destination == NULL)) return NULL;

  ps_destination->W = ps_source->W * d_Value;
  ps_destination->I = ps_source->I * d_Value;
  ps_destination->J = ps_source->J * d_Value;
  ps_destination->K = ps_source->K * d_Value;

  return ps_destination;
}


ts_Quaternion *memory_quaternion_multiply(ts_Quaternion *ps_first, ts_Quaternion *ps_second)
{
  debug_functions ();

  ts_Quaternion *ps_destination = memory_quaternion_new ();
/*
  R(0) = A(0)*B(0) - A(1)*B(1) - A(2)*B(2) - A(3)*B(3);
  R(1) = A(0)*B(1) + A(1)*B(0) + A(2)*B(3) - A(3)*B(2);
  R(2) = A(0)*B(2) - A(1)*B(3) + A(2)*B(0) + A(3)*B(1);
  R(3) = A(0)*B(3) + A(1)*B(2) - A(2)*B(1) + A(3)*B(0);
*/

  if ((ps_first == NULL) || (ps_second == NULL) || (ps_destination == NULL)) return NULL;

  ps_destination->W = ps_first->W * ps_second->W - ps_first->I * ps_second->I - ps_first->J * ps_second->J - ps_first->K * ps_second->K;
  ps_destination->I = ps_first->W * ps_second->I + ps_first->I * ps_second->W + ps_first->J * ps_second->K - ps_first->K * ps_second->J;
  ps_destination->J = ps_first->W * ps_second->J - ps_first->I * ps_second->K + ps_first->J * ps_second->W + ps_first->K * ps_second->I;
  ps_destination->K = ps_first->W * ps_second->K + ps_first->I * ps_second->J - ps_first->J * ps_second->I + ps_first->K * ps_second->W;

  return ps_destination;
}
