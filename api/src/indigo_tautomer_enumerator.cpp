/****************************************************************************
 * Copyright (C) 2010-2015 GGA Software Services LLC
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

#include "graph/embedding_enumerator.h"
#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_inchi_parser.h"
#include "molecule/molecule_tautomer_enumerator.h"

CEXPORT int indigoTautomerEnumerate(int molecule, const char *params)
{
   INDIGO_BEGIN
   {
   Molecule &mol = self.getObject(molecule).getMolecule();

   return self.addObject(new IndigoTautomerIter(mol, params));
   }
   INDIGO_END(-1)
}

IndigoTautomerIter::IndigoTautomerIter(Molecule &molecule, const char *params) :
IndigoObject(TAUTOMER_ITER),
_enumerator(molecule, params)
{
   _layer = 0;
}

const char * IndigoTautomerIter::debugInfo()
{
   return "<tautomer iterator>";
}

IndigoTautomerIter::~IndigoTautomerIter()
{
}

IndigoObject * IndigoTautomerIter::next()
{
   if (_layer < _enumerator.size())
      return new IndigoMoleculeTautomer(_enumerator, _layer++);
   return NULL;
}

bool IndigoTautomerIter::hasNext()
{
   return _layer < _enumerator.size() - 1;
}

IndigoMoleculeTautomer::IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int layer) :
IndigoObject(TAUTOMER_MOLECULE)
{
   enumerator.constructMolecule(_molInstance, layer);
}

const char * IndigoMoleculeTautomer::debugInfo()
{
   return "<molecule tautomer>";
}

IndigoMoleculeTautomer::~IndigoMoleculeTautomer()
{
}

IndigoObject *IndigoMoleculeTautomer::clone()
{
   return IndigoMolecule::cloneFrom(*this);
}

Molecule & IndigoMoleculeTautomer::getMolecule()
{
   return _molInstance;
}

RedBlackStringObjMap< Array<char> > * IndigoMoleculeTautomer::getProperties()
{
   return 0;
}

TautomerEnumerator::TautomerEnumerator(Molecule &molecule, const char *params)
: _hyperMolecule(molecule)
{
   // We need a canonical mapping. This is something that MoleculeInChI does.
   // This is the only reason I use it. Maybe it's better to implement this procedure outside of MoleculeInChI.
   Array<int> mapping;
   MoleculeInChI::getCanonicalOrdering(molecule, mapping);

   MoleculeInChICodeParser inchiParser(params);

   // Construct tautomers
   EmbeddingEnumerator ee(_hyperMolecule);

   Array<int> hydrogens;
   // For each position get number of fixed hydrogens
   hydrogens.expandFill(molecule.vertexCount(), 0);
   for (auto i = inchiParser.staticHydrogenPositionBegin(); i != inchiParser.staticHydrogenPositionEnd(); i = inchiParser.staticHydrogenPositionNext(i))
   {
      ++hydrogens[mapping[inchiParser.getHydrogen(i)]];
   }

   // Indicate places for mobile hydrogens
   for (auto i = inchiParser.mobileHydrogenPositionBegin(); i != inchiParser.mobileHydrogenPositionEnd(); i = inchiParser.mobileHydrogenPositionNext(i))
   {
      // Actually this is the only thing we need from InChI: a hint which positions mobile hydrogen can occupy.
      // If we don't use this, we can avoid using InChI at all.
      _hyperMolecule.setMobilePosition(mapping[inchiParser.getHydrogen(i)], true);
   }
   /*
   for (auto i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
   {
      // Alternative: set all positions as possible mobile hydrogen positions.
      // This dramatically icreases total number of tautomers found.
      _hyperMolecule.setMobilePosition(i, true);
   } */

   // Indicate occupied mobile positions
   // Probably this could be done somehow inside of hypermolecule
   for (auto i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
   {
      Dbitset Ox01;
      Ox01.set(0);
      bool occupied = molecule.getAtomTotalH(i) - hydrogens[i];
      if (occupied)
      {
         occupied = false;
         const Vertex &v = molecule.getVertex(i);
         for (auto i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
         {
            int e_inx = v.neiEdge(i);
            if (molecule.getBondOrder(e_inx) == 1)
            {
               occupied = true;
               break;
            }
         }

      }
      _hyperMolecule.setMobilePositionOccupiedMask(i, Ox01, occupied);
   }


   // Look for "zebra" pattern (like -=-=-=-=... or =-=-=-=-...)
   Graph zebraPattern;
   int v1 = zebraPattern.addVertex();
   for (int i = 1; i <= _hyperMolecule.vertexCount(); ++i)
   {
      int v2 = zebraPattern.addVertex();
      zebraPattern.addEdge(v1, v2);
      v1 = v2;
   }

   ee.setSubgraph(zebraPattern);
   ee.cb_match_edge = matchEdge;
   ee.cb_match_vertex = matchVertex;
   ee.cb_edge_add = edgeAdd;
   ee.cb_vertex_add = vertexAdd;
   ee.cb_vertex_remove = vertexRemove;

   Breadcrumps breadcrumps;
   ee.userdata = &breadcrumps;
   ee.process();
   //printf("Layers found: %d\n", _hyperMolecule.layers);
   ee.userdata = &breadcrumps;
   ee.setSubgraph(zebraPattern);
   ee.process();
   //printf("Layers found (2-nd attempt): %d\n", _hyperMolecule.layers);
}

bool TautomerEnumerator::matchEdge(Graph &subgraph, Graph &supergraph,
   int sub_idx, int super_idx, void *userdata)
{
   HyperMolecule &hyperMolecule = (HyperMolecule &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;

   int forwardSubBondOrder = breadcrumps.forwardEdgesHistory.size() % 2 == 0 ? 1 : 2;
   int backwardSubBondOrder = breadcrumps.backwardEdgesHistory.size() % 2 == 0 ? 2 : 1;
   Dbitset &forwardMask = hyperMolecule.getBondMaskIND(super_idx, forwardSubBondOrder);
   Dbitset &backwardMask = hyperMolecule.getBondMaskIND(super_idx, backwardSubBondOrder);

   return breadcrumps.forwardMask.intersects(forwardMask) || breadcrumps.backwardMask.intersects(backwardMask);
}

bool TautomerEnumerator::matchVertex(Graph &subgraph, Graph &supergraph,
   const int *core_sub, int sub_idx, int super_idx, void *userdata)
{
   // The first vertice matched shall be the mobile hydrogen position.
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;
   HyperMolecule &hyperMolecule = (HyperMolecule &)supergraph;
   if (breadcrumps.nodesHistory.size() == 0)
      return hyperMolecule.isMobilePosition(super_idx);
   return true;
}

void TautomerEnumerator::edgeAdd(Graph &subgraph, Graph &supergraph,
   int sub_idx, int super_idx, void *userdata)
{
   HyperMolecule &hyperMolecule = (HyperMolecule &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;

   int forwardSubBondOrder = breadcrumps.forwardEdgesHistory.size() % 2 == 0 ? 1 : 2;
   int backwardSubBondOrder = breadcrumps.backwardEdgesHistory.size() % 2 == 0 ? 2 : 1;
   Dbitset &forwardMask = hyperMolecule.getBondMaskIND(super_idx, forwardSubBondOrder);
   Dbitset &backwardMask = hyperMolecule.getBondMaskIND(super_idx, backwardSubBondOrder);

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
   HyperMolecule &hyperMolecule = (HyperMolecule &)supergraph;
   Breadcrumps &breadcrumps = *(Breadcrumps *)userdata;
   breadcrumps.nodesHistory.push(super_idx);
   if (breadcrumps.nodesHistory.size() > 1 && breadcrumps.nodesHistory.size() % 2 && hyperMolecule.isMobilePosition(super_idx))
   {
      if (breadcrumps.forwardMask.complements(hyperMolecule.getMobilePositionOccupiedMask(super_idx)))
      {
         hyperMolecule.addTautomer(breadcrumps.forwardMask, breadcrumps.edgesHistory, breadcrumps.nodesHistory.at(0), breadcrumps.nodesHistory.top(), true);
      }
      if (breadcrumps.backwardMask.intersects(hyperMolecule.getMobilePositionOccupiedMask(super_idx)))
      {
         Dbitset mask;
         mask.copy(breadcrumps.backwardMask);
         mask.andWith(hyperMolecule.getMobilePositionOccupiedMask(super_idx));
         hyperMolecule.addTautomer(mask, breadcrumps.edgesHistory, breadcrumps.nodesHistory.at(0), breadcrumps.nodesHistory.top(), false);
      }
   }
   else if (breadcrumps.nodesHistory.size() == 1)
   {
      // Magic number. We expect no more than 2048 tautomers.
      // This shall be fixed later.
      breadcrumps.forwardMask.resize(2048);
      breadcrumps.backwardMask.resize(2048);
      breadcrumps.forwardMask.copy(hyperMolecule.getMobilePositionOccupiedMask(super_idx));
      breadcrumps.backwardMask.set(0, hyperMolecule.layers);
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

void TautomerEnumerator::constructMolecule(Molecule &molecule, int layer)
{
   _hyperMolecule.constructMolecule(molecule, layer);
}

int TautomerEnumerator::size()
{
   return _hyperMolecule.layers;
}
