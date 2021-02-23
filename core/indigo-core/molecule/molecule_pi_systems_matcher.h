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

#ifndef __molecule_pi_systems_matcher__
#define __molecule_pi_systems_matcher__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_decomposer.h"
#include "molecule/molecule.h"
#include "molecule/molecule_electrons_localizer.h"
#include "molecule/query_molecule.h"

namespace indigo
{
    class Molecule;

    class MoleculePiSystemsMatcher
    {
    public:
        MoleculePiSystemsMatcher(Molecule& target);

        bool isAtomInPiSystem(int atom);
        bool isBondInPiSystem(int bond);

        bool checkEmbedding(QueryMolecule& query, const int* mapping);

        void copyLocalization(Molecule& target);

        DECL_ERROR;

    private:
        // Returns number of pi-systems
        int _initMarks(void);

        void _markAtomsFirst();
        void _markUnstablePiSystems(Array<bool>& pi_system_used);

        void _markVerticesInPiSystemsWithCycles();
        void _markVerticesInUnusedPiSystems(Array<bool>& pi_system_used);
        void _markVerticesInSingleAtomPiSystem(int n_pi_systems);

        void _calculatePiSystemsSizes(int n_pi_systems, Array<int>& sizes);

        void _copyPiSystemsIdFromDecomposer();

        void _extractPiSystem(int pi_system_index);
        void _findPiSystemLocalization(int pool_id);

        bool _fixAtoms(QueryMolecule& query, const int* mapping);
        bool _fixBonds(QueryMolecule& query, const int* mapping);

        bool _findMatching();
        bool _findMatchingForPiSystem(int pool_id);

        void _markMappedPiSystems(QueryMolecule& query, const int* mapping);

        bool _canAtomBeInPiSystem(int v);

        void _calcConnectivity(Molecule& mol, Array<int>& conn);

        enum
        {
            _NOT_IN_PI_SYSTEM = -3,
            _UNKNOWN = -2,
            _IN_AROMATIC = -1
        };

        Molecule& _target;
        Obj<GraphDecomposer> _decomposer;

        CP_DECL;
        TL_CP_DECL(Array<int>, _atom_pi_system_idx);

        struct _Pi_System
        {
            Molecule pi_system;
            Array<int> inv_mapping, mapping;
            Obj<MoleculeElectronsLocalizer> localizer;

            struct Localizations
            {
                int double_bonds, primary_lp, seconary_lp;
            };
            Array<Localizations> localizations;

            bool pi_system_mapped;
            bool initialized;

            void clear();
        };
        TL_CP_DECL(ReusableObjArray<_Pi_System>, _pi_systems);
        TL_CP_DECL(Array<int>, _connectivity);
    };

} // namespace indigo

#endif // __molecule_pi_systems_matcher__
