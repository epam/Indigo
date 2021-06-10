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

#ifndef __molecule_inchi_layers_h__
#define __molecule_inchi_layers_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "molecule/molecule_inchi_utils.h"

namespace indigo
{

    class Molecule;
    class Output;
    class MoleculeStereocenters;

    // Namespace with layers
    // Each layers are independent and contains all
    // nessesary information
    namespace MoleculeInChILayers
    {

        // Abtract layer
        class AbstractLayer
        {
        public:
            AbstractLayer();
            virtual ~AbstractLayer(){};

            // Method for constructing internal layer information
            void construct(Molecule& mol);

            DECL_ERROR;

        protected:
            Molecule& _getMolecule();

            virtual void _construct(){};

        private:
            Molecule* _mol;
        };

        // Main layer formula
        class MainLayerFormula : public AbstractLayer
        {
        public:
            void printFormula(Array<char>& result);

            static int compareComponentsAtomsCountNoH(MainLayerFormula& comp1, MainLayerFormula& comp2);
            static int compareComponentsTotalHydrogensCount(MainLayerFormula& comp1, MainLayerFormula& comp2);

        protected:
            void _construct() override;

        private:
            Array<int> _atoms_count;

            void _printAtom(Output& output, int label) const;
            void _collectAtomsCount();
        };

        // Main layer connections
        class MainLayerConnections : public AbstractLayer
        {
        public:
            void printConnectionTable(Array<char>& result);

            int compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2);

            static int compareComponentsConnectionTables(MainLayerConnections& comp1, MainLayerConnections& comp2);

        protected:
            void _construct() override;

        private:
            Array<int> _connection_table;

            void _linearizeConnectionTable();
        };

        // Layer with hydrogens
        class HydrogensLayer : public AbstractLayer
        {
        public:
            static int compareComponentsHydrogens(HydrogensLayer& comp1, HydrogensLayer& comp2);

            bool checkAutomorphism(const Array<int>& mapping);
            int compareMappings(MoleculeInChIUtils::Mapping& m1, MoleculeInChIUtils::Mapping& m2);

            void print(Array<char>& result);

        protected:
            void _construct() override;

        private:
            // Number of immobile hydrogens for each atom
            Array<int> _per_atom_immobile;
            // Atom indices in the 'mol' to avoid vertexBegin/vertexEnd iterations
            // when comparing components
            Array<int> _atom_indices;

            // TODO: Mobile hydrogens, fixed hydrogens
        };

        // Cis-trans stereochemistry
        class CisTransStereochemistryLayer : public AbstractLayer
        {
        public:
            void print(Array<char>& result);

            bool checkAutomorphism(const Array<int>& mapping);
            int compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2);

            static int compareComponents(CisTransStereochemistryLayer& comp1, CisTransStereochemistryLayer& comp2);

        protected:
            void _construct() override;

        private:
            Array<int> bond_is_cis_trans;
        };

        // Tetrahedral stereochemistry
        class TetrahedralStereochemistryLayer : public AbstractLayer
        {
        public:
            void print(Array<char>& result);

            void printEnantiomers(Array<char>& result);

            bool checkAutomorphism(const Array<int>& mapping);
            int compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2);

            static int compareComponentsEnantiomers(TetrahedralStereochemistryLayer& comp1, TetrahedralStereochemistryLayer& comp2);

            static int compareComponents(TetrahedralStereochemistryLayer& comp1, TetrahedralStereochemistryLayer& comp2);

        private:
            int _getMappingSign(const MoleculeStereocenters& stereocenters, const MoleculeInChIUtils::Mapping* m, int index);

            int _getFirstSign();
        };

    }; // namespace MoleculeInChILayers

} // namespace indigo

#endif // __molecule_inchi_layers_h__
