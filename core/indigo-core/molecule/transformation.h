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

#include "common/base_cpp/exception.h"
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
        DECL_ERROR;

        enum class FlipType
        {
            none,
            horizontal,
            vertical
        };

        Transformation() : angle(0), shift(Vec2f(0, 0)), flip(FlipType::none){};
        Transformation(float rotate) : angle(rotate), shift(Vec2f(0, 0)), flip(FlipType::none){};
        Transformation(const Vec2f& shift) : angle(0), shift(shift), flip(FlipType::none){};
        Transformation(float rotate, const Vec2f& shift) : angle(rotate), shift(shift), flip(FlipType::none){};
        Transformation(float rotation, const Vec2f& shift, std::string flip);
        Transformation(const Transformation& other) : angle(other.angle), shift(other.shift), flip(other.flip){};
        Transformation& operator=(const Transformation& other)
        {
            angle = other.angle;
            shift = other.shift;
            flip = other.flip;
            return *this;
        }
        const std::string getFlip() const;

        const bool hasTransformation() const
		{
			return angle != 0 || shift.x != 0 || shift.y != 0 || flip != FlipType::none;
		}

        void apply(Vec3f& vec) const
        {
            Vec2f vec2(vec.x, vec.y);
            apply(vec2);
            vec.x = vec2.x;
            vec.y = vec2.y;
        }

        void apply(Vec2f& vec) const
        {
            // first goes flip
            switch ( flip )
            {
                case FlipType::horizontal:
					vec.x = -vec.x;
					break;
				case FlipType::vertical:
					vec.y = -vec.y;
					break;
                default:
                    break;
            }
            // then goes rotation
            if (angle != 0)
                vec.rotate(angle);

            // then goes shift
            vec.add(shift);
        }

        float angle;
        Vec2f shift;
        FlipType flip;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
