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

#include "indigo_deconvolution.h"
#include "indigo_array.h"
#include "indigo_molecule.h"
#include "molecule/query_molecule.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/molecule_rgroups.h"
#include "molecule/molecule_scaffold_detection.h"
#include "molecule/max_common_submolecule.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_arom.h"
#include "molecule/molfile_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule_substructure_matcher.h"

IndigoDeconvolution::IndigoDeconvolution(bool aromatize):
IndigoObject(IndigoObject::DECONVOLUTION),
cbEmbedding(0),
embeddingUserdata(0),
_aromatic(aromatize)
{
}

void IndigoDeconvolution::makeRGroups (QueryMolecule& scaffold) {

   /*
    * Clone molecules
    */
   _scaffold.clone_KeepIndices(scaffold, 0);
   _fullScaffold.clone_KeepIndices(scaffold, 0);

   if(_aromatic) {
      QueryMoleculeAromatizer::aromatizeBonds(_scaffold);
      QueryMoleculeAromatizer::aromatizeBonds(_fullScaffold);
   }
   
   for (int mol_idx = 0; mol_idx < _deconvolutionItems.size(); ++mol_idx) {
      Item& elem = _deconvolutionItems[mol_idx];
      _makeRGroup(elem);
   }

}


void IndigoDeconvolution::_makeRGroup(Item& elem) {
   
   Molecule& mol_in = elem.mol_in;
   Molecule& rgroup_out = elem.rgroup_mol;
   Molecule& mol_out = elem.mol_out;
   Molecule& mol_scaffold = elem.mol_scaffold;
   
   if (mol_in.vertexCount() == 0)
      return;

   mol_out.clone_KeepIndices(mol_in, 0);

   EmbContext emb_context;
   emb_context.lastMapping.clear();

   if(_aromatic) 
      MoleculeAromatizer::aromatizeBonds(mol_out);

   AutoPtr<AromaticityMatcher> am;

   if (_aromatic && AromaticityMatcher::isNecessary(_scaffold))
   {
      am.reset(new AromaticityMatcher(_scaffold, mol_out));
      emb_context.am = am.get();
   }
   else
      emb_context.am = 0;


   EmbeddingEnumerator emb_enum(mol_out);
   QS_DEF(MoleculeSubstructureMatcher::FragmentMatchCache, fmcache);

   fmcache.clear();
   emb_context.fmcache = &fmcache;

   /*
    * Set options
    */
   emb_enum.setSubgraph(_scaffold);
   emb_enum.cb_embedding = _rGroupsEmbedding;
   emb_enum.cb_match_edge = _matchBonds;
   emb_enum.cb_match_vertex = _matchAtoms;
   emb_enum.cb_vertex_remove = _removeAtom;
   emb_enum.cb_edge_add = _addBond;
   emb_enum.userdata = &emb_context;
   /*
    * Find subgraph
    */
   emb_enum.process();

   if(emb_context.lastMapping.size() == 0)
      throw Error("no embeddings obtained");

   mol_out.highlightSubmolecule(_scaffold, emb_context.lastMapping.ptr(), true);

   _createRgroups(mol_out, rgroup_out, emb_context);

   Filter sub_filter(emb_context.visitedAtoms.ptr(), Filter::EQ, 1);
   mol_scaffold.makeSubmolecule(mol_out, sub_filter, 0, 0);
   mol_scaffold.unhighlightAll();

}


int IndigoDeconvolution::_rGroupsEmbedding(Graph &graph1, Graph &graph2, int *map, int *inv_map, void *userdata) {

   QS_DEF(Array<int>, queue);
   QS_DEF(Array<int>, queue_markers);
   int result = 0;
   int n_rgroups = 0;

   EmbContext& emb_context = *(EmbContext*)userdata;

   emb_context.lastMapping.copy(map, graph1.vertexEnd());
   emb_context.lastInvMapping.copy(inv_map, graph2.vertexEnd());

   Array<int>& visited_atoms = emb_context.visitedAtoms;

   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;

   visited_atoms.clear_resize(graph2.vertexEnd());
   visited_atoms.zerofill();

   attachment_index.clear();
   attachment_order.clear();

   attachment_index.push();
   attachment_order.push();

   for (int atom_idx = graph1.vertexBegin(); atom_idx < graph1.vertexEnd(); atom_idx = graph1.vertexNext(atom_idx)) {
      int start_idx = map[atom_idx];

      if (visited_atoms[start_idx] > 0)
         continue;

      const Vertex &start_vertex = graph2.getVertex(start_idx);

      for (int cc = start_vertex.neiBegin(); cc != start_vertex.neiEnd(); cc = start_vertex.neiNext(cc)) {
         int cc_start_idx = start_vertex.neiVertex(cc);

         if (inv_map[cc_start_idx] >= 0 || visited_atoms[cc_start_idx] > 1)
            continue;

         int top = 1, bottom = 0;

         queue.clear();
         queue_markers.clear_resize(graph2.vertexEnd());
         queue_markers.zerofill();
         queue.push(cc_start_idx);
         queue_markers[cc_start_idx] = 1;

         while (top != bottom) {

            int cur_idx = queue[bottom];
            const Vertex &vertex = graph2.getVertex(cur_idx);

            for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i)) {
               int nei_idx = vertex.neiVertex(i);

               if (visited_atoms[nei_idx] > 1)
                  continue;

               if (queue_markers[nei_idx] != 0)
                  continue;

               if (inv_map[nei_idx] >= 0) {
                  attachment_index[n_rgroups].push(cur_idx);
                  attachment_order[n_rgroups].push(nei_idx);
               } else {
                  queue.push(nei_idx);
                  queue_markers[nei_idx] = 1;
                  ++top;
               }
            }

            visited_atoms[cur_idx] = n_rgroups + SHIFT_IDX;

            ++bottom;
         }

         ++n_rgroups;
         attachment_index.push();
         attachment_order.push();

      }

      visited_atoms[start_idx] = 1;

   }

   return result;
}

void IndigoDeconvolution::_createRgroups(Molecule& mol_set, Molecule& r_molecule, EmbContext& emb_context) {

   QS_DEF(Array<int>, inv_scaf_map);
   QS_DEF(Array<int>, rg_mapping);

   Array<int>& visited_atoms = emb_context.visitedAtoms;

   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;

   int n_rgroups = emb_context.getRgroupNumber();
   /*
    * Create copy
    */
   Filter sub_filter(visited_atoms.ptr(), Filter::EQ, 1);
   r_molecule.makeSubmolecule(mol_set, sub_filter, 0, &inv_scaf_map);
   r_molecule.unhighlightAll();

   /*
    * Add rgroups (if not exist) to the full scaffold
    */
   _addCompleteRGroup(mol_set, emb_context);

   for (int rg_idx = 0; rg_idx < n_rgroups; ++rg_idx) {
      Array<int>& att_idx = attachment_index[rg_idx];
      Array<int>& att_ord = attachment_order[rg_idx];

      /*
       * Add new atom r-site
       */
      int new_atom_idx = r_molecule.addAtom(ELEM_RSITE);
      Vec3f& atom_xyz = mol_set.getAtomXyz(att_idx[0]);
      /*
       * Copy coordinates
       */
      r_molecule.setAtomXyz(new_atom_idx, atom_xyz.x, atom_xyz.y, atom_xyz.z);

      /*
       * Add Rsites
       */
       r_molecule.allowRGroupOnRSite(new_atom_idx, rg_idx + 1);
      
      /*
       * Add all bonds
       */
      for(int point_att = 0; point_att < att_idx.size(); ++point_att) {
         int att_order = att_ord[point_att];

         if(r_molecule.findEdgeIndex(new_atom_idx, inv_scaf_map[att_order]) == -1) {
            int edge_idx = mol_set.findEdgeIndex(att_order, att_idx[point_att]);
            if(edge_idx == -1)
               throw Error("inner error while converting molecule to query");
            r_molecule.addBond(new_atom_idx, inv_scaf_map[att_order], mol_set.getBondOrder(edge_idx));
         }

         r_molecule.setRSiteAttachmentOrder(new_atom_idx, inv_scaf_map[att_order], point_att);
      }
   }

   /*
    * Add all Rgroups
    */
   MoleculeRGroups & mol_rgroups = r_molecule.rgroups;
   
   for (int i = r_molecule.vertexBegin(); i != r_molecule.vertexEnd(); i = r_molecule.vertexNext(i)) {
      if (!r_molecule.isRSite(i))
         continue;
      
      int rg_idx = r_molecule.getSingleAllowedRGroup(i);
      RGroup & r_group = mol_rgroups.getRGroup(rg_idx);
      /*
       * Get internal rg_idx
       */
      --rg_idx;
      Molecule & fragment = r_group.fragments.at(r_group.fragments.add(new Molecule()))->asMolecule();

      Filter sub_filter_fr(visited_atoms.ptr(), Filter::EQ, rg_idx + SHIFT_IDX);
      fragment.makeSubmolecule(mol_set, sub_filter_fr, 0, &rg_mapping);

      for (int att_idx = 0; att_idx < attachment_index[rg_idx].size(); ++att_idx) {
         fragment.addAttachmentPoint(att_idx + 1, rg_mapping.at(attachment_index[rg_idx][att_idx]));
      }

   }
}

IndigoDeconvolutionElem::IndigoDeconvolutionElem (IndigoDeconvolution::Item &item_, int index) :
IndigoObject(DECONVOLUTION_ELEM), item(item_), idx(index)
{
}

int IndigoDeconvolutionElem::getIndex () {
   return idx;
}

IndigoDeconvolutionElem::~IndigoDeconvolutionElem ()
{
}

IndigoDeconvolutionIter::IndigoDeconvolutionIter(ObjArray<IndigoDeconvolution::Item>& items) :
IndigoObject(DECONVOLUTION_ITER),
_items(items)
{
   _index = -1;
}

IndigoObject * IndigoDeconvolutionIter::next () {
   if (!hasNext())
      return 0;

   _index++;
   return new IndigoDeconvolutionElem(_items[_index], _index);
}

bool IndigoDeconvolutionIter::hasNext () {
   return _index + 1 < _items.size();
}

IndigoDeconvolutionIter::~IndigoDeconvolutionIter ()
{
}

void IndigoDeconvolution::addMolecule(Molecule& mol, RedBlackStringObjMap< Array<char> >* props) {
   Item & item = _deconvolutionItems.push(mol);
   int i;
   
   if (props != 0)
   {
      for (i = props->begin(); i != props->end(); i = props->next(i))
         item.properties.value(item.properties.insert(props->key(i))).copy(props->value(i));
   }
}

QueryMolecule& IndigoDeconvolution::getDecomposedScaffold() {
   return _fullScaffold;
}

ObjArray<IndigoDeconvolution::Item>& IndigoDeconvolution::getItems ()
{
   return _deconvolutionItems;
}

IndigoDeconvolution::~IndigoDeconvolution() {
}

void IndigoDeconvolution::EmbContext::renumber(Array<int>& map, Array<int>& inv_map) {
   QS_DEF(Array<int>, tmp_array);
   for(int i = 0; i < lastMapping.size(); ++i) {
      int old_value = lastMapping[i];
      if(old_value >= 0) {
         lastMapping[i] = inv_map[old_value];
      }
   }
   tmp_array.resize(map.size());
   tmp_array.zerofill();
   for(int i = 0; i < inv_map.size(); ++i) {
      if(inv_map[i] >= 0)
         tmp_array.at(inv_map[i]) = visitedAtoms[i];
   }
   visitedAtoms.copy(tmp_array);
   for(int i = 0; i < attachmentIndex.size(); ++i) {
      for(int j = 0; j < attachmentIndex[i].size(); ++j) {
         attachmentIndex[i][j] = inv_map.at(attachmentIndex[i][j]);
      }
   }
   for(int i = 0; i < attachmentOrder.size(); ++i) {
      for(int j = 0; j < attachmentOrder[i].size(); ++j) {
         attachmentOrder[i][j] = inv_map.at(attachmentOrder[i][j]);
      }
   }
}


/*
 * Add rgroup if not exists
 */
void IndigoDeconvolution::_addCompleteRGroup(Molecule& mol_set, EmbContext& emb_context) {
   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;
   Array<int>& map = emb_context.lastInvMapping;

   int n_rgroups = emb_context.getRgroupNumber();
   /*
    * Search for existing rgroups
    */
   QS_DEF(RedBlackStringMap<int>, match_rgroups);
   match_rgroups.clear();

   QS_DEF(RedBlackSet<int>, str_keys);
   QS_DEF(Array<char>, str_key);
   ArrayOutput str_out(str_key);
   
   int new_rg_idx = 0;
   /*
    * Create match strings
    */
   int vert_idx = _fullScaffold.vertexBegin();
   for (; vert_idx != _fullScaffold.vertexEnd(); vert_idx = _fullScaffold.vertexNext(vert_idx)) {
      if (!_fullScaffold.isRSite(vert_idx))
         continue;
      int cur_rg_idx = _fullScaffold.getSingleAllowedRGroup(vert_idx);

      if(new_rg_idx < cur_rg_idx)
         new_rg_idx = cur_rg_idx;

      const Vertex& vert = _fullScaffold.getVertex(vert_idx);
      /*
       * Strings contain only attachment indexes
       */
      str_keys.clear();
      for (int nei_idx = vert.neiBegin(); nei_idx != vert.neiEnd(); nei_idx = vert.neiNext(nei_idx)) {
         int nei_vert = vert.neiVertex(nei_idx);
         str_keys.find_or_insert(nei_vert);
      }
      /*
       * Call sort and create string
       */
      str_out.clear();
      for (int key_idx = str_keys.begin(); key_idx != str_keys.end(); key_idx = str_keys.next(key_idx)) {
         str_out.printf("%d;", str_keys.key(key_idx));
      }
      str_out.writeChar(0);
      /*
       * Insert match string
       */
      if(match_rgroups.find(str_key.ptr())) {
         match_rgroups.at(str_key.ptr()) += 1;
      } else {
         match_rgroups.insert(str_key.ptr(), 1);
      }
   }
//   printf("***************\nmatch keys\n");
//   for (int i = match_rgroups.begin(); i != match_rgroups.end(); i = match_rgroups.next(i)) {
//      printf("key = %s val = %d\n", match_rgroups.key(i), match_rgroups.value(i));
//   }
//   FileOutput fo("res/fullscaf.mol");
//   MolfileSaver ms(fo);
//   ms.saveQueryMolecule(_fullScaffold);
   
   /*
    * Loop through all rgroups and seek for matchings
    */
   bool match_not_found;
   for (int rg_idx = 0; rg_idx < n_rgroups; ++rg_idx) {
      Array<int>& att_idx = attachment_index[rg_idx];
      Array<int>& att_ord = attachment_order[rg_idx];
      /*
       * Create match string
       */
      str_keys.clear();
      for (int a_x = 0; a_x < att_ord.size(); ++a_x) {
         str_keys.find_or_insert(map.at(att_ord[a_x]));
      }
      /*
       * Call sort and create string
       */
      str_out.clear();
      for (int key_idx = str_keys.begin(); key_idx != str_keys.end(); key_idx = str_keys.next(key_idx)) {
         str_out.printf("%d;", str_keys.key(key_idx));
      }
      str_out.writeChar(0);
      /*
       * Search for matches
       */
      match_not_found = false;
      if(match_rgroups.find(str_key.ptr())) {
         int& match_count = match_rgroups.at(str_key.ptr());
         /*
          * Remove matches
          */
         --match_count;
         if(match_count < 0) {
            match_not_found = true;
         }
      } else {
         match_not_found = true;
      }
      /*
       * Add rgroup to full scaffold
       */
      if(match_not_found) {
         ++new_rg_idx;
         _addFullRGroup(att_ord, att_idx, mol_set, map, new_rg_idx);
      }
   }
}

/*
 */
void IndigoDeconvolution::_addFullRGroup(Array<int>& att_order, Array<int>& att_idx, Molecule& qmol, Array<int>& map, int new_rg_idx) {
   /*
    * If not found then add Rsite to the full scaffold
    */
   int new_atom_idx = _fullScaffold.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));

   Vec3f& atom_xyz = qmol.getAtomXyz(att_idx[0]);
   /*
    * Copy coordinates
    */
   _fullScaffold.setAtomXyz(new_atom_idx, atom_xyz.x, atom_xyz.y, atom_xyz.z);

   /*
    * Add Rsites
    */
   _fullScaffold.allowRGroupOnRSite(new_atom_idx, new_rg_idx);

   /*
    * Add all bonds
    */
   for (int point_att = 0; point_att < att_idx.size(); ++point_att) {
      int att_ord = att_order[point_att];
      int att_self = map[att_ord];

      if (_fullScaffold.findEdgeIndex(new_atom_idx, att_self) == -1) {
         int edge_idx = qmol.findEdgeIndex(att_ord, att_idx[point_att]);
         if (edge_idx == -1)
            throw Error("internal error while converting molecule to query");
         _fullScaffold.addBond(new_atom_idx, att_self, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
      }
   }
}


CEXPORT int indigoDecomposeMolecules (int scaffold, int structures) {
   INDIGO_BEGIN
   {
      IndigoArray& mol_array = IndigoArray::cast(self.getObject(structures));

      AutoPtr<IndigoDeconvolution> deco(new IndigoDeconvolution(self.deconvolution_aromatization));
      int i;

      for (i = 0; i < mol_array.objects.size(); i++)
      {
         IndigoObject &obj = *mol_array.objects[i];

         deco->addMolecule(obj.getMolecule(), obj.getProperties());
      }

      QueryMolecule& scaf = self.getObject(scaffold).getQueryMolecule();

      deco->makeRGroups(scaf);
      return self.addObject(deco.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateDecomposedMolecules (int decomp)
{
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION)
         throw IndigoError("indigoIterateDecomposedMolecules(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolution& deco = (IndigoDeconvolution &)obj;

      return self.addObject(new IndigoDeconvolutionIter(deco.getItems()));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDecomposedMoleculeScaffold (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      AutoPtr<IndigoObject> mol_ptr(new IndigoQueryMolecule());

      if (obj.type == IndigoObject::DECONVOLUTION) {
         IndigoDeconvolution& deco = (IndigoDeconvolution &) obj;
         mol_ptr.reset(new IndigoQueryMolecule());
         IndigoQueryMolecule& qmol = (IndigoQueryMolecule&) mol_ptr.ref();
         qmol.qmol.clone(deco.getDecomposedScaffold(), 0, 0);
      } else if (obj.type == IndigoObject::DECONVOLUTION_ELEM) {
         IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;
         mol_ptr.reset(new IndigoMolecule());
         IndigoMolecule& mol = (IndigoMolecule&) mol_ptr.ref();

         mol.mol.clone(elem.item.mol_scaffold, 0, 0);
      } else {
         throw IndigoError("indigoDecomposedMoleculeScaffold(): not applicable to %s", obj.debugInfo());
      }

      return self.addObject(mol_ptr.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDecomposedMoleculeHighlighted (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeHighlighted(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;

      AutoPtr<IndigoMolecule> mol;
      mol.create();

      mol->mol.clone_KeepIndices(elem.item.mol_out, 0);
      mol->copyProperties(elem.item.properties);

      return self.addObject(mol.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDecomposedMoleculeSubstituents (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeSubstituents(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;

      Molecule* qmol = &elem.item.rgroup_mol;
      return self.addObject(new IndigoRGroupsIter(qmol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDecomposedMoleculeWithRGroups (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeWithRGroups(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;

      AutoPtr<IndigoMolecule> qmol_ptr(new IndigoMolecule());
      qmol_ptr->mol.clone(elem.item.rgroup_mol,0, 0);
      qmol_ptr->copyProperties(elem.item.properties);

      return self.addObject(qmol_ptr.release());
   }
   INDIGO_END(-1)
}

IndigoDeconvolution::EmbContext::EmbContext(){
   am = 0;
   fmcache = 0;
}

bool IndigoDeconvolution::_matchAtoms (Graph &g1, Graph &g2, const int *, int sub_idx, int super_idx, void* userdata){
   QueryMolecule::Atom* q_atom = &((BaseMolecule &)g1).asQueryMolecule().getAtom(sub_idx);
   BaseMolecule &target = (BaseMolecule &)g2;
   EmbContext& emb_context = *(EmbContext*)userdata;

   return MoleculeSubstructureMatcher::matchQueryAtom(q_atom, target, super_idx, emb_context.fmcache, 0xFFFFFFFF);
}


bool IndigoDeconvolution::_matchBonds (Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void* userdata){
   EmbContext& emb_context = *(EmbContext*)userdata;

   QueryMolecule &query = (QueryMolecule &)subgraph;
   BaseMolecule &target  = (BaseMolecule &)supergraph;
   QueryMolecule::Bond &sub_bond = query.getBond(sub_idx);

   if (!MoleculeSubstructureMatcher::matchQueryBond(&sub_bond, target, sub_idx, super_idx, emb_context.am, 0xFFFFFFFF))
      return false;

   return true;
}

void IndigoDeconvolution::_removeAtom (Graph &subgraph, int sub_idx, void *userdata){
   EmbContext& emb_context = *(EmbContext*)userdata;

   if (emb_context.am != 0)
      emb_context.am->unfixNeighbourQueryBond(sub_idx);
}

void IndigoDeconvolution::_addBond (Graph &subgraph, Graph &supergraph,
                                    int sub_idx, int super_idx, void *userdata)
{
   EmbContext& emb_context = *(EmbContext*)userdata;
   BaseMolecule &target = (BaseMolecule &)supergraph;
   
   if (emb_context.am != 0)
      emb_context.am->fixQueryBond(sub_idx, target.getBondOrder(super_idx) == BOND_AROMATIC);
}
