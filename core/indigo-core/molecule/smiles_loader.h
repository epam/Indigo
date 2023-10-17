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

#ifndef __smiles_loader__
#define __smiles_loader__

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;
    class BaseMolecule;
    class Molecule;
    class QueryMolecule;

    class DLLEXPORT SmilesLoader
    {
    public:
        DECL_ERROR;

        SmilesLoader(Scanner& scanner);
        ~SmilesLoader();

        void loadMolecule(Molecule& mol);
        void loadQueryMolecule(QueryMolecule& mol);

        void loadSMARTS(QueryMolecule& mol);

        Array<int>* ignorable_aam;

        bool inside_rsmiles;

        bool smarts_mode;

        // set to true to accept buggy SMILES like 'N/C=C\1CCCN[C@H]\1S'
        // (see http://groups.google.com/group/indigo-bugs/browse_thread/thread/de7da07a3a5cb3ee
        //  for details)
        bool ignore_closing_bond_direction_mismatch;

        StereocentersOptions stereochemistry_options;
        bool ignore_cistrans_errors;
        bool ignore_bad_valence;
        bool ignore_no_chiral_flag{false};

        static void readSmartsAtomStr(const std::string& atom_str, std::unique_ptr<QueryMolecule::Atom>& qatom);
        static void readSmartsBondStr(const std::string& bond_str, std::unique_ptr<QueryMolecule::Bond>& qbond);

    protected:
        enum
        {
            _ANY_BOND = -2
        };

        enum
        {
            _POLYMER_START = 1,
            _POLYMER_END = 2
        };

        class DLLEXPORT _AtomDesc
        {
        public:
            _AtomDesc(Pool<List<int>::Elem>& neipool);
            ~_AtomDesc();

            void pending(int cycle);
            void closure(int cycle, int end);

            List<int> neighbors;
            int parent;

            int label;
            int isotope;
            int charge;
            int hydrogens;
            int chirality;
            int aromatic;
            int aam;
            bool ignorable_aam;
            bool brackets;

            bool star_atom;

            bool starts_polymer;
            bool ends_polymer;
            int polymer_index;
            int rsite_num;
        };

        struct _BondDesc
        {
            _BondDesc();
            int beg;
            int end;
            int type;
            int dir; // 0 -- undirected; 1 -- goes 'up' from beg to end, 2 -- goes 'down'
            int topology;
            int index;
        };

        struct _CycleDesc
        {
            void clear()
            {
                beg = -1;
                pending_bond = -1;
                pending_bond_str = -1;
            }

            int beg;
            int pending_bond;
            int pending_bond_str; // index in pending_bonds_pool;
        };

        Scanner& _scanner;

        //        CP_DECL;
        //        TL_CP_DECL(Array<int>, _atom_stack);
        //        TL_CP_DECL(Array<_CycleDesc>, _cycles);
        //        TL_CP_DECL(StringPool, _pending_bonds_pool);
        //        TL_CP_DECL(Pool<List<int>::Elem>, _neipool);
        //        TL_CP_DECL(ObjArray<_AtomDesc>, _atoms);
        //        TL_CP_DECL(Array<_BondDesc>, _bonds);
        //        TL_CP_DECL(Array<int>, _polymer_repetitions);

        Array<int> _atom_stack;
        Array<_CycleDesc> _cycles;
        StringPool _pending_bonds_pool;
        Pool<List<int>::Elem> _neipool;
        ObjArray<_AtomDesc> _atoms;
        Array<_BondDesc> _bonds;
        Array<int> _polymer_repetitions;

        int _balance;
        int _current_compno;
        bool _inside_smarts_component;
        bool _has_atom_coordinates = false;
        bool _has_directions_on_rings = false;

        BaseMolecule* _bmol;
        QueryMolecule* _qmol;
        Molecule* _mol;

        void _loadMolecule();
        void _parseMolecule();
        void _loadParsedMolecule();
        void _validateStereoCenters();

        void _calcStereocenters();
        void _calcCisTrans();
        void _readOtherStuff();
        void _markAromaticBonds();
        void _setRadicalsAndHCounts();
        void _forbidHydrogens();
        void _addExplicitHForStereo();
        void _addLigandsForStereo();
        bool _isAlleneLike(int i);
        void _handleCurlyBrace(_AtomDesc& atom, bool& inside_polymer);
        void _handlePolymerRepetition(int i);

        static void _readAtom(Array<char>& atom_str, bool first_in_brackets, _AtomDesc& atom, std::unique_ptr<QueryMolecule::Atom>& qatom,
                              bool smarts_mode = false, bool inside_rsmiles = false);

        static bool _readAtomLogic(Array<char>& atom_str, bool first_in_brackets, _AtomDesc& atom, std::unique_ptr<QueryMolecule::Atom>& qatom,
                                   bool smarts_mode = false, bool inside_rsmiles = false);

        int _parseCurly(Array<char>& curly, int& repetitions);

        static void _readBond(Array<char>& bond_str, _BondDesc& bond, std::unique_ptr<QueryMolecule::Bond>& qbond, bool smarts_mode);
        static void _readBondSub(Array<char>& bond_str, _BondDesc& bond, std::unique_ptr<QueryMolecule::Bond>& qbond, bool smarts_mode);
        void _readRGroupOccurrenceRanges(const char* str, Array<int>& ranges);

    private:
        SmilesLoader(const SmilesLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
