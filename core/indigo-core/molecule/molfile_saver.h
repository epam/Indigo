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

#ifndef __molfile_saver__
#define __molfile_saver__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "molecule/base_molecule.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;

    class DLLEXPORT MolfileSaver
    {
    public:
        class MonomersToSgroupFilter : public BaseMolecule::MonomerFilterBase
        {
        public:
            MonomersToSgroupFilter(BaseMolecule& mol, const std::vector<std::map<int, int>>& directions_map) : MonomerFilterBase(mol, directions_map)
            {
            }
            bool operator()(int atom_idx) const override;
        };
        enum
        {
            MODE_AUTO = 0, // save to v3000 only if the given molecule has any
                           // v3000-specific features
            MODE_2000,     // force saving to v2000 format
            MODE_3000      // force saving to v3000 format
        };
        constexpr static int MAX_RING_BOND_COUNT = 4;
        constexpr static int MAX_SUBSTITUTION_COUNT = 6;

        MolfileSaver(Output& output);

        void saveBaseMolecule(BaseMolecule& mol);
        void saveMolecule(Molecule& mol);
        void saveQueryMolecule(QueryMolecule& mol);

        void saveCtab3000(Molecule& mol);
        void saveQueryCtab3000(QueryMolecule& mol);

        int mode;             // one of MODE_***, MODE_AUTO by default
        bool no_chiral;       // skip the 'chiral' flag, not regarding of the actual stereochemistry (depricated)
        int chiral_flag;      // set chiral to pre-defined value, not regarding of the actual stereochemistry
                              // (the default value = -1, use actual stereochemistry)
        bool skip_date;       // If true then zero date is written
        bool add_stereo_desc; // If true then stereo descriptors will be added as DAT S-groups
        bool add_implicit_h;  // If true then MRV_IMPLICIT_H Data S-groups will be added for saving
                              // the number of implicit H for aromatic atoms
                              // (if it is required for correct de-aromatization) (default value is true)
        bool add_mrv_sma;     // If true then "MRV SMA" extension will be added for query molecules (default value is true)
        static int parseFormatMode(const char* mode);
        static void saveFormatMode(int mode, Array<char>& output);

        DECL_ERROR;

    protected:
        friend class MoleculeCIPCalculator;

        void _saveMolecule(BaseMolecule& bmol, bool query);
        void _validate(BaseMolecule& bmol);
        void _handleCIP(BaseMolecule& mol);
        void _handleMonomers(BaseMolecule& mol);
        void _calculateSEQIDs(BaseMolecule& mol, const std::vector<std::map<int, int>>& directions_map, std::vector<std::deque<int>>& sequences);

        void _writeHeader(BaseMolecule& mol, Output& output, bool zcoord);
        void _writeCtabHeader(Output& output);
        void _writeAtomLabel(Output& output, int label);
        void _writeMultiString(Output& output, const char* string, int len);
        void _writeCtab(Output& output, BaseMolecule& mol, bool query);
        void _writeRGroup(Output& output, BaseMolecule& mol, int rg_idx);
        void _writeTGroup(Output& output, BaseMolecule& mol, int tg_idx);
        void _writeCtabHeader2000(Output& output, BaseMolecule& mol);
        void _writeCtab2000(Output& output, BaseMolecule& mol, bool query);
        void _checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs);
        void _writeRGroupIndices2000(Output& output, BaseMolecule& mol);
        void _writeAttachmentValues2000(Output& output, BaseMolecule& fragment);
        void _writeGenericSGroup3000(SGroup& sgroup, int idx, Output& output);
        void _writeDataSGroupDisplay(DataSGroup& datasgroup, Output& out);
        void _writeFormattedString(Output& output, Array<char>& str, int length);
        static bool _checkAttPointOrder(BaseMolecule& mol, int rsite);
        static bool _hasNeighborEitherBond(BaseMolecule& mol, int edge_idx);

        static int _getStereocenterParity(BaseMolecule& mol, int idx);

        Output& _output;
        bool _v2000;

        CP_DECL;
        TL_CP_DECL(Array<int>, _atom_mapping);
        TL_CP_DECL(Array<int>, _bond_mapping);

        enum
        {
            _SGROUP_TYPE_SUP = 1,
            _SGROUP_TYPE_DAT,
            _SGROUP_TYPE_SRU,
            _SGROUP_TYPE_MUL,
            _SGROUP_TYPE_GEN
        };

    private:
        MolfileSaver(const MolfileSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
