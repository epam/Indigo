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

#ifndef __transformation__
#define __transformation__

#include <memory>

#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class Transformation
    {
    public:
        Transformation() : rotate(0), shift(Vec2f(0, 0)){};
        Transformation(float rotate) : rotate(rotate), shift(Vec2f(0, 0)){};
        Transformation(const Vec2f& shift) : rotate(0), shift(shift){};
        Transformation(float rotate, const Vec2f& shift) : rotate(rotate), shift(shift){};
        Transformation(const Transformation& other) : rotate(other.rotate), shift(other.shift){};
        Transformation& operator=(const Transformation& other)
        {
            rotate = other.rotate;
            shift = other.shift;
            return *this;
        }

        float rotate;
        Vec2f shift;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
