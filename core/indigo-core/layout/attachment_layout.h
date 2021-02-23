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

#ifndef __attachment_layout_h__
#define __attachment_layout_h__

#include "graph/biconnected_decomposer.h"
#include "layout/molecule_layout_graph.h"

namespace indigo
{

    class AttachmentLayout
    {
    public:
        explicit AttachmentLayout(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components, const Array<int>& bc_tree,
                                  MoleculeLayoutGraph& graph, int src_vertex);

        float calculateEnergy();
        virtual void applyLayout() = 0;
        void markDrawnVertices();

    public:
        int _src_vertex;
        CP_DECL;
        TL_CP_DECL(Array<int>, _src_vertex_map); // _src_vertex_map[j] - index of the vertex _src_vertex in j component
        TL_CP_DECL(Array<int>, _attached_bc);    // BCnumbers[j] - index of j component attached;
                                                 // BCnumbers[size-1] - drawn
        TL_CP_DECL(Array<float>, _bc_angles);    // BCangles[j] - internal angle of j component attached, 0 if single edge
        TL_CP_DECL(Array<int>, _vertices_l);     // _vertices_l[j] - index of the vertex in j component such the j component
                                                 // lays on the left (CCW) from edge (v, _vertices_l[j]];
        float _alpha;                            // if positive then angle between components
        TL_CP_DECL(Array<int>, _new_vertices);   // indices in source graph of new verices
        TL_CP_DECL(Array<Vec2f>, _layout);       // layout of new vertices
        float _energy;                           // current energy between drawn part and new part

        const PtrArray<MoleculeLayoutGraph>& _bc_components;
        MoleculeLayoutGraph& _graph;
    };

    class AttachmentLayoutSimple : public AttachmentLayout
    {
    public:
        explicit AttachmentLayoutSimple(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components, const Array<int>& bc_tree,
                                        MoleculeLayoutGraph& graph, int src_vertex);

        void applyLayout();
    };

    class AttachmentLayoutSmart : public AttachmentLayout
    {
    public:
        explicit AttachmentLayoutSmart(const BiconnectedDecomposer& bc_decom, const PtrArray<MoleculeLayoutGraph>& bc_components, const Array<int>& bc_tree,
                                       MoleculeLayoutGraph& graph, int src_vertex);

        void applyLayout();
    };

    class LayoutChooser
    {
    public:
        LayoutChooser(AttachmentLayout& layout);

        void perform()
        {
            _perform(_layout._attached_bc.size() - 1);
        }

    private:
        void _perform(int level);
        void _makeLayout();

        int _n_components;
        float _cur_energy;
        CP_DECL;
        TL_CP_DECL(Array<int>, _comp_permutation);
        TL_CP_DECL(Array<int>, _rest_numbers);
        AttachmentLayout& _layout;
    };

} // namespace indigo

#endif
