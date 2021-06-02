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
        virtual ~Molecule();

        virtual Molecule& asMolecule();

        virtual void clear();

        virtual BaseMolecule* neu();

        int addAtom(int label);
        int resetAtom(int idx, int label);

        void setPseudoAtom(int idx, const char* text);

        void setTemplateAtom(int idx, const char* text);
        void setTemplateAtomName(int idx, const char* text);
        void setTemplateAtomClass(int idx, const char* text);
        void setTemplateAtomSeqid(int idx, int seq_id);
        void setTemplateAtomDisplayOption(int idx, int contracted);

        int addBond(int beg, int end, int order);
        int addBond_Silent(int beg, int end, int order);

        void setAtomCharge(int idx, int charge);
        void setAtomCharge_Silent(int idx, int charge);
        void setAtomIsotope(int idx, int isotope);
        void setAtomRadical(int idx, int radical);
        void setValence(int idx, int valence);
        void setExplicitValence(int idx, int valence);
        void resetExplicitValence(int idx);
        bool isExplicitValenceSet(int idx);

        void setImplicitH(int idx, int impl_h);
        bool isImplicitHSet(int idx);

        // Set bond order method.
        // If keep_connectivity is false then connectivity to bond ends
        // will be recalculated. Connectivity should be kept only when bond order
        // is changed to/from aromatic.
        void setBondOrder(int idx, int order, bool keep_connectivity = false);

        void setBondOrder_Silent(int idx, int order);

        virtual int getAtomNumber(int idx);
        virtual int getAtomCharge(int idx);
        virtual int getAtomIsotope(int idx);
        virtual int getAtomRadical(int idx);
        virtual int getBondOrder(int idx);
        virtual int getBondTopology(int idx);
        virtual int getAtomAromaticity(int idx);
        virtual int getExplicitValence(int idx);
        virtual int getAtomValence(int idx);
        virtual int getAtomSubstCount(int idx);
        virtual int getAtomRingBondsCount(int idx);
        virtual int getAtomConnectivity(int idx);

        virtual int getAtomMaxH(int idx);
        virtual int getAtomMinH(int idx);
        virtual int getAtomTotalH(int idx);

        virtual bool isPseudoAtom(int idx);
        virtual const char* getPseudoAtom(int idx);

        virtual bool isTemplateAtom(int idx);
        virtual const char* getTemplateAtom(int idx);
        virtual const int getTemplateAtomSeqid(int idx);
        virtual const char* getTemplateAtomClass(int idx);
        virtual const int getTemplateAtomDisplayOption(int idx);

        virtual bool isRSite(int atom_idx);
        virtual dword getRSiteBits(int atom_idx);
        virtual void allowRGroupOnRSite(int atom_idx, int rg_idx);
        void setRSiteBits(int atom_idx, int bits);

        virtual bool bondStereoCare(int idx);

        virtual bool aromatize(const AromaticityOptions& options);
        virtual bool dearomatize(const AromaticityOptions& options);

        int getImplicitH(int idx);
        int getImplicitH_NoThrow(int idx, int fallback);
        int calcImplicitHForConnectivity(int idx, int conn);

        int getAtomConnectivity_noImplH(int idx);
        int getAtomConnectivity_NoThrow(int idx, int fallback);
        int calcAtomConnectivity_noImplH(int idx);
        void calcAromaticAtomConnectivity(int idx, int& n_arom, int& min_conn);
        bool isSaturatedAtom(int idx);

        int totalHydrogensCount();

        virtual bool atomNumberBelongs(int idx, const int* numbers, int count);
        virtual bool possibleAtomNumber(int idx, int number);
        virtual bool possibleAtomNumberAndCharge(int idx, int number, int charge);
        virtual bool possibleAtomNumberAndIsotope(int idx, int number, int isotope);
        virtual bool possibleAtomIsotope(int idx, int isotope);
        virtual bool possibleAtomCharge(int idx, int charge);
        virtual void getAtomDescription(int idx, ArrayChar& description);
        virtual void getBondDescription(int idx, ArrayChar& description);
        virtual bool possibleBondOrder(int idx, int order);

        int getVacantPiOrbitals(int atom_idx, int* lonepairs_out);
        int getVacantPiOrbitals(int atom_idx, int conn, int* lonepairs_out);

        static int matchAtomsCmp(Graph& g1, Graph& g2, int idx1, int idx2, void* userdata);

        void unfoldHydrogens(Array<int>* markers_out, int max_h_cnt = -1, bool impl_h_no_throw = false);

        static void saveBondOrders(Molecule& mol, Array<int>& orders);
        static void loadBondOrders(Molecule& mol, Array<int>& orders);

        bool convertableToImplicitHydrogen(int idx);

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

        virtual void invalidateAtom(int index, int mask);

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
            ArrayChar ap_id;
        };

        struct _TemplateOccurrence
        {
            int name_idx;   // index in _template_names
            int class_idx;  // index in _template_classes
            int seq_id;     // sequence id
            int contracted; // display option (-1 if undefined, 0 - expanded, 1 - contracted)

            Array<_AttachOrder> order; // attach order info
        };
        ObjPool<_TemplateOccurrence> _template_occurrences;

        StringPool _template_classes;
        StringPool _template_names;

        bool _aromatized;

        bool _ignore_bad_valence;

        virtual void _mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags);

        virtual void _flipBond(int atom_parent, int atom_from, int atom_to);
        virtual void _removeAtoms(const Array<int>& indices, const int* mapping);

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
