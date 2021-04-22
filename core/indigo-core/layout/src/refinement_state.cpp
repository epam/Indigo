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

#include "layout/refinement_state.h"

using namespace indigo;

IMPL_ERROR(RefinementState, "refinement");

CP_DEF(RefinementState);

RefinementState::RefinementState(MoleculeLayoutGraph& graph) : dist(0.f), energy(0), height(0.f), CP_INIT, TL_CP_GET(layout), _graph(graph)
{
}

void RefinementState::calcDistance(int v1, int v2)
{
    Vec2f d;

    d.diff(layout[v1], layout[v2]);

    dist = d.lengthSqr();
}

void RefinementState::calcHeight()
{
    float min = 1000.f, max = -1000.f;
    int i;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (layout[i].y < min)
            min = layout[i].y;
        if (layout[i].y > max)
            max = layout[i].y;
    }

    height = max - min;
}

void RefinementState::copy(const RefinementState& other)
{
    dist = other.dist;
    energy = other.energy;
    height = other.height;

    layout.copy(other.layout);
}

void RefinementState::copyFromGraph()
{
    int i;

    layout.clear_resize(_graph.vertexEnd());

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        layout[i] = _graph.getPos(i);
}

void RefinementState::applyToGraph()
{
    int i;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        _graph.getPos(i) = layout[i];
}

void RefinementState::calcEnergy()
{
    int i, j;
    float r;
    Vec2f d;

    energy = 0;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        for (j = _graph.vertexBegin(); j < _graph.vertexEnd(); j = _graph.vertexNext(j))
        {
            if (i == j)
                continue;

            d.diff(layout[i], layout[j]);
            r = d.lengthSqr();

            if (r < 0.0001f)
                r = 5000000.f;
            else
                r = 1 / r;

            energy += r;
        }

    energy /= 2;
}

// Flip all verices from branch around (v1,v2)
void RefinementState::flipBranch(const Filter& branch, const RefinementState& state, int v1_idx, int v2_idx)
{
    int i;
    float r, t;

    const Vec2f& v1 = state.layout[v1_idx];
    const Vec2f& v2 = state.layout[v2_idx];
    Vec2f d;

    d.diff(v2, v1);

    r = d.lengthSqr();

    if (r < 0.000000001f)
        throw Error("too small edge");

    layout.clear_resize(state.layout.size());

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (!branch.valid(i))
        {
            const Vec2f& vi = state.layout[i];

            t = ((vi.x - v1.x) * d.x + (vi.y - v1.y) * d.y) / r;
            layout[i].set(2 * d.x * t + 2 * v1.x - vi.x, 2 * d.y * t + 2 * v1.y - vi.y);
        }
        else
            layout[i] = state.layout[i];
    }
}

// Rotate branch around vertex v1
void RefinementState::rotateBranch(const Filter& branch, const RefinementState& state, int v_idx, float angle)
{
    int i;
    float co, si;

    const Vec2f& v = state.layout[v_idx];
    Vec2f d;

    angle = _2FLOAT(DEG2RAD(angle));

    co = cos(angle);
    si = sin(angle);

    layout.clear_resize(state.layout.size());

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (!branch.valid(i))
        {
            d.diff(state.layout[i], v);
            d.rotate(si, co);

            layout[i].sum(d, v);
        }
        else
            layout[i] = state.layout[i];
    }
}

// Translate branch on 0.1 of vector [v1,v2]
void RefinementState::stretchBranch(const Filter& branch, const RefinementState& state, int v1_idx, int v2_idx, int val)
{
    int i;
    float r, sh = 0.1f * val;

    const Vec2f& v1 = state.layout[v1_idx];
    const Vec2f& v2 = state.layout[v2_idx];
    Vec2f d;

    d.diff(v2, v1);
    r = d.length();

    if (r < EPSILON)
        throw Error("too small edge");

    d.scale(sh / r);

    if (branch.valid(v1_idx))
        d.negate();

    layout.clear_resize(state.layout.size());

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (!branch.valid(i))
            layout[i].sum(state.layout[i], d);
        else
            layout[i] = state.layout[i];
    }
}

// Rotate layout around vertex v (in degrees)
void RefinementState::rotateLayout(const RefinementState& state, int v_idx, float angle)
{
    int i;
    float co, si;

    const Vec2f& v = state.layout[v_idx];
    Vec2f d;

    angle = _2FLOAT(DEG2RAD(angle));

    co = cos(angle);
    si = sin(angle);

    layout.clear_resize(state.layout.size());

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        d.diff(state.layout[i], v);
        d.rotate(si, co);

        layout[i].sum(d, v);
    }
}

bool RefinementState::is_small_cycle()
{
    if (_graph.vertexCount() >= 10)
        return false;

    bool answ = true;
    for (int v = _graph.vertexBegin(); v != _graph.vertexEnd(); v++)
        if (_graph.getVertex(v).degree() != 2)
            answ = false;

    return answ;
}

float RefinementState::calc_best_angle()
{
    QS_DEF(Array<int>, convex_hull);
    QS_DEF(Array<bool>, take);
    convex_hull.resize(_graph.vertexEnd() + 1);
    take.resize(_graph.vertexEnd());
    take.zerofill();

    int index = 0;

    int first = _graph.vertexBegin();
    for (int v = _graph.vertexBegin(); v != _graph.vertexEnd(); v = _graph.vertexNext(v))
        if (layout[v].y < layout[first].y || (layout[v].y == layout[first].y && layout[v].x < layout[first].x))
            first = v;

    convex_hull[index++] = first;
    take[first] = true;

    while (true)
    {
        int next = -1;
        float bestcross = 0;
        for (int v = _graph.vertexBegin(); v != _graph.vertexEnd(); v = _graph.vertexNext(v))
            if (v != convex_hull[index - 1])
            {
                if (next < 0 || Vec2f::cross(layout[v] - layout[convex_hull[index - 1]], layout[next] - layout[convex_hull[index - 1]]) > 0 ||
                    (Vec2f::cross(layout[v] - layout[convex_hull[index - 1]], layout[next] - layout[convex_hull[index - 1]]) == 0 &&
                     Vec2f::distSqr(layout[convex_hull[index - 1]], layout[v]) > Vec2f::distSqr(layout[convex_hull[index - 1]], layout[next])))
                    next = v;
            }
        if (next >= 0 && next != first)
        {
            convex_hull[index++] = next;
            take[next] = true;
        }
        else
        {
            convex_hull[index] = next;
            break;
        }
    }

    int base = -1;
    int oppsite = 1;
    float high = -1;
    int best_base = 0;
    do
    {
        base++;
        Vec2f vec = layout[convex_hull[base + 1]] - layout[convex_hull[base]];
        float A = vec.y;
        float B = -vec.x;
        float C = Vec2f::cross(layout[convex_hull[base + 1]], layout[convex_hull[base]]);
        float len = sqrt(A * A + B * B);
        A /= len;
        B /= len;
        C /= len;

        while (fabs(A * layout[convex_hull[oppsite + 1]].x + B * layout[convex_hull[oppsite + 1]].y + C) >
               fabs(A * layout[convex_hull[oppsite]].x + B * layout[convex_hull[oppsite]].y + C))
            oppsite = (oppsite + 1) % index;

        if (high < 0 || fabs(A * layout[convex_hull[oppsite]].x + B * layout[convex_hull[oppsite]].y + C) < high)
        {
            best_base = base;
            high = fabs(A * layout[convex_hull[oppsite]].x + B * layout[convex_hull[oppsite]].y + C);
        }
    } while (base != index - 1);

    Vec2f vec = layout[convex_hull[best_base + 1]] - layout[convex_hull[best_base]];
    return -atan2(vec.y, vec.x);
}
