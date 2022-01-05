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

#ifndef __molecule_allene_stereo__
#define __molecule_allene_stereo__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"
#include "math/algebra.h"

namespace indigo
{

    class BaseMolecule;

    class DLLEXPORT MoleculeAlleneStereo
    {
    public:
        explicit MoleculeAlleneStereo();

        void clear();

        void buildFromBonds(BaseMolecule& baseMolecule, bool ignore_errors, int* sensible_bonds_out);
        void markBonds(BaseMolecule& baseMolecule);
        static int sameside(const Vec3f& dir1, const Vec3f& dir2, const Vec3f& sep);
        void buildOnSubmolecule(BaseMolecule& baseMolecule, BaseMolecule& super, int* mapping);
        static bool checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping);

        static bool possibleCenter(BaseMolecule& mol, int idx, int& left, int& right, int subst[4], bool pure_h[4]);

        bool isCenter(int atom_idx);
        int size();
        int begin() const;
        int end() const;
        int next(int i) const;
        void get(int i, int& atom_idx, int& left, int& right, int subst[4], int& parity);
        void getByAtomIdx(int atom_idx, int& left, int& right, int subst[4], int& parity);
        void invert(int atom_idx);
        void reset(int atom_idx);

        void add(int atom_idx, int left, int right, int subst[4], int parity);

        void removeAtoms(BaseMolecule& baseMolecule, const Array<int>& indices);
        void removeBonds(BaseMolecule& baseMolecule, const Array<int>& indices);
        void registerUnfoldedHydrogen(int atom_idx, int added_hydrogen);

        DECL_ERROR;

    protected:
        struct _Atom
        {
            int left;  // number of the "left" neighbor atom
            int right; // number of the "right" neighbor atom

            // substituens: [0] and [1] are connected to the "left" neighbor,
            //              [2] and [3] are connected to the "right" neighbor.
            //              [1] and [3] may be -1 (implicit H)
            //              [0] and [2] are never -1
            int subst[4];

            // parity = 1  if [2]-nd substituent is rotated CCW w.r.t. [0]-th
            //             substituent when we look at it from "left" to "right"
            // parity = 2  if it is rotated CW
            int parity;
        };

        bool _isAlleneCenter(BaseMolecule& mol, int idx, _Atom& atom, int* sensible_bonds_out);

        RedBlackMap<int, _Atom> _centers;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
