/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "molecule/query_molecule.h"
#include "graph/graph_highlighting.h"
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

IndigoDeconvolution::IndigoDeconvolution(bool aromatize):
IndigoObject(IndigoObject::DECONVOLUTION),
flags(MoleculeExactMatcher::CONDITION_ALL&(~MoleculeExactMatcher::CONDITION_STEREO)),
cbEmbedding(0),
embeddingUserdata(0),
_aromatic(aromatize)
{
}

void IndigoDeconvolution::makeRGroups (Molecule& scaffold) {

   _scaffold.clone(scaffold, 0, 0);
   _fullScaffold.clone(scaffold, 0, 0);

   if(_aromatic) {
      MoleculeAromatizer::aromatizeBonds(_scaffold);
      MoleculeAromatizer::aromatizeBonds(_fullScaffold);
   }
   
   for (int mol_idx = 0; mol_idx < _deconvolutionItems.size(); ++mol_idx) {
      Item& elem = _deconvolutionItems[mol_idx];
      _makeRGroup(elem);
   }

}


void IndigoDeconvolution::_makeRGroup(Item& elem) {
   
   Molecule& mol_in = elem.mol_in;
   QueryMolecule& rgroup_out = elem.rgroup_mol;
   GraphHighlighting& graph_high = elem.highlight;
   Molecule& mol_out = elem.mol_out;
   
   graph_high.init(mol_in);

   if (mol_in.vertexCount() == 0)
      return;

//   QS_DEF(Array<int>, map);
//   QS_DEF(Array<int>, inv_map);
   
//   mol_out.clone(mol_in, &map, &inv_map, 0);
   mol_out.clone(mol_in, 0, 0, 0);

   EmbContext emb_context(flags);
   emb_context.lastMapping.clear();

   if(_aromatic) 
      MoleculeAromatizer::aromatizeBonds(mol_out);

   EmbeddingEnumerator emb_enum(mol_out);
   /*
    * Set options
    */
   emb_enum.setSubgraph(_scaffold);
   emb_enum.cb_embedding = _rGroupsEmbedding;
   emb_enum.cb_match_edge = IndigoDeconvolution::matchBonds;
   emb_enum.cb_match_vertex = IndigoDeconvolution::matchAtoms;
   emb_enum.userdata = &emb_context;
   /*
    * Find subgraph
    */
   emb_enum.process();

   if(emb_context.lastMapping.size() == 0)
      throw Error("no embeddings obtained");

//   emb_context.renumber(map, inv_map);

   graph_high.onSubgraph(_scaffold, emb_context.lastMapping.ptr());

   _createRgroups(mol_out, rgroup_out, emb_context);

}


int IndigoDeconvolution::_rGroupsEmbedding(Graph &graph1, Graph &graph2, int *map, int *inv_map, void *userdata) {

   QS_DEF(Array<int>, queue);
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
         queue.push(cc_start_idx);

         while (top != bottom) {

            int cur_idx = queue[bottom];
            const Vertex &vertex = graph2.getVertex(cur_idx);

            for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i)) {
               int nei_idx = vertex.neiVertex(i);

               if (visited_atoms[nei_idx] > 1)
                  continue;

               if (inv_map[nei_idx] >= 0) {
                  attachment_index[n_rgroups].push(cur_idx);
                  attachment_order[n_rgroups].push(nei_idx);

               } else {
                  queue.push(nei_idx);
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

void IndigoDeconvolution::_createRgroups(Molecule& mol_set, QueryMolecule& r_molecule, EmbContext& emb_context) {

   QS_DEF(Array<int>, inv_scaf_map);
   QS_DEF(Array<int>, rg_mapping);

   Array<int>& visited_atoms = emb_context.visitedAtoms;

   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;

   int n_rgroups = emb_context.getRgroupNumber();

   /*
    * TODO solve molecule->query convertion
    * Save and load molecule from buffer
    */
   QS_DEF(Array<char>, buffer);
   QS_DEF(QueryMolecule, qmol_set);

   ArrayOutput arr_buffer(buffer);
   MolfileSaver mol_saver(arr_buffer);
   mol_set.stereocenters.clear();
   mol_saver.saveMolecule(mol_set);

   BufferScanner buf_scanner(buffer);
   MolfileLoader mol_loader(buf_scanner);
   mol_loader.loadQueryMolecule(qmol_set);

   Filter sub_filter(visited_atoms.ptr(), Filter::EQ, 1);
   r_molecule.makeSubmolecule(qmol_set, sub_filter, 0, &inv_scaf_map);

   RedBlackMap<int, int> rgroup_idx_map;
   rgroup_idx_map.clear();

   for (int rg_idx = 0; rg_idx < n_rgroups; ++rg_idx) {
      Array<int>& att_idx = attachment_index[rg_idx];
      Array<int>& att_ord = attachment_order[rg_idx];

      /*
       * Find if rgroup already exists
       */
      int new_rg_idx = _findOrAddFullRGroup(att_ord, att_idx, qmol_set, emb_context.lastInvMapping);

      /*
       * TODO: check for duplicates
       */
      if(rgroup_idx_map.find(new_rg_idx))
          continue;
      /*
       * Store internal rg index
       */
      rgroup_idx_map.insert(new_rg_idx, rg_idx);
      /*
       * Add new atom r-site
       */
      AutoPtr<QueryMolecule::Atom> r_atom(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
      int new_atom_idx = r_molecule.addAtom(r_atom.release());
      Vec3f& atom_xyz = qmol_set.getAtomXyz(att_idx[0]);
      /*
       * Copy coordinates
       */
      r_molecule.setAtomXyz(new_atom_idx, atom_xyz.x, atom_xyz.y, atom_xyz.z);

      /*
       * Add Rsites
       */
       r_molecule.allowRGroupOnRSite(new_atom_idx, new_rg_idx);
      
      /*
       * Add all bonds
       */
      for(int point_att = 0; point_att < att_idx.size(); ++point_att) {
         int att_order = att_ord[point_att];

         if(r_molecule.findEdgeIndex(new_atom_idx, inv_scaf_map[att_order]) == -1) {
            int edge_idx = qmol_set.findEdgeIndex(att_order, att_idx[point_att]);
            if(edge_idx == -1)
               throw Error("inner error while converting molecule to query");
            AutoPtr<QueryMolecule::Bond> r_bond(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER,
                    qmol_set.getBondOrder(edge_idx)));
            r_molecule.addBond(new_atom_idx, inv_scaf_map[att_order], r_bond.release());
         }
      }
   }
   r_molecule.stereocenters.clear();

   /*
    * Add all Rgroups
    */
   MoleculeRGroups & mol_rgroups = r_molecule.rgroups;
   
   for (int i = r_molecule.vertexBegin(); i != r_molecule.vertexEnd(); i = r_molecule.vertexNext(i)) {
      if (!r_molecule.isRSite(i))
         continue;
      
      int r = r_molecule.getSingleAllowedRGroup(i);
      RGroup & r_group = mol_rgroups.getRGroup(r);
      /*
       * Get internal rg_idx
       */
      int rg_idx = rgroup_idx_map.at(r);
      QueryMolecule & fragment = r_group.fragments.add(new QueryMolecule());

      Filter sub_filter_fr(visited_atoms.ptr(), Filter::EQ, rg_idx + SHIFT_IDX);
      fragment.makeSubmolecule(qmol_set, sub_filter_fr, 0, &rg_mapping);

      if (!fragment.isRGroupFragment())
         fragment.createRGroupFragment();

      MoleculeRGroupFragment& rfragment = fragment.getRGroupFragment();

      for (int att_idx = 0; att_idx < attachment_index[rg_idx].size(); ++att_idx) {
         rfragment.addAttachmentPoint(att_idx, rg_mapping.at(attachment_index[rg_idx][att_idx]));
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

Molecule& IndigoDeconvolution::getDecomposedScaffold() {
   return _fullScaffold;
}

ObjArray<IndigoDeconvolution::Item>& IndigoDeconvolution::getItems ()
{
   return _deconvolutionItems;
}

IndigoDeconvolution::~IndigoDeconvolution() {
}

bool IndigoDeconvolution::matchBonds(Graph &g1, Graph &g2, int i, int j, void* userdata){
   Molecule &mol1 = (Molecule &)g1;
   Molecule &mol2 = (Molecule &)g2;

   EmbContext& emb_context = *(EmbContext*)userdata;
   int flags = emb_context.flags;

   return MoleculeExactMatcher::matchBonds(mol1, mol2, i, j, flags);
}

bool IndigoDeconvolution::matchAtoms (Graph &g1, Graph &g2, const int *, int i, int j, void* userdata){
   Molecule &mol1 = (Molecule &)g1;
   Molecule &mol2 = (Molecule &)g2;

   EmbContext& emb_context = *(EmbContext*)userdata;
   int flags = emb_context.flags;

   return MoleculeExactMatcher::matchAtoms(mol1, mol2, i, j, flags);
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
 * Find if rgroup already exists
 * Returns rgroup index
 */
int IndigoDeconvolution::_findOrAddFullRGroup(Array<int>& att_order, Array<int>& att_idx, QueryMolecule& qmol, Array<int>& map) {
   /*
    * Search for existing rsites
    */
   QS_DEF(RedBlackSet<int>, exist_atoms);
   exist_atoms.clear();

   for (int i = 0; i < att_order.size(); ++i) {
      exist_atoms.find_or_insert(map.at(att_order[i]));
   }

   int new_rg_idx = 0;
   int vert_idx = _fullScaffold.vertexBegin();
   for (; vert_idx != _fullScaffold.vertexEnd(); vert_idx = _fullScaffold.vertexNext(vert_idx)) {
      if (!_fullScaffold.isRSite(vert_idx))
         continue;
      int cur_rg_idx = _fullScaffold.getSingleAllowedRGroup(vert_idx);

      if(new_rg_idx < cur_rg_idx)
         new_rg_idx = cur_rg_idx;
      
      const Vertex& vert = _fullScaffold.getVertex(vert_idx);

      bool not_found = (vert.degree() != exist_atoms.size());
      for (int nei_idx = vert.neiBegin(); !not_found && nei_idx != vert.neiEnd(); nei_idx = vert.neiNext(nei_idx)) {
         if (!exist_atoms.find(vert.neiVertex(nei_idx))) {
            not_found = true;
         }
      }
      
      /*
       * Find the match between new order and already added atom
       */
      if(!not_found)
         return cur_rg_idx;
   }
   /*
    * Increase rgroup index
    */
   ++new_rg_idx;
   
   /*
    * If not found then add Rsite to the full scaffold
    */
   int new_atom_idx = _fullScaffold.addAtom(ELEM_RSITE);
   
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
            throw Error("inner error while converting molecule to query");
         _fullScaffold.addBond(new_atom_idx, att_self, BOND_SINGLE);
      }
   }
   return new_rg_idx;
}


CEXPORT int indigoDecomposeMolecules (int scaffold, int structures) {
   INDIGO_BEGIN
   {
      IndigoArray& mol_array = self.getObject(structures).asArray();

      AutoPtr<IndigoDeconvolution> deco(new IndigoDeconvolution(self.deconvolution_aromatization));
      int i;

      for (i = 0; i < mol_array.objects.size(); i++)
      {
         IndigoObject &obj = *mol_array.objects[i];

         deco->addMolecule(obj.getMolecule(), obj.getProperties());
      }

      Molecule& scaf = self.getObject(scaffold).getMolecule();

      deco->makeRGroups(scaf);
      return self.addObject(deco.release());
   }
   INDIGO_END(0, -1)
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
   INDIGO_END(0, -1)
}

CEXPORT int indigoDecomposedMoleculeScaffold (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      AutoPtr<IndigoMolecule> mol_ptr(new IndigoMolecule());

      if (obj.type != IndigoObject::DECONVOLUTION)
         throw IndigoError("indigoDecomposedMoleculeScaffold(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolution& deco = (IndigoDeconvolution &)obj;

      mol_ptr->mol.clone(deco.getDecomposedScaffold(), 0, 0);

      return self.addObject(mol_ptr.release());
   }
   INDIGO_END(0, -1)
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

      QS_DEF(Array<int>, map);
      mol->mol.clone(elem.item.mol_out, &map, 0);
      mol->highlighting.init(mol->mol);
      mol->highlighting.copy(elem.item.highlight, map);
      mol->copyProperties(elem.item.properties);

      return self.addObject(mol.release());
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoDecomposedMoleculeSubstituents (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeSubstituents(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;

      QueryMolecule* qmol = &elem.item.rgroup_mol;
      return self.addObject(new IndigoRGroupsIter(qmol));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoDecomposedMoleculeWithRGroups (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeWithRGroups(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;

      AutoPtr<IndigoQueryMolecule> qmol_ptr(new IndigoQueryMolecule());
      qmol_ptr->qmol.clone(elem.item.rgroup_mol, 0, 0);
      qmol_ptr->copyProperties(elem.item.properties);
      return self.addObject(qmol_ptr.release());
   }
   INDIGO_END(0, -1)
}
