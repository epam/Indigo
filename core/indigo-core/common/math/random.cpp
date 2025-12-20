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

#include "math/random.h"
#include "base_c/defs.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace indigo;

Random::Random()
{
    randSeed = rand();
}

Random::Random(int seed)
{
    randSeed = seed;
}

void Random::setSeed(unsigned long long x)
{
    randSeed = x;
}

unsigned int Random::next()
{
    //   printf("%lld %lld \n", randSeed, 6364136223846793005LL);
    randSeed = 6364136223846793005LL * randSeed + 1;
    //   printf("%lld %lld \n", randSeed, 6364136223846793005LL);
    return (unsigned int)(randSeed >> 16);
}

unsigned int Random::next(int mod)
{
    if (mod > 0)
        return next() % mod;
    if (mod < 0)
        return next() % -mod;
    return 0;
}

unsigned int Random::nextBounded(int l, int r)
{
    return std::min(l, r) + next(abs(r - l));
}

unsigned int Random::nextLarge(int mod)
{
    int x = next();
    if ((1LL << 32) - x > mod)
        return x % mod;

    int max = static_cast<int>((1LL << 32) - (1LL << 32) % mod);
    if (x < max)
        return x % mod;
    return nextLarge(mod);
}

unsigned long long Random::nextLong()
{
    return ((unsigned long long)next() << 32) + next();
}

unsigned long long Random::nextLong(unsigned long long mod)
{
    return nextLong() % mod;
}

double Random::nextDouble()
{
    return 1.0 * next() / (1LL << 32);
}

double Random::nextDoubleBounded(double l, double r)
{
    return std::min(l, r) + nextDouble() * fabs(r - l);
}
