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

#ifndef __molecule_scaffold_detection_h_
#define __molecule_scaffold_detection_h_

#include "graph/max_common_subgraph.h"
#include "graph/scaffold_detection.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    // class for searching scaffold molecule from molecules set
    class MoleculeScaffoldDetection : public ScaffoldDetection
    {
    public:
        // class for keeping molecules
        class MoleculeBasket : public ScaffoldDetection::GraphBasket
        {
        public:
            MoleculeBasket();
            ~MoleculeBasket() override;

            // initializes molecules basket
            void initBasket(ObjArray<Molecule>* mol_set, ObjArray<QueryMolecule>* basket_set, int max_number);
            // this method adds molecules from set (defines with edges and vertices lists) to basket queue
            void addToNextEmptySpot(Graph& graph, Array<int>& v_list, Array<int>& e_list) override;

            Graph& getGraphFromSet(int idx) override
            {
                return (Graph&)_searchStructures->at(_orderArray[idx]);
            }

            int getMaxGraphIndex() override;

            // returns ptr of molecule in basket with index
            Graph& getGraph(int index) const override;
            // adds new molecule to queue and returns ptr of that
            QueryMolecule& pickOutNextMolecule();

            int (*cbSortSolutions)(BaseMolecule& mol1, BaseMolecule& mol2, void* userdata);

            DECL_ERROR;

        private:
            void _sortGraphsInSet() override;

            static int _compareEdgeCount(int& i1, int& i2, void* context);
            static int _compareRingsCount(BaseMolecule& m1, BaseMolecule& m2, void* context);

            ObjArray<Molecule>* _searchStructures;
            ObjArray<QueryMolecule>* _basketStructures;

            MoleculeBasket(const MoleculeBasket&); // no implicit copy
        };

    private:
        void _searchScaffold(QueryMolecule& scaffold, bool approximate);

    public:
        MoleculeScaffoldDetection(ObjArray<Molecule>* mol_set);

        // two main methods for extracting scaffolds
        // extracting exact scaffold from molecules set
        void extractExactScaffold(QueryMolecule& scaffold)
        {
            _searchScaffold(scaffold, false);
        }
        void extractApproximateScaffold(QueryMolecule& scaffold)
        {
            _searchScaffold(scaffold, true);
        }
        // extracting approximate scaffold from molecule set

        int (*cbSortSolutions)(Molecule& mol1, Molecule& mol2, const void* userdata);

        static void clone(QueryMolecule& mol, Molecule& other);
        static void makeEdgeSubmolecule(QueryMolecule& mol, Molecule& other, Array<int>& v_list, Array<int>& e_list);

        static bool matchBonds(Graph& g1, Graph& g2, int i, int j, void* userdata);
        static bool matchAtoms(Graph& g1, Graph& g2, const int* core_sub, int i, int j, void* userdata);

        ObjArray<Molecule>* searchStructures;
        ObjArray<QueryMolecule>* basketStructures;

        DECL_ERROR;
    };

} // namespace indigo

#endif
