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

// Characterization tests for indigo::Graph vertex/edge lifecycle.
//
// Rationale (task #3766): Graph is the heaviest consumer of ObjPool<Vertex>
// (and Pool<Edge>), and the perf baseline shows Graph::getVertex/getEdge +
// pool accessors are ~29% of the core suite. Graph has no dedicated unit test;
// all confidence rests on incidental coverage. These tests lock the invariants
// most at risk in the pool migration: index allocation, LIFO slot reuse after
// removal, incident-edge removal, iteration skipping holes, throwing access to
// a removed slot, and cloneGraph preserving every live vertex.

#include <gtest/gtest.h>

#include <base_cpp/array.h>
#include <base_cpp/exception.h>
#include <graph/graph.h>

using namespace indigo;

TEST(GraphContract, AddVerticesAndEdgesAssignMonotonicIndices)
{
    Graph g;
    EXPECT_EQ(0, g.addVertex());
    EXPECT_EQ(1, g.addVertex());
    EXPECT_EQ(2, g.addVertex());
    EXPECT_EQ(3, g.vertexCount());
    EXPECT_EQ(0, g.addEdge(0, 1));
    EXPECT_EQ(1, g.addEdge(1, 2));
    EXPECT_EQ(2, g.edgeCount());
}

// Core golden-master: removeVertex drops incident edges and frees the slot;
// the next addVertex reuses the freed index (LIFO).
TEST(GraphContract, RemoveVertexRemovesIncidentEdgesAndReusesIndexLIFO)
{
    Graph g;
    g.addVertex(); // 0
    g.addVertex(); // 1
    g.addVertex(); // 2
    g.addEdge(0, 1);
    g.addEdge(1, 2); // both edges incident to vertex 1
    ASSERT_EQ(2, g.edgeCount());

    g.removeVertex(1);
    EXPECT_EQ(2, g.vertexCount());
    EXPECT_EQ(0, g.edgeCount()); // incident edges removed with the vertex
    EXPECT_FALSE(g.hasVertex(1));

    int reused = g.addVertex();
    EXPECT_EQ(1, reused); // LIFO reuse of the freed slot
    EXPECT_TRUE(g.hasVertex(1));
    EXPECT_EQ(3, g.vertexCount());
}

TEST(GraphContract, AccessRemovedVertexThrows)
{
    Graph g;
    int v = g.addVertex();
    g.addVertex();
    g.removeVertex(v);
    EXPECT_THROW(g.getVertex(v), Exception);
}

TEST(GraphContract, EdgeIndexReuseLIFO)
{
    Graph g;
    g.addVertex();
    g.addVertex();
    g.addVertex();
    g.addEdge(0, 1);      // edge 0
    int e1 = g.addEdge(1, 2); // edge 1
    g.removeEdge(e1);
    int reused = g.addEdge(0, 2);
    EXPECT_EQ(e1, reused);
}

TEST(GraphContract, VertexIterationSkipsRemovedHoles)
{
    Graph g;
    for (int k = 0; k < 5; k++)
        g.addVertex(); // 0..4
    g.removeVertex(1);
    g.removeVertex(3);

    std::vector<int> visited;
    for (int i = g.vertexBegin(); i != g.vertexEnd(); i = g.vertexNext(i))
        visited.push_back(i);

    ASSERT_EQ(3u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
    EXPECT_EQ(4, visited[2]);
}

// cloneGraph must carry over every live vertex of the source (even when the
// source has holes from prior removals), producing a valid mapping old->new.
TEST(GraphContract, CloneGraphPreservesAllLiveVertices)
{
    Graph src;
    src.addVertex(); // 0
    src.addVertex(); // 1
    src.addVertex(); // 2
    src.addEdge(0, 1);
    src.addEdge(1, 2);
    src.removeVertex(1); // hole at 1; live vertices {0, 2}
    ASSERT_EQ(2, src.vertexCount());

    Graph dst;
    Array<int> mapping;
    dst.cloneGraph(src, &mapping);

    EXPECT_EQ(src.vertexCount(), dst.vertexCount());
    for (int i = src.vertexBegin(); i != src.vertexEnd(); i = src.vertexNext(i))
    {
        ASSERT_GE(mapping[i], 0);
        EXPECT_TRUE(dst.hasVertex(mapping[i]));
    }
}
