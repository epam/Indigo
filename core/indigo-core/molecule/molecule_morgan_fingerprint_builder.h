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

#ifndef PROJECT_MOLECULE_MORGAN_FINGERPRINT_H
#define PROJECT_MOLECULE_MORGAN_FINGERPRINT_H

#include <set>
#include <vector>

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_molecule.h"

namespace indigo
{

    class DLLEXPORT MoleculeMorganFingerprintBuilder : public NonCopyable
    {
    public:
        explicit MoleculeMorganFingerprintBuilder(BaseMolecule& mol);

        void calculateDescriptorsECFP(int fp_depth, Array<dword>& res);
        void calculateDescriptorsFCFP(int fp_depth, Array<dword>& res);

        void packFingerprintECFP(int fp_depth, Array<byte>& res);
        void packFingerprintFCFP(int fp_depth, Array<byte>& res);

    private:
        enum
        {
            MAGIC_HASH_NUMBER = 37
        };

        static void setBits(dword hash, byte* fp, int size);

        typedef dword (*InitialStateCallback)(BaseMolecule& mol, int idx);

        void initDescriptors(MoleculeMorganFingerprintBuilder::InitialStateCallback initialStateCallback);
        void buildDescriptors(int fp_depth);
        void calculateNewAtomDescriptors(int iterationNumber);

        /**
         * ECFP: (hash of 7 ints)
         *  - number of  non-hydrogen neighbors;
         *  - valence minus the number of hydrogens;
         *  - the atomic number;
         *  - the atomic mass;
         *  - the atomic charge;
         *  - the number of attached hydrogens (both implicit and explicit)
         *  - whether the atom is contained in at least one ring
         *  */
        static dword initialStateCallback_ECFP(BaseMolecule& mol, int idx);

        /**
         * FCFP: (6 bits)
         *  - hydrogen-bond acceptor
         *  - hydrogen-bond donor
         *  - negatively ionizable
         *  - positively ionizable
         *  - aromatic
         *  - halogen
         *  */
        static dword initialStateCallback_FCFP(BaseMolecule& mol, int idx);

        struct BondDescriptor
        {
            int bond_type;
            int vertex_idx;
            int edge_idx;
        };

        struct FeatureDescriptor
        {
            uint32_t hash;
            std::set<int> bond_set;

            bool operator==(const FeatureDescriptor& rhs) const;

            bool operator<(const FeatureDescriptor& rhs) const;
        };

        struct AtomDescriptor
        {
            FeatureDescriptor descr;
            FeatureDescriptor new_descr;
            std::vector<BondDescriptor> bond_descriptors;
        };

        int bondDescriptorCmp(const BondDescriptor& bd1, const BondDescriptor& bd2);

        BaseMolecule& mol;
        std::vector<FeatureDescriptor> features;
        std::vector<AtomDescriptor> atom_descriptors;
    };

}; // namespace indigo

#endif // PROJECT_MOLECULE_MORGAN_FINGERPRINT_H
