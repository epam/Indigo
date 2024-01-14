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

#ifndef __molecule_arom_h__
#define __molecule_arom_h__

#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Graph;
    class Molecule;
    class QueryMolecule;
    class BaseMolecule;

    struct AromaticityOptions
    {
        enum Method
        {
            BASIC,
            GENERIC
        };

        Method method;
        bool dearomatize_check;

        bool unique_dearomatization;
        bool aromatize_skip_superatoms;

        AromaticityOptions(Method method = BASIC) : method(method), dearomatize_check(true), unique_dearomatization(false), aromatize_skip_superatoms(false)
        {
        }
    };

    // Aromatization classes
    class DLLEXPORT AromatizerBase
    {
    public:
        explicit AromatizerBase(BaseMolecule& molecule);
        virtual ~AromatizerBase();

        void aromatize();
        void reset(void);

        bool isBondAromatic(int e_idx);
        const byte* isBondAromaticArray(void);

        void addAromaticCycle(int id, const int* cycle, int cycle_len);
        void removeAromaticCycle(int id, const int* cycle, int cycle_len);
        bool handleUnsureCycles();

        void setBondAromaticCount(int e_idx, int count);

        DECL_ERROR;

    protected:
        // Functions for overloading
        virtual bool _checkVertex(int v_idx);
        virtual bool _isCycleAromatic(const int* cycle, int cycle_len) = 0;
        virtual void _handleAromaticCycle(const int* cycle, int cycle_len);
        virtual bool _acceptOutgoingDoubleBond(int /*atom*/, int /*bond*/)
        {
            return false;
        }

    protected:
        enum
        {
            MAX_CYCLE_LEN = 22
        };

        struct CycleDef
        {
            int id;
            bool is_empty;
            int length;
            int cycle[MAX_CYCLE_LEN];
        };

        BaseMolecule& _basemol;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _bonds_arom);
        TL_CP_DECL(Array<int>, _bonds_arom_count);
        TL_CP_DECL(Array<CycleDef>, _unsure_cycles);
        TL_CP_DECL(Array<int>, _cycle_atoms);
        TL_CP_DECL(RedBlackSet<int>, _inside_superatoms);

        int _cycle_atoms_mark;

        bool _checkDoubleBonds(const int* cycle, int cycle_len);
        void _aromatizeCycle(const int* cycle, int cycle_len);
        void _handleCycle(const Array<int>& vertices);

        static bool _cb_check_vertex(Graph& graph, int v_idx, void* context);
        static bool _cb_handle_cycle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        int _cyclesHandled;
        int _unsureCyclesCount;
    };

    class DLLEXPORT MoleculeAromatizer : public AromatizerBase
    {
    public:
        // Interface function for aromatization
        static bool aromatizeBonds(Molecule& mol, const AromaticityOptions& options);

        MoleculeAromatizer(Molecule& molecule, const AromaticityOptions& options);
        void precalculatePiLabels();

        static void findAromaticAtoms(BaseMolecule& mol, Array<int>* atoms, Array<int>* bonds, const AromaticityOptions& options);

    protected:
        bool _checkVertex(int v_idx) override;
        bool _isCycleAromatic(const int* cycle, int cycle_len) override;
        bool _acceptOutgoingDoubleBond(int atom, int bond) override;

        int _getPiLabel(int v_idx);
        int _getPiLabelByConn(int v_idx, int conn);

        AromaticityOptions _options;

        CP_DECL;
        TL_CP_DECL(Array<int>, _pi_labels);
    };

    class QueryMoleculeAromatizer : public AromatizerBase
    {
    public:
        // Interface function for query molecule aromatization
        static bool aromatizeBonds(QueryMolecule& mol, const AromaticityOptions& options);

        enum
        {
            EXACT,
            FUZZY
        };

        QueryMoleculeAromatizer(QueryMolecule& molecule, const AromaticityOptions& options);

        void setMode(int mode);
        void precalculatePiLabels();

    protected:
        struct PiValue
        {
            PiValue()
            {
            }
            PiValue(int min, int max) : min(min), max(max)
            {
            }

            bool canBeAromatic()
            {
                return min != -1;
            }

            int min, max;
        };

        bool _checkVertex(int v_idx) override;
        bool _isCycleAromatic(const int* cycle, int cycle_len) override;
        void _handleAromaticCycle(const int* cycle, int cycle_len) override;
        bool _acceptOutgoingDoubleBond(int atom, int bond) override;

        static bool _aromatizeBondsExact(QueryMolecule& mol, const AromaticityOptions& options);
        static bool _aromatizeBondsFuzzy(QueryMolecule& mol, const AromaticityOptions& options);

        static bool _aromatizeBonds(QueryMolecule& mol, int additional_atom, const AromaticityOptions& options);

        static bool _aromatizeRGroupFragment(QueryMolecule& fragment, bool add_single_bonds, const AromaticityOptions& options);

        PiValue _getPiLabel(int v_idx);

        CP_DECL;
        TL_CP_DECL(Array<PiValue>, _pi_labels);
        TL_CP_DECL(Array<CycleDef>, _aromatic_cycles);

        int _mode;
        bool _collecting;
        AromaticityOptions _options;
    };

    // Structure that keeps query infromation abount bonds that
    // can be aromatic in the substructure search.
    class DLLEXPORT QueryMoleculeAromaticity
    {
    public:
        bool canBeAromatic(int edge_index) const;
        void setCanBeAromatic(int edge_index, bool state);
        void clear();

    private:
        Array<bool> can_bond_be_aromatic;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
