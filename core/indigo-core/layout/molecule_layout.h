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

#ifndef __molecule_layout_h__
#define __molecule_layout_h__

#include "base_cpp/cancellation_handler.h"
#include "layout/metalayout.h"
#include "layout/molecule_layout_graph.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT MoleculeLayout
    {
    public:
        enum
        {
            LAYOUT_MAX_ITERATION = 20
        };
        static constexpr float DEFAULT_BOND_LENGTH = 1.6f;

        explicit MoleculeLayout(BaseMolecule& molecule, bool smart_layout = false);

        void make();

        void updateSGroups();

        void setCancellationHandler(CancellationHandler* cancellation);

        float bond_length;
        bool respect_existing_layout;
        Filter* filter;
        int max_iterations;
        bool _smart_layout;
        layout_orientation_value layout_orientation;

        DECL_ERROR;

    protected:
        Metalayout::LayoutItem& _pushMol(Metalayout::LayoutLine& line, BaseMolecule& mol);
        BaseMolecule& _getMol(int id);
        void _make();
        void _makeLayout();
        void _updateRepeatingUnits();
        void _updateMultipleGroups();

        static BaseMolecule& cb_getMol(int id, void* context);
        static void cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context);

        void _updateDataSGroups();

        void _init(bool smart_layout);

        Metalayout _ml;
        BaseMolecule& _molecule;
        std::unique_ptr<BaseMolecule> _molCollapsed;
        BaseMolecule* _bm;
        Array<int> _atomMapping;
        std::unique_ptr<MoleculeLayoutGraph> _layout_graph;
        Array<BaseMolecule*> _map;
        bool _query;
        bool _hasMulGroups;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
