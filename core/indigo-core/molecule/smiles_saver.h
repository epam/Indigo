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

#ifndef __smiles_saver__
#define __smiles_saver__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/list.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"
#include "molecule/query_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;
    class BaseMolecule;
    class QueryMolecule;
    class Molecule;

    class DLLEXPORT SmilesSaver
    {
    public:
        DECL_ERROR;
        enum class SMILES_MODE
        {
            SMILES_CHEMAXON = 0,
            SMILES_DAYLIGHT
        };

        SmilesSaver(Output& output);
        ~SmilesSaver();

        void saveMolecule(Molecule& mol);
        void saveQueryMolecule(QueryMolecule& mol);

        int* vertex_ranks;

        bool ignore_hydrogens;
        bool canonize_chiralities;
        bool write_extra_info;
        bool chemaxon;
        bool separate_rsites;
        bool rsite_indices_as_aam;

        int writtenComponents();
        const Array<int>& writtenAtoms();
        const Array<int>& writtenBonds();

        static void writePseudoAtom(const char* label, Output& out);
        void writeSpecialAtom(int aid, Output& out);
        static SMILES_MODE parseFormatMode(const std::string& format);
        static void saveFormatMode(SMILES_MODE mode, std::string& output);

        bool inside_rsmiles;

        bool smarts_mode;
        bool ignore_invalid_hcount;

        const Array<int>& getSavedCisTransParities();

    protected:
        void _saveMolecule();

        BaseMolecule* _bmol;
        Molecule* _mol;
        QueryMolecule* _qmol;

        struct _Atom
        {
            _Atom(Pool<List<int>::Elem>& neipool);
            ~_Atom();

            List<int> neighbors;
            int parent;

            bool aromatic;
            bool lowercase;
            int chirality;  // 0 means no chirality, 1 means CCW pyramid, 2 means CW pyramid
            int branch_cnt; // runs from 0 to (branches - 1)
            bool paren_written;
            bool starts_polymer;
            bool ends_polymer;
        };

        Output& _output;

        void _writeCycleNumber(int n) const;
        void _writeAtom(int idx, bool aromatic, bool lowercase, int chirality) const;
        void _writeChirality(int chirality) const;
        void _writeCharge(int charge) const;
        void _writeSmartsAtom(int idx, QueryMolecule::Atom* atom, int chirality, int depth, bool has_or_parent, bool has_not_parent) const;
        void _writeSmartsBond(int idx, QueryMolecule::Bond* bond, bool has_or_parent) const;
        void _markCisTrans();
        void _banSlashes();
        int _calcBondDirection(int idx, int vprev);
        bool _updateSideBonds(int bond_idx);
        void _writeRingCisTrans();
        void _writeStereogroups();
        void _writeRadicals();
        void _writePseudoAtoms();
        void _writeHighlighting();
        void _writeRGroups();
        void _writeSGroups();
        void _writeSGroupAtoms(const SGroup& sgroup);

        void _writeRingBonds();
        void _writeUnsaturated();
        void _writeSubstitutionCounts();
        void _writeWedges();
        void _writeBondDirs(const std::string& tag, const std::vector<std::pair<int, int>>& bonds);

        bool _shouldWriteAromaticBond(int bond_idx);
        void _startExtension();

        void _filterCisTransParity();

        int _countRBonds();

        void _checkSRU();
        void _checkRGroupsAndAttachmentPoints();

        void _writeOccurrenceRanges(Output& out, const Array<int>& occurrences);

        struct _DBond // directed bond (near cis-trans bond)
        {
            int ctbond_beg; // cis-trans bond attached to the beginning (-1 if there isn't any)
            int ctbond_end; // cis-trans bond attached to the end (-1 if there isn't any)
            int saved;      // 0 -- not saved; 1 -- goes 'up' from begin to end; 2 -- goes 'down'
        };

        CP_DECL;
        TL_CP_DECL(Pool<List<int>::Elem>, _neipool);
        TL_CP_DECL(ObjArray<_Atom>, _atoms);
        TL_CP_DECL(Array<int>, _hcount);
        TL_CP_DECL(Array<int>, _hcount_ignored);
        TL_CP_DECL(Array<_DBond>, _dbonds);
        TL_CP_DECL(Array<int>, _written_atoms);
        TL_CP_DECL(Array<int>, _written_atoms_inv);
        TL_CP_DECL(Array<int>, _written_bonds);
        TL_CP_DECL(Array<int>, _polymer_indices);
        TL_CP_DECL(Array<int>, _attachment_indices);
        TL_CP_DECL(Array<int>, _attachment_cycle_numbers);
        TL_CP_DECL(Array<int>, _aromatic_bonds);
        TL_CP_DECL(Array<int>, _ignored_vertices);

        // Some cis-trans bonds are considered "complicated", they are either:
        // 1.   Ring bonds, which can not be saved with slash notation (conflicts are
        //      unavoidable)
        // 2.   Bonds that share their adjacent single with other cis-trans bonds that
        //      have "unset" parity, for example: OC=CC=CC=CN |t:1,5|
        //      for another example: C/C=C/C(/C=C/C)=C(/C=C/C)/C=C/C
        //      Marvin erroneously saves that structure as N\C=C\C=C\C=C\O, which is
        //      incorrect, as it introduces cis-trans parity on the middle bond
        // 1+2: C[N+](O)=C1C=CC(=CO)C=C1
        TL_CP_DECL(Array<int>, _complicated_cistrans);
        // single bonds that can not be written as slashes; see item 2 above
        TL_CP_DECL(Array<int>, _ban_slashes);
        // array with cis-trans parity marks
        // 0 means ignored
        TL_CP_DECL(Array<int>, _cis_trans_parity);

        // This flag does not necessarily mean "any of _complicated_cistrans == 1".
        // If all _complicated_cistrans are actually ring CIS bonds, then the flag
        // is not set.
        bool _have_complicated_cistrans;

        int _n_attachment_points;

        int _written_components;
        int _touched_cistransbonds;
        bool _comma;

    private:
        SmilesSaver(const SmilesSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
