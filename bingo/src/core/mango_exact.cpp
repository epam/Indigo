/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "core/mango_matchers.h"

#include "base_cpp/scanner.h"
#include "graph/filter.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_substructure_matcher.h"
#include "graph/subgraph_hash.h"
#include "molecule/cmf_loader.h"
#include "core/bingo_error.h"
#include "core/bingo_context.h"
#include "base_cpp/crc32.h"
#include "molecule/elements.h"
#include "core/mango_index.h"

MangoExact::MangoExact (BingoContext &context) :
_context(context) 
{
   _flags = 0;
   _rms_threshold = 0;
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
}

void MangoExact::loadQuery (Scanner &scanner)
{
   MoleculeAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(_query);
   Molecule::checkForConsistency(_query);

   _initQuery(_query);

   calculateHash(_query, _query_hash);
}

void MangoExact::calculateHash (Molecule &mol, Hash &hash)
{
   hash.clear();

   QS_DEF(Molecule, mol_without_h);
   QS_DEF(Array<int>, vertices);
   int i;

   vertices.clear();
   
   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.getAtomNumber(i) != ELEM_H)
         vertices.push(i);

   mol_without_h.makeSubmolecule(mol, vertices, 0);

   // Decompose into connected components
   int n_comp = mol_without_h.countComponents();
   QS_DEF(Molecule, component);

   for (int i = 0; i < n_comp; i++)
   {
      Filter filter(mol_without_h.getDecomposition().ptr(), Filter::EQ, i);
      component.makeSubmolecule(mol_without_h, filter, 0, 0);

      SubgraphHash hh(component);

      hh.cb_vertex_code = _vertex_code;
      hh.max_iterations = (component.edgeCount() + 1) / 2;

      dword component_hash = hh.getHash();

      // Find component hash in all hashes
      bool found = false;

      for (int j = 0; j < hash.size(); j++)
         if (hash[j].hash == component_hash)
         {
            hash[j].count++;
            found = true;
            break;
         }

      if (!found)
      {
         HashElement &hash_element = hash.push();
         hash_element.count = 1;
         hash_element.hash = component_hash;
      }
   }
}

void MangoExact::loadQuery (const Array<char> &buf)
{
   BufferScanner scanner(buf);

   loadQuery(scanner);
}

void MangoExact::loadQuery (const char *buf)
{
   BufferScanner scanner(buf);

   loadQuery(scanner);
}

const MangoExact::Hash& MangoExact::getQueryHash () const
{
   return _query_hash;
}

void MangoExact::setParameters (const char *conditions)
{
   MoleculeExactMatcher::parseConditions(conditions, _flags, _rms_threshold);
}

void MangoExact::loadTarget (Scanner &scanner)
{
   MoleculeAutoLoader loader(scanner);

   loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           ignore_closing_bond_direction_mismatch;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(_target);
   Molecule::checkForConsistency(_target);
   _initTarget(_target, false);
}

void MangoExact::loadTarget (const Array<char> &target_buf)
{
   BufferScanner scanner(target_buf);

   loadTarget(scanner);
}

void MangoExact::loadTarget (const char *target)
{
   BufferScanner scanner(target);

   loadTarget(scanner);
}

bool MangoExact::matchLoadedTarget ()
{
   MoleculeExactMatcher matcher(_query, _target);

   matcher.flags = _flags;
   matcher.rms_threshold = _rms_threshold;

   return matcher.find();
}

void MangoExact::_initQuery (Molecule &query)
{
   int i;
   MoleculeAromatizer::aromatizeBonds(query);

   if (_flags & MoleculeExactMatcher::CONDITION_STEREO)
   {
      for (i = query.edgeBegin(); i != query.edgeEnd(); i = query.edgeNext(i))
         if (query.getEdgeTopology(i) == TOPOLOGY_RING)
            query.cis_trans.setParity(i, 0);
   }
}

void MangoExact::_initTarget (Molecule &target, bool from_database)
{
   if (!from_database)
      MoleculeAromatizer::aromatizeBonds(target);
}

bool MangoExact::matchBinary (Scanner &scanner, Scanner *xyz_scanner)
{
   CmfLoader loader(_context.cmf_dict, scanner);

   loader.loadMolecule(_target);
   if (xyz_scanner != 0)
      loader.loadXyz(*xyz_scanner);

   _initTarget(_target, true);

   MoleculeExactMatcher matcher(_query, _target);

   matcher.flags = _flags;
   matcher.rms_threshold = _rms_threshold;

   return matcher.find();
}

bool MangoExact::matchBinary (const Array<char> &target_buf, const Array<char> *xyz_buf)
{
   BufferScanner scanner(target_buf);

   if (xyz_buf == 0)
      return matchBinary(scanner, 0);

   BufferScanner xyz_scanner(*xyz_buf);
   return matchBinary(scanner, &xyz_scanner);
}

bool MangoExact::needCoords () const
{
   return (_flags & MoleculeExactMatcher::CONDITION_3D) != 0;
}

bool MangoExact::needComponentMatching () const
{
   return (_flags & MoleculeExactMatcher::CONDITION_FRAGMENTS) == 0;
}

bool MangoExact::parse (const char *params)
{
   if (params == 0)
   {
      setParameters("");
      return true;
   }

   QS_DEF(Array<char>, params_upper);

   params_upper.upper(params);

   if (strstr(params_upper.ptr(), "TAU") != NULL)
      return false;

   setParameters(params);
   return true;
}

int MangoExact::_vertex_code (Graph &graph, int vertex_idx, void *context)
{
   Molecule &mol = (Molecule &)graph;

   if (mol.isPseudoAtom(vertex_idx))
      return CRC32::get(mol.getPseudoAtom(vertex_idx));

   if (mol.isRSite(vertex_idx))
      return ELEM_RSITE;

   return mol.getAtomNumber(vertex_idx);
}

int MangoExact::_edge_code (Graph &graph, int edge_idx, void *context)
{
   Molecule &mol = (Molecule &)graph;
   return mol.getBondOrder(edge_idx);
}
