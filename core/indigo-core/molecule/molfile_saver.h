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

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "molecule/base_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;

    class DLLEXPORT MolfileSaver
    {
    public:
        enum
        {
            MODE_AUTO = 0, // save to v3000 only if the given molecule has any
                           // v3000-specific features
            MODE_2000,     // force saving to v2000 format
            MODE_3000      // force saving to v3000 format
        };

        enum
        {
            CIP_DESC_NONE = 0,
            CIP_DESC_UNKNOWN,
            CIP_DESC_s,
            CIP_DESC_r,
            CIP_DESC_S,
            CIP_DESC_R,
            CIP_DESC_E,
            CIP_DESC_Z
        };

        struct CIPContext
        {
            BaseMolecule* mol;
            ArrayInt* cip_desc;
            ArrayInt* used1;
            ArrayInt* used2;
            bool next_level;
            bool isotope_check;
            bool use_stereo;
            bool use_rule_4;
            int ref_cip1;
            int ref_cip2;
            bool use_rule_5;

            inline void clear()
            {
                mol = 0;
                cip_desc = 0;
                used1 = 0;
                used2 = 0;
                ref_cip1 = 0;
                ref_cip2 = 0;
            }
        };

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

        DECL_ERROR;

    protected:
        void _saveMolecule(BaseMolecule& mol, bool query);

        void _writeHeader(BaseMolecule& mol, Output& output, bool zcoord);
        void _writeCtabHeader(Output& output);
        void _writeAtomLabel(Output& output, int label);
        void _writeMultiString(Output& output, const char* string, int len);
        void _writeCtab(Output& output, BaseMolecule& mol, bool query);
        void _writeOccurrenceRanges(Output& out, const ArrayInt& occurrences);
        void _writeRGroup(Output& output, BaseMolecule& mol, int rg_idx);
        void _writeTGroup(Output& output, BaseMolecule& mol, int tg_idx);
        void _writeCtabHeader2000(Output& output, BaseMolecule& mol);
        void _writeCtab2000(Output& output, BaseMolecule& mol, bool query);
        void _checkSGroupIndices(BaseMolecule& mol, ArrayInt& sgs);
        void _writeRGroupIndices2000(Output& output, BaseMolecule& mol);
        void _writeAttachmentValues2000(Output& output, BaseMolecule& fragment);
        void _writeGenericSGroup3000(SGroup& sgroup, int idx, Output& output);
        void _writeDataSGroupDisplay(DataSGroup& datasgroup, Output& out);
        void _writeFormattedString(Output& output, ArrayChar& str, int length);
        static bool _checkAttPointOrder(BaseMolecule& mol, int rsite);
        static bool _hasNeighborEitherBond(BaseMolecule& mol, int edge_idx);

        static int _getStereocenterParity(BaseMolecule& mol, int idx);

        bool _getRingBondCountFlagValue(QueryMolecule& qmol, int idx, int& value);
        bool _getSubstitutionCountFlagValue(QueryMolecule& qmol, int idx, int& value);

        void _updateCIPStereoDescriptors(BaseMolecule& mol);
        void _addCIPStereoDescriptors(BaseMolecule& mol);
        void _addCIPSgroups(BaseMolecule& mol, ArrayInt& attom_cip_desc, ArrayInt& bond_cip_desc);
        void _calcRSStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, ArrayInt& atom_cip_desc, ArrayInt& stereo_passed,
                                     bool use_stereo, Array<IntPair>& equiv_ligands, bool& digrap_cip_used);
        void _calcEZStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, ArrayInt& bond_cip_desc);
        bool _checkLigandsEquivalence(ArrayInt& ligands, Array<IntPair>& equiv_ligands, CIPContext& context);
        static int _getNumberOfStereoDescritors(ArrayInt& atom_cip_desc);
        bool _isPseudoAssymCenter(BaseMolecule& mol, int idx, ArrayInt& atom_cip_desc, ArrayInt& ligands, Array<IntPair>& equiv_ligands);

        int _calcCIPDigraphDescriptor(BaseMolecule& mol, int atom_idx, ArrayInt& ligands, Array<IntPair>& equiv_ligands);
        void _addNextLevel(Molecule& source, Molecule& target, int s_idx, int t_idx, ArrayInt& used, ArrayInt& mapping);
        void _calcStereocenters(Molecule& source, Molecule& mol, ArrayInt& mapping);

        static int _cip_rules_cmp(int& i1, int& i2, void* context);

        Output& _output;
        bool _v2000;

        CP_DECL;
        TL_CP_DECL(ArrayInt, _atom_mapping);
        TL_CP_DECL(ArrayInt, _bond_mapping);

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

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
