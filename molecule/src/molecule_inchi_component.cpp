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

#include "molecule/molecule_inchi_component.h"

#include "graph/automorphism_search.h"
#include "molecule/elements.h"

using namespace indigo;
using namespace indigo::MoleculeInChILayers;

// Code for debug
void (*dbg_handle_canonical_component_cb) (const Molecule &cano_component);

void MoleculeInChICompoment::construct (Molecule &original_component)
{
   // Array with all layers
   AbstractLayer* layers[] = { 
      &main_layer_formula, 
      &main_layer_connections, 
      &hydrogens_layer, 
      &cistrans_stereochemistry_layer,
      &tetra_stereochemistry_layer
   };

   // Construct layers for original molecule
   for (int i = 0; i < NELEM(layers); i++)
      layers[i]->construct(original_component);

   // Construct canonical molecule
   _getCanonicalMolecule(original_component, mol);
   
   // Reconstruct layers for canonical molecule
   for (int i = 0; i < NELEM(layers); i++)
      layers[i]->construct(mol);
}

void MoleculeInChICompoment::_getCanonicalMolecule 
      (Molecule &source_mol, Molecule &cano_mol)
{
   QS_DEF(Array<int>, ignored);
   ignored.clear_resize(source_mol.vertexEnd());
   ignored.zerofill();
   for (int i = source_mol.vertexBegin(); i < source_mol.vertexEnd(); i = source_mol.vertexNext(i))
      if (source_mol.getAtomNumber(i) == ELEM_H && source_mol.getVertex(i).degree() == 1)
         ignored[i] = 1;

   AutomorphismSearch as;
   as.getcanon = true;
   as.compare_vertex_degree_first = false;
   as.refine_reverse_degree = true;
   as.refine_by_sorted_neighbourhood = true;
   as.ignored_vertices = ignored.ptr();
   as.cb_vertex_cmp = _cmpVertex;
   as.cb_compare_mapped = _cmpMappings;
   as.cb_check_automorphism = _checkAutomorphism;
   as.context = (void *)this;

   as.process(source_mol);

   QS_DEF(Array<int>, canonical_order);
   as.getCanonicalNumbering(canonical_order);

   cano_mol.makeSubmolecule(source_mol, canonical_order, NULL);

   if (dbg_handle_canonical_component_cb != NULL)
      dbg_handle_canonical_component_cb(cano_mol);
}

int MoleculeInChICompoment::_cmpVertex (Graph &graph, int v1, int v2, const void *context)
{
   Molecule &mol = (Molecule &)graph;

   const Array<int> &atom_lables_ranks = MoleculeInChIUtils::getLexSortedAtomLablesRanks();

   int ret = atom_lables_ranks[mol.getAtomNumber(v1)] - atom_lables_ranks[mol.getAtomNumber(v2)];
   if (ret != 0)
      return ret;

   // Compare number of bonds
   int bonds_1 = graph.getVertex(v1).degree();// + a1.implicit_h;
   int bonds_2 = graph.getVertex(v2).degree();// + a2.implicit_h;

   ret = bonds_1 - bonds_2;
   if (ret != 0)
      return ret;

   return 0;
}

int MoleculeInChICompoment::_cmpVertexStereo (Molecule &mol, int v1, int v2, const void *context)
{
   // TODO: Implement as in InChI
   int diff = mol.stereocenters.getType(v1) - mol.stereocenters.getType(v2);
   if (diff != 0)
      return diff;

   return 0;
}


bool MoleculeInChICompoment::_checkAutomorphism (Graph &graph, const Array<int> &mapping, 
                                         const void *context)
{
   Molecule &mol = (Molecule &)graph;

   // Check bond mapping
   for (int e_idx = mol.edgeBegin(); e_idx != mol.edgeEnd(); e_idx = mol.edgeNext(e_idx))
   {
      const Edge &edge = mol.getEdge(e_idx);

      if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
         continue;

      int mapped_idx = mol.findEdgeIndex(mapping[edge.beg], mapping[edge.end]);

      if (mapped_idx == -1)
         return false;

      if (mol.getBondOrder(e_idx) != mol.getBondOrder(mapped_idx))
         return false;
   }

   MoleculeInChICompoment &self = *(MoleculeInChICompoment *)context;

   if (!self.hydrogens_layer.checkAutomorphism(mapping))
      return false;
   if (!self.cistrans_stereochemistry_layer.checkAutomorphism(mapping))
      return false;
   if (!self.tetra_stereochemistry_layer.checkAutomorphism(mapping))
      return false;

   return true;
}

int MoleculeInChICompoment::_cmpMappings (Graph &graph, const Array<int> &mapping1, 
   const Array<int> &mapping2, const void *context)
{
   QS_DEF(Array<int>, inv_mapping1);
   QS_DEF(Array<int>, inv_mapping2);

   inv_mapping1.clear_resize(graph.vertexEnd());
   inv_mapping2.clear_resize(graph.vertexEnd());

   inv_mapping1.fffill();
   inv_mapping2.fffill();

   for (int i = 0; i < mapping1.size(); i++)
   {
      inv_mapping1[mapping1[i]] = i;
      inv_mapping2[mapping2[i]] = i;
   }

   MoleculeInChIUtils::Mapping m1(mapping1, inv_mapping1);
   MoleculeInChIUtils::Mapping m2(mapping2, inv_mapping2);

   MoleculeInChICompoment &self = *(MoleculeInChICompoment *)context;

   int diff = self.main_layer_connections.compareMappings(m1, m2);
   if (diff != 0)
      return diff;

   diff = self.hydrogens_layer.compareMappings(m1, m2);
   if (diff != 0)
      return diff;

   diff = self.cistrans_stereochemistry_layer.compareMappings(m1, m2);
   if (diff != 0)
      return diff;

   diff = self.tetra_stereochemistry_layer.compareMappings(m1, m2);
   if (diff != 0)
      return diff;

   return 0;
}

