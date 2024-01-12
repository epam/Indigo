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

#ifndef __molecule_dearom_h__
#define __molecule_dearom_h__

#include "base_cpp/array.h"
#include "base_cpp/d_bitset.h"
#include "base_cpp/exception.h"
#include "base_cpp/gray_codes.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_perfect_matching.h"
#include "molecule/molecule_arom.h"

namespace indigo
{

    class BaseMolecule;
    class Molecule;
    class Scanner;
    class Output;

    DECL_EXCEPTION(DearomatizationException);
    DECL_EXCEPTION2(NonUniqueDearomatizationException, DearomatizationException);

    // Storage for dearomatizations
    class DearomatizationsStorage
    {
        friend class DearomatizationsStorageWrapper;

    public:
        DECL_ERROR2(DearomatizationException);

        explicit DearomatizationsStorage();

        void clear(void);
        void clearIndices(void);
        void clearBondsState(void);

        void setGroupsCount(int groupsCount);
        void setGroup(int group, int boundsCount, const int* bonds, int heteroAtomsCount, const int* hetroAtoms);
        void addGroupDearomatization(int group, const byte* dearomBondsState);
        void addGroupHeteroAtomsState(int group, const byte* heteroAtomsState);

        int getGroupDearomatizationsCount(int group) const;
        byte* getGroupDearomatization(int group, int dearomatizationIndex);
        const int* getGroupBonds(int group) const;
        int getGroupBondsCount(int group) const;

        int getGroupHeterAtomsStateCount(int group) const;
        const byte* getGroupHeterAtomsState(int group, int index) const;
        const int* getGroupHeteroAtoms(int group) const;
        int getGroupHeteroAtomsCount(int group) const;

        int getGroupsCount(void) const;

        void saveBinary(Output& output) const;
        void loadBinary(Scanner& scanner);

        int getDearomatizationParams(void)
        {
            return _dearomParams;
        }
        void setDearomatizationParams(byte params)
        {
            _dearomParams = params;
        }

    protected:
        struct PseudoArray
        {
            int count;
            int offset;
        };
        struct Group
        {
            PseudoArray aromBondsIndices;
            PseudoArray dearomBondsState;
            PseudoArray heteroAtomsIndices;
            PseudoArray heteroAtomsState;
        };

    protected:
        Array<int> _aromBondsArray;          // Bonds used in this connectivity group
        Array<int> _heteroAtomsIndicesArray; // Heteroatoms indices
        Array<Group> _aromaticGroups;

        // Data for I/O
        Array<byte> _dearomBondsStateArray; // Array of array of dearomatization configuration
        Array<byte> _heteroAtomsStateArray; // States for heteroatoms
        byte _dearomParams;
    };

    // Class for handling aromatic groups in molecule (contains helpful functions)
    class DearomatizationsGroups
    {
    public:
        // Constants for Prepare function
        enum
        {
            GET_HETERATOMS_INDICES = 0x01,
            GET_VERTICES_FILTER = 0x02
        };
        struct GROUP_DATA
        {
            Array<int> bonds;
            Array<int> bondsInvMapping;
            Array<int> vertices;
            Array<int> verticesFilter;
            Array<int> heteroAtoms;
            Array<int> heteroAtomsInvMapping;
        };

    public:
        DearomatizationsGroups(BaseMolecule& molecule, bool skip_superatoms = false);

        // for flags see GET_***
        void getGroupData(int group, int flags, GROUP_DATA* data);
        // Construct bondsInvMapping, vertices and heteroAtomsInvMapping
        void getGroupDataFromStorage(DearomatizationsStorage& storage, int group, GROUP_DATA* data);

        int detectAromaticGroups(const int* atom_external_conn);
        void constructGroups(DearomatizationsStorage& storage, bool needHeteroAtoms);

        bool* getAcceptDoubleBonds(void);
        bool isAcceptDoubleBond(int atom);

        DECL_ERROR2(DearomatizationException);

    protected:
        void _detectAromaticGroups(int v_idx, const int* atom_external_conn);

        int _getFixedConnectivitySpecific(int label, int charge, int min_conn, int n_arom);

    protected:
        BaseMolecule& _molecule;
        int _aromaticGroups;
        bool _isQueryMolecule;

        // Additional data stored here to prevent reallocatoins
        CP_DECL;
        TL_CP_DECL(Array<int>, _vertexAromaticGroupIndex);
        TL_CP_DECL(Array<bool>, _vertexIsAcceptDoubleEdge);
        TL_CP_DECL(Array<bool>, _vertexIsAcceptSingleEdge);
        TL_CP_DECL(Array<int>, _vertexProcessed);

        TL_CP_DECL(Array<int>, _groupVertices);
        TL_CP_DECL(Array<int>, _groupEdges);
        TL_CP_DECL(Array<int>, _groupHeteroAtoms);
        TL_CP_DECL(GROUP_DATA, _groupData);
        TL_CP_DECL(RedBlackSet<int>, _inside_superatoms);
    };

    // Molecule dearomatization class.
    class Dearomatizer
    {
    public:
        enum
        {
            PARAMS_NO_DEAROMATIZATIONS,
            PARAMS_SAVE_ALL_DEAROMATIZATIONS, // Store all dearomatizations
            PARAMS_SAVE_ONE_DEAROMATIZATION,  // Store just one dearomatization for every heteroatom configuration
            PARAMS_SAVE_JUST_HETERATOMS       // Store just heteroatoms configuration
        };

    public:
        explicit Dearomatizer(BaseMolecule& molecule, const int* atom_external_conn, const AromaticityOptions& options);
        virtual ~Dearomatizer();

        void enumerateDearomatizations(DearomatizationsStorage& dearomatizations);

        static void setDearomatizationParams(int params);

    protected:
        class GraphMatchingFixed : public GraphPerfectMatching
        {
        public:
            GraphMatchingFixed(BaseMolecule& molecule);

            void setFixedInfo(const Dbitset* edgesFixed, const Dbitset* verticesFixed);

            bool checkVertex(int v_idx) override;
            bool checkEdge(int e_idx) override;

        protected:
            const Dbitset* _edgesFixed;
            const Dbitset* _verticesFixed;
        };

    protected:
        GraphMatchingFixed _graphMatching;

        BaseMolecule& _molecule;
        const AromaticityOptions& _options;
        int _connectivityGroups;
        int _activeGroup;
        bool _isQueryMolecule;

        DearomatizationsGroups _aromaticGroups;
        DearomatizationsStorage* _dearomatizations;

        CP_DECL;
        TL_CP_DECL(DearomatizationsGroups::GROUP_DATA, _aromaticGroupData);
        /*TL_CP_DECL(*/ Dbitset /*,    */ _edgesFixed /*)*/;
        /*TL_CP_DECL(*/ Dbitset /*,    */ _verticesFixed /*)*/;
        TL_CP_DECL(Array<int>, _submoleculeMapping);

    protected:
        void _initEdges(void);
        void _initVertices(void);

        void _prepareGroup(int group, BaseMolecule& submolecule);

        void _fixHeteratom(int atom_idx, bool toFix);
        void _processMatching(BaseMolecule& submolecule, int group, const byte* hetroAtomsState);
        void _enumerateMatching(void);
        void _handleMatching(void);
    };

    // Dearomatization matcher with delayed initialization
    class DearomatizationMatcher
    {
    public:
        DECL_ERROR2(DearomatizationException);

        DearomatizationMatcher(DearomatizationsStorage& dearomatizations, BaseMolecule& molecule, const int* atom_external_conn, bool skip_superatoms = false);

        bool isAbleToFixBond(int edge_idx, int type);
        bool fixBond(int edge_idx, int type);
        void unfixBond(int edge_idx);
        void unfixBondByAtom(int atom_idx);

    protected:
        void _prepare(void);
        void _prepareGroup(int group);

        void _generateUsedVertices(void);
        bool _tryToChangeActiveIndex(int dearom_idx, int group, byte* groupEdgesPtr, byte* groupEdgesStatePtr);
        bool _fixBondInMatching(int group, int indexInGroup, int type);

    protected:
        struct GroupExData
        {
            int offsetInEdgesState; // Offset in matched edges state
            int activeEdgeState;
            int offsetInVertices;
            int verticesUsed;
            bool needPrepare;
        };
        // Graph edge matching class to support current dearomatization
        class GraphMatchingEdgeFixed : public GraphPerfectMatching
        {
        public:
            GraphMatchingEdgeFixed(BaseMolecule& molecule);

            void setExtraInfo(byte* edgesEdges);

            bool checkEdge(int e_idx) override;

        protected:
            byte* _edgesState;
        };
        // Graph edge matching class to find dearomatization by heteroatoms state
        class GraphMatchingVerticesFixed : public GraphPerfectMatching
        {
        public:
            GraphMatchingVerticesFixed(BaseMolecule& molecule);

            void setVerticesState(const byte* verticesState);
            void setVerticesMapping(int* verticesMapping);
            void setVerticesAccept(bool* verticesAcceptDoubleBond);

            bool checkVertex(int v_idx) override;

        protected:
            const byte* _verticesState;
            int* _verticesMapping;
            bool* _verticesAcceptDoubleBond;
        };

    protected:
        BaseMolecule& _molecule;
        DearomatizationsStorage& _dearomatizations;
        GraphMatchingEdgeFixed _graphMatchingFixedEdges;
        DearomatizationsGroups _aromaticGroups;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _matchedEdges);       // Edges that have already been matched
        TL_CP_DECL(Array<byte>, _matchedEdgesState);  // State of such edges
        TL_CP_DECL(Array<GroupExData>, _groupExInfo); // Additional data for group
        TL_CP_DECL(Array<int>, _verticesInGroup);
        TL_CP_DECL(Dbitset, _verticesAdded);
        TL_CP_DECL(Array<int>, _edges2GroupMapping);
        TL_CP_DECL(Array<int>, _edges2IndexInGroupMapping);
        TL_CP_DECL(Array<byte>, _correctEdgesArray);
        TL_CP_DECL(Array<int>, _verticesFixCount);
        TL_CP_DECL(DearomatizationsGroups::GROUP_DATA, _aromaticGroupsData);

        bool _needPrepare;
        int _lastAcceptedEdge;
        int _lastAcceptedEdgeType;
    };

    class MoleculeDearomatizer
    {
    public:
        MoleculeDearomatizer(BaseMolecule& mol, DearomatizationsStorage& dearomatizations);

        // Function dearomatizes as much as possible.
        // Returns true if all bonds were dearomatized, false overwise
        static bool dearomatizeMolecule(BaseMolecule& mol, const AromaticityOptions& options);

        static bool restoreHydrogens(BaseMolecule& mol, const AromaticityOptions& options);
        static bool restoreHydrogens(BaseMolecule& mol, bool unambiguous_only);

        void dearomatizeGroup(int group, int dearomatization_index);
        void restoreHydrogens(int group, int dearomatization_index);

    private:
        DearomatizationsStorage& _dearomatizations;
        BaseMolecule& _mol;
        bool _isQueryMolecule;

        int _countDoubleBonds(int group, int dearomatization_index);
        int _getBestDearomatization(int group);

        CP_DECL;
        TL_CP_DECL(Array<int>, vertex_connectivity);
    };

} // namespace indigo

#endif // __molecule_dearom_h__
