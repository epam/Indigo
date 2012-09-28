/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/ 

#ifndef __molecule_inchi_component_h__
#define __molecule_inchi_component_h__

#include "base_cpp/array.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "molecule/molecule_inchi_layers.h"

namespace indigo {

// Whole InChI component with component molecule and InChI layers
struct MoleculeInChICompoment
{
   // Canonicaly-ordered molecule
   Molecule mol; 

   // Layers
   MoleculeInChILayers::MainLayerFormula     main_layer_formula;
   MoleculeInChILayers::MainLayerConnections main_layer_connections;
   MoleculeInChILayers::HydrogensLayer       hydrogens_layer;

   MoleculeInChILayers::CisTransStereochemistryLayer    cistrans_stereochemistry_layer;
   MoleculeInChILayers::TetrahedralStereochemistryLayer tetra_stereochemistry_layer;

   void construct (Molecule &original_component);

   void clear() {}

private:
   void _getCanonicalMolecule (Molecule &source_mol, Molecule &cano_mol);

   static int  _cmpVertex         (Graph &graph, int v1, int v2, const void *context);
   static int  _cmpVertexStereo   (Molecule &graph, int v1, int v2, const void *context);
   static int  _cmpMappings       (Graph &graph, const Array<int> &mapping1, 
                                   const Array<int> &mapping2, const void *context);

   static bool _checkAutomorphism (Graph &graph, const Array<int> &mapping, const void *context);
};

}

#endif // __molecule_inchi_component_h__

