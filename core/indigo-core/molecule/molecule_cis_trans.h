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

#ifndef __molecule_cis_trans__
#define __molecule_cis_trans__

#include "base_cpp/red_black.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;
    class Filter;

    class DLLEXPORT MoleculeCisTrans
    {
    public:
        enum
        {
            CIS = 1,
            TRANS = 2
        };

        explicit MoleculeCisTrans();

        void clear();
        void build(BaseMolecule& baseMolecule, int* exclude_bonds);
        void buildFromSmiles(BaseMolecule& baseMolecule, int* dirs);

        bool exists() const;

        int count();

        void setParity(int bond_idx, int parity);
        int getParity(int bond_idx) const;
        bool isIgnored(int bond_idx) const;
        void ignore(int bond_idx);

        void registerBond(int idx);

        void flipBond(BaseMolecule& baseMolecule, int atom_parent, int atom_from, int atom_to);

        const int* getSubstituents(int bond_idx) const;
        void getSubstituents_All(BaseMolecule& baseMolecule, int bond_idx, int subst[4]);

        void add(int bond_idx, int substituents[4], int parity);
        bool registerBondAndSubstituents(BaseMolecule& baseMolecule, int idx);

        int applyMapping(int idx, const int* mapping, bool sort) const;
        static int applyMapping(int parity, const int* substituents, const int* mapping, bool sort);

        // Returns -2 if mapping is not valid
        static int getMappingParitySign(BaseMolecule& query, BaseMolecule& target, int bond_idx, const int* mapping);

        static bool checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping);

        void buildOnSubmolecule(BaseMolecule& baseMolecule, BaseMolecule& super, int* mapping);

        static bool sortSubstituents(BaseMolecule& mol, int* substituents, bool* parity_changed);

        void restoreSubstituents(BaseMolecule& baseMolecule, int bond_idx);
        void registerUnfoldedHydrogen(BaseMolecule& baseMolecule, int atom_idx, int added_hydrogen);

        static bool isAutomorphism(BaseMolecule& mol, const Array<int>& mapping, const Filter* edge_filter = NULL);

        bool isRingTransBond(BaseMolecule& baseMolecule, int bond_idx);

        bool convertableToImplicitHydrogen(BaseMolecule& baseMolecule, int idx);

        void validate(BaseMolecule& baseMolecule);

        DECL_ERROR;

        static bool isGeomStereoBond(BaseMolecule& mol, int bond_idx, int* substituents, bool have_xyz);
        static int sameside(const Vec3f& beg, const Vec3f& end, const Vec3f& nei_beg, const Vec3f& nei_end);
        static bool sameline(const Vec3f& beg, const Vec3f& end, const Vec3f& nei_beg);

        bool sameside(int edge_idx, int v1, int v2);

    protected:
        struct _Bond
        {
            void clear()
            {
                parity = 0;
                ignored = 0;
            }

            int parity;  // CIS ot TRANS
            int ignored; // explicitly ignored cis-trans configuration on this bond
            int substituents[4];
        };

        Array<_Bond> _bonds;

        static bool _pureH(BaseMolecule& mol, int idx);
        static int _sameside(BaseMolecule& mol, int i_beg, int i_end, int i_nei_beg, int i_nei_end);
        static bool _sameline(BaseMolecule& molecule, int i_beg, int i_end, int i_nei_beg);

        static int _getPairParity(int v1, int v2, const int* mapping, bool sort);
        static bool _commonHasLonePair(BaseMolecule& mol, int v1, int v2);

        static void _fillExplicitHydrogens(BaseMolecule& mol, int bond_idx, int subst[4]);
        static void _fillAtomExplicitHydrogens(BaseMolecule& mol, int atom_idx, int subst[2]);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
