/****************************************************************************
 * Copyright (C) 2015 GGA Software Services LLC
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
#include "molecule/molecule_tautomer_enumerator.h"

#include "graph/embedding_enumerator.h"
#include "molecule/inchi_parser.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/molecule_tautomer_utils.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_layered_molecules.h"

using namespace indigo;

TautomerEnumerator::TautomerEnumerator(Molecule &molecule, const char *options)
: layeredMolecules(molecule)
{
   InchiWrapper indigo_inchi;

   Array<char> tmp;
   indigo_inchi.saveMoleculeIntoInchi(molecule, tmp);
   const char *params = tmp.ptr();

   // We need a canonical mapping. This is something that MoleculeInChI does.
   // This is the only reason I use it. Maybe it's better to implement this procedure outside of MoleculeInChI.
   Array<int> canonical_mapping;
   QS_DEF(Array<int>, ignored);
   ignored.clear_resize(molecule.vertexEnd());
   ignored.zerofill();

   MoleculeAutomorphismSearch of;
   of.detect_invalid_cistrans_bonds = false;
   of.detect_invalid_stereocenters = false;
   of.find_canonical_ordering = true;
   of.ignored_vertices = ignored.ptr();

   of.getcanon = true;
   of.compare_vertex_degree_first = false;
   of.refine_reverse_degree = true;
   of.refine_by_sorted_neighbourhood = true;
   of.cb_vertex_cmp = MoleculeInChICompoment::cmpVertex;
   of.process(molecule);
   of.getCanonicalNumbering(canonical_mapping);

   InChICodeParser inchiParser(params);

   Array<int> hydrogens;
   // For each position get number of fixed hydrogens
   hydrogens.expandFill(molecule.vertexCount(), 0);

   Array<int> inv_mapping;
   inv_mapping.expandFill(molecule.vertexEnd(), -1);
   int j = 0;
   for(auto i : molecule.vertices())
   {
      inv_mapping[i] = j++;
   }

   for (auto i = inchiParser.staticHydrogenPositionBegin(); i != inchiParser.staticHydrogenPositionEnd(); i = inchiParser.staticHydrogenPositionNext(i))
   {
      int inchiIndex = inchiParser.getHydrogen(i);
      int molIndex = canonical_mapping[inchiIndex];
      int simpleIndex = inv_mapping[molIndex];
      ++hydrogens[simpleIndex];
   }

   // Indicate places for mobile hydrogens
   for (auto i = inchiParser.mobileHydrogenPositionBegin(); i != inchiParser.mobileHydrogenPositionEnd(); i = inchiParser.mobileHydrogenPositionNext(i))
   {
      // Actually this is the only thing we need from InChI: a hint which positions mobile hydrogen can occupy.
      // If we don't use this, we can avoid using InChI at all.
      int inchiIndex = inchiParser.getHydrogen(i);
      int molIndex = canonical_mapping[inchiIndex];
      int simpleIndex = inv_mapping[molIndex];
      layeredMolecules.setMobilePosition(simpleIndex, true);
   }
   /*
   for (auto i : molecule.vertices())
   {
      // Alternative: set all positions as possible mobile hydrogen positions.
      // This dramatically icreases total number of tautomers found.
      int inchiIndex = inchiParser.getHydrogen(i);
      int molIndex = canonical_mapping[inchiIndex];
      int simpleIndex = inv_mapping[molIndex];
      layeredMolecules.setMobilePosition(simpleIndex, true);
   } */

   // Indicate occupied mobile positions
   // Probably this could be done somehow inside of hypermolecule
   Dbitset Ox01;
   Ox01.set(0);
   for (auto i : molecule.vertices())
   {
      bool occupied = (molecule.getAtomTotalH(i) - hydrogens[inv_mapping[i]]) != 0;
      if (occupied)
      {
         occupied = false;
         const Vertex &v = molecule.getVertex(i);
         for (auto i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
         {
            int e_inx = v.neiEdge(i);
            if (molecule.getBondOrder(e_inx) == 1 || molecule.getBondOrder(e_inx) == 4)
            {
               occupied = true;
               break;
            }
         }

      }
      layeredMolecules.setMobilePositionOccupiedMask(inv_mapping[i], Ox01, occupied);
   }

   // Look for "zebra" pattern (like -=-=-=-=... or =-=-=-=-...)
   int v1 = _zebraPattern.addVertex();
   for (auto i : layeredMolecules.vertices())
   {
      int v2 = _zebraPattern.addVertex();
      _zebraPattern.addEdge(v1, v2);
      v1 = v2;
   }

   _complete = false;
   aromatizedRange[0] = 0;
   aromatizedRange[1] = 0;
}

void TautomerEnumerator::enumerateAll(bool needAromatization)
{
   while (!_performProcedure())
      ;
   if(needAromatization)
      aromatize();
}

int TautomerEnumerator::beginNotAromatized()
{
   _enumeratedHistory.clear();
   return 1;
}

int TautomerEnumerator::beginAromatized()
{
   _enumeratedHistory.clear();
   if(aromatizedRange[1] == 0)
   {
      _aromatize(aromatizedRange[1], layeredMolecules.layers);
      aromatizedRange[1] = layeredMolecules.layers;
   }
   return -1;
}

bool TautomerEnumerator::isValid(int n)
{
   if(n > 0)
   {
      if(n - 1 < layeredMolecules.layers)
         return true;
      if(_complete)
         return false;
      if(_performProcedure())
      {
         _complete = true;
         return false;
      }
      return true;
   }
   if(n < 0)
   {
      if(-(n + 1) < layeredMolecules.layers)
      {
         if(-(n + 1) >= aromatizedRange[1])
         {
            _aromatize(aromatizedRange[1], layeredMolecules.layers);
            aromatizedRange[1] = layeredMolecules.layers;
         }
         unsigned hash = layeredMolecules.getHash(-(n + 1), true);
         return !_enumeratedHistory.find(hash);
      }
      if(_complete)
         return false;
      if(_performProcedure())
      {
         _complete = true;
         return false;
      }
     _aromatize(aromatizedRange[1], layeredMolecules.layers);
      aromatizedRange[1] = layeredMolecules.layers;
      unsigned hash = layeredMolecules.getHash(-(n + 1), true);
      return !_enumeratedHistory.find(hash);
   }
   return false;
}

int TautomerEnumerator::next(int n)
{
   if(n > 0)
      return n + 1;
   else if(n < 0)
   {
      unsigned hash = layeredMolecules.getHash(-(n + 1), true);
      _enumeratedHistory.insert(hash);
      --n;
      while(!_complete && !isValid(n))
      {
         --n;
      }

      return n;
   }
   return 1;
}

void TautomerEnumerator::constructMolecule(Molecule &molecule, int n) const
{
   if(n > 0)
      layeredMolecules.constructMolecule(molecule, n - 1, false);
   else if(n < 0)
      layeredMolecules.constructMolecule(molecule, -(n + 1), true);
   else
      ;//error!
}

bool TautomerEnumerator::_performProcedure()
{
   // Construct tautomers
   EmbeddingEnumerator ee(layeredMolecules);

   ee.setSubgraph(_zebraPattern);
   ee.cb_match_edge = matchEdge;
   ee.cb_match_vertex = matchVertex;
   ee.cb_edge_add = edgeAdd;
   ee.cb_vertex_add = vertexAdd;
   ee.cb_vertex_remove = vertexRemove;

   Breadcrumps breadcrumps;
   ee.userdata = &breadcrumps;

   int layersBefore = layeredMolecules.layers;
   ee.process();
   return layeredMolecules.layers == layersBefore;
}

bool TautomerEnumerator::aromatize()
{
    return layeredMolecules.aromatize(AromaticityOptions());
}

bool TautomerEnumerator::_aromatize(int from, int to)
{
    return layeredMolecules.aromatize(from, to, AromaticityOptions());
}

bool TautomerEnumerator::matchEdge(Graph &subgraph, Graph &supergraph,
   int sub_idx, int super_idx, void *userdata)
{
   LayeredMolecules &layeredMolecules = (LayeredMolecules &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;

   int forwardSubBondOrder = breadcrumps.forwardEdgesHistory.size() % 2 == 0 ? 1 : 2;
   int backwardSubBondOrder = breadcrumps.backwardEdgesHistory.size() % 2 == 0 ? 2 : 1;
   const Dbitset &forwardMask = layeredMolecules.getBondMask(super_idx, forwardSubBondOrder);
   const Dbitset &backwardMask = layeredMolecules.getBondMask(super_idx, backwardSubBondOrder);

   return breadcrumps.forwardMask.intersects(forwardMask) || breadcrumps.backwardMask.intersects(backwardMask);
}

bool TautomerEnumerator::matchVertex(Graph &subgraph, Graph &supergraph,
   const int *core_sub, int sub_idx, int super_idx, void *userdata)
{
   // The first vertice matched shall be the mobile hydrogen position.
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;
   LayeredMolecules &layeredMolecules = (LayeredMolecules &)supergraph;
   if (breadcrumps.nodesHistory.size() == 0)
      return layeredMolecules.isMobilePosition(super_idx);
   return true;
}

void TautomerEnumerator::edgeAdd(Graph &subgraph, Graph &supergraph,
   int sub_idx, int super_idx, void *userdata)
{
   LayeredMolecules &layeredMolecules = (LayeredMolecules &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;

   int forwardSubBondOrder = breadcrumps.forwardEdgesHistory.size() % 2 == 0 ? 1 : 2;
   int backwardSubBondOrder = breadcrumps.backwardEdgesHistory.size() % 2 == 0 ? 2 : 1;
   const Dbitset &forwardMask = layeredMolecules.getBondMask(super_idx, forwardSubBondOrder);
   const Dbitset &backwardMask = layeredMolecules.getBondMask(super_idx, backwardSubBondOrder);

   breadcrumps.edgesHistory.push(super_idx);
   breadcrumps.forwardEdgesHistory.expand(breadcrumps.forwardEdgesHistory.size() + 1);
   breadcrumps.forwardEdgesHistory.top().copy(breadcrumps.forwardMask);
   breadcrumps.backwardEdgesHistory.expand(breadcrumps.backwardEdgesHistory.size() + 1);
   breadcrumps.backwardEdgesHistory.top().copy(breadcrumps.backwardMask);

   breadcrumps.forwardMask.andWith(forwardMask);
   breadcrumps.backwardMask.andWith(backwardMask);
}

void TautomerEnumerator::vertexAdd(Graph &subgraph, Graph &supergraph,
   int sub_idx, int super_idx, void *userdata)
{
   LayeredMolecules &layeredMolecules = (LayeredMolecules &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;
   breadcrumps.nodesHistory.push(super_idx);
   if (breadcrumps.nodesHistory.size() > 1 && breadcrumps.nodesHistory.size() % 2 && layeredMolecules.isMobilePosition(super_idx))
   {
      if (breadcrumps.forwardMask.complements(layeredMolecules.getMobilePositionOccupiedMask(super_idx)))
      {
         layeredMolecules.addLayersWithInvertedPath(breadcrumps.forwardMask, breadcrumps.edgesHistory, breadcrumps.nodesHistory.at(0), breadcrumps.nodesHistory.top(), true);
      }
      if (breadcrumps.backwardMask.intersects(layeredMolecules.getMobilePositionOccupiedMask(super_idx)))
      {
         Dbitset mask;
         mask.copy(breadcrumps.backwardMask);
         mask.andWith(layeredMolecules.getMobilePositionOccupiedMask(super_idx));
         layeredMolecules.addLayersWithInvertedPath(mask, breadcrumps.edgesHistory, breadcrumps.nodesHistory.at(0), breadcrumps.nodesHistory.top(), false);
      }
   }
   else if (breadcrumps.nodesHistory.size() == 1)
   {
      breadcrumps.forwardMask.resize(layeredMolecules.layers);
      breadcrumps.backwardMask.resize(layeredMolecules.layers);
      breadcrumps.forwardMask.copy(layeredMolecules.getMobilePositionOccupiedMask(super_idx));
      breadcrumps.backwardMask.set(0, layeredMolecules.layers);
   }
}

void TautomerEnumerator::vertexRemove(Graph &subgraph, int sub_idx, void *userdata)
{
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;

   if (breadcrumps.backwardEdgesHistory.size() > 0)
   {
      breadcrumps.edgesHistory.pop();
      breadcrumps.forwardMask.copy(breadcrumps.forwardEdgesHistory.top());
      breadcrumps.forwardEdgesHistory.pop();
      breadcrumps.backwardMask.copy(breadcrumps.backwardEdgesHistory.top());
      breadcrumps.backwardEdgesHistory.pop();
   }
   breadcrumps.nodesHistory.pop();
}

void TautomerEnumerator::constructMolecule(Molecule &molecule, int layer, bool needAromatize) const
{
   layeredMolecules.constructMolecule(molecule, layer, needAromatize);
}
