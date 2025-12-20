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

#ifndef __tversky_coef__
#define __tversky_coef__

#include "base_c/bitarray.h"
#include "bingo_sim_coef.h"
#include <math.h>

namespace bingo
{
    class TverskyCoef : public SimCoef
    {
    public:
        TverskyCoef(int fp_size);

        TverskyCoef(int fp_size, double a, double b);

        double calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01);

    private:
        int _fp_size;
        double _alpha;
        double _beta;
    };
}; // namespace bingo

#endif /* __tversky_coef__ */
