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
#include <math/random.h>

using namespace indigo;

// Return:
//   0 - no intersection
//   1 - intersection
//  -1 - unknown. Ray is too near to the one of the points
static int _isRayIntersectWithCheck(float a, float b, const Vec2f& p, const Vec2f& v1, const Vec2f& v2, bool check_precision)
{
    // Ray x=at+p.x, y=bt+p.y, t>=0 and segment [V1,V2];
    float a11, a12, a21, a22, b1, b2;
    float delta, delta1, delta2, t, s, a0, b0, pr;
    const float eps = 0.0001f;

    a11 = a;
    a12 = v1.x - v2.x;
    b1 = v1.x - p.x;
    a21 = b;
    a22 = v1.y - v2.y;
    b2 = v1.y - p.y;
    delta = a11 * a22 - a12 * a21;
    delta2 = a11 * b2 - a21 * b1;
    delta1 = b1 * a22 - b2 * a12;

    if (fabs(delta) < eps)
    {
        if (fabs(b1 * a21 - b2 * a11) > eps)
            return 0;

        if (fabs(a11) > eps)
        {
            a0 = b1 / a11;
            b0 = (b1 - a12) / a11;
            if (b0 < a0)
            {
                pr = a0;
                a0 = b0;
                b0 = pr;
            }
        }
        else
        {
            a0 = b2 / a21;
            b0 = (b2 - a22) / a21;
            if (b0 < a0)
            {
                pr = a0;
                a0 = b0;
                b0 = pr;
            }
        }
        if (check_precision)
            if (fabs(a0) < eps && fabs(b0) <= eps)
                return -1;

        if (a0 <= -eps && b0 <= -eps)
            return 0;
        return 1;
    }

    t = delta1 / delta;
    s = delta2 / delta;

    if (check_precision)
        if (fabs(s) < eps || fabs(s - 1) < eps)
            return -1;

    if (t < -eps || s < -eps || s > 1 + eps)
        return 0;
    return 1;
}

static bool _isRayIntersect(float a, float b, const Vec2f& p, const Vec2f& v1, const Vec2f& v2)
{
    return _isRayIntersectWithCheck(a, b, p, v1, v2, false) == 1;
}

// Check if point is outside biconnected component
// By calculating number of intersections of ray
bool MoleculeLayoutGraphSimple::_isPointOutside(const Vec2f& p) const
{
    Random rand(SOME_MAGIC_INT_FOR_RANDOM_3);
    int i, count = 0;
    float a, b;
    const float eps = 0.01f;

    bool success = false;

    while (!success)
    {
        success = true;

        a = (float)rand.nextDouble();
        b = (float)rand.nextDouble();
        a = 2.f * (a - 0.5f);
        b = 2.f * (b - 0.5f);

        if (fabs(a) < eps || fabs(b) < eps)
        {
            success = false;
            continue;
        }

        if (_outline.get() == 0)
        {
            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            {
                const Vec2f& pos = getPos(i);

                if (_layout_vertices[i].type == ELEMENT_BOUNDARY && fabs((pos.x - p.x) / a - (pos.y - p.y) / b) < 0.1f)
                {
                    count++;

                    if (count > 100)
                        return false;

                    success = false;
                    break;
                }
            }
        }
        else
        {
            const Array<Vec2f>& outline = *_outline.get();

            for (i = 0; i < outline.size(); i++)
            {
                if (fabs((outline[i].x - p.x) / a - (outline[i].y - p.y) / b) < 0.1f)
                {
                    count++;

                    if (count > 100)
                        return false;

                    success = false;
                    break;
                }
            }
        }
    }

    // Calculate
    count = 0;

    if (_outline.get() == 0)
    {
        for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
        {
            if (_layout_edges[i].type == ELEMENT_BOUNDARY)
            {
                const Edge& edge = getEdge(i);

                if (_isRayIntersect(a, b, p, getPos(edge.beg), getPos(edge.end)))
                    count++;
            }
        }
    }
    else
    {
        const Array<Vec2f>& outline = *_outline.get();

        for (i = 0; i < outline.size(); i++)
            if (_isRayIntersect(a, b, p, outline[i], outline[(i + 1) % outline.size()]))
                count++;
    }

    if (count & 1)
        return false;
    return true;
}

// Check if point is outside cycle
// By calculating number of intersections of ray
bool MoleculeLayoutGraphSimple::_isPointOutsideCycle(const Cycle& cycle, const Vec2f& p) const
{
    Random rand(SOME_MAGIC_INT_FOR_RANDOM_3);
    int i, count = 0;
    float a, b;
    Vec2f v1, v2;
    const float eps = 0.01f;

    bool success = false;

    while (!success)
    {
        success = true;

        a = (float)rand.nextDouble();
        b = (float)rand.nextDouble();
        a = 2.f * (a - 0.5f);
        b = 2.f * (b - 0.5f);

        if (fabs(a) < eps || fabs(b) < eps)
        {
            success = false;
            continue;
        }

        for (i = 0; i < cycle.vertexCount(); i++)
        {
            const Vec2f& pos = getPos(cycle.getVertex(i));

            if (fabs((pos.x - p.x) / a - (pos.y - p.y) / b) < EPSILON)
            {
                count++;

                if (count > 50)
                    return false;

                success = false;
                break;
            }
        }
    }

    // Calculate
    count = 0;

    for (i = 0; i < cycle.vertexCount(); i++)
        if (_isRayIntersect(a, b, p, getPos(cycle.getVertex(i)), getPos(cycle.getVertex((i + 1) % cycle.vertexCount()))))
            count++;

    if (count & 1)
        return false;
    return true;
}

// The same but with mapping
bool MoleculeLayoutGraph::_isPointOutsideCycleEx(const Cycle& cycle, const Vec2f& p, const Array<int>& mapping) const
{
    Random rand(SOME_MAGIC_INT_FOR_RANDOM_3);
    // TODO: check that point 'p' is equal to the one of cycle points (sometimes it happens)
    float a, b;

    int tries = 0;
    while (tries < 50)
    {
        tries++;

        // Choose random direction
        a = (float)rand.nextDouble();
        b = (float)rand.nextDouble();
        a = 2.f * (a - 0.5f);
        b = 2.f * (b - 0.5f);

        // Calculate number of intersection with boundary
        int count = 0;

        for (int i = 0; i < cycle.vertexCount(); i++)
        {
            int ret =
                _isRayIntersectWithCheck(a, b, p, getPos(mapping[cycle.getVertex(i)]), getPos(mapping[cycle.getVertex((i + 1) % cycle.vertexCount())]), true);
            if (ret == -1)
            {
                // Ray is too near to the point. Choose another one point
                count = -1;
                break;
            }
            if (ret == 1)
                count++;
        }

        if (count == -1)
            // Try again
            continue;

        // If number of intersections is even then point is outside
        if (count & 1)
            return false;
        return true;
    }

    // Return any value hoping it will never happen
    return false;
}

// Extract component border
void MoleculeLayoutGraphSimple::_getBorder(Cycle& border) const
{
    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, edges);
    int i, n = 0;

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
        if (_layout_edges[i].type == ELEMENT_BOUNDARY)
            n++;

    if (n == 0)
        return;

    vertices.clear();
    edges.clear();

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
        if (_layout_edges[i].type == ELEMENT_BOUNDARY)
            break;

    Edge edge = getEdge(i);

    vertices.push(edge.beg);
    edges.push(i);

    while (edge.end != vertices[0])
    {
        const Vertex& vert = getVertex(edge.end);
        bool found = false;

        for (int i = vert.neiBegin(); !found && i < vert.neiEnd(); i = vert.neiNext(i))
        {
            int nei_v = vert.neiVertex(i);
            int nei_e = vert.neiEdge(i);

            if (getEdgeType(nei_e) == ELEMENT_BOUNDARY && nei_v != edge.beg)
            {
                edge.beg = edge.end;
                edge.end = nei_v;

                vertices.push(edge.beg);
                edges.push(nei_e);

                found = true;
            }
        }

        if (!found || vertices.size() > n)
            throw Error("corrupted border");
    }

    border.copy(vertices, edges);
    border.canonize();
}

// Split border in two parts by two vertices
void MoleculeLayoutGraphSimple::_splitBorder(int v1, int v2, Array<int>& part1v, Array<int>& part1e, Array<int>& part2v, Array<int>& part2e) const
{
    Cycle border;

    _getBorder(border);

    int idx1 = border.findVertex(v1);
    int idx2 = border.findVertex(v2);
    int i;

    if (idx1 == -1 || idx2 == -1)
        throw Error("border division by non-boundary vertex");

    if (idx1 > idx2)
        std::swap(idx1, idx2);

    part1v.clear();
    part1e.clear();
    part2v.clear();
    part2e.clear();

    for (i = idx1; i < idx2 + 1; i++)
    {
        part1v.push(border.getVertex(i));
        part1e.push(border.getEdge(i));
    }

    part1e.pop(); // edge count is less

    for (i = idx2; i < border.vertexCount(); i++)
    {
        part2v.push(border.getVertex(i));
        part2e.push(border.getEdge(i));
    }

    for (i = 0; i < idx1 + 1; i++)
    {
        part2v.push(border.getVertex(i));
        part2e.push(border.getEdge(i));
    }

    part2e.pop(); // edge count is less
}

// Cycle enumerator callback
// Check if cycle is boundary and mark vertices and edges as boundary/internal
bool MoleculeLayoutGraph::_border_cb(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context)
{
    MoleculeLayoutGraph& self = *(MoleculeLayoutGraph*)context;
    Cycle cycle(vertices, edges);

    // cycle.canonize();

    int i;
    QS_DEF(Array<int>, types);

    types.clear_resize(self.vertexEnd());

    for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
        types[i] = ELEMENT_INTERNAL;

    for (i = 0; i < cycle.vertexCount(); i++)
        types[cycle.getVertex(i)] = ELEMENT_BOUNDARY;

    // Check vertices not in cycle are inside it
    for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
        if (types[i] == ELEMENT_INTERNAL)
            if (self._isPointOutsideCycle(cycle, self.getPos(i)))
                return true; // continue

    // Check edge centers are inside cycle
    types.clear_resize(self.edgeEnd());

    for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
        types[i] = ELEMENT_INTERNAL;

    for (i = 0; i < cycle.vertexCount(); i++)
        types[cycle.getEdge(i)] = ELEMENT_BOUNDARY;

    for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
        if (types[i] == ELEMENT_INTERNAL)
        {
            Vec2f p;
            const Edge& edge = self.getEdge(i);

            p.lineCombin2(self.getPos(edge.beg), 0.5f, self.getPos(edge.end), 0.5f);

            if (self._isPointOutsideCycle(cycle, p))
                return true; // continue
        }

    // Mark edges and bonds
    for (i = self.vertexBegin(); i < self.vertexEnd(); i = self.vertexNext(i))
        self._layout_vertices[i].type = ELEMENT_INTERNAL;

    for (i = self.edgeBegin(); i < self.edgeEnd(); i = self.edgeNext(i))
        self._layout_edges[i].type = ELEMENT_INTERNAL;

    for (i = 0; i < cycle.vertexCount(); i++)
    {
        self._layout_vertices[cycle.getVertex(i)].type = ELEMENT_BOUNDARY;
        self._layout_edges[cycle.getEdge(i)].type = ELEMENT_BOUNDARY;
    }

    return false;
}

// Check if point is outside biconnected component
// By calculating number of intersections of ray
bool MoleculeLayoutGraphSmart::_isPointOutside(const Vec2f& p) const
{
    //   return true;
    QS_DEF(Array<Vec2f>, point);
    Cycle surround_cycle;
    _getSurroundCycle(surround_cycle, p);

    if (surround_cycle.vertexCount() == 0)
        return 0;

    return _isPointOutsideCycle(surround_cycle, p);
}

// Check if point is outside cycle
// By calculating number of intersections of ray
bool MoleculeLayoutGraphSmart::_isPointOutsideCycle(const Cycle& cycle, const Vec2f& p) const
{
    QS_DEF(Array<Vec2f>, point);
    float rotate_angle = 0;
    int size = cycle.vertexCount();
    point.resize(size + 1);
    for (int i = 0; i <= size; i++)
        point[i] = _layout_vertices[cycle.getVertexC(i)].pos - p;
    for (int i = 0; i < size; i++)
    {
        float cs = Vec2f::dot(point[i], point[i + 1]) / (point[i].length() * point[i + 1].length());
        if (cs > 1)
            cs = 1;
        if (cs < -1)
            cs = -1;
        float angle = acos(cs);
        if (Vec2f::cross(point[i], point[i + 1]) < 0)
            angle = -angle;
        rotate_angle += angle;
    }

    // if point is outside, rotate angle is equals to 0. If point is inside rotate angle is equals to 2*M_PI of -2*M_PI
    return fabs(rotate_angle) < M_PI;
}

float MoleculeLayoutGraphSmart::_get_square()
{

    Cycle cycle;
    _getBorder(cycle);

    int len = cycle.vertexCount();

    float sq = 0;

    for (int i = 1; i < len - 1; i++)
        sq += Vec2f::cross(getPos(cycle.getVertex(i)) - getPos(cycle.getVertex(0)), getPos(cycle.getVertex(i + 1)) - getPos(cycle.getVertex(0)));

    // printf("sq = %.20f\n", sq);

    return fabs(sq / 2);
}

void MoleculeLayoutGraphSmart::flipped()
{
}

// Extract component border
void MoleculeLayoutGraphSmart::_getBorder(Cycle& border) const
{
    Vec2f outside_point(0, 0);
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (_layout_vertices[i].type != ELEMENT_NOT_DRAWN)
        {
            outside_point.max(_layout_vertices[i].pos);
        }

    outside_point += Vec2f(1, 1);
    _getSurroundCycle(border, outside_point);
}

void MoleculeLayoutGraphSmart::_getSurroundCycle(Cycle& cycle, Vec2f p) const
{
    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<Vec2f>, pos);
    int i, n = 0;
    const float eps = 1e-5f;

    Random rand(SOME_MAGIC_INT_FOR_RANDOM_3);
    /*   for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    if  (_layout_edges[i].type == ELEMENT_BOUNDARY)
    n++;

    if (n == 0)
    return;*/

    vertices.clear();
    edges.clear();

    float sn = 0;
    float cs = 0;
    while (sn == 0 && cs == 0)
    {
        sn = _2FLOAT(2.0 * rand.nextDouble() - 1.);
        cs = _2FLOAT(2.0 * rand.nextDouble() - 1.);
    }
    float len = sqrt(sn * sn + cs * cs);
    sn /= len;
    cs /= len;

    pos.resize(vertexEnd());
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (getVertexType(i) != ELEMENT_NOT_DRAWN)
        {
            pos[i].copy(getPos(i) - p);
        }

    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (getVertexType(i) != ELEMENT_NOT_DRAWN)
        {
            pos[i].rotate(sn, cs);
        }

    int first_edge = -1;
    float first_edge_x = 1e20f;
    for (int i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
        if (_layout_edges[i].type != ELEMENT_NOT_DRAWN)
        {
            Edge e = getEdge(i);
            if (pos[e.beg].y * pos[e.end].y <= 0)
            {
                float mid_x = (pos[e.beg].x * pos[e.end].y - pos[e.end].x * pos[e.beg].y) / (pos[e.end].y - pos[e.beg].y);
                if (fabs(mid_x) < eps)
                    return;
                if (mid_x > 0 && (first_edge == -1 || mid_x < first_edge_x))
                {
                    first_edge = i;
                    first_edge_x = mid_x;
                }
            }
        }

    int firts_vertex = -1;
    if (first_edge != -1)
        if (pos[getEdge(first_edge).beg].y < pos[getEdge(first_edge).end].y)
            firts_vertex = getEdge(first_edge).beg;
        else
            firts_vertex = getEdge(first_edge).end;
    else
    {
        // in this case no edges are intersecs (OX) ray
        // and so p is outside point
        // Then we are looking for border
        // and we can take the lowest vertex as the start vertex for searching of surround cycle
        float first_vertex_y;
        for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
            if (_layout_vertices[i].type != ELEMENT_NOT_DRAWN)
                if (firts_vertex == -1 || pos[i].y < first_vertex_y)
                {
                    first_vertex_y = pos[i].y;
                    firts_vertex = i;
                }

        // and now lets find first edge...

        int second_vertex = -1;
        for (int i = getVertex(firts_vertex).neiBegin(); i != getVertex(firts_vertex).neiEnd(); i = getVertex(firts_vertex).neiNext(i))
        {
            int new_vertex = getVertex(firts_vertex).neiVertex(i);
            if (isVertexDrawn(new_vertex) &&
                (second_vertex == -1 || Vec2f::cross(pos[second_vertex] - pos[firts_vertex], pos[new_vertex] - pos[firts_vertex]) > 0))
            {
                second_vertex = new_vertex;
                first_edge = getVertex(firts_vertex).neiEdge(i);
            }
        }
    }

    vertices.push(firts_vertex);
    edges.push(first_edge);

    while (true)
    {
        int current_vertex = vertices.top();
        int current_edge = edges.top();

        int next_vertex = getEdge(current_edge).findOtherEnd(current_vertex);
        int next_edge = -1;
        int next_vertex2 = current_vertex;

        for (int i = getVertex(next_vertex).neiBegin(); i != getVertex(next_vertex).neiEnd(); i = getVertex(next_vertex).neiNext(i))
        {
            int try_vertex = getVertex(next_vertex).neiVertex(i);
            if (isVertexDrawn(try_vertex) && try_vertex != current_vertex)
            {
                bool need_to_update = false;
                if (next_vertex2 == current_vertex)
                {
                    need_to_update = true;
                }
                else
                {
                    if (Vec2f::cross(pos[next_vertex2] - pos[next_vertex], pos[current_vertex] - pos[next_vertex]) >= 0)
                    {
                        need_to_update = Vec2f::cross(pos[next_vertex2] - pos[next_vertex], pos[try_vertex] - pos[next_vertex]) >= 0 &&
                                         Vec2f::cross(pos[try_vertex] - pos[next_vertex], pos[current_vertex] - pos[next_vertex]) >= 0;
                    }
                    else
                    {
                        need_to_update = Vec2f::cross(pos[next_vertex2] - pos[next_vertex], pos[try_vertex] - pos[next_vertex]) >= 0 ||
                                         Vec2f::cross(pos[try_vertex] - pos[next_vertex], pos[current_vertex] - pos[next_vertex]) >= 0;
                    }
                }
                if (need_to_update)
                {
                    next_vertex2 = try_vertex;
                    next_edge = getVertex(next_vertex).neiEdge(i);
                }
            }
        }

        if (next_vertex == vertices[0] && next_edge == edges[0])
            break;
        else
        {
            vertices.push(next_vertex);
            edges.push(next_edge);
            if (vertices.size() > vertexCount())
            {
                vertices.clear_resize(0);
                edges.clear_resize(0);
                break;
            }
        }
    }

    cycle.copy(vertices, edges);
    // cycle.canonize();
}
