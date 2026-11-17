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

// The shifts along a direction at which one convex polygon meets another.

#include <vector>

#include <gtest/gtest.h>

#include <math/algebra.h>

using namespace indigo;

namespace
{
    std::vector<Vec2f> square(float left, float bottom, float side)
    {
        return {Vec2f(left, bottom), Vec2f(left + side, bottom), Vec2f(left + side, bottom + side), Vec2f(left, bottom + side)};
    }

    std::vector<Vec2f> moved(const std::vector<Vec2f>& polygon, const Vec2f& direction, float shift)
    {
        std::vector<Vec2f> result;
        for (const Vec2f& point : polygon)
            result.push_back(point + direction * shift);
        return result;
    }
}

TEST(ConvexPolygonsIntersectAlong, ApproachingSquaresMeetFromFirstContactToLast)
{
    float low, high;
    ASSERT_TRUE(convexPolygonsIntersectAlong(square(0, 0, 1), square(3, 0, 1), Vec2f(1, 0), low, high));
    EXPECT_FLOAT_EQ(2, low);
    EXPECT_FLOAT_EQ(4, high);
}

TEST(ConvexPolygonsIntersectAlong, SquareMovingAwayMetOnlyBehindItsStart)
{
    float low, high;
    ASSERT_TRUE(convexPolygonsIntersectAlong(square(0, 0, 1), square(3, 0, 1), Vec2f(-1, 0), low, high));
    EXPECT_FLOAT_EQ(-4, low);
    EXPECT_FLOAT_EQ(-2, high);
}

TEST(ConvexPolygonsIntersectAlong, ParallelPathThatNeverMeets)
{
    float low, high;
    EXPECT_FALSE(convexPolygonsIntersectAlong(square(0, 0, 1), square(3, 5, 1), Vec2f(1, 0), low, high));
}

TEST(ConvexPolygonsIntersectAlong, DiagonalPathThatPassesBy)
{
    // Moving up and to the right, the square overlaps the other one across x for shifts in [2, 4] times the square root
    // of 2, and across y only for shifts in [-1, 1] times it: no shift does both.
    Vec2f direction(1, 1);
    direction.normalize();
    float low, high;
    EXPECT_FALSE(convexPolygonsIntersectAlong(square(0, 0, 1), square(3, 0, 1), direction, low, high));
}

TEST(ConvexPolygonsIntersectAlong, StandingStillMeetsEverywhereOrNowhere)
{
    float low, high;
    ASSERT_TRUE(convexPolygonsIntersectAlong(square(0, 0, 2), square(1, 1, 2), Vec2f(0, 0), low, high));
    EXPECT_LT(low, 0);
    EXPECT_GT(high, 0);
    EXPECT_FALSE(convexPolygonsIntersectAlong(square(0, 0, 1), square(3, 0, 1), Vec2f(0, 0), low, high));
}

TEST(ConvexPolygonsIntersectAlong, ATiltedBandMetAcrossItsWidth)
{
    // The band between the lines y = x - 1 and y = x + 1 for x in [-10, 10], crossed by the unit square [0, 1] x
    // [-5, -4] moving straight up: the lines rise to the right, so its top left corner (0, -4 + t) reaches the line
    // y = x - 1 first, at t = 3, and its bottom right corner (1, -5 + t) leaves the line y = x + 1 last, at t = 7.
    const std::vector<Vec2f> band = {Vec2f(-10, -11), Vec2f(10, 9), Vec2f(10, 11), Vec2f(-10, -9)};
    float low, high;
    ASSERT_TRUE(convexPolygonsIntersectAlong(square(0, -5, 1), band, Vec2f(0, 1), low, high));
    EXPECT_FLOAT_EQ(3, low);
    EXPECT_FLOAT_EQ(7, high);
}

TEST(ConvexPolygonsIntersectAlong, AgreesWithTheStaticTestAtEveryShift)
{
    const std::vector<Vec2f> label = square(0, 0, 1);
    const std::vector<Vec2f> band = {Vec2f(-10, -11), Vec2f(10, 9), Vec2f(10, 11), Vec2f(-10, -9)};
    Vec2f direction(1, -0.5f);
    direction.normalize();

    float low, high;
    ASSERT_TRUE(convexPolygonsIntersectAlong(label, band, direction, low, high));
    for (int step = -200; step <= 200; ++step)
    {
        const float shift = step * 0.05f;
        // Away from the two ends, where rounding decides either way.
        if (std::abs(shift - low) < 1e-3f || std::abs(shift - high) < 1e-3f)
            continue;
        const bool inside = low < shift && shift < high;
        EXPECT_EQ(inside, convexPolygonsIntersect(moved(label, direction, shift), band)) << "shift " << shift;
    }
}
