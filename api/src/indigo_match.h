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
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer_matcher.h"
#include "reaction/reaction_substructure_matcher.h"
#include "reaction/reaction.h"
#include "molecule/molecule_neighbourhood_counters.h"

class IndigoQueryMolecule;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

struct IndigoTautomerParams
{
   int conditions;
   bool force_hydrogens;
   bool ring_chain;
};

// Iterator for all possible matches
class IndigoMoleculeSubstructureMatchIter : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatchIter (Molecule &target, QueryMolecule &query,
           Molecule &original_target, bool resonance, bool disable_folding_query_h);

   virtual ~IndigoMoleculeSubstructureMatchIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   int countMatches (int embeddings_limit);

   const char * debugInfo ();

   MoleculeSubstructureMatcher matcher;
   MoleculeSubstructureMatcher::FragmentMatchCache fmcache;

   Molecule &target, &original_target;
   QueryMolecule &query;

   Array<int> mapping;
   int max_embeddings;

private:
   bool _initialized, _found, _need_find;
   int _embedding_index;
};

// Matcher class for matching queries on a specified target molecule
class DLLEXPORT IndigoMoleculeSubstructureMatcher : public IndigoObject
{
public:

   enum
   {
      NORMAL = 1,
      RESONANCE = 2,
      TAUTOMER = 3
   };

   IndigoMoleculeSubstructureMatcher (Molecule &target, int mode);

   virtual ~IndigoMoleculeSubstructureMatcher ();

   IndigoMoleculeSubstructureMatchIter* iterateQueryMatches (IndigoObject &query_object,
      bool embedding_edges_uniqueness, bool find_unique_embeddings, 
      bool for_iteration, int max_embeddings);

   static IndigoMoleculeSubstructureMatcher & cast (IndigoObject &obj);
   void ignoreAtom (int atom_index);
   void unignoreAtom (int atom_index);
   void unignoreAllAtoms ();

   const char * debugInfo ();

   Molecule &target;

   Obj<MoleculeTautomerMatcher> tau_matcher;
   IndigoTautomerParams tau_params;
   bool findTautomerMatch (QueryMolecule &query,
         PtrArray<TautomerRule> &tautomer_rules, Array<int> &mapping_out);

   IndigoMoleculeSubstructureMatchIter * getMatchIterator (Indigo &self, int query,
                    bool for_iteration, int max_embeddings);

   int mode; // NORMAL, TAUTOMER, or RESONANCE
private:

   Molecule _target_arom_h_unfolded, _target_arom;
   Array<int> _mapping_arom_h_unfolded, _mapping_arom, _ignored_atoms;
   bool _arom_h_unfolded_prepared, _arom_prepared, _aromatized;
   MoleculeAtomNeighbourhoodCounters _nei_counters, _nei_counters_h_unfolded;
};

class DLLEXPORT IndigoReactionSubstructureMatcher : public IndigoObject
{
public:

   IndigoReactionSubstructureMatcher (Reaction &target);
   virtual ~IndigoReactionSubstructureMatcher ();

   static IndigoReactionSubstructureMatcher & cast (IndigoObject &obj);

   const char * debugInfo ();

   Reaction &original_target;
   Reaction target;
   bool daylight_aam;

   Obj<ReactionSubstructureMatcher> matcher;
   ObjArray< Array<int> > mappings;
   Array<int> mol_mapping;
};


#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
