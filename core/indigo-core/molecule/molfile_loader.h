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

#ifndef __molfile_loader__
#define __molfile_loader__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    class Scanner;
    class Molecule;
    class QueryMolecule;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

    class DLLEXPORT MolfileLoader
    {
    public:
        DECL_ERROR;

        MolfileLoader(Scanner& scanner);

        void loadMolecule(Molecule& mol);
        void loadQueryMolecule(QueryMolecule& mol);

        // for Rxnfiles v3000
        void loadCtab3000(Molecule& mol);
        void loadQueryCtab3000(QueryMolecule& mol);

        StereocentersOptions stereochemistry_options;
        bool treat_x_as_pseudoatom; // normally 'X' means 'any halogen'
        bool skip_3d_chirality;     // do not compute chirality from 3D coordinates
        bool ignore_no_chiral_flag; // ignore chiral flag absence (treat stereo "as drawn")
                                    // (depricated, use treat_stereo-as instead of this option)
        bool ignore_bad_valence;    // ignore bad valence (default value is false)

        // When true, the "bond topology", "stereo care", "ring bond count", and "unsaturation"
        // specifications are ignored when a non-query molecule is being loaded.
        // Otherwise, an error is thrown (this is the default).
        bool ignore_noncritical_query_features;

        int treat_stereo_as; // treat stereo as 'ucf', 'abs', 'rel', 'rac', 'any'
                             //  = 0 ('ucf') - use chiral flag (default value)
                             //  = ATOM_ABS ('abs')
                             //  = ATOM_OR  ('rel')
                             //  = ATOM_AND ('rac')
                             //  = ATOM_ANY ('any')

    protected:
        Scanner& _scanner;
        bool _rgfile;

        CP_DECL;
        TL_CP_DECL(Array<int>, _stereo_care_atoms);
        TL_CP_DECL(Array<int>, _stereo_care_bonds);
        TL_CP_DECL(Array<int>, _stereocenter_types);
        TL_CP_DECL(Array<int>, _stereocenter_groups);
        TL_CP_DECL(Array<int>, _sensible_bond_directions);
        TL_CP_DECL(Array<int>, _ignore_cistrans);

        enum
        {
            _SGROUP_TYPE_SUP = 1,
            _SGROUP_TYPE_DAT,
            _SGROUP_TYPE_SRU,
            _SGROUP_TYPE_MUL,
            _SGROUP_TYPE_GEN,
            _SGROUP_TYPE_OTHER // one of unsupported types
        };

        enum
        {
            _BRKTYP_SQUARE = 0,
            _BRKTYP_ROUND
        };

        TL_CP_DECL(Array<int>, _atom_types);
        TL_CP_DECL(Array<int>, _hcount);
        TL_CP_DECL(Array<int>, _sgroup_types);
        TL_CP_DECL(Array<int>, _sgroup_mapping);

        bool _v2000;
        int _atoms_num;
        int _bonds_num;
        bool _chiral;

        void _readHeader();
        void _readCtabHeader();
        void _readCtab2000();
        void _convertCharge(int value, int& charge, int& radical);
        void _read3dFeature2000();
        void _readRGroupOccurrenceRanges(const char* str, Array<int>& ranges);
        void _readRGroups2000();
        void _readCtab3000();
        void _readSGroup3000(const char* str);
        void _readRGroups3000();
        void _readTGroups3000();
        void _readSGroupDisplay(Scanner& scanner, DataSGroup& dsg);
        void _readCollectionBlock3000();
        void _readSGroupsBlock3000();
        void _preparePseudoAtomLabel(Array<char>& pseudo);
        void _readMultiString(Array<char>& str);
        void _readStringInQuotes(Scanner& scanner, Array<char>* str);
        void _init();
        void _appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom);

        void _fillSGroupsParentIndices();

        int _getElement(const char* buf);
        char* _strtrim(char* buf);

        static int _asc_cmp_cb(int& v1, int& v2, void* context);
        void _postLoad();

        void _loadMolecule();

        Molecule* _mol;
        BaseMolecule* _bmol;
        QueryMolecule* _qmol;

    private:
        MolfileLoader(const MolfileLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
