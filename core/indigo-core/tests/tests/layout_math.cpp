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

// The arithmetic the 2D layout is built on - the elementary-function macros of
// algebra.h and the rotations and tilt angles of Vec2f - checked against
// mathematical constants. That Windows and Linux compute the same bits is
// checked by the layout reference tests of the integration suite.

#include <cmath>

#include <gtest/gtest.h>

#include <math/algebra.h>

using namespace indigo;

namespace
{
    // A float near 2 pi is rounded by about 5e-7, and a platform's libm may be a
    // unit in the last place off; the tolerance covers both and stays far below
    // any angle or coordinate a drawing depends on.
    const float TOLERANCE = 1e-5f;

    // A point off both axes, so that a rotation of it mixes the sine and the
    // cosine into both coordinates.
    const float POINT_X = 1.3f;
    const float POINT_Y = -0.7f;

    struct Turn
    {
        float angle;
        Vec2f counterclockwise;
        Vec2f clockwise;
    };

    const Turn TURNS[] = {
        {0.f, {POINT_X, POINT_Y}, {POINT_X, POINT_Y}},
        {_2FLOAT(M_PI / 2), {-POINT_Y, POINT_X}, {POINT_Y, -POINT_X}},
        {_2FLOAT(M_PI), {-POINT_X, -POINT_Y}, {-POINT_X, -POINT_Y}},
        {_2FLOAT(-M_PI / 2), {POINT_Y, -POINT_X}, {-POINT_Y, POINT_X}},
    };

    struct Direction
    {
        Vec2f vector;
        double tilt;
        double full_turn_tilt;
    };

    // The tilt runs from -pi to pi and takes the sign of y; the full-turn tilt
    // runs from 0 to 2 pi. The length of a vector does not matter, and a vector
    // too short to have a direction gives 0 for both.
    const Direction DIRECTIONS[] = {
        {{1.f, 0.f}, 0., 0.},
        {{1.f, 1.f}, M_PI / 4, M_PI / 4},
        {{0.f, 1.f}, M_PI / 2, M_PI / 2},
        {{-1.f, 1.f}, 3 * M_PI / 4, 3 * M_PI / 4},
        {{-1.f, 0.f}, M_PI, M_PI},
        {{-1.f, -1.f}, -3 * M_PI / 4, 5 * M_PI / 4},
        {{0.f, -1.f}, -M_PI / 2, 3 * M_PI / 2},
        {{1.f, -1.f}, -M_PI / 4, 7 * M_PI / 4},
        {{2.f, 2.f}, M_PI / 4, M_PI / 4},
        {{0.f, 1e-7f}, 0., 0.},
        {{0.f, 0.f}, 0., 0.},
    };

    void expectPoint(const Vec2f& expected, const Vec2f& actual)
    {
        EXPECT_NEAR(expected.x, actual.x, TOLERANCE);
        EXPECT_NEAR(expected.y, actual.y, TOLERANCE);
    }
}

TEST(LayoutMath, SineCosineAndTangentOfTabulatedAngles)
{
    EXPECT_NEAR(1., COS(0.f), TOLERANCE);
    EXPECT_NEAR(0.5, COS(_2FLOAT(M_PI / 3)), TOLERANCE);
    EXPECT_NEAR(-1., COS(_2FLOAT(M_PI)), TOLERANCE);
    EXPECT_NEAR(0.5, SIN(_2FLOAT(M_PI / 6)), TOLERANCE);
    EXPECT_NEAR(-1., SIN(_2FLOAT(-M_PI / 2)), TOLERANCE);
    EXPECT_NEAR(1., TAN(_2FLOAT(M_PI / 4)), TOLERANCE);
    EXPECT_NEAR(-1., TAN(_2FLOAT(3 * M_PI / 4)), TOLERANCE);
}

TEST(LayoutMath, ArcCosineAndArcSineOfTabulatedValues)
{
    EXPECT_NEAR(M_PI / 2, ACOS(0.f), TOLERANCE);
    EXPECT_NEAR(M_PI / 3, ACOS(0.5f), TOLERANCE);
    EXPECT_NEAR(M_PI, ACOS(-1.f), TOLERANCE);
    EXPECT_NEAR(M_PI / 6, ASIN(0.5f), TOLERANCE);
    EXPECT_NEAR(-M_PI / 2, ASIN(-1.f), TOLERANCE);
}

TEST(LayoutMath, ArcTangentTakesTheQuadrantAndHypotenuseTheLength)
{
    EXPECT_NEAR(M_PI / 4, ATAN2(1.f, 1.f), TOLERANCE);
    EXPECT_NEAR(3 * M_PI / 4, ATAN2(1.f, -1.f), TOLERANCE);
    EXPECT_NEAR(-3 * M_PI / 4, ATAN2(-1.f, -1.f), TOLERANCE);
    EXPECT_NEAR(-M_PI / 2, ATAN2(-2.f, 0.f), TOLERANCE);
    EXPECT_NEAR(5., HYPOT(3.f, 4.f), TOLERANCE);
    EXPECT_NEAR(13., HYPOT(-5.f, 12.f), TOLERANCE);
}

TEST(LayoutMath, RotateTurnsCounterclockwiseAndRotateLClockwise)
{
    for (const Turn& turn : TURNS)
    {
        SCOPED_TRACE(testing::Message() << "turn by " << turn.angle);

        Vec2f counterclockwise(POINT_X, POINT_Y);
        counterclockwise.rotate(turn.angle);
        expectPoint(turn.counterclockwise, counterclockwise);

        Vec2f clockwise(POINT_X, POINT_Y);
        clockwise.rotateL(turn.angle);
        expectPoint(turn.clockwise, clockwise);
    }
}

TEST(LayoutMath, TiltAnglesOfTabulatedDirections)
{
    for (const Direction& direction : DIRECTIONS)
    {
        Vec2f vector = direction.vector;
        SCOPED_TRACE(testing::Message() << "direction " << vector.x << ", " << vector.y);

        EXPECT_NEAR(direction.tilt, vector.tiltAngle(), TOLERANCE);
        EXPECT_NEAR(direction.full_turn_tilt, vector.tiltAngle2(), TOLERANCE);
    }
}
