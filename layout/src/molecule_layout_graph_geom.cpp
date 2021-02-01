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

#include "layout/molecule_layout_graph.h"

using namespace indigo;

float f1(float X, int L, float s)
{
    int i, min1;
    float f;

    min1 = 1;
    f = (1 - s) / 2;

    for (i = 1; i <= L; i++)
    {
        min1 = -min1;
        f += cos(i * X) * min1;
    }
    return f;
}

float f2(float X, int L, float s)
{
    int i, min1;
    float f;

    min1 = -1;
    f = -s / 2;

    for (i = 0; i <= L; i++)
    {
        min1 = -min1;
        f += sin((2 * i + 1) * X / 2) * min1;
    }
    return f;
}

void MoleculeLayoutGraph::_findAngles(int k, float s, float& x, float& y)
{
    int L;
    float a0, b0;

    if (k % 2 == 0)
        L = k / 2;
    else
        L = (k - 1) / 2;

    // Choose most right segment with root
    b0 = _2FLOAT(M_PI - EPSILON + M_PI / 100.);
    a0 = _2FLOAT(b0 - M_PI / 100.);

    bool repeat = true;

    while (repeat)
    {
        b0 = _2FLOAT(b0 - M_PI / 100.);
        a0 = _2FLOAT(a0 - M_PI / 100.);

        if ((a0 < M_PI / 2 + EPSILON) && k > 3)
            throw Error("there are no roots");

        if (k % 2 == 0)
        {
            if (f1(a0, L, s) * f1(b0, L, s) > 0)
                continue;
        }
        else
        {
            if (f2(a0, L, s) * f2(b0, L, s) > 0)
                continue;
        }

        repeat = false;
    }

    // Find root
    if (k % 2 == 0)
    {
        x = _dichotomy1(a0, b0, L, s);
        y = _2FLOAT(L * (M_PI - x));
    }
    else
    {
        x = _dichotomy2(a0, b0, L, s);
        y = _2FLOAT((2. * L + 1.) * (M_PI - x) / 2.);
    }
}

float MoleculeLayoutGraph::_dichotomy1(float a0, float b0, int L, float s)
{
    // Return root of the equation f1 ( x,l,S]=0;
    // if there are a root at the [a0,b0].;
    // Assumption:f1 ( a0]*f1 ( b0]<0;
    float C, pr;
    float pr1, fa0;

    fa0 = f1(a0, L, s);

    if (fa0 * f1(b0, L, s) > 0)
        throw Error("there are no roots");

    while (true)
    {
        C = (a0 + b0) / 2;
        pr = f1(C, L, s);

        if (C - a0 < EPSILON)
            return C;

        pr1 = pr * fa0;

        if (pr1 < 0)
            b0 = C;
        else
        {
            a0 = C;
            fa0 = pr;
        }
    }
}

float MoleculeLayoutGraph::_dichotomy2(float a0, float b0, int L, float s)
{
    // Return root of the equation f2 ( x,l,S]=0;
    // if there are a root at the [a0,b0].;
    // Assumption:f1 ( a0]*f2 ( b0]<0;
    float C, pr;
    float pr1, fa0;

    fa0 = f2(a0, L, s);

    if (fa0 * f2(b0, L, s) > 0)
        throw Error("there are no roots");

    while (true)
    {
        C = (a0 + b0) / 2;
        pr = f2(C, L, s);

        if (C - a0 < EPSILON)
            return C;

        pr1 = pr * fa0;

        if (pr1 < 0)
            b0 = C;
        else
        {
            a0 = C;
            fa0 = pr;
        }
    }
}

// Complete regular curve from v1 to v2 by vertices in chain
bool MoleculeLayoutGraph::_drawRegularCurve(const Array<int>& chain, int v1, int v2, float length, bool ccw, int type)
{
    QS_DEF(Array<int>, mapping);

    mapping.clear_resize(vertexEnd());

    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        mapping[i] = i;

    return _drawRegularCurveEx(chain, v1, v2, length, ccw, type, mapping);
}

bool MoleculeLayoutGraph::_drawRegularCurveEx(const Array<int>& chain, int v1, int v2, float length, bool ccw, int type, const Array<int>& mapping)
{
    float s, x0 = 0.f, y0 = 0.f;
    int i, k, L;
    float x1, x2, y1, y2;
    int min1;
    float cosa, sina;

    k = chain.size() - 2;
    s = Vec2f::dist(getPos(mapping[v1]), getPos(mapping[v2]));

    if (s > (k + 1) * length - EPSILON)
        return false;

    _findAngles(k, s / length, x0, y0);

    // Calculate coordinates so that
    // v1(0,0) and v2(s,0)
    if (k % 2 == 0)
        L = k / 2;
    else
        L = (k - 1) / 2;

    x1 = 0;
    y1 = 0;
    min1 = -1;

    for (i = 0; i <= L - 1; i++)
    {
        min1 = -min1;
        x2 = x1 + min1 * cos(y0 + i * x0) * length;
        y2 = y1 + min1 * sin(y0 + i * x0) * length;
        getPos(mapping[chain[i + 1]]).set(x2, y2);
        x1 = x2;
        y1 = y2;
    }
    if (k % 2 == 1)
    {
        x2 = x1 + sin(x0 / 2) * length;
        y2 = y1 + cos(x0 / 2) * length;
        getPos(mapping[chain[L + 1]]).set(x2, y2);
    }
    x1 = s;
    y1 = 0;
    min1 = 1;
    for (i = 0; i <= L - 1; i++)
    {
        y2 = y1 + min1 * sin(y0 + i * x0) * length;
        min1 = -min1;
        x2 = x1 + min1 * cos(y0 + i * x0) * length;
        getPos(mapping[chain[chain.size() - i - 2]]).set(x2, y2);
        x1 = x2;
        y1 = y2;
    }
    // If CW - flip
    if (!ccw)
        for (i = 1; i < chain.size() - 1; i++)
            getPos(mapping[chain[i]]).y *= -1;

    // Return to source coordinates
    // Rotate on the angle between (v1,v2) and Ox and translate on vector v1
    if (s > EPSILON)
    {
        Vec2f& pos1 = getPos(mapping[v1]);
        Vec2f& pos2 = getPos(mapping[v2]);

        cosa = (pos2.x - pos1.x) / s;
        sina = (pos2.y - pos1.y) / s;
    }
    else
    {
        cosa = 1;
        sina = 0;
    }

    for (i = 1; i < chain.size() - 1; i++)
    {
        Vec2f old_pos = getPos(mapping[chain[i]]);
        Vec2f& pos = getPos(mapping[chain[i]]);

        pos.set(old_pos.x * cosa - old_pos.y * sina, old_pos.x * sina + old_pos.y * cosa);
        pos.add(getPos(mapping[chain[0]]));
    }

    // Set new types
    for (i = 0; i < chain.size() - 1; i++)
    {
        int beg = mapping[chain[i]];

        if (i > 0)
            _layout_vertices[beg].type = type;

        const Vertex& vert = getVertex(beg);
        int edge_idx = vert.neiEdge(vert.findNeiVertex(mapping[chain[i + 1]]));

        _layout_edges[edge_idx].type = type;
    }

    return true;
}

// Check vertex is inside the edge
bool MoleculeLayoutGraph::_isVertexOnEdge(int vert_idx, int edge_beg, int edge_end) const
{
    float a1, a0, b1, b0, t;
    const float eps = 0.05f;
    const Vec2f& pos = getPos(vert_idx);
    const Vec2f& pos1 = getPos(edge_beg);
    const Vec2f& pos2 = getPos(edge_end);

    a1 = pos2.x - pos1.x;
    a0 = pos.x - pos1.x;
    b1 = pos2.y - pos1.y;
    b0 = pos.y - pos1.y;

    if (a1 * a1 + b1 * b1 < eps)
    {
        if (a0 * a0 + b0 * b0 < eps)
            return true;
        else
            return false;
    }

    if (fabs(a1) < eps)
    {
        if (fabs(a0) > eps)
            return false;
        else
        {
            t = b0 / b1;
            if (t > -eps && t < 1 + eps)
                return true;
            else
                return false;
        }
    }

    if (fabs(b1) < eps)
    {
        if (fabs(b0) > eps)
            return false;
        else
        {
            t = a0 / a1;
            if (t > -eps && t < 1 + eps)
                return true;
            else
                return false;
        }
    }

    t = a0 / a1;

    if (fabs(t - (b0 / b1)) < eps)
    {
        if (t > -eps && t < 1 + eps)
            return true;
        else
            return false;
    }
    else
        return false;
}

bool MoleculeLayoutGraph::_isVertexOnSomeEdge(int vert_idx) const
{
    int i;

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        int type = _layout_edges[i].type;

        if (type == ELEMENT_INTERNAL || type == ELEMENT_BOUNDARY)
        {
            const Edge& edge = getEdge(i);

            if (vert_idx != edge.beg && vert_idx != edge.end && _isVertexOnEdge(vert_idx, edge.beg, edge.end))
                return true;
        }
    }
    return false;
}

// Translate edge by delta orthogonally
void MoleculeLayoutGraph::_shiftEdge(int edge_idx, float delta)
{
    float norm;
    const Edge& edge = getEdge(edge_idx);
    Vec2f& pos1 = getPos(edge.beg);
    Vec2f& pos2 = getPos(edge.end);

    norm = Vec2f::dist(pos1, pos2);

    Vec2f a(delta * (pos2.y - pos1.y) / norm, delta * (pos1.x - pos2.x) / norm);

    pos1.add(a);
    pos2.add(a);
}

// Calculate angle v1vv2 such the edge (v,v1) is on the right and (v,v2) is on the left
// if component is trivial return 0
// if v is internal return 2pi
float MoleculeLayoutGraphSimple::calculateAngle(int v, int& v1, int& v2) const
{
    int i, j;
    Vec2f p, p0;
    float beta = 0.f;
    QS_DEF(Array<float>, angles);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<int>, on_left);

    if (vertexCount() == 2)
    {
        if (v == vertexBegin())
            v1 = v2 = vertexNext(v);
        else
            v1 = v2 = vertexBegin();
        return 0.f;
    }

    const Vertex& vert = getVertex(v);

    // Calculate polar angles
    angles.clear();
    edges.clear();
    on_left.clear_resize(vert.degree());

    for (i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
    {
        edges.push(i);
        p0.diff(getPos(vert.neiVertex(i)), getPos(v));
        angles.push(p0.tiltAngle2());
    }

    // Sort
    for (i = 0; i < angles.size(); i++)
        for (j = i + 1; j < angles.size(); j++)
            if (angles[i] > angles[j])
            {
                angles.swap(i, j);
                edges.swap(i, j);
            }

    // Find v1
    for (i = 0; i < angles.size() - 1; i++)
    {
        beta = (angles[i + 1] + angles[i]) / 2;
        p = getPos(v);
        p.x += 0.2f * cos(beta);
        p.y += 0.2f * sin(beta);
        on_left[i] = _isPointOutside(p);
    }

    beta = _2FLOAT(M_PI + (angles.top() + angles[0]) / 2.);
    p = getPos(v);
    p.x += 0.2f * cos(beta);
    p.y += 0.2f * sin(beta);
    on_left.top() = _isPointOutside(p);

    float comp_angle;

    if (vert.degree() == 2)
    {
        if (on_left[0] || (!on_left[1] && angles[1] - angles[0] > M_PI))
        {
            comp_angle = _2FLOAT(2. * M_PI - (angles[1] - angles[0]));
            v1 = vert.neiVertex(edges[1]);
            v2 = vert.neiVertex(edges[0]);
        }
        else
        {
            comp_angle = angles[1] - angles[0];
            v1 = vert.neiVertex(edges[0]);
            v2 = vert.neiVertex(edges[1]);
        }
        return comp_angle;
    }

    // Find sector outside component
    for (i = 0; i < vert.degree() - 1; i++)
    {
        if (on_left[i])
        {
            comp_angle = _2FLOAT(2. * M_PI - (angles[i + 1] - angles[i]));
            v1 = vert.neiVertex(edges[i + 1]);
            v2 = vert.neiVertex(edges[i]);
            return comp_angle;
        }
    }

    if (on_left.top())
    {
        comp_angle = angles.top() - angles[0];
        v1 = vert.neiVertex(edges[0]);
        v2 = vert.neiVertex(edges.top());
        return comp_angle;
    }

    // TODO: if vertex is internal - choose maximal free angle
    float max_angle = 0.f;

    for (i = 0; i < vert.degree() - 1; i++)
    {
        comp_angle = _2FLOAT(2 * M_PI - (angles[i + 1] - angles[i]));
        if (comp_angle > max_angle)
        {
            max_angle = comp_angle;
            v1 = vert.neiVertex(edges[i + 1]);
            v2 = vert.neiVertex(edges[i]);
        }
    }

    comp_angle = angles.top() - angles[0];
    if (comp_angle > max_angle)
    {
        max_angle = comp_angle;
        v1 = vert.neiVertex(edges[0]);
        v2 = vert.neiVertex(edges.top());
    }

    return max_angle;
}

// Calculate position by adding one unit with given angle to the segment
void MoleculeLayoutGraph::_calculatePos(float phi, const Vec2f& v1, const Vec2f& v2, Vec2f& v)
{
    float alpha;
    Vec2f dir;

    dir.diff(v2, v1);

    alpha = dir.tiltAngle();
    alpha += phi;

    v.set(v1.x + cos(alpha), v1.y + sin(alpha));
}

// Explore intersection of two edges:
// 0     at least one edge is not drawn
// 1     no intersection
// 21    one common vertex
// 222   one common point: beginning of first edge which lays inside second edge
// 223   one common point: end of first edge which lays inside second edge
// 224   one common point: beginning of second edge which lays inside first edge
// 225   one common point: end of second edge which lays inside first edge
// 23    one common point inside both edges
// 3     have common segment but not equal
// 4     equal
// 5     error
// Parametric equation:
//  x = (x1 - x0) * t + x0;
//  y = (y1 - y0) * t + y0;
//  0 <= t <= 1;
int MoleculeLayoutGraph::_calcIntersection(int edge1_idx, int edge2_idx) const
{
    float a11, a12, a21, a22, b1, b2;
    float delta, delta1, delta2, t, s;
    float a, b, pr;

    const Edge& edge1 = getEdge(edge1_idx);
    const Edge& edge2 = getEdge(edge2_idx);

    const float eps = 0.01f;

    if (getVertexType(edge1.beg) == ELEMENT_NOT_DRAWN || getVertexType(edge1.end) == ELEMENT_NOT_DRAWN || getVertexType(edge2.beg) == ELEMENT_NOT_DRAWN ||
        getVertexType(edge2.end) == ELEMENT_NOT_DRAWN)
        return 0;

    const Vec2f& v1 = getPos(edge1.beg);
    const Vec2f& v2 = getPos(edge1.end);
    const Vec2f& v3 = getPos(edge2.beg);
    const Vec2f& v4 = getPos(edge2.end);

    a11 = v2.x - v1.x;
    a12 = v3.x - v4.x;
    b1 = v3.x - v1.x;
    a21 = v2.y - v1.y;
    a22 = v3.y - v4.y;
    b2 = v3.y - v1.y;
    delta = a11 * a22 - a12 * a21;
    delta2 = a11 * b2 - a21 * b1;
    delta1 = b1 * a22 - b2 * a12;

    if (fabs(delta) < eps)
    {
        if (fabs(b1 * a21 - b2 * a11) > eps)
            return 1;
        if (fabs(a11) > eps)
        {
            a = b1 / a11;
            b = (b1 - a12) / a11;
            if (b < a)
            {
                pr = a;
                a = b;
                b = pr;
            }
        }
        else
        {
            a = b2 / a21;
            b = (b2 - a22) / a21;
            if (b < a)
            {
                pr = a;
                a = b;
                b = pr;
            }
        }
        if (a <= -eps)
        {
            if (b <= -eps)
                return 1;
            if (fabs(b) <= eps)
                return 21;
            return 3;
        }
        if (fabs(a) <= eps)
        {
            if (fabs(1 - b) <= eps)
                return 4;
            return 3;
        }
        if (a <= 1 - eps)
            return 3;
        if (fabs(a - 1) <= eps)
            return 21;
        if (a >= eps)
            return 1;
    }
    else
    {
        t = delta1 / delta;
        s = delta2 / delta;

        if (t < -eps || t > 1 + eps || s < -eps || s > 1 + eps)
            return 1;
        if (t > eps && t < 1 - eps && s > eps && s < 1 - eps)
            return 23;
        if (t > eps && t < 1 - eps)
        {
            if (s > -eps && s < eps)
                return 224; // v3
            if (s > 1 - eps && s < 1 + eps)
                return 225; // v4
        }
        if (s > eps && s < 1 - eps)
        {
            if ((t > -eps && t < eps))
                return 222; // v1
            if ((t > 1 - eps && t < 1 + eps))
                return 223; // v2
        }
        if (((t > -eps && t < eps) || (t > 1 - eps && t < 1 + eps)) && ((s > -eps && s < eps) || (s > 1 - eps && s < 1 + eps)))
            return 21;
    }
    return 5;
}

// Calculate angle v1vv2 such the edge (v,v1) is on the right and (v,v2) is on the left
// if component is trivial return 0
// if v is internal return 2pi
float MoleculeLayoutGraphSmart::calculateAngle(int v, int& v1, int& v2) const
{
    int i, j;
    Vec2f p, p0;
    float beta = 0.f;
    QS_DEF(Array<float>, angles);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<int>, on_left);

    if (vertexCount() == 2)
    {
        if (v == vertexBegin())
            v1 = v2 = vertexNext(v);
        else
            v1 = v2 = vertexBegin();
        return 0.f;
    }

    const Vertex& vert = getVertex(v);

    // Calculate polar angles
    angles.clear();
    edges.clear();
    on_left.clear_resize(vert.degree());

    for (i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
    {
        edges.push(i);
        p0.diff(getPos(vert.neiVertex(i)), getPos(v));
        angles.push(p0.tiltAngle2());
    }

    // Sort
    for (i = 0; i < angles.size(); i++)
        for (j = i + 1; j < angles.size(); j++)
            if (angles[i] > angles[j])
            {
                angles.swap(i, j);
                edges.swap(i, j);
            }

    // Find v1
    for (i = 0; i < angles.size() - 1; i++)
    {
        beta = (angles[i + 1] + angles[i]) / 2.f;
        p = getPos(v);
        p.x += 0.2f * cos(beta);
        p.y += 0.2f * sin(beta);
        on_left[i] = _isPointOutside(p);
    }

    beta = _2FLOAT(M_PI + (angles.top() + angles[0]) / 2.);
    p = getPos(v);
    p.x += 0.2f * cos(beta);
    p.y += 0.2f * sin(beta);
    on_left.top() = _isPointOutside(p);

    float comp_angle;
    float cur_energy = 0;

    /*   if (vert.degree() == 2)
    {


    //      if (on_left[0] || (!on_left[1] && angles[1] - angles[0] > M_PI))
    if (on_left[0] > on_left[1] || (on_left[0] == on_left[1] && angles[1] - angles[0] > M_PI))
    {
    comp_angle = 2 * M_PI - (angles[1] - angles[0]);
    v1 = vert.neiVertex(edges[1]);
    v2 = vert.neiVertex(edges[0]);
    } else
    {
    comp_angle = angles[1] - angles[0];
    v1 = vert.neiVertex(edges[0]);
    v2 = vert.neiVertex(edges[1]);
    }
    return comp_angle;
    }*/

    // Find sector outside component

    if (_molecule->cis_trans.getParity(getEdgeExtIdx(vert.neiEdge(vert.neiBegin()))) != 0 ||
        _molecule->cis_trans.getParity(getEdgeExtIdx(vert.neiEdge(vert.neiNext(vert.neiBegin())))) != 0)
    {

        float best_angle = _2FLOAT(2. * M_PI);

        for (i = 0; i < vert.degree(); i++)
        {
            int ii = i + 1;
            if (ii == vert.degree())
                ii = 0;

            comp_angle = _2FLOAT(2. * M_PI - (angles[ii] - angles[i]));
            if (ii == 0)
                comp_angle -= _2FLOAT(2 * M_PI);
            const float eps = 0.1f;
            if (i == 0 || comp_angle < best_angle - eps)
            {
                best_angle = comp_angle;
                v1 = vert.neiVertex(edges[ii]);
                v2 = vert.neiVertex(edges[i]);
            }
        }

        /*   comp_angle = angles.top() - angles[0];
        if (comp_angle < best_angle)
        {
        best_angle = comp_angle;
        v1 = vert.neiVertex(edges[0]);
        v2 = vert.neiVertex(edges.top());
        }*/

        return best_angle;
    }

    if (_graph->getVertexType(getVertexExtIdx(vert.neiVertex(vert.neiBegin()))) == ELEMENT_NOT_DRAWN)
    {
        // if this is not layouted component
        for (i = 0; i < vert.degree() - 1; i++)
        {
            if (on_left[i])
            {
                comp_angle = _2FLOAT(2. * M_PI - (angles[i + 1] - angles[i]));
                v1 = vert.neiVertex(edges[i + 1]);
                v2 = vert.neiVertex(edges[i]);
                return comp_angle;
            }
        }

        if (on_left.top())
        {
            comp_angle = angles.top() - angles[0];
            v1 = vert.neiVertex(edges[0]);
            v2 = vert.neiVertex(edges.top());
            return comp_angle;
        }
    }
    // TODO: if vertex is internal - choose maximal free angle
    float best_angle = _2FLOAT(2 * M_PI);

    for (i = 0; i < vert.degree(); i++)
    {
        int ii = i + 1;
        if (ii == vert.degree())
            ii = 0;

        beta = (angles[ii] + angles[i]) / 2.f;
        if (ii == 0)
        {
            beta += _2FLOAT(M_PI);
            if (beta >= _2FLOAT(2. * M_PI))
                beta -= _2FLOAT(2. * M_PI);
        }
        p = _graph->getPos(getVertexExtIdx(v));

        p.x += 1 * cos(beta);
        p.y += 1 * sin(beta);
        float energy = _energyOfPoint(p);
        comp_angle = _2FLOAT(2. * M_PI - (angles[ii] - angles[i]));
        if (ii == 0)
            comp_angle -= _2FLOAT(2 * M_PI);
        const float eps = 0.1f;
        //      printf("%d: %5.5f %5.5f %d\n", i, energy, comp_angle, on_left[i]);
        if (i == 0 || energy + eps < cur_energy || (fabs(energy - cur_energy) < eps && comp_angle < best_angle - eps) ||
            (fabs(energy - cur_energy) < eps && fabs(comp_angle - best_angle) < eps && on_left[i]))
        {
            cur_energy = energy;
            best_angle = comp_angle;
            v1 = vert.neiVertex(edges[ii]);
            v2 = vert.neiVertex(edges[i]);
        }

        /*      comp_angle = 2 * M_PI - (angles[i + 1] - angles[i]);
        if (comp_angle < best_angle)
        {
        best_angle = comp_angle;
        v1 = vert.neiVertex(edges[i + 1]);
        v2 = vert.neiVertex(edges[i]);
        }*/
    }

    /*   comp_angle = angles.top() - angles[0];
    if (comp_angle < best_angle)
    {
    best_angle = comp_angle;
    v1 = vert.neiVertex(edges[0]);
    v2 = vert.neiVertex(edges.top());
    }*/

    return best_angle;
}

const float MoleculeLayoutGraphSmart::_energyOfPoint(Vec2f p) const
{

    float energy = 0.f;
    for (int i = _graph->vertexBegin(); i < _graph->vertexEnd(); i = _graph->vertexNext(i))
        if (_graph->getLayoutVertex(i).type != ELEMENT_NOT_DRAWN)
        {
            float d = Vec2f::dist(p, _graph->getPos(i));
            if (d <= 1.5f)
                if (d >= 0.5f)
                    energy += 1.0f / d;
                else
                    energy += 2.0f;
        }

    return energy;
}

// Calculate position by adding one unit with given angle to the segment

int MoleculeLayoutGraphSmart::_isCisConfiguratuin(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4)
{
    int rotateCounterclockwise1 = (p3.x - p2.x) * (p2.y - p1.y) - (p3.y - p2.y) * (p2.x - p1.x) > 0;
    int rotateCounterclockwise2 = (p4.x - p3.x) * (p3.y - p2.y) - (p4.y - p3.y) * (p3.x - p2.x) > 0;

    return rotateCounterclockwise1 == rotateCounterclockwise2;
}
