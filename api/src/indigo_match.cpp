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

#include "indigo_match.h"
#include "indigo_molecule.h"
#include "reaction/reaction_substructure_matcher.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/reaction_exact_matcher.h"

CEXPORT int indigoExactMatch (int handler1, int handler2)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj1 = self.getObject(handler1);
      IndigoObject &obj2 = self.getObject(handler2);

      if (obj1.isBaseMolecule())
      {
         Molecule &mol1 = obj1.getMolecule();
         Molecule &mol2 = obj2.getMolecule();

         MoleculeExactMatcher matcher(mol1, mol2);
         matcher.flags = MoleculeExactMatcher::CONDITION_ALL;
         if (!matcher.find())
            return 0;
         return 1;
      }
      else if (obj1.isBaseReaction())
      {
         Reaction &rxn1 = obj1.getReaction();
         Reaction &rxn2 = obj2.getReaction();

         ReactionExactMatcher matcher(rxn1, rxn2);
         matcher.flags = MoleculeExactMatcher::CONDITION_ALL;
         if (!matcher.find())
            return 0;
         return 1;
      }

      throw IndigoError("indigoExactMatch(): %s is neither a molecule nor a reaction", obj1.debugInfo());
   }
   INDIGO_END(-1);
}

IndigoMoleculeSubstructureMatch::IndigoMoleculeSubstructureMatch (Molecule &target, QueryMolecule &query) :
   IndigoObject(MOLECULE_SUBSTRUCTURE_MATCH), target(target), query(query)
{
}

IndigoMoleculeSubstructureMatch::~IndigoMoleculeSubstructureMatch ()
{
}

const char * IndigoMoleculeSubstructureMatch::debugInfo ()
{
   return "<molecule substructure match>";
}

CEXPORT int indigoHighlightedTarget (int match)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(match);
      if (obj.type != IndigoObject::MOLECULE_SUBSTRUCTURE_MATCH)
         throw IndigoError("indigoHighlightedTarget(): match must be given, not %s", obj.debugInfo());

      IndigoMoleculeSubstructureMatch &match = (IndigoMoleculeSubstructureMatch &)obj;

      AutoPtr<IndigoMolecule> mol(new IndigoMolecule());

      QS_DEF(Array<int>, mapping);
      mol->mol.clone(match.target, &mapping, 0);

      mol->highlighting.init(mol->mol);

      for (int i = mol->mol.vertexBegin(); i != mol->mol.vertexEnd(); i = mol->mol.vertexNext(i))
      {
         if (match.highlighting.hasVertex(mapping[i]))
            mol->highlighting.onVertex(i);
      }

      for (int i = mol->mol.edgeBegin(); i != mol->mol.edgeEnd(); i = mol->mol.edgeNext(i))
      {
         const Edge &edge = mol->mol.getEdge(i);

         if (match.highlighting.hasEdge(match.target.findEdgeIndex(mapping[edge.beg], mapping[edge.end])))
            mol->highlighting.onEdge(i);
      }

      return self.addObject(mol.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoMapBond (int match, int query_bond)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(match);
      if (obj.type != IndigoObject::MOLECULE_SUBSTRUCTURE_MATCH)
         throw IndigoError("indigoMapBond(): match must be given, not %s", obj.debugInfo());
      IndigoBond &ib = IndigoBond::cast(self.getObject(query_bond));

      IndigoMoleculeSubstructureMatch &match = (IndigoMoleculeSubstructureMatch &)obj;
      const Edge &edge = match.query.getEdge(ib.idx);

      int beg_mapped = match.query_atom_mapping[edge.beg];
      int end_mapped = match.query_atom_mapping[edge.end];
      if (beg_mapped < 0 || end_mapped < 0)
         return 0;

      int idx = match.target.findEdgeIndex(match.query_atom_mapping[edge.beg],
                                           match.query_atom_mapping[edge.end]);

      if (idx == -1)
         throw IndigoError("indigoMapBond(): internal error, idx == -1");

      return self.addObject(new IndigoBond(match.target, idx));
   }
   INDIGO_END(-1)
}

IndigoMoleculeSubstructureMatchIter::IndigoMoleculeSubstructureMatchIter (Molecule &target_,
                                                                          QueryMolecule &query_,
                                                                          Molecule &original_target_) :
        IndigoObject(MOLECULE_SUBSTRUCTURE_MATCH_ITER),
        matcher(target_),
        target(target_),
        original_target(original_target_),
        query(query_)
{
   matcher.setQuery(query);
   matcher.fmcache = &fmcache;
   matcher.highlighting = &highlighting;

   highlighting.init(target);

   _initialized = false;
   _found = false;
   _need_find = true;
   _embedding_index = 0;
}

IndigoMoleculeSubstructureMatchIter::~IndigoMoleculeSubstructureMatchIter ()
{
}

const char * IndigoMoleculeSubstructureMatchIter::debugInfo ()
{
   return "<molecule substructure match iterator>";
}

IndigoObject * IndigoMoleculeSubstructureMatchIter::next ()
{
   if (!hasNext())
      return 0;

   AutoPtr<IndigoMoleculeSubstructureMatch> mptr(
         new IndigoMoleculeSubstructureMatch(original_target, query));

   // Expand mapping to fit possible implicit hydrogens
   mapping.expandFill(target.vertexEnd(), -1);

   mptr->highlighting.init(original_target);

   if (!matcher.getEmbeddingsStorage().isEmpty())
   {
      const GraphEmbeddingsStorage& storage = matcher.getEmbeddingsStorage();
      int count;
      const int *query_mapping = storage.getMappingSub(_embedding_index, count);
      mptr->query_atom_mapping.copy(query_mapping, query.vertexEnd());

      // Initialize highlighting
      int e_count, v_count;
      const int *edges = storage.getEdges(_embedding_index, e_count);
      const int *vertices = storage.getVertices(_embedding_index, v_count);

      highlighting.init(target);
      for (int i = 0; i < v_count; i++)
         highlighting.onVertex(vertices[i]);
      for (int i = 0; i < e_count; i++)
         highlighting.onEdge(edges[i]);
   }
   else
   {
      mptr->query_atom_mapping.copy(matcher.getQueryMapping(), query.vertexEnd());
   }

   for (int v = query.vertexBegin(); v != query.vertexEnd(); v = query.vertexNext(v))
   {
      int mapped = mptr->query_atom_mapping[v];
      if (mapped >= 0)
         mptr->query_atom_mapping[v] = mapping[mapped];
   }
   mptr->highlighting.copy(highlighting, &mapping);

   _need_find = true;
   return mptr.release();
}

bool IndigoMoleculeSubstructureMatchIter::hasNext ()
{
   if (!_need_find)
      return _found;

   if (!_initialized)
   {
      _initialized = true;
      _found = matcher.find();
   }
   else
   {
      _embedding_index++;
      int cur_count = matcher.getEmbeddingsStorage().count();
      if (_embedding_index < cur_count)
         _found = true;
      else
         _found = matcher.findNext();
   }
   if (_embedding_index >= max_embeddings)
      _found = false;

   _need_find = false;
   return _found;
}

struct MatchCountContext
{
   int embeddings_count, max_count;
};

static bool _matchCountEmbeddingsCallback (Graph &sub, Graph &super,
                                           const int *core1, const int *core2, void *context_)
{
   MatchCountContext *context = (MatchCountContext *)context_;
   context->embeddings_count++;
   if (context->embeddings_count >= context->max_count)
      return false;
   return true;
}

int IndigoMoleculeSubstructureMatchIter::countMatches ()
{
   if (max_embeddings <= 0)
      return 0;

   MatchCountContext context;
   context.embeddings_count = 0;
   context.max_count = max_embeddings;

   matcher.find_all_embeddings = true;
   matcher.cb_embedding = _matchCountEmbeddingsCallback;
   matcher.cb_embedding_context = &context;
   matcher.highlighting = 0;
   matcher.find();
   return context.embeddings_count;
}


IndigoMoleculeSubstructureMatcher::IndigoMoleculeSubstructureMatcher (Molecule &target) :
   IndigoObject(MOLECULE_SUBSTRUCTURE_MATCHER),
   target(target)
{
}

IndigoMoleculeSubstructureMatcher::~IndigoMoleculeSubstructureMatcher ()
{
}

const char * IndigoMoleculeSubstructureMatcher::debugInfo ()
{
   return "<molecule substructure matcher>";
}

void IndigoMoleculeSubstructureMatcher::ignoreAtom (int atom_index)
{
   _ignored_atoms.push(atom_index);
}

void IndigoMoleculeSubstructureMatcher::unignoreAllAtoms ()
{
   _ignored_atoms.clear();
}

IndigoMoleculeSubstructureMatchIter*
   IndigoMoleculeSubstructureMatcher::iterateQueryMatches (QueryMolecule &query,
      bool embedding_edges_uniqueness, bool find_unique_embeddings, bool for_iteration, 
      int max_embeddings)
{
   Molecule *target_prepared;
   Array<int> *mapping;
   if (MoleculeSubstructureMatcher::shouldUnfoldTargetHydrogens(query))
   {
      _target_arom_h_unfolded.clone(target, &_mapping_arom_h_unfolded, 0);
      target_prepared = &_target_arom_h_unfolded;
      mapping = &_mapping_arom_h_unfolded;
   }
   else
   {
      _target_arom.clone(target, &_mapping_arom, 0);
      target_prepared = &_target_arom;
      mapping = &_mapping_arom;
   }
   if (!target.isAromatized() && !target_prepared->isAromatized())
      target_prepared->aromatize();

   AutoPtr<IndigoMoleculeSubstructureMatchIter>
      iter(new IndigoMoleculeSubstructureMatchIter(*target_prepared, query, target));

   iter->matcher.find_unique_embeddings = find_unique_embeddings;
   iter->matcher.find_unique_by_edges = embedding_edges_uniqueness;
   iter->matcher.save_for_iteration = for_iteration;

   for (int i = 0; i < _ignored_atoms.size(); i++)
      iter->matcher.ignoreTargetAtom(_ignored_atoms[i]);

   iter->matcher.restore_unfolded_h = false;
   iter->mapping.copy(*mapping);
   iter->max_embeddings = max_embeddings;

   return iter.release();
}

CEXPORT int indigoSubstructureMatcher (int target, const char *mode)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(target).getMolecule();

      AutoPtr<IndigoMoleculeSubstructureMatcher> matcher(
         new IndigoMoleculeSubstructureMatcher(mol));

      return self.addObject(matcher.release());
   }
   INDIGO_END(-1)
}

static IndigoMoleculeSubstructureMatcher& getMatcher (int target_matcher, Indigo &self)
{
   IndigoObject &obj = self.getObject(target_matcher);
   if (obj.type != IndigoObject::MOLECULE_SUBSTRUCTURE_MATCHER)
      throw IndigoError("%s is not a matcher object", obj.debugInfo());

   return (IndigoMoleculeSubstructureMatcher &)obj;
}

static IndigoMoleculeSubstructureMatchIter* getMatchIterator (int target_matcher, 
      int query, Indigo &self, bool for_iteration, int max_embeddings)
{
   IndigoMoleculeSubstructureMatcher &matcher = getMatcher(target_matcher, self);

   QueryMolecule &querymol = self.getObject(query).getQueryMolecule();
   return matcher.iterateQueryMatches(querymol, self.embedding_edges_uniqueness, 
      self.find_unique_embeddings, for_iteration, max_embeddings);
}

CEXPORT int indigoIgnoreAtom (int target_matcher, int atom_index)
{
   INDIGO_BEGIN
   {
      IndigoMoleculeSubstructureMatcher &matcher = getMatcher(target_matcher, self);
      matcher.ignoreAtom(atom_index);
      return 0;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoUnignoreAllAtoms (int target_matcher)
{
   INDIGO_BEGIN
   {
      IndigoMoleculeSubstructureMatcher &matcher = getMatcher(target_matcher, self);
      matcher.unignoreAllAtoms();
      return 0;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoMatch (int target_matcher, int query)
{
   INDIGO_BEGIN
   {
      AutoPtr<IndigoMoleculeSubstructureMatchIter>
         match_iter(getMatchIterator(target_matcher, query, self, false, 1));

      match_iter->matcher.find_unique_embeddings = false;

      if (!match_iter->hasNext())
         return 0;
      return self.addObject(match_iter->next());
   }
   INDIGO_END(-1)
}

int indigoCountMatches (int target_matcher, int query)
{
   INDIGO_BEGIN
   {
      AutoPtr<IndigoMoleculeSubstructureMatchIter>
         match_iter(getMatchIterator(target_matcher, query, self, false, self.max_embeddings));

      return match_iter->countMatches();
   }
   INDIGO_END(-1)
}

int indigoIterateMatches (int target_matcher, int query)
{
   INDIGO_BEGIN
   {
      AutoPtr<IndigoMoleculeSubstructureMatchIter>
         match_iter(getMatchIterator(target_matcher, query, self, true, self.max_embeddings));

      return self.addObject(match_iter.release());
   }
   INDIGO_END(-1)
}
