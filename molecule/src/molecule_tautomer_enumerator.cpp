/****************************************************************************
 * Copyright (C) 2015 EPAM Systems
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

#include "base_cpp/scanner.h"
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

#include "reaction/reaction_product_enumerator.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/reaction_transformation.h"
#include "reaction/query_reaction.h"

using namespace indigo;

TautomerEnumerator::TautomerEnumerator(Molecule &molecule, const char *options)
: layeredMolecules(molecule),
_currentLayer(0),
_currentRule(0)
{
   InchiWrapper indigo_inchi;

   QS_DEF(Array<char>, tmp);
   indigo_inchi.saveMoleculeIntoInchi(molecule, tmp);
   const char *params = tmp.ptr();

   // We need a canonical mapping. This is something that MoleculeInChI does.
   // This is the only reason I use it. Maybe it's better to implement this procedure outside of MoleculeInChI.
   QS_DEF(Array<int>, canonical_mapping);
   canonical_mapping.resize(molecule.vertexEnd());
   canonical_mapping.zerofill();
   QS_DEF(Array<int>, ignored);
   ignored.resize(molecule.vertexEnd());
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

   QS_DEF(Array<int>, hydrogens);
   // For each position get number of fixed hydrogens
   hydrogens.resize(molecule.vertexCount());
   hydrogens.zerofill();

   QS_DEF(Array<int>, inv_mapping);
   inv_mapping.resize(molecule.vertexEnd());
   inv_mapping.fill(-1);
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

void TautomerEnumerator::product_proc( Molecule &product, Array<int> &monomers_indices, Array<int> &mapping, void *userdata )
{
   LayeredMolecules *lm = (LayeredMolecules *)userdata;
   lm->addLayerFromMolecule(product, mapping);
}

bool TautomerEnumerator::enumerateLazy()
{
   return _performProcedure();
}

bool TautomerEnumerator::_performProcedure()
{
#if 0
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
#endif
   char* reactionSmarts[] = {
      "[#1:0][N,O:1][*:2]=[N,O:3]>>[N,O:1]=[A:2][N,O:3][#1:0]",
      "[#1:0][N,O:1][*:2]=[*:3][*:4]=[N,O:5]>>[N,O:1]=[A:2][A:3]=[A:4][N,O:5][#1:0]",
      "[#1:0][N,O:1][*:2]=[*:3][*:4]=[*:5][*:6]=[N,O:7]>>[N,O:1]=[A:2][A:3]=[A:4][A:5]=[A:6][N,O:7][#1:0]",
      "[#1:0][N,O:1][*:2]=[*:3][*:4]=[*:5][*:6]=[*:7][*:8]=[N,O:9]>>[N,O:1]=[A:2][A:3]=[A:4][A:5]=[A:6][A:7]=[A:8][N,O:9][#1:0]"
      /*"[O,S,Se,Te;X1:1]=[C:2][CX4;R0,R1,R2:3][#1:0]>>[#1:0][O,S,Se,Te;X2:1][#6;X3:2]=,:[C,c;X3:3]", // Rule 1:  1,3 Keto-enol
      "[#1:0][O,S,Se,Te;X2:1][#6;X3:2]=,:[C,c;X3:3]>>[O,S,Se,Te;X1:1]=[C:2][CX4;R0,R1,R2:3][#1:0]", // Rule 1:  1,3 Keto-enol
      "[O,S,Se,Te;X1:1]=[CX3:2]([#6:6])[C:3]=[C:4][CX4,NX3:5]([C:7])[#1:0]>>[#1:0][O,S,Se,Te;X2:1][CX3:2]([C:6])=[C:3][C:4]=[CX3,N:5]([C:7])", // Rule 2:  1,5 Keto-enol
      "[#1:0][O,S,Se,Te;X2:1][CX3:2]([C:6])=[C:3][C:4]=[CX3,N:5]([C:7])>>[O,S,Se,Te;X1:1]=[CX3:2]([#6:6])[C:3]=[C:4][CX4,NX3:5]([C:7])[#1:0]", // Rule 2:  1,5 Keto-enol
      "[#1,a,O:5][NX2:1]=[CX3:2]([C,#1:4])[CX4;R0,R1,R2:3][#1:0]>>[#1,a,O:5][NX3:1]([#1:0])[CX3:2]([C,#1:4])=[CX3:3]",   // Rule 3:  simple (aliphatic) imine
      "[#1,a,O:5][NX3:1]([#1:0])[CX3:2]([C,#1:4])=[CX3:3]>>[#1,a,O:5][NX2:1]=[CX3:2]([C,#1:4])[CX4;R0,R1,R2:3][#1:0]",   // Rule 3:  simple (aliphatic) imine
      "[CX3R0:1]([C,#1:5])([C:4])=[C:2][N:3]([C,#1:6])[#1:0]>>[#1:0][CX4R0:1]([C,#1:5])([C:4])[c:2]:[n:3]:[c:6]",  // Rule 4:  special imine
      "[#1:0][CX4R0:1]([C,#1:5])([C:4])[c:2]:[n:3]:[c:6]>>[CX3R0:1]([C,#1:5])([C:4])=[C:2][N:3]([C,#1:6])[#1:0]",  // Rule 4:  special imine
      "[#1:0][N:1]@[C:2]=[O,NX2:3]>>[NX2,nX2:1]=&@,:[C,c:2][O,N:3][#1:0]",          // Rule 5:  aromatic heteroatom H shift
      "[NX2,nX2:1]=&@,:[C,c:2][O,N:3][#1:0]>>[#1:0][N:1]@[C:2]=[O,NX2:3]",          // Rule 5:  aromatic heteroatom H shift
      "[N,n,S,s,O,o,Se,Te:1]=[NX2,nX2,C,c,P,p:2][N,n,S,O,Se,Te:3][#1:0]>>[#1:0][N,n,S,O,Se,Te:1][NX2,nX2,C,c,P,p:2]=[N,n,S,s,O,o,Se,Te:3]",  // Rule 6:  1,3 heteroatom H shift
      "[NX2,nX2,S,O,Se,Te:1]=[C,c,NX2,nX2:2][C,c:3]=[C,c,nX2:4][N,n,S,s,O,o,Se,Te:5][#1:0]>>[#1:0][N,n,S,O,Se,Te:1][C,c,NX2,nX2:2]=[C,c:3][C,c,nX2:4]=[NX2,S,O,Se,Te:5]",  // Rule 7:  1,5 (aromatic) heteroatom H shift (1)
      "[n,s,o:1]=[c,n:2][c:3]=[c,n:4][n,s,o:5][#1:0]>>[#1:0][n,s,o:1][c,n:2]=[c:3][c,n:4]=[n,s,o:5]",  // Rule 8:  1,5 aromatic heteroatom H shift (2)
      "[NX2,nX2,S,O,Se,Te:1]=[C,c,NX2,nX2:2][C,c:3]=[C,c,NX2,nX2:4][C,c,NX2,nX2:5]=[C,c,NX2,nX2:6][N,n,S,s,O,o,Se,Te:7][#1:0]>>[#1:0][N,n,S,O,Se,Te:1][C,c,NX2,nX2:2]=[C,c:3][C,c,NX2,nX2:4]=[C,c,NX2,nX2:5][C,c,NX2,nX2:6]=[NX2,S,O,Se,Te:7]",  // Rule 9:  1,7 (aromatic) heteroatom H shift 
      "[CX3:1]([C,#1:8])([C,#1:9])=[C,c,NX2,nX2:2][C,c:3]=[C,c,NX2,nX2:4][C,c,NX2,nX2:5]=[C,c,NX2,nX2:6][N,n,S,s,O,o,Se,Te:7][#1:0]>>[#1:0][CX3:1]([C,#1:8])([C,#1:9])[C,c,NX2,nX2:2]=[C,c:3][C,c,NX2,nX2:4]=[C,c,NX2,nX2:5][C,c,NX2,nX2:6]=[NX2,S,O,Se,Te:7]",  // Rule 9:  1,7 (aromatic) heteroatom H shift (1) 
      "[#1:0][N,n,O:1][C,c,nX2:2]=[C,c,nX2:3][c,nX2:4]=[c,nX2:5][c,nX2:6]=[c,nX2:7][C,c,nX2:8]=[N,n,O:9]>>[NX2,nX2,O:1]=[C,c,nX2:2][c,nX2:3]=[c,nX2:4][c,nX2:5]=[c,nX2:6][c,nX2:7]=[c,nX2:8][n,O:9][#1:0]",  // Rule 10:  1,9 (aromatic) heteroatom H shift
      "[#1:0][N,n,O:1][C,c,nX2:2]=[C,c,nX2:3][c,nX2:4]=[C,c,nX2:5][C,c,nX2:6]=[C,c,nX2:7][C,c,nX2:8]=[C,c,nX2:9][C,c,nX2:10]=[NX2,nX2,O:11]>>[NX2,nX2,O:1]=[C,c,nX2:2][C,c,nX2:3]=[C,c,nX2:4][C,c,nX2:5]=[C,c,nX2:6][C,c,nX2:7]=[C,c,nX2:8][C,c,nX2:9]=[C,c,nX2:10][O,nX2:11][#1:0]",   // Rule 11:  1,11 (aromatic) heteroatom H shift
      "[#1:0][O,S,N:1][C,c;r5:2]([!C&!c&!#1])=[C,c;r5:3][C,c;r5:4]>>[O,S,N:1]=[C;r5:2]([!C&!c&!#1])[Cr5;R0,R1,R2:3]([#1:0])[C,c;r5:4]",          // Rule 12:  furanones
      "[#1:0][C:1][N+:2]([O-:4])=[O:3]>>[C:1]=[N+:2]([O-:4])[O:3][#1:0]",   // Rule 14:  ionic nitro/aci-nitro
      "[#1:0][C:1][N:2](=[O:4])=[O:3]>>[C:1]=[N:2](=[O:4])[O:3][#1:0]",   // Rule 15:  pentavalent nitro/aci-nitro
      "[#1:0][O:1][N:2]=[C:3]>>[O:1]=[N:2][C:3][#1:0]",   // Rule 16:  oxim/nitroso
      "[#1:0][O:1][N:2]=[C:3][C:4]=[C:5][C:6]=[O:7]>>[O:1]=[N:2][c:3]=[c:4][c:5]=[c:6][O:7][#1:0]"   // Rule 17:  oxim/nitroso via phenol*/
   };

   while(_currentLayer < layeredMolecules.layers)
   {
      Molecule mol;
      constructMolecule(mol, _currentLayer, false);
      while(true)
      {
         if(_currentRule == sizeof(reactionSmarts) / sizeof(reactionSmarts[0]))
         {
            _currentRule = 0;
            break;
         }
         char *rule = reactionSmarts[_currentRule++];
         QueryReaction reaction;
         AutoPtr<Scanner> _scanner(new BufferScanner(rule));
         RSmilesLoader loader(*_scanner.get());
         loader.smarts_mode = true;
         loader.loadQueryReaction(reaction);

#if 0
         ReactionTransformation rt;
         //rt.arom_options.method = AromaticityOptions::BASIC;
         //rt.arom_options.dearomatize_check = true;
         //rt.arom_options.unique_dearomatization = false;
         bool res = rt.transform(mol, reaction);

         ++_currentRule;
         if(res)
         {
            //mol.clone(layeredMolecules.asMolecule(), NULL, NULL);
            // add molecule as layer
            return true;
         }
#else
         ReactionProductEnumerator rpe(reaction);
         rpe.addMonomer(0, mol);
         rpe.is_multistep_reaction = false;
         rpe.is_one_tube = true;
         rpe.is_self_react = true;
         rpe.max_deep_level = 1;
         rpe.max_product_count = 10;
         rpe.product_proc = product_proc;
         rpe.userdata = &layeredMolecules;

         int layersBefore = layeredMolecules.layers;
         rpe.buildProducts();
         if(layersBefore < layeredMolecules.layers)
            return false;

#endif
      }
      ++_currentLayer;
   }
   return true;
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
