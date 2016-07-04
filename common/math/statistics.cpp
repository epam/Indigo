/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "math/statistics.h"

#include <math.h>

using namespace indigo;

//
// MeanEstimator
//

MeanEstimator::MeanEstimator () : _count(0), _sum(0), _sum_sq(0)
{
}

void MeanEstimator::addValue (float value)
{
   _sum += value;
   _sum_sq += value * value;
   _count++;
}

int MeanEstimator::getCount () const
{
   return _count;
}

void MeanEstimator::setCount (int count)
{
   _count = count;
}
 
float MeanEstimator::mean () const
{
   if (_count == 0)
      return 0;

   return _sum / _count;
}

float MeanEstimator::meanEsimationError () const
{
   if (_count == 0)
      return 0;

   float sigma = sqrt(_sum_sq / _count - pow(_sum / _count, 2));

   return 2 * sigma / sqrt((float)_count);
}
