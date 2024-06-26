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

#ifndef __molecule_h__
#define __molecule_h__

#include "molecule/base_molecule.h"
#include "molecule/molecule_sgroups.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT Molecule : public BaseMolecule
    {
    public:
        Molecule();
        Molecule& asMolecule() override;

        void clear() override;

        BaseMolecule* neu() override;

        int addAtom(int label) override;
        int resetAtom(int idx, int label);

        void setPseudoAtom(int idx, const char* text);

        void renameTemplateAtom(int idx, const char* text);
        void setTemplateAtom(int idx, const char* text);
        void setTemplateAtomName(int idx, const char* text);
        void setTemplateAtomClass(int idx, const char* text);
        void setTemplateAtomSeqid(int idx, int seq_id);
        void setTemplateAtomSeqName(int idx, const char* seq_name);

        void setTemplateAtomDisplayOption(int idx, int contracted);
        void setTemplateAtomTemplateIndex(int idx, int temp_idx);

        int addBond(int beg, int end, int order) override;
        int addBond_Silent(int beg, int end, int order);

        void setAtomCharge(int idx, int charge);
        void setAtomCharge_Silent(int idx, int charge);
        void setAtomIsotope(int idx, int isotope);
        void setAtomRadical(int idx, int radical);
        void setValence(int idx, int valence);
        void setExplicitValence(int idx, int valence) override;
        void resetExplicitValence(int idx);
        bool isExplicitValenceSet(int idx);

        void setImplicitH(int idx, int impl_h) override;
        bool isImplicitHSet(int idx);

        // Set bond order method.
        // If keep_connectivity is false then connectivity to bond ends
        // will be recalculated. Connectivity should be kept only when bond order
        // is changed to/from aromatic.
        void setBondOrder(int idx, int order, bool keep_connectivity = false);

        void setBondOrder_Silent(int idx, int order);

        int getAtomNumber(int idx) override;
        int getAtomCharge(int idx) override;
        int getAtomIsotope(int idx) override;
        int getAtomRadical(int idx) override;
        int getBondOrder(int idx) const override;
        int getBondTopology(int idx) override;
        int getAtomAromaticity(int idx) override;
        int getExplicitValence(int idx) override;
        int getAtomValence(int idx) override;
        int getAtomSubstCount(int idx) override;
        int getAtomRingBondsCount(int idx) override;
        int getAtomConnectivity(int idx) override;

        int getAtomMaxH(int idx) override;
        int getAtomMinH(int idx) override;
        int getAtomTotalH(int idx) override;

        bool isPseudoAtom(int idx) override;
        const char* getPseudoAtom(int idx) override;

        bool isTemplateAtom(int idx) override;
        const char* getTemplateAtom(int idx) override;
        const int getTemplateAtomSeqid(int idx) override;
        const char* getTemplateAtomSeqName(int idx) override;
        const char* getTemplateAtomClass(int idx) override;
        const int getTemplateAtomTemplateIndex(int idx) override;
        const int getTemplateAtomDisplayOption(int idx) override;

        bool isRSite(int atom_idx) override;
        dword getRSiteBits(int atom_idx) override;
        void allowRGroupOnRSite(int atom_idx, int rg_idx) override;
        void setRSiteBits(int atom_idx, int bits);

        bool bondStereoCare(int idx) override;

        bool aromatize(const AromaticityOptions& options) override;
        bool dearomatize(const AromaticityOptions& options) override;

        int getImplicitH(int idx, bool impl_h_no_throw) override;
        int getImplicitH(int idx);
        int getImplicitH_NoThrow(int idx, int fallback);
        int calcImplicitHForConnectivity(int idx, int conn);

        int getAtomConnectivity_noImplH(int idx);
        int getAtomConnectivity_NoThrow(int idx, int fallback);
        int calcAtomConnectivity_noImplH(int idx);
        void calcAromaticAtomConnectivity(int idx, int& n_arom, int& min_conn);
        bool isSaturatedAtom(int idx) override;

        int totalHydrogensCount();

        bool atomNumberBelongs(int idx, const int* numbers, int count) override;
        bool possibleAtomNumber(int idx, int number) override;
        bool possibleAtomNumberAndCharge(int idx, int number, int charge) override;
        bool possibleAtomNumberAndIsotope(int idx, int number, int isotope) override;
        bool possibleAtomIsotope(int idx, int isotope) override;
        bool possibleAtomCharge(int idx, int charge) override;
        void getAtomDescription(int idx, Array<char>& description) override;
        void getBondDescription(int idx, Array<char>& description) override;
        bool possibleBondOrder(int idx, int order) override;

        int getVacantPiOrbitals(int atom_idx, int* lonepairs_out);
        int getVacantPiOrbitals(int atom_idx, int conn, int* lonepairs_out);

        static int matchAtomsCmp(Graph& g1, Graph& g2, int idx1, int idx2, void* userdata);

        static void saveBondOrders(Molecule& mol, Array<int>& orders);
        static void loadBondOrders(Molecule& mol, Array<int>& orders);

        void invalidateHCounters();

        static void checkForConsistency(Molecule& mol);

        static bool shouldWriteHCount(Molecule& mol, int idx);
        static bool shouldWriteHCountEx(Molecule& mol, int idx, int h_to_ignore);

        bool isAromatized();

        bool getIgnoreBadValenceFlag();
        void setIgnoreBadValenceFlag(bool flag);

        // Check
        bool isNitrogenV5(int atom_index);
        bool isNitrogenV5ForConnectivity(int atom_index, int conn);
        bool isPiBonded(int atom_index) const;

        void invalidateAtom(int index, int mask) override;

        bool restoreAromaticHydrogens(bool unambiguous_only = true);

        bool standardize(const StandardizeOptions& options);

        bool ionize(float ph, float ph_toll, const IonizeOptions& options);

        bool isPossibleFischerProjection(const char* options);

    protected:
        struct _Atom
        {
            int number;
            bool explicit_valence;
            bool explicit_impl_h;
            int isotope;
            int charge;
            int pseudoatom_value_idx; // if number == ELEM_PSEUDO, this is the corresponding
                                      // index from _pseudo_atom_values
            int rgroup_bits;          // if number == ELEM_RSITE, these are 32 bits, each allowing
                                      // an r-group with corresponding number to go for this atom.
                                      // Simple 'R' atoms have this field equal to zero.
            int template_occur_idx;   // if number == ELEM_TEMPLATE, this is the corresponding
                                      // index from _template_occurrences
        };

        Array<_Atom> _atoms;
        Array<int> _bond_orders;
        Array<int> _connectivity; // implicit H not included
        Array<int> _aromaticity;
        Array<int> _implicit_h;
        Array<int> _total_h;
        Array<int> _valence;
        Array<int> _radicals;

        StringPool _pseudo_atom_values;

        struct _AttachOrder
        {
            int ap_idx;
            Array<char> ap_id;
        };

        struct _TemplateOccurrence
        {
            int name_idx;              // index in _template_names
            int class_idx;             // index in _template_classes
            int seq_id;                // sequence id
            int template_idx;          // template idx
            Array<char> seq_name;      // sequence name
            DisplayOption contracted;  // display option (-1 if undefined, 0 - expanded, 1 - contracted)
            Array<_AttachOrder> order; // attach order info
        };
        ObjPool<_TemplateOccurrence> _template_occurrences;

        StringPool _template_classes;
        StringPool _template_names;

        bool _aromatized;

        bool _ignore_bad_valence;

        void _mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags) override;

        void _flipBond(int atom_parent, int atom_from, int atom_to) override;
        void _removeAtoms(const Array<int>& indices, const int* mapping) override;

        // If 'validate' is true then vertex connectivity and implicit hydrogens
        // are calculates and stored. If 'validate' is false then connectivity
        // information is cleared.
        void _validateVertexConnectivity(int idx, bool validate);

        void _invalidateVertexCache(int idx);

    private:
        int _getImplicitHForConnectivity(int idx, int conn, bool use_cache);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
