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

#ifndef _RANDOM_H_
#define _RANDOM_H_
namespace indigo
{

    class Random
    {
    private:
        unsigned long long randSeed;

    public:
        Random();
        Random(int seed);

        void setSeed(unsigned long long x);

        unsigned int next();
        unsigned int next(int mod);
        unsigned int nextBounded(int l, int r);
        unsigned int nextLarge(int mod);

        unsigned long long nextLong();
        unsigned long long nextLong(unsigned long long mod);

        double nextDouble();
        double nextDoubleBounded(double l, double r);
    };

} // namespace indigo

#endif