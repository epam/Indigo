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

#ifndef __query_molecule_h__
#define __query_molecule_h__

#include "base_cpp/ptr_array.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_arom.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    enum
    {
        SKIP_3D_CONSTRAINTS = 0x0100,
        SKIP_FIXED_ATOMS = 0x0200,
        SKIP_RGROUPS = 0x0400,
        SKIP_AROMATICITY = 0x0800,
        SKIP_COMPONENTS = 0x1000
    };

    enum
    {
        _ATOM_R,
        _ATOM_A,
        _ATOM_X,
        _ATOM_Q,
        _ATOM_M,
        _ATOM_AH,
        _ATOM_XH,
        _ATOM_QH,
        _ATOM_MH,
        _ATOM_LIST,
        _ATOM_NOTLIST,
        _ATOM_PSEUDO,
        _ATOM_TEMPLATE,
        _ATOM_ELEMENT
    };

    class Output;

    class DLLEXPORT QueryMolecule : public BaseMolecule
    {
    public:
        enum OpType
        {
            OP_NONE, // used on totally unconstrained atoms
            OP_AND,
            OP_OR,
            OP_NOT,

            ATOM_NUMBER,
            ATOM_PSEUDO,
            ATOM_RSITE,
            ATOM_CHARGE,
            ATOM_ISOTOPE,
            ATOM_RADICAL,
            ATOM_VALENCE,
            // ATOM_DEGREE,
            ATOM_CONNECTIVITY,
            ATOM_TOTAL_BOND_ORDER,
            ATOM_TOTAL_H,
            ATOM_IMPLICIT_H,
            ATOM_SUBSTITUENTS,
            ATOM_SUBSTITUENTS_AS_DRAWN,
            ATOM_SSSR_RINGS,
            ATOM_SMALLEST_RING_SIZE,
            ATOM_RING_BONDS,
            ATOM_RING_BONDS_AS_DRAWN,
            ATOM_UNSATURATION,
            ATOM_FRAGMENT,
            ATOM_AROMATICITY,
            ATOM_TEMPLATE,
            ATOM_TEMPLATE_SEQID,
            ATOM_TEMPLATE_CLASS,
            ATOM_PI_BONDED,

            BOND_ORDER,
            BOND_TOPOLOGY,

            HIGHLIGHTING
        };

        class DLLEXPORT Node
        {
        public:
            Node(int type_);
            virtual ~Node();

            OpType type; // OP_*** or ATOM_*** or BOND_***

            // type is OP_NOT: one child
            // type is OP_AND or OP_OR: more that one child
            // otherwise: no children
            PtrArray<Node> children;

            // Check if node has any constraint of the specific type
            bool hasConstraint(int what_type);

            // Check if there is no other constraint, except specified ones
            bool hasNoConstraintExcept(int what_type);
            bool hasNoConstraintExcept(int what_type1, int what_type2);

            // Remove all constraints of the given type
            void removeConstraints(int what_type);

            bool sureValue(int what_type, int& value) const;
            bool sureValueInv(int what_type, int& value) const;
            bool possibleValue(int what_type, int what_value);
            bool possibleValueInv(int what_type, int what_value);
            bool possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2);
            bool possibleValuePairInv(int what_type1, int what_value1, int what_type2, int what_value2);

            bool sureValueBelongs(int what_type, const int* arr, int count);
            bool sureValueBelongsInv(int what_type, const int* arr, int count);

            // Optimize query for faster substructure search
            void optimize();

        protected:
            // "neu" means "new" in German. This should have been a static
            // method, but static methods can not be virtual, and so it is not static.
            virtual Node* _neu() = 0;

            static Node* _und(Node* node1, Node* node2);
            static Node* _oder(Node* node1, Node* node2);
            static Node* _nicht(Node* node);

            virtual bool _possibleValue(int what_type, int what_value) = 0;
            virtual bool _possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2) = 0;

            Node* _findSureConstraint(int what_type, int& count);

            virtual bool _sureValue(int what_type, int& value_out) const = 0;
            virtual bool _sureValueBelongs(int what_type, const int* arr, int count) = 0;

            virtual void _optimize(){};
        };

        class DLLEXPORT Atom : public Node
        {
        public:
            Atom();

            Atom(int type, int value);
            Atom(int type, int value_min, int value_max);
            Atom(int type, const char* value);
            Atom(int type, QueryMolecule* value);

            ~Atom() override;

            Atom* clone();
            void copy(Atom& other);

            Atom* child(int idx);

            bool valueWithinRange(int value);

            bool hasConstraintWithValue(int what_type, int what_value);

            Atom* sureConstraint(int what_type);

            int value_min;
            int value_max;

            // available only when type is ATOM_PSEUDO or ATOM_TEMPLATE or ATOM_TEMPLATE_CLASS
            Array<char> alias;

            // available only when type is ATOM_FRAGMENT
            std::unique_ptr<QueryMolecule> fragment;

            // when type is ATOM_RSITE, the value (value_min=valuemax)
            // are 32 bits, each allowing an r-group with corresponding number
            // to go for this atom. Simple 'R' atoms have this field equal to zero.

            // "und" means "and" in German. "and" is a C++ keyword.
            static Atom* und(Atom* atom1, Atom* atom2);

            // "oder" means "or" in German. "or" is a C++ keyword.
            static Atom* oder(Atom* atom1, Atom* atom2);

            // "nicht" means "not" in German. "not" is a C++ keyword.
            static Atom* nicht(Atom* atom);

        protected:
            Node* _neu() override;

            bool _possibleValue(int what_type, int what_value) override;
            bool _possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2) override;
            bool _sureValue(int what_type, int& value_out) const override;
            bool _sureValueBelongs(int what_type, const int* arr, int count) override;

            void _optimize() override;

            DECL_ERROR;
        };

        class DLLEXPORT Bond : public Node
        {
        public:
            Bond();
            Bond(int type_, int value_);
            ~Bond() override;

            int value;

            Bond* clone();

            Bond* child(int idx);

            // "und" means "and" in German. "and" is a C++ keyword.
            static Bond* und(Bond* node1, Bond* node2);

            // "oder" means "or" in German. "or" is a C++ keyword.
            static Bond* oder(Bond* node1, Bond* node2);

            // "nicht" means "not" in German. "not" is a C++ keyword.
            static Bond* nicht(Bond* node);

        protected:
            Node* _neu() override;

            bool _possibleValue(int what_type, int what_value) override;
            bool _possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2) override;
            bool _sureValue(int what_type, int& value_out) const override;
            bool _sureValueBelongs(int what_type, const int* arr, int count) override;
        };

        QueryMolecule();
        ~QueryMolecule() override;

        void clear() override;

        BaseMolecule* neu() override;

        QueryMolecule& asQueryMolecule() override;
        bool isQueryMolecule() override;

        int getAtomNumber(int idx) override;
        int getAtomCharge(int idx) override;
        int getAtomIsotope(int idx) override;
        int getAtomRadical(int idx) override;
        int getExplicitValence(int idx) override;
        int getAtomAromaticity(int idx) override;
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
        const char* getTemplateAtomClass(int idx) override;
        const int getTemplateAtomDisplayOption(int idx) override;

        bool isRSite(int atom_idx) override;
        dword getRSiteBits(int atom_idx) override;
        void allowRGroupOnRSite(int atom_idx, int rg_idx) override;

        bool isSaturatedAtom(int idx) override;

        int getBondOrder(int idx) const override;
        int getBondTopology(int idx) override;
        bool atomNumberBelongs(int idx, const int* numbers, int count) override;
        bool possibleAtomNumber(int idx, int number) override;
        bool possibleAtomNumberAndCharge(int idx, int number, int charge) override;
        bool possibleAtomNumberAndIsotope(int idx, int number, int isotope) override;
        bool possibleAtomIsotope(int idx, int number) override;
        bool possibleAtomCharge(int idx, int charge) override;
        virtual bool possibleAtomRadical(int idx, int radical);
        void getAtomDescription(int idx, Array<char>& description) override;
        void getBondDescription(int idx, Array<char>& description) override;
        bool possibleBondOrder(int idx, int order) override;

        bool possibleNitrogenV5(int idx);

        enum QUERY_ATOM
        {
            QUERY_ATOM_A,
            QUERY_ATOM_X,
            QUERY_ATOM_Q,
            QUERY_ATOM_M,
            QUERY_ATOM_AH,
            QUERY_ATOM_XH,
            QUERY_ATOM_QH,
            QUERY_ATOM_MH,
            QUERY_ATOM_LIST,
            QUERY_ATOM_NOTLIST
        };
        enum QUERY_BOND
        {
            QUERY_BOND_DOUBLE_OR_AROMATIC = 0,
            QUERY_BOND_SINGLE_OR_AROMATIC,
            QUERY_BOND_SINGLE_OR_DOUBLE,
            QUERY_BOND_ANY
        };

        static bool isKnownAttr(QueryMolecule::Atom& qa);
        static bool isNotAtom(QueryMolecule::Atom& qa, int elem);
        static QueryMolecule::Atom* stripKnownAttrs(QueryMolecule::Atom& qa);
        static bool collectAtomList(Atom& qa, Array<int>& list, bool& notList);
        static int parseQueryAtom(QueryMolecule& qm, int aid, Array<int>& list);
        static bool queryAtomIsRegular(QueryMolecule& qm, int aid);
        static bool queryAtomIsSpecial(QueryMolecule& qm, int aid);
        static Bond* getBondOrderTerm(Bond& qb, bool& complex);
        static bool isOrBond(Bond& qb, int type1, int type2);
        static bool isSingleOrDouble(Bond& qb);
        static int getQueryBondType(Bond& qb);
        static int getAtomType(const char* label);
        static void getQueryAtomLabel(int qa, Array<char>& result);

        bool bondStereoCare(int idx) override;
        void setBondStereoCare(int idx, bool stereo_care);

        bool aromatize(const AromaticityOptions& options) override;
        bool dearomatize(const AromaticityOptions& options) override;

        int addAtom(Atom* atom);
        Atom& getAtom(int idx);
        Atom* releaseAtom(int idx);
        void resetAtom(int idx, Atom* atom);

        Bond& getBond(int idx);
        Bond* releaseBond(int idx);
        void resetBond(int idx, Bond* bond);
        int addBond(int beg, int end, Bond* bond);

        void optimize();

        Molecule3dConstraints spatial_constraints;
        Array<int> fixed_atoms;

        QueryMoleculeAromaticity aromaticity;

        Array<char> fragment_smarts;

        // for component-level grouping of SMARTS
        // components[i] = 0 means nothing;
        // components[i] = components[j] > 0 means that i-th and j-th vertices
        // must belong to the same connected component of the target molecule;
        // components[i] != components[j] > 0 means that i-th and j-th vertices
        // must belong to different connected components of the target molecule
        Array<int> components;

        void invalidateAtom(int index, int mask) override;

        int getAtomMaxExteralConnectivity(int idx);
        int _calcAtomConnectivity(int idx);

        bool standardize(const StandardizeOptions& options);

    protected:
        void _getAtomDescription(Atom* atom, Output& out, int depth);
        void _getBondDescription(Bond* bond, Output& out);
        int _getAtomMinH(Atom* atom);

        void _flipBond(int atom_parent, int atom_from, int atom_to) override;
        void _mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags) override;
        void _postMergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping,
                                       int skip_flags) override;
        void _removeAtoms(const Array<int>& indices, const int* mapping) override;
        void _removeBonds(const Array<int>& indices) override;

        Array<int> _min_h;

        Array<bool> _bond_stereo_care;

        PtrArray<Atom> _atoms;
        PtrArray<Bond> _bonds;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
