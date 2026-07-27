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
        const double EPS_ZERO = 1e-5;
        const double SCALE_TOL = 1e-4;
        const double ORTHO_TOL = 1e-3;

    public:
        DECL_ERROR;

        enum class FlipType
        {
            none,
            horizontal,
            vertical
        };

        Transformation() : rotate(0), shift(Vec2f(0, 0)), flip(FlipType::none), scale(1.0f){};
        Transformation(float rotate) : rotate(rotate), shift(Vec2f(0, 0)), flip(FlipType::none), scale(1.0f){};
        Transformation(const Vec2f& shift) : rotate(0), shift(shift), flip(FlipType::none), scale(1.0f){};
        Transformation(float rotate, const Vec2f& shift) : rotate(rotate), shift(shift), flip(FlipType::none), scale(1.0f){};
        Transformation(float rotation, const Vec2f& shift, std::string flip);
        Transformation(const Transformation& other) : rotate(other.rotate), shift(other.shift), flip(other.flip), scale(1.0f){};
        Transformation& operator=(const Transformation& other)
        {
            rotate = other.rotate;
            shift = other.shift;
            flip = other.flip;
            scale = other.scale;
            return *this;
        }
        const std::string getFlip() const;
        bool fromAffineMatrix(const Mat23& M)
        {
            const double a11 = M[0][0], a12 = M[0][1], tx = M[0][2];
            const double a21 = M[1][0], a22 = M[1][1], ty = M[1][2];

            const double sx = std::hypot(a11, a21);
            const double sy = std::hypot(a12, a22);
            if (sx < EPS_ZERO || sy < EPS_ZERO || std::fabs(sx - sy) > SCALE_TOL * std::max(sx, sy))
                return false;

            const double s = 0.5 * (sx + sy);

            const int diag[3][2] = {{1, 1}, {1, -1}, {-1, 1}};
            const FlipType kinds[3] = {FlipType::none, FlipType::vertical, FlipType::horizontal};

            int best_k = -1;
            double best_abs = 0.0;
            double best_r00 = 1.0, best_r10 = 0.0;

            for (int k = 0; k < 3; ++k)
            {
                const double fx = diag[k][0], fy = diag[k][1];

                const double r00 = (a11 * fx) / s;
                const double r01 = (a12 * fy) / s;
                const double r10 = (a21 * fx) / s;
                const double r11 = (a22 * fy) / s;

                const double c0 = r00 * r00 + r10 * r10;
                const double c1 = r01 * r01 + r11 * r11;
                const double dot = r00 * r01 + r10 * r11;
                const double det = r00 * r11 - r01 * r10;

                if (std::fabs(c0 - 1) > ORTHO_TOL || std::fabs(c1 - 1) > ORTHO_TOL || std::fabs(dot) > ORTHO_TOL || det <= 0)
                    continue;

                const double ang = std::atan2(r10, r00);
                const double abs_ang = std::fabs(std::remainder(ang, 2 * M_PI));

                if (best_k == -1 || abs_ang < best_abs)
                {
                    best_k = k;
                    best_abs = abs_ang;
                    best_r00 = r00;
                    best_r10 = r10;
                }
            }

            if (best_k == -1)
                return false;

            flip = kinds[best_k];
            rotate = static_cast<float>(std::atan2(best_r10, best_r00));
            if (std::fabs(rotate) < EPS_ZERO)
                rotate = 0;
            scale = static_cast<float>(s);
            if (std::fabs(scale) < EPS_ZERO)
                scale = 0;
            shift.set(static_cast<float>(tx), static_cast<float>(ty));
            return true;
        }

        const bool hasTransformation() const
        {
            return std::fabs(rotate) > EPS_ZERO || std::fabs(shift.x) > EPS_ZERO || std::fabs(shift.y) > EPS_ZERO || flip != FlipType::none;
        }

        float rotate;
        float scale;
        Vec2f shift;
        FlipType flip;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
