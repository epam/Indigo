/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "layout/layout_pattern.h"

#include "base_cpp/scanner.h"
#include "base_cpp/os_sync_wrapper.h"
#include "math/algebra.h"
#include "graph/graph.h"
#include "molecule/query_molecule.h"
#include "molecule/molfile_loader.h"
#include "molecule/molecule_substructure_matcher.h"
#include "layout/molecule_layout_graph.h"

#include <memory>
#include <vector>

#include "templates/layout_patterns.inc"

using namespace indigo;
using namespace std;

class DLLEXPORT PatternLayout
{      
public:
   QueryMolecule query_molecule;
   MoleculeLayoutGraph layout_graph;
};

static vector<unique_ptr<PatternLayout>> _patterns;
static OsLock _patterns_lock;

bool PatternLayoutFinder::tryToFindPattern (MoleculeLayoutGraph &layout_graph)
{
   _initPatterns();

   layout_graph.calcMorganCode();

   for (auto & pattern : _patterns)
   {
      MoleculeLayoutGraph &plg = pattern->layout_graph;

      // Compare morgan code and graph size
      if (plg.getMorganCode() != layout_graph.getMorganCode())
         continue;

      if (plg.vertexCount() != layout_graph.vertexCount())
         continue;

      if (plg.edgeCount() != layout_graph.edgeCount())
         continue;

      OsLocker locker(_patterns_lock);

      // Check if substructure matching found
      EmbeddingEnumerator ee(layout_graph);

      ee.setSubgraph(pattern->query_molecule);
      ee.cb_match_edge = _matchPatternBond;

      if (!ee.process())
      {
         // Embedding has been found -> copy coordinates
         const int *mapping = ee.getSubgraphMapping();
         QueryMolecule &qm = pattern->query_molecule;
         for (int v = qm.vertexBegin(); v != qm.vertexEnd(); v = qm.vertexNext(v))
            layout_graph.getPos(mapping[v]) = qm.getAtomXyz(v).projectZ();

         layout_graph.assignFirstVertex(layout_graph.vertexBegin());

         return true;
      }
   }

   return false;
}

void PatternLayoutFinder::_initPatterns ()
{
   if (!_patterns.empty())
      return;

   OsLocker locker(_patterns_lock);
   
   if (!_patterns.empty())
      return;

   _patterns.reserve(NELEM(layout_templates));
   for (const char *tpl : layout_templates)
   {
      _patterns.emplace_back(new PatternLayout);
      auto &pattern = _patterns.back();

      BufferScanner scanner(tpl);
      MolfileLoader loader(scanner);

      loader.loadQueryMolecule(pattern->query_molecule);
      pattern->layout_graph.makeOnGraph(pattern->query_molecule);

      // Copy coordinates
      QueryMolecule &qm = pattern->query_molecule;
      for (int v = qm.vertexBegin(); v != qm.vertexEnd(); v = qm.vertexNext(v))
         pattern->layout_graph.getPos(v) = qm.getAtomXyz(v).projectZ();

      pattern->layout_graph.calcMorganCode();
   }
}

bool PatternLayoutFinder::_matchPatternBond (Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void *userdata)
{
   MoleculeLayoutGraph &target = (MoleculeLayoutGraph &)supergraph;
   BaseMolecule *mol = (BaseMolecule *)target.getMolecule();

   int layout_idx = target.getLayoutEdge(super_idx).ext_idx;

   QueryMolecule &qmol = (QueryMolecule &)subgraph;
   QueryMolecule::Bond &sub_bond = qmol.getBond(layout_idx);

   if (!MoleculeSubstructureMatcher::matchQueryBond(&sub_bond, *mol, sub_idx, super_idx, nullptr, 0xFFFFFFFF))
      return false;

   return true;
}
