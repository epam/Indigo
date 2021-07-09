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

#include "layout/attachment_layout.h"

using namespace indigo;

CP_DEF(AttachmentLayout);

AttachmentLayout::AttachmentLayout(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components, const Array<int>& bc_tree,
                                   MoleculeLayoutGraph& graph, int src_vertex)
    : _src_vertex(src_vertex), CP_INIT, TL_CP_GET(_src_vertex_map), TL_CP_GET(_attached_bc), TL_CP_GET(_bc_angles), TL_CP_GET(_vertices_l), _alpha(0.f),
      TL_CP_GET(_new_vertices), TL_CP_GET(_layout), _energy(0.f), _bc_components(bc_components), _graph(graph)
{
    int i, v1, v2;
    float sum = 0.f;

    int n_comp = bc_decom.getIncomingCount(_src_vertex);

    if (bc_tree[_src_vertex] != -1)
    {
        _attached_bc.clear_resize(n_comp + 1);
        _attached_bc.top() = bc_tree[_src_vertex];
    }
    else
        _attached_bc.clear_resize(n_comp);

    _src_vertex_map.clear_resize(_attached_bc.size());
    _bc_angles.clear_resize(_attached_bc.size());
    _vertices_l.clear_resize(_attached_bc.size());

    for (i = 0; i < _attached_bc.size(); i++)
    {
        if (i < n_comp)
            _attached_bc[i] = bc_decom.getIncomingComponents(_src_vertex)[i];

        const MoleculeLayoutGraph& cur_bc = *bc_components[_attached_bc[i]];

        _src_vertex_map[i] = cur_bc.findVertexByExtIdx(_src_vertex);
        _bc_angles[i] = cur_bc.calculateAngle(_src_vertex_map[i], v1, v2);
        sum += _bc_angles[i];
        if (cur_bc.isFlipped())
            _vertices_l[i] = v2;
        else
            _vertices_l[i] = v1;
    }

    _alpha = _2FLOAT((2. * M_PI - sum) / _attached_bc.size());
    // TODO: what if negative?

    // find the one component which is drawn and put it to the end
    for (i = 0; i < _attached_bc.size() - 1; i++)
    {
        if (_graph.getVertexType(_bc_components[_attached_bc[i]]->getVertexExtIdx(_vertices_l[i])) != ELEMENT_NOT_DRAWN)
        {
            _src_vertex_map.swap(i, _attached_bc.size() - 1);
            _attached_bc.swap(i, _attached_bc.size() - 1);
            _bc_angles.swap(i, _attached_bc.size() - 1);
            _vertices_l.swap(i, _attached_bc.size() - 1);

            break;
        }
    }

    int n_new_vert = 0;

    for (i = 0; i < _attached_bc.size() - 1; i++)
        n_new_vert += _bc_components[_attached_bc[i]]->vertexCount() - 1;

    _new_vertices.clear_resize(n_new_vert);
    _layout.clear_resize(n_new_vert);
    _layout.zerofill();
}

AttachmentLayoutSimple::AttachmentLayoutSimple(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components,
                                               const Array<int>& bc_tree, MoleculeLayoutGraph& graph, int src_vertex)
    : AttachmentLayout(bc_decom, bc_components, bc_tree, graph, src_vertex)
{
}

AttachmentLayoutSmart::AttachmentLayoutSmart(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components,
                                             const Array<int>& bc_tree, MoleculeLayoutGraph& graph, int src_vertex)
    : AttachmentLayout(bc_decom, bc_components, bc_tree, graph, src_vertex)
{
}

// Calculate energy of the drawn part of graph
float AttachmentLayout::calculateEnergy()
{
    int i, j;
    float sum_a;
    float r;
    QS_DEF(Array<float>, norm_a);
    QS_DEF(Array<int>, drawn_vertices);

    drawn_vertices.clear_resize(_graph.vertexEnd());
    drawn_vertices.zerofill();

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        if (_graph.getLayoutVertex(i).type != ELEMENT_NOT_DRAWN)
            drawn_vertices[i] = 1;

    for (i = 0; i < _new_vertices.size(); i++)
        drawn_vertices[_new_vertices[i]] = 2 + i;

    norm_a.clear_resize(_graph.vertexEnd());

    sum_a = 0.0f;
    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (drawn_vertices[i] > 0)
        {
            norm_a[i] = _2FLOAT(_graph.getLayoutVertex(i).morgan_code);
            sum_a += norm_a[i] * norm_a[i];
        }
    }

    sum_a = sqrt(sum_a);

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        if (drawn_vertices[i] > 0)
            norm_a[i] = (norm_a[i] / sum_a) + 0.5f;

    _energy = 0.0;

    const Vec2f* pos_i = 0;
    const Vec2f* pos_j = 0;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        if (drawn_vertices[i] > 0)
        {
            if (drawn_vertices[i] == 1)
                pos_i = &_graph.getPos(i);
            else
                pos_i = &_layout[drawn_vertices[i] - 2];

            for (j = _graph.vertexBegin(); j < _graph.vertexEnd(); j = _graph.vertexNext(j))
                if (drawn_vertices[j] > 0 && i != j)
                {
                    if (drawn_vertices[j] == 1)
                        pos_j = &_graph.getPos(j);
                    else
                        pos_j = &_layout[drawn_vertices[j] - 2];

                    r = Vec2f::distSqr(*pos_i, *pos_j);

                    if (r < EPSILON)
                        r = EPSILON;

                    _energy += (norm_a[i] * norm_a[j] / r);
                }
        }

    return _energy;
}

void AttachmentLayoutSimple::applyLayout()
{
    int i;

    for (i = 0; i < _new_vertices.size(); i++)
        _graph.getPos(_new_vertices[i]) = _layout[i];
}

void AttachmentLayoutSmart::applyLayout()
{
    int i;

    for (i = 0; i < _new_vertices.size(); i++)
        _graph.getPos(_new_vertices[i]) = _layout[i];

    for (int i = 0; i < _attached_bc.size(); i++)
    {
        MoleculeLayoutGraph& comp = (MoleculeLayoutGraph&)*_bc_components[_attached_bc[i]];

        for (int v = comp.vertexBegin(); v != comp.vertexEnd(); v = comp.vertexNext(v))
        {
            comp.getPos(v) = _graph.getPos(comp.getVertexExtIdx(v));
        }
    }
}

void AttachmentLayout::markDrawnVertices()
{
    int i, j;

    for (i = 0; i < _attached_bc.size(); i++)
    {
        const MoleculeLayoutGraph& comp = *_bc_components[_attached_bc[i]];

        for (j = comp.vertexBegin(); j < comp.vertexEnd(); j = comp.vertexNext(j))
        {
            const LayoutVertex& vert = comp.getLayoutVertex(j);

            _graph.setVertexType(vert.ext_idx, vert.type);
        }

        for (j = comp.edgeBegin(); j < comp.edgeEnd(); j = comp.edgeNext(j))
        {
            const LayoutEdge& edge = comp.getLayoutEdge(j);

            _graph.setEdgeType(edge.ext_idx, edge.type);
        }
    }
}

CP_DEF(LayoutChooser);

LayoutChooser::LayoutChooser(AttachmentLayout& layout)
    : _n_components(layout._attached_bc.size() - 1), _cur_energy(1E+20f), CP_INIT, TL_CP_GET(_comp_permutation), TL_CP_GET(_rest_numbers), _layout(layout)
{
    _comp_permutation.clear_resize(_n_components);
    _rest_numbers.clear_resize(_n_components);

    for (int i = 0; i < _n_components; i++)
        _rest_numbers[i] = i;
}

// Look through all perturbations recursively
void LayoutChooser::_perform(int level)
{
    int i;

    if (level == 0)
    {
        // Try current perturbation
        // Draw new components on vertex
        _makeLayout();

        // Check if new layout is better
        if (_layout.calculateEnergy() < _cur_energy - EPSILON)
        {
            _layout.applyLayout();
            _cur_energy = _layout._energy;
        }

        return;
    }

    for (i = 0; i < level; i++)
    {
        _comp_permutation[level - 1] = _rest_numbers[i];
        _rest_numbers[i] = _rest_numbers[level - 1];
        _rest_numbers[level - 1] = _comp_permutation[level - 1];
        _perform(level - 1);
        _rest_numbers[level - 1] = _rest_numbers[i];
        _rest_numbers[i] = _comp_permutation[level - 1];
    }
}

// Draw components connected with respect to current order
void LayoutChooser::_makeLayout()
{
    int i, j, k;
    float cur_angle;
    int v1C, v2C, v1, v2;
    Vec2f p, p1;
    float phi, phi1, phi2;
    float cosa, sina;
    int v;

    // cur_angle - angle between first edge of the drawn component and most "right" edge of current component
    k = -1;
    v = _layout._src_vertex;
    cur_angle = _layout._bc_angles[_n_components];
    v2 = _layout._bc_components[_layout._attached_bc[_n_components]]->getVertexExtIdx(_layout._vertices_l[_n_components]);
    // number of the last vertex of drown biconnected component in the connected component

    for (i = 0; i < _n_components; i++)
    {
        cur_angle += _layout._alpha;

        // Shift and rotate component so cur_angle is the angle between [v1C,v2C] and drawn edge [v,v2]
        int comp_idx = _comp_permutation[i];
        const MoleculeLayoutGraph& comp = *_layout._bc_components[_layout._attached_bc[comp_idx]];

        v1C = _layout._src_vertex_map[comp_idx];
        v2C = _layout._vertices_l[comp_idx];
        v1 = comp.getVertexExtIdx(v2C);

        // Calculate angle (phi2-phi1) between [v,v2] and [v=v1C,v2C] in CCW order;

        p.diff(_layout._graph.getPos(v2), _layout._graph.getPos(v));

        phi1 = p.tiltAngle();

        p.diff(comp.getPos(v2C), comp.getPos(v1C));

        phi2 = p.tiltAngle();

        // Save current component's coordinates
        phi = cur_angle - (phi2 - phi1);
        cosa = cos(phi);
        sina = sin(phi);

        p.diff(_layout._graph.getPos(v), comp.getPos(v1C));

        for (j = comp.vertexBegin(); j < comp.vertexEnd(); j = comp.vertexNext(j))
            if (comp.getVertexExtIdx(j) != v)
            {
                k = k + 1;
                Vec2f& cur_pos = _layout._layout[k];
                // 1. Shift
                cur_pos.sum(comp.getPos(j), p);
                // 2. Rotate around v
                p1.diff(cur_pos, _layout._graph.getPos(v));
                p1.rotate(sina, cosa);
                cur_pos.sum(p1, _layout._graph.getPos(v));
                _layout._new_vertices[k] = comp.getVertexExtIdx(j);
            }

        cur_angle += _layout._bc_angles[comp_idx];
    }

    // respect cis/trans
    const int* molecule_edge_mapping = 0;
    const BaseMolecule* molecule = _layout._graph.getMolecule(&molecule_edge_mapping);
    const MoleculeLayoutGraph& drawn_comp = *_layout._bc_components[_layout._attached_bc[1]];
    MoleculeLayoutGraph& attach_comp = (MoleculeLayoutGraph&)*_layout._bc_components[_layout._attached_bc[0]];

    if (_n_components == 1 && molecule != 0 && drawn_comp.isSingleEdge())
    {
        int drawn_idx = drawn_comp.edgeBegin();
        int drawn_ext_idx = drawn_comp.getEdgeExtIdx(drawn_idx);
        int parity = molecule->cis_trans.getParity(_layout._graph.getEdgeExtIdx(drawn_ext_idx));

        if (parity != 0)
        {
            int substituents[4];
            ((BaseMolecule*)molecule)->getSubstituents_All(_layout._graph.getEdgeExtIdx(drawn_ext_idx), substituents);

            int drawn_substituent = -1;
            int drawn_substituent_idx = -1;
            int to_draw_substituent = -1;
            int to_draw_substituent_idx = -1;
            int drawn_end_idx = -1;
            const Vertex& vert = attach_comp.getVertex(_layout._src_vertex_map[0]);

            to_draw_substituent_idx = attach_comp.getVertexExtIdx(vert.neiVertex(vert.neiBegin()));

            drawn_end_idx = drawn_comp.getVertexExtIdx(drawn_comp.vertexBegin());

            if (drawn_end_idx == _layout._src_vertex)
                drawn_end_idx = drawn_comp.getVertexExtIdx(drawn_comp.vertexNext(drawn_comp.vertexBegin()));

            const Vertex& drawn_end = _layout._graph.getVertex(drawn_end_idx);
            drawn_substituent_idx = drawn_end.neiVertex(drawn_end.neiBegin());

            if (drawn_substituent_idx == _layout._src_vertex)
                drawn_substituent_idx = drawn_end.neiVertex(drawn_end.neiNext(drawn_end.neiBegin()));

            for (i = 0; i < 4; i++)
            {
                if (substituents[i] == _layout._graph.getVertexExtIdx(drawn_substituent_idx))
                    drawn_substituent = i;
                else if (substituents[i] == _layout._graph.getVertexExtIdx(to_draw_substituent_idx))
                    to_draw_substituent = i;
            }

            bool same_side = false;

            if ((parity == MoleculeCisTrans::CIS) == (abs(to_draw_substituent - drawn_substituent) == 2))
                same_side = true;

            int to_draw_layout_idx = _layout._new_vertices.find(to_draw_substituent_idx);

            int side_sign = MoleculeCisTrans::sameside(Vec3f(_layout._graph.getPos(drawn_end_idx)), Vec3f(_layout._graph.getPos(_layout._src_vertex)),
                                                       Vec3f(_layout._graph.getPos(drawn_substituent_idx)), Vec3f(_layout._layout[to_draw_layout_idx]));

            bool flip = false;

            if (same_side)
            {
                if (side_sign == -1)
                    flip = true;
            }
            else if (side_sign == 1)
                flip = true;

            // flip around double bond
            if (flip)
            {
                const Vec2f& v1 = _layout._graph.getPos(drawn_end_idx);
                const Vec2f& v2 = _layout._graph.getPos(_layout._src_vertex);
                Vec2f d;

                d.diff(v2, v1);

                float r = d.lengthSqr();

                // if (r < 0.000000001f)
                //   throw Error("too small edge");

                for (i = 0; i < _layout._layout.size(); i++)
                {
                    const Vec2f& vi = _layout._layout[i];

                    float t = ((vi.x - v1.x) * d.x + (vi.y - v1.y) * d.y) / r;
                    _layout._layout[i].set(2 * d.x * t + 2 * v1.x - vi.x, 2 * d.y * t + 2 * v1.y - vi.y);
                }

                // There's only one possible layout and the component is being flipped
                attach_comp.flipped();
            }
        }
    }
}
