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

#include "base_cpp/auto_ptr.h"
#include "base_cpp/ptr_array.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_arom.h"

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
            // ATOM_IMPLICIT_H,
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

            bool sureValue(int what_type, int& value);
            bool sureValueInv(int what_type, int& value);
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

            virtual bool _sureValue(int what_type, int& value_out) = 0;
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

            virtual ~Atom();

            Atom* clone();
            void copy(Atom& other);

            Atom* child(int idx);

            bool valueWithinRange(int value);

            bool hasConstraintWithValue(int what_type, int what_value);

            Atom* sureConstraint(int what_type);

            int value_min;
            int value_max;

            // available only when type is ATOM_PSEUDO or ATOM_TEMPLATE or ATOM_TEMPLATE_CLASS
            std::string alias;

            // available only when type is ATOM_FRAGMENT
            AutoPtr<QueryMolecule> fragment;

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
            virtual Node* _neu();

            virtual bool _possibleValue(int what_type, int what_value);
            virtual bool _possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2);
            virtual bool _sureValue(int what_type, int& value_out);
            virtual bool _sureValueBelongs(int what_type, const int* arr, int count);

            virtual void _optimize();

            DECL_ERROR;
        };

        class DLLEXPORT Bond : public Node
        {
        public:
            Bond();
            Bond(int type_, int value_);
            virtual ~Bond();

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
            virtual Node* _neu();

            virtual bool _possibleValue(int what_type, int what_value);
            virtual bool _possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2);
            virtual bool _sureValue(int what_type, int& value_out);
            virtual bool _sureValueBelongs(int what_type, const int* arr, int count);
        };

        QueryMolecule();
        virtual ~QueryMolecule();

        virtual void clear();

        virtual BaseMolecule* neu();

        virtual QueryMolecule& asQueryMolecule();
        virtual bool isQueryMolecule();

        virtual int getAtomNumber(int idx);
        virtual int getAtomCharge(int idx);
        virtual int getAtomIsotope(int idx);
        virtual int getAtomRadical(int idx);
        virtual int getExplicitValence(int idx);
        virtual int getAtomAromaticity(int idx);
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

        virtual bool isSaturatedAtom(int idx);

        virtual int getBondOrder(int idx);
        virtual int getBondTopology(int idx);
        virtual bool atomNumberBelongs(int idx, const int* numbers, int count);
        virtual bool possibleAtomNumber(int idx, int number);
        virtual bool possibleAtomNumberAndCharge(int idx, int number, int charge);
        virtual bool possibleAtomNumberAndIsotope(int idx, int number, int isotope);
        virtual bool possibleAtomIsotope(int idx, int number);
        virtual bool possibleAtomCharge(int idx, int charge);
        virtual bool possibleAtomRadical(int idx, int radical);
        virtual void getAtomDescription(int idx, std::string& description);
        virtual void getBondDescription(int idx, std::string& description);
        virtual bool possibleBondOrder(int idx, int order);

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
        static int getAtomType( const char* label );

        virtual bool bondStereoCare(int idx);
        void setBondStereoCare(int idx, bool stereo_care);

        virtual bool aromatize(const AromaticityOptions& options);
        virtual bool dearomatize(const AromaticityOptions& options);

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

        std::string fragment_smarts;

        // for component-level grouping of SMARTS
        // components[i] = 0 means nothing;
        // components[i] = components[j] > 0 means that i-th and j-th vertices
        // must belong to the same connected component of the target molecule;
        // components[i] != components[j] > 0 means that i-th and j-th vertices
        // must belong to different connected components of the target molecule
        Array<int> components;

        virtual void invalidateAtom(int index, int mask);

        int getAtomMaxExteralConnectivity(int idx);
        int _calcAtomConnectivity(int idx);

        bool standardize(const StandardizeOptions& options);

    protected:
        void _getAtomDescription(Atom* atom, Output& out, int depth);
        void _getBondDescription(Bond* bond, Output& out);
        int _getAtomMinH(Atom* atom);

        virtual void _flipBond(int atom_parent, int atom_from, int atom_to);
        virtual void _mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags);
        virtual void _postMergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping,
                                               int skip_flags);
        virtual void _removeAtoms(const Array<int>& indices, const int* mapping);
        virtual void _removeBonds(const Array<int>& indices);

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
