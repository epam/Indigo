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

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <array>
#include <string>
#include <unordered_map>

#include "base_c/defs.h"
#include "base_cpp/array.h"

namespace indigo
{
    class BaseMolecule;
    class Molecule;

    enum class CIPDesc : int
    {
        NONE = 0,
        UNKNOWN,
        s,
        r,
        S,
        R,
        E,
        Z
    };

    struct CIPContext
    {
        BaseMolecule* mol;
        Array<CIPDesc>* cip_desc;
        Array<int>* used1;
        Array<int>* used2;
        bool next_level;
        bool isotope_check;
        bool use_stereo;
        bool use_rule_4;
        CIPDesc ref_cip1;
        CIPDesc ref_cip2;
        bool use_rule_5;

        inline void clear()
        {
            mol = nullptr;
            cip_desc = nullptr;
            used1 = nullptr;
            used2 = nullptr;
            ref_cip1 = CIPDesc::NONE;
            ref_cip2 = CIPDesc::NONE;
        }
    };

    using EquivLigand = std::array<int, 2>;

    class DLLEXPORT MoleculeCIPCalculator
    {
    public:
        bool addCIPStereoDescriptors(BaseMolecule& mol);
        void addCIPSgroups(BaseMolecule& mol);
        void removeCIPSgroups(BaseMolecule& mol);
        void convertSGroupsToCIP(BaseMolecule& mol);

        const std::unordered_map<std::string, CIPDesc> KSGroupToCIP = {{"(R)", CIPDesc::R}, {"(S)", CIPDesc::S}, {"(r)", CIPDesc::r},
                                                                       {"(s)", CIPDesc::s}, {"(E)", CIPDesc::E}, {"(Z)", CIPDesc::Z}};

    private:
        void _calcRSStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, Array<CIPDesc>& atom_cip_desc, Array<int>& stereo_passed,
                                     bool use_stereo, Array<EquivLigand>& equiv_ligands, bool& digraph_cip_used);
        void _calcEZStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, Array<CIPDesc>& bond_cip_desc);
        bool _checkLigandsEquivalence(Array<int>& ligands, Array<EquivLigand>& equiv_ligands, CIPContext& context);
        static int _getNumberOfStereoDescritors(const Array<CIPDesc>& atom_cip_desc);
        bool _isPseudoAssymCenter(BaseMolecule& mol, int idx, Array<CIPDesc>& atom_cip_desc, Array<int>& ligands, Array<EquivLigand>& equiv_ligands);

        CIPDesc _calcCIPDigraphDescriptor(BaseMolecule& mol, int atom_idx, Array<int>& ligands, Array<EquivLigand>& equiv_ligands);
        void _addNextLevel(Molecule& source, Molecule& target, int s_idx, int t_idx, Array<int>& used, Array<int>& mapping);
        void _calcStereocenters(Molecule& source, Molecule& mol, Array<int>& mapping);

        static int _cip_rules_cmp(int i1, int i2, void* cur_context);

        static void cipSort(Array<int>& ligands, CIPContext* context);
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
