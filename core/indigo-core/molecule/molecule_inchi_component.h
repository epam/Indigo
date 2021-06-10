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

#ifndef __molecule_inchi_component_h__
#define __molecule_inchi_component_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/reusable_obj_array.h"
#include "molecule/molecule.h"
#include "molecule/molecule_inchi_layers.h"

namespace indigo
{

    // Whole InChI component with component molecule and InChI layers
    struct MoleculeInChICompoment
    {
        // Canonicaly-ordered molecule
        Molecule mol;

        // Layers
        MoleculeInChILayers::MainLayerFormula main_layer_formula;
        MoleculeInChILayers::MainLayerConnections main_layer_connections;
        MoleculeInChILayers::HydrogensLayer hydrogens_layer;

        MoleculeInChILayers::CisTransStereochemistryLayer cistrans_stereochemistry_layer;
        MoleculeInChILayers::TetrahedralStereochemistryLayer tetra_stereochemistry_layer;

        void construct(Molecule& original_component);

        void clear()
        {
        }

        void getCanonicalOrdering(Molecule& source_mol, Array<int>& mapping);
        static int cmpVertex(Graph& graph, int v1, int v2, const void* context);

    private:
        void _getCanonicalMolecule(Molecule& source_mol, Molecule& cano_mol);

        static int _cmpVertex(Graph& graph, int v1, int v2, const void* context);
        static int _cmpVertexStereo(Molecule& graph, int v1, int v2, const void* context);
        static int _cmpMappings(Graph& graph, const Array<int>& mapping1, const Array<int>& mapping2, const void* context);

        static bool _checkAutomorphism(Graph& graph, const Array<int>& mapping, const void* context);
    };

} // namespace indigo

#endif // __molecule_inchi_component_h__
