/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "math/statistics.h"

#include <math.h>

using namespace indigo;

//
// MeanEstimator
//

MeanEstimator::MeanEstimator() : _count(0), _sum(0), _sum_sq(0)
{
}

void MeanEstimator::addValue(float value)
{
    _sum += value;
    _sum_sq += value * value;
    _count++;
}

int MeanEstimator::getCount() const
{
    return _count;
}

void MeanEstimator::setCount(int count)
{
    _count = count;
}

float MeanEstimator::mean() const
{
    if (_count == 0)
        return 0;

    return _sum / _count;
}

float MeanEstimator::meanEsimationError() const
{
    if (_count == 0)
        return 0;

    float sigma = sqrt(_sum_sq / _count - pow(_sum / _count, 2));

    return 2 * sigma / sqrt((float)_count);
}
