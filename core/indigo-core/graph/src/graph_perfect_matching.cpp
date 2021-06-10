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

//
// Solve graph perfect matching problem module:
//   To find maximum independent edge set
// Algorithm is based on alternating path algorithm
//

#include "graph/graph_perfect_matching.h"

#include "base_c/bitarray.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_ERROR(GraphPerfectMatching, "graph perfect matching");
CP_DEF(GraphPerfectMatching);

GraphPerfectMatching::GraphPerfectMatching(const Graph& graph, int params)
    : _graph(graph), CP_INIT, TL_CP_GET(_matchingEdgesLocal), TL_CP_GET(_verticesInfo), TL_CP_GET(_path), TL_CP_GET(_edgesMappingLocal),
      TL_CP_GET(_verticesUsedLocal)
{
    if (params & USE_EXTERNAL_EDGES_PTR)
        _matchingEdges = NULL;
    else
    {
        _matchingEdgesLocal.resize(bitGetSize(_graph.edgeEnd()));
        _matchingEdgesLocal.zerofill();
        _matchingEdges = _matchingEdgesLocal.ptr();
    }

    if (params & USE_EDGES_MAPPING)
        _edgesMapping = NULL;
    else
    {
        _edgesMappingLocal.resize(_graph.edgeEnd());
        for (int i = 0; i < _edgesMappingLocal.size(); i++)
            _edgesMappingLocal[i] = i;
        _edgesMapping = _edgesMappingLocal.ptr();
    }

    if (params & USE_VERTICES_SET)
    {
        _verticesUsed = NULL;
        _verticesUsedCount = 0;
    }
    else
    {
        _verticesUsedLocal.resize(_graph.vertexEnd());
        for (int v_idx = _graph.vertexBegin(); v_idx < _graph.vertexEnd(); v_idx = _graph.vertexNext(v_idx))
            _verticesUsedLocal.push(v_idx);
        _verticesUsed = _verticesUsedLocal.ptr();
    }

    _verticesInfo.resize(_graph.vertexEnd());
    _verticesInfo.zerofill();
    _pathFinderUsedMark = 1;

    _path.clear();

    _leftExposedVertices = 0;
}

GraphPerfectMatching::~GraphPerfectMatching()
{
}

void GraphPerfectMatching::reset(void)
{
    _verticesInfo.zerofill();
    if (_matchingEdges == _matchingEdgesLocal.ptr())
        _matchingEdgesLocal.zerofill();
    else
        throw Error("reset: internal error");
}

void GraphPerfectMatching::setEdgeMatching(int edge_idx, bool matching)
{
    const Edge& edge = _graph.getEdge(edge_idx);

    if (matching)
    {
        if (_verticesInfo[edge.beg].isInMatching || _verticesInfo[edge.end].isInMatching)
            throw Error("setEdgeMatching: internal error");
        _verticesInfo[edge.beg].isInMatching = _verticesInfo[edge.end].isInMatching = true;
        bitSetBit(_matchingEdges, _edgesMapping[edge_idx], 1);
        _leftExposedVertices -= 2;
    }
    else
    {
        if (!_verticesInfo[edge.beg].isInMatching || !_verticesInfo[edge.end].isInMatching)
            throw Error("setEdgeMatching: internal error");

        _verticesInfo[edge.beg].isInMatching = _verticesInfo[edge.end].isInMatching = false;
        bitSetBit(_matchingEdges, _edgesMapping[edge_idx], 0);
        _leftExposedVertices += 2;
    }
}

void GraphPerfectMatching::setAllVerticesInMatching(void)
{
    for (int v_idx = _graph.vertexBegin(); v_idx < _graph.vertexEnd(); v_idx = _graph.vertexNext(v_idx))
        _verticesInfo[v_idx].isInMatching = true;
}

void GraphPerfectMatching::setMatchingEdgesPtr(byte* matchingEdges)
{
    _matchingEdges = matchingEdges;
}

void GraphPerfectMatching::setEdgesMappingPtr(int* edgesMap)
{
    _edgesMapping = edgesMap;
}

void GraphPerfectMatching::setVerticesSetPtr(int* verticesSet, int count)
{
    _verticesUsed = verticesSet;
    _verticesUsedCount = count;
}

int GraphPerfectMatching::isEdgeMatching(int edge_idx)
{
    return bitGetBit(_matchingEdges, _edgesMapping[edge_idx]);
}

const byte* GraphPerfectMatching::getEdgesState(void)
{
    return _matchingEdges;
}

int GraphPerfectMatching::isVertexInMatching(int v_idx)
{
    return _verticesInfo[v_idx].isInMatching;
}

void GraphPerfectMatching::removeVertexFromMatching(int v_idx)
{
    const Vertex& vertex = _graph.getVertex(v_idx);

    for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
    {
        int e_idx = vertex.neiEdge(j);
        int vn_idx = vertex.neiVertex(j);
        if (_edgesMapping[e_idx] == -1)
            continue;
        if (!checkEdge(e_idx) || !bitGetBit(_matchingEdges, _edgesMapping[e_idx]) || !checkVertex(vn_idx))
            continue;

        _verticesInfo[vn_idx].isInMatching = false;
        _verticesInfo[v_idx].isInMatching = false;
        bitSetBit(_matchingEdges, _edgesMapping[e_idx], 0);
        break;
    }
}

bool GraphPerfectMatching::findMatching(void)
{
    _leftExposedVertices = 0;

    for (int i = 0; i < _verticesUsedCount; i++)
    {
        int v_idx = _verticesUsed[i];
        if (checkVertex(v_idx) && !_verticesInfo[v_idx].isInMatching)
            _leftExposedVertices++;
    }

    if (_leftExposedVertices % 2 == 1)
        return false;

    while (_leftExposedVertices > 0 && findAlternatingPath())
        processPath();

    return _leftExposedVertices == 0;
}

bool GraphPerfectMatching::findAlternatingPath(void)
{
    _pathFinderState = FIND_ANY_ALTERNATING_PATH;
    for (int i = 0; i < _verticesUsedCount; i++)
    {
        int v_idx = _verticesUsed[i];
        if (!_verticesInfo[v_idx].isInMatching && checkVertex(v_idx))
        {
            _path.clear();
            _path.push(v_idx);
            _verticesInfo[v_idx].inPathMark = _pathFinderUsedMark;
            if (_PathFinder(v_idx, false))
            {
                _pathFinderUsedMark++;
                return true;
            }
            _verticesInfo[v_idx].inPathMark = -1;
        }
    }
    _pathFinderUsedMark++;
    return false;
}

bool GraphPerfectMatching::findAlternatingPath(int v1, int v2, bool isFirstEdgeMatching, bool isLastEdgeMatching)
{
    _pathFinderState = FIND_ALTERNATING_CIRCLE;
    _pathFinderStopVertex = v2;
    _pathFinderIsLastMatching = isLastEdgeMatching;

    _path.clear();
    _path.push(v1);
    _verticesInfo[v1].inPathMark = _pathFinderUsedMark;
    bool ret = _PathFinder(v1, isFirstEdgeMatching);
    _pathFinderUsedMark++;

    return ret;
}

// Remap edges in path
void GraphPerfectMatching::processPath(void)
{
    int cur_idx = _path[0];

    const Vertex& vertex = _graph.getVertex(cur_idx);
    int edge_idx = vertex.neiEdge(_path[1]);
    int matchingState = bitGetBit(_matchingEdges, _edgesMapping[edge_idx]);

    if (_verticesInfo[cur_idx].isInMatching == !matchingState)
        throw Error("processPath: invalid alternating path");
    _verticesInfo[cur_idx].isInMatching = !matchingState;
    _leftExposedVertices += matchingState ? 1 : -1;

    for (int i = 1; i < _path.size(); i++)
    {
        const Vertex& vertex = _graph.getVertex(cur_idx);
        int next_idx = vertex.neiVertex(_path[i]);
        int edge_idx = vertex.neiEdge(_path[i]);

        if (_edgesMapping[edge_idx] == -1)
            continue;

        if (!matchingState ^ bitGetBit(_matchingEdges, _edgesMapping[edge_idx]))
            bitSetBit(_matchingEdges, _edgesMapping[edge_idx], !matchingState);
        else
            throw Error("processPath: invalid alternating path");

        matchingState = !matchingState;
        cur_idx = next_idx;
    }

    if (_verticesInfo[cur_idx].isInMatching == matchingState)
        throw Error("processPath: invalid alternating path");

    _verticesInfo[cur_idx].isInMatching = matchingState;
    _leftExposedVertices -= matchingState ? 1 : -1;
}

int* GraphPerfectMatching::getPath(void)
{
    return _path.ptr();
}

int GraphPerfectMatching::getPathSize(void)
{
    return _path.size();
}

void GraphPerfectMatching::setPath(int* path, int length)
{
    _path.resize(length);
    memcpy(_path.ptr(), path, length * sizeof(int));
}

bool GraphPerfectMatching::_PathFinder(int v_idx, int needMatchingEdge)
{
    const Vertex& vertex = _graph.getVertex(v_idx);

    for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);
        int vn_idx = vertex.neiVertex(i);

        if (_edgesMapping[e_idx] == -1)
            continue;
        if (_verticesInfo[vn_idx].inPathMark == _pathFinderUsedMark)
            continue;
        if (!checkVertex(vn_idx) || !checkEdge(e_idx))
            continue;

        int isInMatching = bitGetBit(_matchingEdges, _edgesMapping[e_idx]);

        if ((needMatchingEdge ^ isInMatching))
            continue;

        _path.push(i);
        _verticesInfo[vn_idx].inPathMark = _pathFinderUsedMark;

        // Check exit condition
        if (_pathFinderState == FIND_ANY_ALTERNATING_PATH)
        {
            if (!needMatchingEdge && !_verticesInfo[vn_idx].isInMatching)
                return true;
        }
        else
        {
            if (vn_idx == _pathFinderStopVertex && needMatchingEdge == _pathFinderIsLastMatching)
                return true;
        }

        if (_PathFinder(vn_idx, !needMatchingEdge))
        {
            return true;
        }
        _verticesInfo[vn_idx].inPathMark = -1;
        _path.pop();
    }
    return false;
}
