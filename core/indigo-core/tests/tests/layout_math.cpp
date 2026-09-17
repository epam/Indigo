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

// The arithmetic the 2D layout is built on: ticket #3844, invariant A10 of
// .memory-bank/invariants.md - the same molecule is drawn the same way on
// Windows and on Linux.

#include <cmath>

#include <gtest/gtest.h>

#include <math/algebra.h>

using namespace indigo;

namespace
{
    // The layout works in angles of a turn or two either way. Over this sweep the
    // float function and the double one disagree on 259 samples of the cosine and
    // 3843 of the arc cosine under MSVC, and on 2563 and 15623 under glibc, so a
    // float spelling cannot pass unseen. The arc tangent differs under glibc
    // only, and the hypotenuse on neither: those two state the contract without
    // discriminating.
    const int SAMPLES = 200000;
    const float TWO_PI = _2FLOAT(2. * M_PI);

    // A point off both axes, so that a rotation of it mixes the sine and the
    // cosine into both coordinates.
    const float POINT_X = 1.3f;
    const float POINT_Y = -0.7f;

    // Far enough from the axis for the sign of the tilt to be unambiguous.
    const float OFF_AXIS = 0.9f;

    float sweptAngle(int i)
    {
        return -TWO_PI + 2.f * TWO_PI * static_cast<float>(i) / static_cast<float>(SAMPLES - 1);
    }

    // The unit interval the arc cosine takes, endpoints included.
    float sweptUnit(int i)
    {
        return -1.f + 2.f * static_cast<float>(i) / static_cast<float>(SAMPLES - 1);
    }

    float rounded(double value)
    {
        return static_cast<float>(value);
    }
}

// Every macro evaluates in double and rounds once. For a float argument a
// compiler picks the float function instead - cosf, sinf, acosf, atan2f - and it
// differs in the last bit; the layout then chooses differently between two equal
// drawings, and the two platforms disagree.
TEST(LayoutMathContract, CosineAndSineEvaluateInDouble)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        const float angle = sweptAngle(i);
        ASSERT_EQ(rounded(std::cos(static_cast<double>(angle))), COS(angle)) << "cosine of " << angle;
        ASSERT_EQ(rounded(std::sin(static_cast<double>(angle))), SIN(angle)) << "sine of " << angle;
    }
}

TEST(LayoutMathContract, ArcCosineEvaluatesInDouble)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        const float cosine = sweptUnit(i);
        ASSERT_EQ(rounded(std::acos(static_cast<double>(cosine))), ACOS(cosine)) << "arc cosine of " << cosine;
    }
}

TEST(LayoutMathContract, ArcTangentAndHypotenuseEvaluateInDouble)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        const float y = sweptAngle(i);
        const float x = 1.f + 3.f * static_cast<float>(i) / static_cast<float>(SAMPLES - 1);

        ASSERT_EQ(rounded(std::atan2(static_cast<double>(y), static_cast<double>(x))), ATAN2(y, x)) << "arc tangent of " << y << ", " << x;
        ASSERT_EQ(rounded(std::hypot(static_cast<double>(y), static_cast<double>(x))), HYPOT(y, x)) << "hypotenuse of " << y << ", " << x;
    }
}

// Both rotations take their sine and cosine the same way. Every placement of the
// layout passes through them, so this is where a float spelling would reach the
// drawing first.
TEST(LayoutMathContract, RotationTurnsByTheDoubleSineAndCosine)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        const float angle = sweptAngle(i);
        const float si = rounded(std::sin(static_cast<double>(angle)));
        const float co = rounded(std::cos(static_cast<double>(angle)));

        Vec2f right(POINT_X, POINT_Y);
        right.rotate(angle);
        ASSERT_EQ(co * POINT_X - si * POINT_Y, right.x) << "rotation by " << angle;
        ASSERT_EQ(si * POINT_X + co * POINT_Y, right.y) << "rotation by " << angle;

        Vec2f left(POINT_X, POINT_Y);
        left.rotateL(angle);
        ASSERT_EQ(co * POINT_X + si * POINT_Y, left.x) << "left rotation by " << angle;
        ASSERT_EQ(-si * POINT_X + co * POINT_Y, left.y) << "left rotation by " << angle;
    }
}

TEST(LayoutMathContract, TiltAngleIsTheDoubleArcCosine)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        Vec2f above(sweptUnit(i), OFF_AXIS);
        ASSERT_EQ(rounded(std::acos(static_cast<double>(above.x / above.length()))), above.tiltAngle()) << "tilt of " << above.x << ", " << above.y;

        Vec2f below(sweptUnit(i), -OFF_AXIS);
        ASSERT_EQ(-rounded(std::acos(static_cast<double>(below.x / below.length()))), below.tiltAngle()) << "tilt of " << below.x << ", " << below.y;
    }
}

// The full-turn tilt rounds the arc cosine to float before subtracting it from
// two pi. Folding the two roundings into one is arithmetically better and moves
// two atoms of layout/macrocycles.py by 1.28 (#3844), so the rounding stays.
TEST(LayoutMathContract, FullTurnTiltRoundsTheArcCosineBeforeSubtracting)
{
    for (int i = 0; i < SAMPLES; i++)
    {
        Vec2f below(sweptUnit(i), -OFF_AXIS);
        const float arc_cosine = rounded(std::acos(static_cast<double>(below.x / below.length())));

        ASSERT_EQ(rounded(2. * M_PI - static_cast<double>(arc_cosine)), below.tiltAngle2()) << "full-turn tilt of " << below.x << ", " << below.y;
    }
}
