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

#include "graph/edge_rotation_matcher.h"
#include "graph/graph.h"
#include "graph/graph_affine_matcher.h"
#include "graph/spanning_tree.h"
#include "math/algebra.h"

using namespace indigo;

IMPL_ERROR(EdgeRotationMatcher, "edge rotation matcher");

EdgeRotationMatcher::EdgeRotationMatcher(Graph& subgraph, Graph& supergraph, const int* mapping)
    : _subgraph(subgraph), _supergraph(supergraph), _mapping(mapping)
{
    cb_get_xyz = 0;
    cb_can_rotate = 0;
    equalize_edges = false;
}

bool EdgeRotationMatcher::match(float rms_threshold, float eps)
{
    if (cb_get_xyz == 0)
        throw Error("cb_get_xyz not specified");

    if (_subgraph.vertexCount() < 2 || _subgraph.edgeCount() < 1)
        return true;

    QS_DEF(Array<int>, in_cycle);
    QS_DEF(Array<_DirEdge>, edge_queue);
    QS_DEF(Array<int>, vertex_queue);
    QS_DEF(Array<int>, states);

    in_cycle.clear_resize(_subgraph.edgeEnd());
    edge_queue.clear();
    states.clear_resize(std::max(_subgraph.edgeEnd(), _subgraph.vertexEnd() + 1));

    int i, j, k, bottom;

    // Find all subgraph bridges
    SpanningTree spt(_subgraph, 0);

    in_cycle.zerofill();

    spt.markAllEdgesInCycles(in_cycle.ptr(), 1);

    // Find the first bridge, put it to the queue 2 times
    for (i = _subgraph.edgeBegin(); i < _subgraph.edgeEnd(); i = _subgraph.edgeNext(i))
        if (!in_cycle[i] && (cb_can_rotate == 0 || cb_can_rotate(_subgraph, i)))
        {
            const Edge& edge = _subgraph.getEdge(i);

            if (_mapping[edge.beg] < 0 || _mapping[edge.end] < 0)
                continue;

            edge_queue.push();
            edge_queue.top().idx = i;
            edge_queue.top().beg = edge.beg;
            edge_queue.top().end = edge.end;

            edge_queue.push();
            edge_queue.top().idx = i;
            edge_queue.top().beg = edge.end;
            edge_queue.top().end = edge.beg;
            break;
        }

    // If the queue is empty, then we have no bridge
    if (edge_queue.size() == 0)
    {
        GraphAffineMatcher afm(_subgraph, _supergraph, _mapping);

        afm.cb_get_xyz = cb_get_xyz;
        return afm.match(rms_threshold);
    }

    float scale = 1.f;

    // detect scaling factor by average bond length
    if (equalize_edges)
    {
        float sum_sub = 0.f, sum_super = 0.f;

        for (i = _subgraph.edgeBegin(); i < _subgraph.edgeEnd(); i = _subgraph.edgeNext(i))
        {
            const Edge& edge = _subgraph.getEdge(i);
            Vec3f beg, end;

            cb_get_xyz(_subgraph, edge.beg, beg);
            cb_get_xyz(_subgraph, edge.end, end);

            sum_sub += Vec3f::dist(beg, end);
        }

        for (i = _supergraph.edgeBegin(); i < _supergraph.edgeEnd(); i = _supergraph.edgeNext(i))
        {
            const Edge& edge = _supergraph.getEdge(i);
            Vec3f beg, end;

            cb_get_xyz(_supergraph, edge.beg, beg);
            cb_get_xyz(_supergraph, edge.end, end);

            sum_super += Vec3f::dist(beg, end);
        }

        if (sum_sub > EPSILON && sum_super > EPSILON)
        {
            sum_sub /= _subgraph.edgeCount();
            sum_super /= _supergraph.edgeCount();
            scale = sum_super / sum_sub;
        }
    }

    // save vertex positions

    QS_DEF(Array<Vec3f>, xyz_sub);
    QS_DEF(Array<Vec3f>, xyz_super);
    QS_DEF(Array<int>, xyzmap);

    xyzmap.clear_resize(_supergraph.vertexEnd());
    xyz_sub.clear();
    xyz_super.clear();

    for (i = _subgraph.vertexBegin(); i != _subgraph.vertexEnd(); i = _subgraph.vertexNext(i))
    {
        if (_mapping[i] < 0)
            continue;

        Vec3f& pos_sub = xyz_sub.push();
        Vec3f& pos_super = xyz_super.push();

        cb_get_xyz(_subgraph, i, pos_sub);
        cb_get_xyz(_supergraph, _mapping[i], pos_super);

        pos_sub.scale(scale);

        xyzmap[_mapping[i]] = xyz_sub.size() - 1;
    }

    // Make queue of edges
    states.zerofill();
    bottom = 0;

    while (edge_queue.size() != bottom)
    {
        // extract edge from queue
        int edge_end = edge_queue[bottom].end;
        int edge_idx = edge_queue[bottom].idx;
        bottom++;

        // mark it as 'completed'
        states[edge_idx] = 2;

        // look for neighbors
        const Vertex& end_vertex = _subgraph.getVertex(edge_end);

        for (i = end_vertex.neiBegin(); i != end_vertex.neiEnd(); i = end_vertex.neiNext(i))
        {
            int nei_edge_idx = end_vertex.neiEdge(i);

            // check that neighbor have 'untouched' status
            if (states[nei_edge_idx] != 0)
                continue;

            const Edge& nei_edge = _subgraph.getEdge(nei_edge_idx);
            int other_end = nei_edge.findOtherEnd(edge_end);

            if (_mapping[other_end] < 0)
                continue;

            // set status 'in process'
            states[nei_edge_idx] = 1;

            // push the neighbor edge to the queue
            edge_queue.push();
            edge_queue.top().idx = nei_edge_idx;
            edge_queue.top().beg = edge_end;
            edge_queue.top().end = other_end;
        }
    }

    // do initial transform (impose first subgraph edge in the queue on corresponding one in the graph)
    int beg2 = edge_queue[0].beg;
    int end2 = edge_queue[0].end;
    int beg1 = _mapping[beg2];
    int end1 = _mapping[end2];
    Vec3f g1_v1, g1_v2, g2_v1, g2_v2, diff1, diff2;
    Transform3f matr;

    cb_get_xyz(_supergraph, beg1, g1_v1);
    cb_get_xyz(_supergraph, end1, g1_v2);

    cb_get_xyz(_subgraph, beg2, g2_v1);
    cb_get_xyz(_subgraph, end2, g2_v2);

    g2_v1.scale(scale);
    g2_v2.scale(scale);

    diff1.diff(g1_v2, g1_v1);
    diff2.diff(g2_v2, g2_v1);

    matr.identity();
    if (!matr.rotationVecVec(diff2, diff1))
        throw Error("error calling RotationVecVec()");

    matr.translateLocal(-g2_v1.x, -g2_v1.y, -g2_v1.z);
    matr.translate(g1_v1);

    for (k = 0; k < xyz_sub.size(); k++)
        xyz_sub[k].transformPoint(matr);

    // for all edges in queue that are subject to rotate...
    for (i = 0; i < edge_queue.size(); i++)
    {
        int edge_beg = edge_queue[i].beg;
        int edge_end = edge_queue[i].end;
        int edge_idx = edge_queue[i].idx;

        if (in_cycle[edge_idx])
            continue;

        if (cb_can_rotate != 0 && !cb_can_rotate(_subgraph, edge_idx))
            continue;

        // start BFS from the end of the edge
        states.zerofill();
        states[edge_end] = 1;

        vertex_queue.clear();
        vertex_queue.push(edge_end);
        bottom = 0;

        while (vertex_queue.size() != bottom)
        {
            // extract vertex from queue
            const Vertex& vertex = _subgraph.getVertex(vertex_queue[bottom]);

            states[vertex_queue[bottom]] = 2;
            bottom++;

            // look over neighbors
            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
                int nei_idx = vertex.neiVertex(j);

                if (nei_idx == edge_beg)
                    continue;
                if (states[nei_idx] != 0)
                    continue;

                states[nei_idx] = 1;
                vertex_queue.push(nei_idx);
            }
        }

        // now states[j] == 0 if j-th vertex shound not be moved

        Vec3f edge_beg_pos, edge_end_pos, rot_axis;

        // get rotation axis
        edge_beg_pos.copy(xyz_sub[xyzmap[_mapping[edge_beg]]]);
        edge_end_pos.copy(xyz_sub[xyzmap[_mapping[edge_end]]]);

        rot_axis.diff(edge_end_pos, edge_beg_pos);
        if (!rot_axis.normalize())
            continue;

        const Vertex& edge_end_vertex = _subgraph.getVertex(edge_end);

        float max_sum_len = -1;

        for (j = edge_end_vertex.neiBegin(); j != edge_end_vertex.neiEnd(); j = edge_end_vertex.neiNext(j))
        {
            int nei_idx_2 = edge_end_vertex.neiVertex(j);
            int nei_idx_1 = _mapping[nei_idx_2];

            if (nei_idx_2 == edge_beg)
                continue;

            if (nei_idx_1 == -1)
                continue;

            Vec3f nei1_pos;
            Vec3f nei2_pos;

            nei1_pos.copy(xyz_super[xyzmap[nei_idx_1]]);
            nei2_pos.copy(xyz_sub[xyzmap[_mapping[nei_idx_2]]]);

            nei1_pos.sub(edge_end_pos);
            nei2_pos.sub(edge_end_pos);

            float dot1 = Vec3f::dot(nei1_pos, rot_axis);
            float dot2 = Vec3f::dot(nei2_pos, rot_axis);

            nei1_pos.addScaled(rot_axis, -dot1);
            nei2_pos.addScaled(rot_axis, -dot2);

            if (max_sum_len > nei1_pos.length() + nei1_pos.length())
                continue;

            max_sum_len = nei1_pos.length() + nei1_pos.length();

            if (!nei1_pos.normalize() || !nei2_pos.normalize())
                continue;

            double dp = Vec3f::dot(nei1_pos, nei2_pos);

            if (dp > 1 - EPSILON)
                dp = 1 - EPSILON;
            if (dp < -1 + EPSILON)
                dp = -1 + EPSILON;

            double ang = acos(dp);

            Vec3f cross;

            cross.cross(nei1_pos, nei2_pos);

            if (Vec3f::dot(cross, rot_axis) < 0)
                ang = -ang;

            matr.rotation(rot_axis.x, rot_axis.y, rot_axis.z, (float)ang);
            matr.translateLocalInv(edge_end_pos);
            matr.translate(edge_end_pos);
        }

        if (max_sum_len > 0)
        {
            for (j = _subgraph.vertexBegin(); j < _subgraph.vertexEnd(); j = _subgraph.vertexNext(j))
                if (_mapping[j] >= 0 && states[j] != 0)
                    xyz_sub[xyzmap[_mapping[j]]].transformPoint(matr);
        }
    }

    float sqsum = 0;

    for (k = 0; k < xyz_sub.size(); k++)
        sqsum += Vec3f::distSqr(xyz_sub[k], xyz_super[k]);

    sqsum = sqrt(sqsum / xyz_sub.size());

    if (sqsum > rms_threshold + eps)
        return false;

    return true;
}
