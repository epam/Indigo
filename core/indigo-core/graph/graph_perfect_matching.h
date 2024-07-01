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
//

#ifndef __graph_perfect_matching_h__
#define __graph_perfect_matching_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class Graph;

    // Graph matching problem solver
    class GraphPerfectMatching
    {
    public:
        enum
        {
            USE_EXTERNAL_EDGES_PTR = 0x01,
            USE_EDGES_MAPPING = 0x02,
            USE_VERTICES_SET = 0x04
        };

    public:
        GraphPerfectMatching(const Graph& graph, int params);
        virtual ~GraphPerfectMatching();

        void reset(void);

        void setEdgeMatching(int edge_idx, bool matching);
        int isEdgeMatching(int edge_idx);
        const byte* getEdgesState(void);

        int isVertexInMatching(int v_idx);
        void removeVertexFromMatching(int v_idx);

        void setAllVerticesInMatching(void);
        void setMatchingEdgesPtr(byte* matchingEdges);
        void setEdgesMappingPtr(int* edgesMap);
        void setVerticesSetPtr(int* verticesSet, int count);

        bool findMatching(void);
        bool findAlternatingPath(void);
        bool findAlternatingPath(int v1, int v2, bool isFirstEdgeMatching, bool isLastEdgeMatching);

        void processPath(void);
        int* getPath(void);
        int getPathSize(void);
        void setPath(int* path, int length);
        void clearPath(void);

        virtual bool checkVertex(int /*v_idx*/)
        {
            return true;
        }
        // e_idx - edge index in graph (not mapping)
        virtual bool checkEdge(int /*e_idx*/)
        {
            return true;
        }

        DECL_ERROR;

    protected:
        bool _PathFinder(int v_idx, int needMatchingEdge);

    protected:
        struct VertexExtInfo
        {
            int inPathMark;
            int isInMatching;
        };
        enum
        {
            FIND_ANY_ALTERNATING_PATH,
            FIND_ALTERNATING_CIRCLE
        };

    protected:
        const Graph& _graph;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _matchingEdgesLocal);
        TL_CP_DECL(Array<VertexExtInfo>, _verticesInfo);
        // Path has the following format: (v0, localEdge0, localEdge1, ...)
        TL_CP_DECL(Array<int>, _path);
        TL_CP_DECL(Array<int>, _edgesMappingLocal);
        TL_CP_DECL(Array<int>, _verticesUsedLocal);

        byte* _matchingEdges;
        int* _edgesMapping;
        int* _verticesUsed;
        int _verticesUsedCount;

        int _pathFinderState;
        int _pathFinderStopVertex;
        int _pathFinderIsLastMatching;
        int _leftExposedVertices;
        int _pathFinderUsedMark;
    };

} // namespace indigo

#endif
