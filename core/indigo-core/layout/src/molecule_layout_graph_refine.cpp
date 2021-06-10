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

#include "base_cpp/obj.h"
#include "base_cpp/red_black.h"
#include "graph/biconnected_decomposer.h"
#include "graph/graph_subchain_enumerator.h"
#include "graph/path_enumerator.h"
#include "layout/molecule_layout_graph.h"
#include "layout/refinement_state.h"

using namespace indigo;

bool MoleculeLayoutGraph::_edge_check(Graph& graph, int e_idx, void* context_)
{
    EnumContext& context = *(EnumContext*)context_;
    if (!context.graph->getLayoutEdge(e_idx).is_cyclic)
    {
        if (context.graph->_n_fixed > 0)
        {
            const Edge& edge = context.graph->getEdge(e_idx);

            if (context.graph->_fixed_vertices[edge.beg] && context.graph->_fixed_vertices[edge.end])
                return false;
        }
        context.edges->find_or_insert(e_idx);
        return true;
    }
    return false;
}

bool MoleculeLayoutGraph::_path_handle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context_)
{
    EnumContext& context = *(EnumContext*)context_;
    int i;

    for (i = 0; i < edges.size(); i++)
        if (!context.graph->getLayoutEdge(edges[i]).is_cyclic)
        {
            if (context.graph->_n_fixed > 0)
            {
                const Edge& edge = context.graph->getEdge(edges[i]);

                if (context.graph->_fixed_vertices[edge.beg] && context.graph->_fixed_vertices[edge.end])
                    continue;
            }
            context.edges->find_or_insert(edges[i]);
        }
    return false;
}

// Split graph in two branches by acyclic edge (its vertices are in different branches)
void MoleculeLayoutGraph::_makeBranches(Array<int>& branches, int edge, Filter& filter) const
{
    branches.clear_resize(vertexEnd());
    branches.zerofill();

    QS_DEF(Array<int>, dfs_stack);

    dfs_stack.clear();
    dfs_stack.push(_first_vertex_idx);

    int i, v, u;

    // DFS: find paths from v avoiding given edge
    while (dfs_stack.size() > 0)
    {
        v = dfs_stack.top();
        branches[v] = 1;

        const Vertex& vert = getVertex(v);
        bool no_push = true;

        for (i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
        {
            if (vert.neiEdge(i) == edge)
                continue;

            u = vert.neiVertex(i);

            if (!branches[u])
            {
                dfs_stack.push(u);
                no_push = false;
                break;
            }
        }

        if (no_push)
            dfs_stack.pop();
    }

    filter.init(branches.ptr(), Filter::EQ, 1);
}

bool MoleculeLayoutGraph::_allowRotateAroundVertex(int idx) const
{
    if (_molecule != 0)
    {
        const Vertex& v = getVertex(idx);

        if (v.degree() == 2)
        {
            int first_nei = v.neiBegin();

            int type1 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[v.neiEdge(first_nei)].ext_idx]);
            int type2 = _molecule->getBondOrder(_molecule_edge_mapping[_layout_edges[v.neiEdge(v.neiNext(first_nei))].ext_idx]);

            if (type1 == BOND_TRIPLE || type2 == BOND_TRIPLE || (type1 == BOND_DOUBLE && type2 == BOND_DOUBLE))
                return false;
        }
    }
    return true;
}

// Increase minimal distance between vertices
void MoleculeLayoutGraph::_refineCoordinates(const BiconnectedDecomposer& bc_decomposer, const PtrArray<MoleculeLayoutGraph>& bc_components,
                                             const Array<int>& bc_tree)
{
    RefinementState beg_state(*this);
    RefinementState best_state(*this);
    RefinementState new_state(*this);
    QS_DEF(Array<int>, branch);
    int v1, v2;
    int v1c, v2c;
    int i, j, n;

    v1c = v1 = vertexBegin();
    v2c = v2 = vertexNext(v1);

    // Calculate initial energy
    beg_state.copyFromGraph();
    beg_state.calcEnergy();
    beg_state.calcDistance(v1, v2);

    best_state.copy(beg_state);
    new_state.copy(beg_state);
    // Look through all vertex pairs which are closer than 0.6
    bool improved = true;
    QS_DEF(RedBlackSet<int>, edges);
    QS_DEF(Array<int>, components1);
    QS_DEF(Array<int>, components2);
    EnumContext context;

    context.edges = &edges;
    context.graph = this;
    context.maxIterationNumber = max_iterations;

    int max_improvements = max_iterations * max_iterations;
    int n_improvements = 0;

    while (improved)
    {
        if (max_improvements > 0 && n_improvements > max_improvements)
            break;

        n_improvements++;

        improved = false;

        edges.clear();

        int n_edges = 0;
        int n_enumerations = 0;
        bool to_break = false;

        new_state.copy(beg_state);

        for (v1 = vertexBegin(); v1 < vertexEnd() && !to_break; v1 = vertexNext(v1))
            for (v2 = vertexNext(v1); v2 < vertexEnd(); v2 = vertexNext(v2))
            {
                new_state.calcDistance(v1, v2);

                if (new_state.dist > 0.36f)
                    continue;

                // Check if they are from the same component
                bool next_pair = false;

                bc_decomposer.getVertexComponents(v1, components1);
                bc_decomposer.getVertexComponents(v2, components2);

                for (i = 0; i < components1.size() && !next_pair; i++)
                    for (j = 0; j < components2.size(); j++)
                    {
                        int comp1 = components1[i];
                        int comp2 = components2[j];

                        if (comp1 == comp2 && !bc_components[comp1]->isSingleEdge() && !bc_components[comp2]->isSingleEdge())
                        {
                            next_pair = true;
                            break;
                        }
                    }

                if (next_pair)
                    continue;

                // check iterations limit
                if (max_iterations > 0 && n_enumerations > max_iterations)
                {
                    to_break = true;
                    break;
                }

                n_enumerations++;

                // Find acyclic edges on the all paths between v1 and v2
                PathEnumerator path_enum(*this, v1, v2);

                path_enum.cb_check_edge = _edge_check;
                path_enum.cb_handle_path = _path_handle;
                path_enum.context = &context;

                context.iterationNumber = 0;

                try
                {
                    path_enum.process();
                }
                catch (Error)
                {
                    // iterations limit reached
                }

                if (edges.size() == n_edges)
                    continue;

                n_edges = edges.size();

                if (beg_state.dist - 0.00001 > new_state.dist)
                {
                    beg_state.dist = new_state.dist;
                    v1c = v1;
                    v2c = v2;
                }
            }

        if (edges.size() == 0)
        {
            beg_state.applyToGraph();
            break;
        }

        // Flipping
        // Look through found edges
        for (i = edges.begin(); i < edges.end(); i = edges.next(i))
        {
            if (max_improvements > 0 && n_improvements > max_improvements)
                break;

            n_improvements++;

            // Try to flip branch
            const Edge& edge = getEdge(edges.key(i));

            if (_molecule != 0 && _molecule->cis_trans.getParity(_molecule_edge_mapping[_layout_edges[edges.key(i)].ext_idx]) != 0)
                continue;

            if (getVertex(edge.beg).degree() == 1 || getVertex(edge.end).degree() == 1)
                continue;

            Filter filter;

            _makeBranches(branch, edges.key(i), filter);
            new_state.flipBranch(filter, beg_state, edge.beg, edge.end);
            new_state.calcEnergy();

            if (new_state.energy < best_state.energy - 0.00001)
            {
                improved = true;
                best_state.copy(new_state);
            }
        }

        if (improved)
        { // finished becouse of flipped
            beg_state.copy(best_state);
            continue;
        }

        // Rotations
        // Look through found edges
        for (i = edges.begin(); i < edges.end(); i = edges.next(i))
        {
            if (max_improvements > 0 && n_improvements > max_improvements)
                break;

            n_improvements += 3;

            // Try to rotate one branch by 10 degrees in both directions around both vertices
            const Edge& edge = getEdge(edges.key(i));

            if (_molecule != 0 && _molecule->cis_trans.getParity(_molecule_edge_mapping[_layout_edges[edges.key(i)].ext_idx]) != 0)
                continue;

            Filter filter;

            _makeBranches(branch, edges.key(i), filter);

            bool around_beg = _allowRotateAroundVertex(edge.beg);
            bool around_end = _allowRotateAroundVertex(edge.end);

            if (around_beg)
            {
                new_state.rotateBranch(filter, beg_state, edge.beg, 10);
                new_state.calcDistance(v1c, v2c);
                new_state.calcEnergy();

                if (new_state.dist > beg_state.dist && new_state.energy < best_state.energy - 0.00001)
                {
                    improved = true;
                    best_state.copy(new_state);
                }

                new_state.rotateBranch(filter, beg_state, edge.beg, -10);
                new_state.calcDistance(v1c, v2c);
                new_state.calcEnergy();

                if (new_state.dist > beg_state.dist && new_state.energy < best_state.energy - 0.00001)
                {
                    improved = true;
                    best_state.copy(new_state);
                }
            }

            if (around_end)
            {
                new_state.rotateBranch(filter, beg_state, edge.end, 10);
                new_state.calcDistance(v1c, v2c);
                new_state.calcEnergy();

                if (new_state.dist > beg_state.dist && new_state.energy < best_state.energy - 0.00001)
                {
                    improved = true;
                    best_state.copy(new_state);
                }

                new_state.rotateBranch(filter, beg_state, edge.end, -10);
                new_state.calcDistance(v1c, v2c);
                new_state.calcEnergy();

                if (new_state.dist > beg_state.dist && new_state.energy < best_state.energy - 0.00001)
                {
                    improved = true;
                    best_state.copy(new_state);
                }
            }

            // Stretching
            // Try to stretch each edge with 1.6 ratio
            if ((n = filter.count(*this)) != 1 && n != vertexCount() - 1)
            {
                new_state.stretchBranch(filter, beg_state, edge.beg, edge.end, 6);
                new_state.calcDistance(v1c, v2c);
                new_state.calcEnergy();

                if (new_state.dist > beg_state.dist && new_state.energy + 20 < beg_state.energy && new_state.energy < best_state.energy - 0.00001)
                {
                    improved = true;
                    best_state.copy(new_state);
                }
            }
        }

        if (improved)
            beg_state.copy(best_state);
    }

    if (_n_fixed == 0)
    {
        if (!beg_state.is_small_cycle())
        {
            int center = -1;
            long max_code = 0;

            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                if (getLayoutVertex(i).morgan_code > max_code)
                {
                    center = i;
                    max_code = getLayoutVertex(i).morgan_code;
                }

            beg_state.calcHeight();

            if (layout_orientation != UNCPECIFIED)
            {
                new_state.rotateLayout(beg_state, center, _2FLOAT(beg_state.calc_best_angle() / M_PI * 180.));
                beg_state.copy(new_state);

                if (layout_orientation == VERTICAL)
                {
                    new_state.rotateLayout(beg_state, center, 90);
                    beg_state.copy(new_state);
                }
            }
            else
            {

                for (float angle = -90.f; angle < 90.f + EPSILON; angle += 30.f)
                {
                    new_state.rotateLayout(beg_state, center, angle);
                    new_state.calcHeight();

                    if (new_state.height < beg_state.height - EPSILON)
                        beg_state.copy(new_state);
                }
            }
        }
    }

    beg_state.applyToGraph();

    _excludeDandlingIntersections();
}

void MoleculeLayoutGraph::_excludeDandlingIntersections()
{
    QS_DEF(Array<int>, edges);
    int i, j, res, beg1, end1, beg2, end2;
    float norm1, norm2;
    Vec2f a, b;

    edges.clear();

    // Find dandling edges
    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        const Vertex& vert = getVertex(i);

        if (vert.degree() == 1)
            edges.push(vert.neiEdge(vert.neiBegin()));
    }

    for (i = 0; i < edges.size(); i++)
    {
        const Edge& edge1 = getEdge(edges[i]);

        if (getVertex(edge1.beg).degree() == 1)
        {
            beg1 = edge1.end;
            end1 = edge1.beg;
        }
        else
        {
            beg1 = edge1.beg;
            end1 = edge1.end;
        }

        for (j = i + 1; j < edges.size(); j++)
        {
            res = _calcIntersection(edges[i], edges[j]);

            const Edge& edge2 = getEdge(edges[j]);

            if (getVertex(edge2.beg).degree() == 1)
            {
                beg2 = edge2.end;
                end2 = edge2.beg;
            }
            else
            {
                beg2 = edge2.beg;
                end2 = edge2.end;
            }

            switch (res)
            {
            case 223: // squeeze (v1,v2)
                a = getPos(beg1);
                b = getPos(end1);
                getPos(end1).lineCombin2(a, 0.3f, b, 0.7f);
                break;
            case 225: // squeeze (v3,v4)
                a = getPos(beg2);
                b = getPos(end2);
                getPos(end2).lineCombin2(a, 0.3f, b, 0.7f);
                break;
            case 23: // swap v2 and v4, equalize lengths
                std::swap(getPos(end1), getPos(end2));

                norm1 = Vec2f::dist(getPos(beg1), getPos(end1));
                norm2 = Vec2f::dist(getPos(beg2), getPos(end2));

                if (norm1 < 0.0001 || norm2 < 0.0001)
                    break;

                if (norm1 < norm2)
                {
                    a = getPos(beg2);
                    b = getPos(end2);

                    float t = norm1 / norm2;

                    getPos(end2).lineCombin2(a, 1.f - t, b, t);
                    break;
                }

                a = getPos(beg1);
                b = getPos(end1);

                float t = norm2 / norm1;

                getPos(end1).lineCombin2(a, 1.f - t, b, t);
                break;
            }
        }
    }
}
