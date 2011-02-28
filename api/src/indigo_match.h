/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#ifndef __indigo_match__
#define __indigo_match__

#include "indigo_internal.h"
#include "graph/graph_highlighting.h"
#include "molecule/molecule_substructure_matcher.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

// Query to the target match instance
class DLLEXPORT IndigoMoleculeSubstructureMatch : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatch (Molecule &target, QueryMolecule &query);
   virtual ~IndigoMoleculeSubstructureMatch ();

   const char * debugInfo ();

   GraphHighlighting highlighting;
   Array<int> query_atom_mapping;
   Molecule &target;
   QueryMolecule &query;
};

// Iterator for all possible matches
class IndigoMoleculeSubstructureMatchIter : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatchIter (Molecule &target, QueryMolecule &query, Molecule &original_target);

   virtual ~IndigoMoleculeSubstructureMatchIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   int countMatches ();

   const char * debugInfo ();

   MoleculeSubstructureMatcher matcher;
   MoleculeSubstructureMatcher::FragmentMatchCache fmcache;
   GraphHighlighting highlighting;
   Molecule &target, &original_target;
   QueryMolecule &query;
   int max_embeddings;

   Array<int> mapping;

private:
   bool _initialized, _found, _need_find;
   int _embedding_index;
};

// Matcher class for matching queries on a specified target molecule
class DLLEXPORT IndigoMoleculeSubstructureMatcher : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatcher (Molecule &target);

   virtual ~IndigoMoleculeSubstructureMatcher ();

   IndigoMoleculeSubstructureMatchIter* iterateQueryMatches (QueryMolecule &query,
      bool embedding_edges_uniqueness, bool find_unique_embeddings, 
      bool for_iteration, int max_embeddings);

   void ignoreAtom (int atom_index);
   void unignoreAllAtoms ();

   const char * debugInfo ();

   Molecule &target;

private:
   Molecule _target_arom_h_unfolded, _target_arom;
   Array<int> _mapping_arom_h_unfolded, _mapping_arom, _ignored_atoms;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
