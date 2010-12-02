/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __molecule_substructure_matcher__
#define __molecule_substructure_matcher__

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "graph/embedding_enumerator.h"
#include "graph/embeddings_storage.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj.h"

namespace indigo {

class Molecule;
class GraphHighlighting;
class AromaticityMatcher;
struct Vec3f;
class GraphVertexEquivalence;
class MoleculeAtomNeighbourhoodCounters;
class MoleculePiSystemsMatcher;

class MoleculeSubstructureMatcher
{
public:
   enum
   {
      AFFINE = 1,
      CONFORMATION = 2
   };

   typedef ObjArray< RedBlackStringMap<int> > FragmentMatchCache;

   DLLEXPORT MoleculeSubstructureMatcher (BaseMolecule &target);
   DLLEXPORT ~MoleculeSubstructureMatcher ();

   DLLEXPORT void setQuery (QueryMolecule &query);
   DLLEXPORT QueryMolecule & getQuery ();

   // Set vertex neibourhood counters for effective matching
   DLLEXPORT void setNeiCounters (const MoleculeAtomNeighbourhoodCounters *query_counters, 
                        const MoleculeAtomNeighbourhoodCounters *target_counters);

   bool use_aromaticity_matcher;
   bool use_pi_systems_matcher;
   GraphVertexEquivalence *vertex_equivalence_handler;

   FragmentMatchCache *fmcache;

   bool disable_unfolding_implicit_h;

   int   match_3d;       // 0 or AFFINE or CONFORMATION
   float rms_threshold;  // for AFFINE and CONFORMATION

   GraphHighlighting *highlighting;

   DLLEXPORT void ignoreQueryAtom (int idx);
   DLLEXPORT void ignoreTargetAtom (int idx);
   DLLEXPORT bool fix (int query_atom_idx, int target_atom_idx);

   // for finding the first embedding
   DLLEXPORT bool find ();
   DLLEXPORT const int * getQueryMapping ();
   DLLEXPORT const int * getTargetMapping ();

   // for finding all embeddings
   bool find_all_embeddings; // false by default
   bool find_unique_embeddings; // true if to find only unique embeddings. false by default
   void (*cb_embedding) (Graph &sub, Graph &super, const int *core1, const int *core2, void *context);
   void  *cb_embedding_context;

   DLLEXPORT static bool needCoords (int match_3d, QueryMolecule &query);

   DLLEXPORT static void removeAtom (Graph &subgraph, int sub_idx, AromaticityMatcher *am);

   DLLEXPORT static void addBond (Graph &subgraph, Graph &supergraph,
      int sub_idx, int super_idx, AromaticityMatcher *am);

   DLLEXPORT static void markIgnoredHydrogens (BaseMolecule &mol, int *arr, int value_keep, int value_ignore);
   DLLEXPORT static void markIgnoredQueryHydrogens (QueryMolecule &mol, int *arr, int value_keep, int value_ignore);

   DLLEXPORT static void getAtomPos (Graph &graph, int vertex_idx, Vec3f &pos);

   // Flags for matchQueryAtom and matchQueryBond (by default all flags should be set)
   enum
   {
      // When some flags are not set it means that checking should be done without 
      // such conditions but as precise as possible because such conditions will be 
      // checked later. 
      // For example, MATCH_ATOM_CHARGE isn't set.
      // It means that 
      // (1) match should return true if it does so for some charge with 
      //     MATCH_ATOM_CHARGE set to true. 
      // (2) if for every charge match returns false, then without MATCH_ATOM_CHARGE 
      //     match should return false. 
      // It it not easy to implement point (2) exactly, but match algorithm should 
      // always satisfy point (1). So it have to satisfy point (2) as precise as 
      // possible.
      MATCH_ATOM_CHARGE = 0x01,
      MATCH_ATOM_VALENCE = 0x02,
      MATCH_BOND_TYPE = 0x04,

      // To satisfy point (2) (not precisely) following flag is introduced.
      // It shows what value should be returned if condition is disabled.
      // When 'not' operation is applied then such flag is inverted. So 
      // points (1) is satisfied and point (2) generally satisfied too (not always).
      MATCH_DISABLED_AS_TRUE = 0x1000, 
   };

   DLLEXPORT static bool matchQueryAtom (QueryMolecule::Atom *query, BaseMolecule &target,
                  int super_idx, FragmentMatchCache *fmcache, dword flags);

   DLLEXPORT static bool matchQueryBond (QueryMolecule::Bond *query,
             BaseMolecule &target, int sub_idx, int super_idx, AromaticityMatcher *am, dword flags);

   DLLEXPORT static void makeTransposition (BaseMolecule &mol, Array<int> &transposition);

   DEF_ERROR("molecule substructure matcher");

protected:
   
   struct MarkushContext
   {
      explicit MarkushContext (QueryMolecule &query_, BaseMolecule &target_);

      TL_CP_DECL(QueryMolecule, query);
      TL_CP_DECL(Array<int>, query_marking);
      TL_CP_DECL(Array<int>, sites);
      int depth;
   };

   static bool _matchAtoms (Graph &subgraph, Graph &supergraph,
                            const int *core_sub, int sub_idx, int super_idx, void *userdata);

   static bool _matchBonds (Graph &subgraph, Graph &supergraph,
                            int sub_idx, int super_idx, void *userdata);

   static void _removeAtom (Graph &subgraph, int sub_idx, void *userdata);

   static void _addBond (Graph &subgraph, Graph &supergraph,
                         int sub_idx, int super_idx, void *userdata);

   static int _embedding (Graph &subgraph, Graph &supergraph,
                          int *core_sub, int *core_super, void *userdata);

   int _embedding_common (int *core_sub, int *core_super);
   int _embedding_markush (int *core_sub, int *core_super);

   static bool _canUseEquivalenceHeuristic (QueryMolecule &query);
   static bool _isSingleBond (Graph &graph, int edge_idx);

   static bool _shouldUnfoldTargetHydrogens (QueryMolecule &query);
   static bool _shouldUnfoldTargetHydrogens_A (QueryMolecule::Atom *atom);

   static int _countSubstituents (Molecule &mol, int idx);
   
   bool _checkRGroupConditions ();
   bool _attachRGroupAndContinue (int *core1, int *core2,
      QueryMolecule *fragment, bool two_attachment_points,
      int att_idx1, int att_idx2, int rgroup_idx);

   BaseMolecule &_target;
   QueryMolecule *_query;

   const MoleculeAtomNeighbourhoodCounters 
      *_query_nei_counters, *_target_nei_counters;

   Obj<EmbeddingEnumerator> _ee;
   AromaticityMatcher  *_am;
   MoleculePiSystemsMatcher *_pi_systems_matcher;

   AutoPtr<MarkushContext> _markush;

   // Because storage can be big it is not stored into TL_CP_***
   // It can be stored as TL_CP_*** if memory allocations will 
   // be critical
   Obj<GraphEmbeddingsStorage> _embeddings_storage;

   bool _did_h_unfold; // implicit target hydrogens unfolded

   TL_CP_DECL(Array<int>, _3d_constrained_atoms);
   TL_CP_DECL(Array<int>, _unfolded_target_h);
   TL_CP_DECL(Array<int>, _used_target_h);

   static int _compare_degree_asc (BaseMolecule &mol, int i1, int i2);
   static int _compare_frequency_base (BaseMolecule &mol, int i1, int i2);
   static int _compare_frequency_asc (BaseMolecule &mol, int i1, int i2);
   static int _compare_in_loop (BaseMolecule &mol, int i1, int i2);
   static int _compare (int &i1, int &i2, void *context);
};

}

#endif
