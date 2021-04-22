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

#include "graph/graph_affine_matcher.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "math/algebra.h"

using namespace indigo;

IMPL_ERROR(GraphAffineMatcher, "graph affine matcher");

GraphAffineMatcher::GraphAffineMatcher(Graph& subgraph, Graph& supergraph, const int* mapping) : _subgraph(subgraph), _supergraph(supergraph), _mapping(mapping)
{
    cb_get_xyz = 0;
    fixed_vertices = 0;
}

bool GraphAffineMatcher::match(float rms_threshold)
{
    if (cb_get_xyz == 0)
        throw Error("cb_get_xyz not set");

    int i;
    Transform3f matr;
    Vec3f pos;

    QS_DEF(Array<Vec3f>, points);
    QS_DEF(Array<Vec3f>, goals);

    points.clear();
    goals.clear();

    if (fixed_vertices != 0)
    {
        for (i = 0; i < fixed_vertices->size(); i++)
        {
            if (_mapping[fixed_vertices->at(i)] < 0)
                continue;
            cb_get_xyz(_subgraph, fixed_vertices->at(i), pos);
            points.push(pos);
            cb_get_xyz(_supergraph, _mapping[fixed_vertices->at(i)], pos);
            goals.push(pos);
        }
    }
    else
        for (i = _subgraph.vertexBegin(); i < _subgraph.vertexEnd(); i = _subgraph.vertexNext(i))
        {
            if (_mapping[i] < 0)
                continue;
            cb_get_xyz(_subgraph, i, pos);
            points.push(pos);
            cb_get_xyz(_supergraph, _mapping[i], pos);
            goals.push(pos);
        }

    if (points.size() < 1)
        return true;

    float sqsum;

    if (!matr.bestFit(points.size(), points.ptr(), goals.ptr(), &sqsum))
        return false;

    if (sqsum > rms_threshold * rms_threshold)
        return false;

    return true;
}
