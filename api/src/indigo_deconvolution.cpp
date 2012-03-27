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
#include "base_cpp/red_black.h"
#include "indigo_mapping.h"




IndigoDeconvolution::IndigoDeconvolution(bool aromatize):
IndigoObject(IndigoObject::DECONVOLUTION),
save_ap_bond_orders(false),
ignore_errors(false),
cbEmbedding(0),
embeddingUserdata(0),
_aromatic(aromatize)
{
}

void IndigoDeconvolution::setScaffold (QueryMolecule& scaffold) {
   /*
    * Clone molecules
    */
   _scaffold.clone_KeepIndices(scaffold, 0);
   _fullScaffold.clone_KeepIndices(scaffold, 0);

   if(_aromatic) {
      QueryMoleculeAromatizer::aromatizeBonds(_scaffold);
      QueryMoleculeAromatizer::aromatizeBonds(_fullScaffold);
   }
}

void IndigoDeconvolution::makeRGroups (QueryMolecule& scaffold) {

   setScaffold(scaffold);
   
   for (int mol_idx = 0; mol_idx < _deconvolutionElems.size(); ++mol_idx) {
      IndigoDeconvolutionElem& elem = _deconvolutionElems[mol_idx];
      makeRGroup(elem);
   }

}


void IndigoDeconvolution::makeRGroup(IndigoDeconvolutionElem& elem) {
   
   Molecule& mol_in = elem.mol_in;
   
   DecompositionEnumerator& deco_enum = elem.deco_enum;
   if (mol_in.vertexCount() == 0) {
      deco_enum.contexts.clear();
      return;
   }

   if(_aromatic) 
      MoleculeAromatizer::aromatizeBonds(mol_in);

   /*
    * Search for the query match
    */
   if (_aromatic && AromaticityMatcher::isNecessary(_scaffold)) 
      deco_enum.am.reset(new AromaticityMatcher(_scaffold, mol_in));

   deco_enum.fmcache.reset(new MoleculeSubstructureMatcher::FragmentMatchCache);
   deco_enum.fmcache->clear();
   
   EmbeddingEnumerator emb_enum(mol_in);

   /*
    * Set options
    */
   emb_enum.setSubgraph(_scaffold);
   emb_enum.cb_embedding = _rGroupsEmbedding;
   emb_enum.cb_match_edge = _matchBonds;
   emb_enum.cb_match_vertex = _matchAtoms;
   emb_enum.cb_vertex_remove = _removeAtom;
   emb_enum.cb_edge_add = _addBond;
   emb_enum.userdata = &deco_enum;
   /*
    * Find subgraph
    */
   emb_enum.process();

   if(deco_enum.contexts.size() == 0) {
      if(ignore_errors) {
         return;
      } else
         throw Error("no embeddings obtained");
   } else {
      EmbContext& emb_context = deco_enum.contexts[0];
      emb_context.mol_out.clone_KeepIndices(mol_in);

      _createRgroups(emb_context);
      
      emb_context.mol_scaffold.makeEdgeSubmolecule(emb_context.mol_out, emb_context.scaffoldAtoms, emb_context.scaffoldBonds, 0, 0);
      emb_context.mol_scaffold.unhighlightAll();
      
      emb_context.mol_out.highlightSubmolecule(_scaffold, emb_context.lastMapping.ptr(), true);
   }
}


int IndigoDeconvolution::_rGroupsEmbedding(Graph &graph1, Graph &graph2, int *map, int *inv_map, void *userdata) {

   QS_DEF(Array<int>, queue);
   QS_DEF(Array<int>, queue_markers);
   int result = 0;
   int n_rgroups = 0;

   DecompositionEnumerator& deco_enum = *(DecompositionEnumerator*)userdata;

   EmbContext& emb_context = deco_enum.contexts.push();

   emb_context.lastMapping.copy(map, graph1.vertexEnd());
   emb_context.lastInvMapping.copy(inv_map, graph2.vertexEnd());
   /*
    * Visited atom = 1 in case of scaffold and > 1 in case of rgroup
    */
   Array<int>& visited_atoms = emb_context.visitedAtoms;
   /*
    * Each array corresponds to a separate Rgroup.
    * Order - atom number for scaffold
    * Index - atom number for Rgroup
    */
   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;

   visited_atoms.clear_resize(graph2.vertexEnd());
   visited_atoms.zerofill();

   attachment_index.clear();
   attachment_order.clear();

   attachment_index.push();
   attachment_order.push();
   /*
    * Calculate scaffold atoms and Rgroup atoms
    */
   
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

   /*
    * Calculate scaffold bonds for a scaffold in the molecule
    * Also define empty Rgroups
    */
   Array<int>& scaf_bonds = emb_context.scaffoldBonds;
   scaf_bonds.clear();

   int v_idx1, v_idx2, e_idx_mol, e_idx_scaf;
   
   for (e_idx_mol = graph2.edgeBegin(); e_idx_mol != graph2.edgeEnd(); e_idx_mol = graph2.edgeNext(e_idx_mol)) {
      const Edge& edge = graph2.getEdge(e_idx_mol);
      v_idx1 = inv_map[edge.beg];
      v_idx2 = inv_map[edge.end];
      
      if (v_idx1 >= 0 && v_idx2 >= 0) {
         
         e_idx_scaf = graph1.findEdgeIndex(v_idx1, v_idx2);

         if (e_idx_scaf >= 0) {
            /*
             * Append existing edge
             */
            scaf_bonds.push(e_idx_mol);
         } else {
            /*
             * Append non existent bond as a Rgroup with the same att idx and att order
             */
            attachment_index[n_rgroups].push(edge.beg);
            attachment_order[n_rgroups].push(edge.end);
            attachment_index[n_rgroups].push(edge.end);
            attachment_order[n_rgroups].push(edge.beg);

            attachment_index.push();
            attachment_order.push();

            ++n_rgroups;
         }
      }
   }
   /*
    * Calculate scaffold atoms
    */
   Array<int>& scaf_atoms = emb_context.scaffoldAtoms;
   scaf_atoms.clear();
   for (int a_idx = graph2.vertexBegin(); a_idx != graph2.vertexEnd(); a_idx = graph2.vertexNext(a_idx)) {
      if(inv_map[a_idx] >= 0)
         scaf_atoms.push(a_idx);
   }

   return result;
}

void IndigoDeconvolution::_createRgroups(EmbContext& emb_context) {

   Molecule& mol_set = emb_context.mol_out;
   Molecule& r_molecule = emb_context.rgroup_mol;
           
   QS_DEF(Array<int>, inv_scaf_map);
   QS_DEF(Array<int>, rg_mapping);
   QS_DEF(Array<int>, rgidx_map);
   RedBlackMap<int, int> att_ord_map;
   RedBlackMap<int, int> att_bond_map;
   int att_order, att_idx;

   Array<int>& visited_atoms = emb_context.visitedAtoms;

   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;

   int n_rgroups = emb_context.getRgroupNumber();
   /*
    * Create a submolecule for a scaffold
    */
   r_molecule.makeEdgeSubmolecule(mol_set, 
           emb_context.scaffoldAtoms,
           emb_context.scaffoldBonds, &inv_scaf_map, 0);
   r_molecule.unhighlightAll();

   /*
    * Add rgroups (if not exist) to the full scaffold
    */
   _addCompleteRGroup(mol_set, emb_context, rgidx_map);

   
   /*
    * Add all Rgroups
    */
   for (int rg_idx = 0; rg_idx < n_rgroups; ++rg_idx) {
      Array<int>& att_indexes = attachment_index[rg_idx];
      Array<int>& att_orders = attachment_order[rg_idx];

      /*
       * Add new atom r-site
       */
      int new_atom_idx = r_molecule.addAtom(ELEM_RSITE);
      Vec3f& atom_xyz = mol_set.getAtomXyz(att_indexes[0]);
      /*
       * Copy coordinates
       */
      r_molecule.setAtomXyz(new_atom_idx, atom_xyz.x, atom_xyz.y, atom_xyz.z);

      /*
       * Add Rsites
       */
       r_molecule.allowRGroupOnRSite(new_atom_idx, rgidx_map[rg_idx]);
      
      /*
       * Add all bonds and order maps
       */
      int att_val = 0;
      att_ord_map.clear();
      for(int p_idx = 0; p_idx < att_indexes.size(); ++p_idx) {
         att_idx = att_indexes[p_idx];
         att_order = att_orders[p_idx];
         /*
          * Fulfil attachment order map
          */
         if (!att_ord_map.find(att_order)) {
            ++att_val;
            att_ord_map.insert(att_order, att_val);
         }
          /*
           * Add a bond with the same order
           */
         if(r_molecule.findEdgeIndex(new_atom_idx, inv_scaf_map[att_order]) == -1) {
            int edge_idx = mol_set.findEdgeIndex(att_order, att_idx);
            if(edge_idx == -1)
               throw Error("internal error: can not find the edge for a scaffold");
            r_molecule.addBond(new_atom_idx, inv_scaf_map[att_order], mol_set.getBondOrder(edge_idx));
         }
         /*
          * Set Rsite attachment order one lower since old api was not changed
          */
         if(!save_ap_bond_orders)
            r_molecule.setRSiteAttachmentOrder(new_atom_idx, inv_scaf_map[att_order], att_ord_map.at(att_order) - 1);
      }
      MoleculeRGroups & mol_rgroups = r_molecule.rgroups;
      
      /*
       * Get an internal rg_idx
       */
      int rg_allow_idx = r_molecule.getSingleAllowedRGroup(new_atom_idx);
      RGroup & r_group = mol_rgroups.getRGroup(rg_allow_idx);
      /*
       * Add a fragment
       */
      Molecule & fragment = r_group.fragments.at(r_group.fragments.add(new Molecule()))->asMolecule();

      Filter sub_filter_fr(visited_atoms.ptr(), Filter::EQ, rg_idx + SHIFT_IDX);
      fragment.makeSubmolecule(mol_set, sub_filter_fr, 0, &rg_mapping);
      /*
       * Add attachment points
       */
      att_bond_map.clear();
      for (int p_idx = 0; p_idx < att_indexes.size(); ++p_idx) {
         att_idx = att_indexes[p_idx];
         att_order = att_orders[p_idx];
         
         int att_idx_m = rg_mapping.at(att_idx);
         int att_order_m = att_ord_map.at(att_order);
         
         int edge_idx = mol_set.findEdgeIndex(att_order, att_idx);
         if (edge_idx == -1)
            throw Error("internal error: can not find the edge for a fragment");
         if(save_ap_bond_orders) {
            /*
             * TODO change to elem AP
             */
            int ap_atom_idx = fragment.addAtom(ELEM_PSEUDO);

            /*
             * Write AP pseudo
             */
            QS_DEF(Array<char>, buf);
            ArrayOutput ap_out(buf);
            ap_out.printf("AP%d", att_order_m);
            ap_out.writeChar(0);
            fragment.setPseudoAtom(ap_atom_idx, buf.ptr());
            /*
             * Add AP bond
             * Check for empty RGroup first
             */
            if(att_idx_m == -1) {
               /*
                * Add AP saved on the previous iteration or add as a new for the following AP
                */
               if(att_bond_map.find(edge_idx)) {
                  fragment.addBond(ap_atom_idx, att_bond_map.at(edge_idx), mol_set.getBondOrder(edge_idx));
               } else {
                  att_bond_map.insert(edge_idx, ap_atom_idx);
               }
            } else {
               /*
                * Add bond order
                */
               fragment.addBond(ap_atom_idx, att_idx_m, mol_set.getBondOrder(edge_idx));
            }
         } else {
            fragment.addAttachmentPoint(att_order_m, att_idx_m);
         }
      }

   }
}

IndigoDeconvolutionElem::IndigoDeconvolutionElem (Molecule& mol, int index) :
IndigoObject(DECONVOLUTION_ELEM), idx(index)
{
   mol_in.clone_KeepIndices(mol, 0);
}

IndigoDeconvolutionElem::IndigoDeconvolutionElem (Molecule& mol) :
IndigoObject(DECONVOLUTION_ELEM), idx(-1)
{
   mol_in.clone_KeepIndices(mol, 0);
}

IndigoDeconvolutionElem::IndigoDeconvolutionElem (IndigoDeconvolutionElem& other):
IndigoObject(DECONVOLUTION_ELEM), idx(other.idx) {
   mol_in.clone_KeepIndices(other.mol_in, 0);
   
   deco_enum.contexts.clear();
   for (int i = 0; i < other.deco_enum.contexts.size(); ++i) {
      deco_enum.contexts.push().copy(other.deco_enum.contexts[i]);
   }
}

int IndigoDeconvolutionElem::getIndex () {
   return idx;
}

IndigoDeconvolutionElem::~IndigoDeconvolutionElem ()
{
}

IndigoDeconvolutionIter::IndigoDeconvolutionIter(ObjArray<IndigoDeconvolutionElem>& items) :
IndigoObject(DECONVOLUTION_ITER),
_items(items)
{
   _index = -1;
}

IndigoObject * IndigoDeconvolutionIter::next () {
   if (!hasNext())
      return 0;

   _index++;
   return new IndigoDeconvolutionElem(_items[_index]);
}

bool IndigoDeconvolutionIter::hasNext () {
   return _index + 1 < _items.size();
}

IndigoDeconvolutionIter::~IndigoDeconvolutionIter ()
{
}

void IndigoDeconvolution::addMolecule(Molecule& mol, RedBlackStringObjMap< Array<char> >* props) {
   IndigoDeconvolutionElem & item = _deconvolutionElems.push(mol);
   int i;
   
   if (props != 0)
   {
      for (i = props->begin(); i != props->end(); i = props->next(i))
         item.properties.value(item.properties.insert(props->key(i))).copy(props->value(i));
   }
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

void IndigoDeconvolution::EmbContext::copy(EmbContext& other) {
   visitedAtoms.copy(other.visitedAtoms);
   scaffoldBonds.copy(other.scaffoldBonds);
   scaffoldAtoms.copy(other.scaffoldAtoms);
   lastMapping.copy(other.lastMapping);
   lastInvMapping.copy(other.lastInvMapping);
   attachmentOrder.clear();
   attachmentIndex.clear();
   for (int i = 0; i < other.attachmentOrder.size(); ++i) {
      attachmentOrder.push().copy(other.attachmentOrder[i]);
   }
   for (int i = 0; i < other.attachmentIndex.size(); ++i) {
      attachmentIndex.push().copy(other.attachmentIndex[i]);
   }

   mol_out.clone_KeepIndices(other.mol_out, 0);
   rgroup_mol.clone_KeepIndices(other.rgroup_mol, 0);
   mol_scaffold.clone_KeepIndices(other.mol_scaffold, 0);
}


/*
 * Add rgroup if not exists
 */
void IndigoDeconvolution::_addCompleteRGroup(Molecule& mol_set, EmbContext& emb_context, Array<int>& rg_map) {
   ObjArray< Array<int> >& attachment_order = emb_context.attachmentOrder;
   ObjArray< Array<int> >& attachment_index = emb_context.attachmentIndex;
   Array<int>& map = emb_context.lastInvMapping;

//   saveMoleculeAsReaction(mol_set, "res/i_mol_set.rxn");
//   saveMoleculeAsReaction(_fullScaffold, "res/i_full_scaf.rxn", true);
//   printf("****map\n");
//   for (int i = 0; i < map.size(); ++i) {
//      printf("%d %d\n", i ,map[i]);
//   }
//   printf("**** end map\n");

   int n_rgroups = emb_context.getRgroupNumber();
   /*
    * Search for existing rgroups
    */
   QS_DEF(RedBlackStringObjMap< Array<int> >, match_rgroups);
   match_rgroups.clear();
   /*
    * A set to keep attachment orders
    */
   QS_DEF(RedBlackSet<int>, str_keys);
   /*
    * A string to keep attachemnt orders strings
    */
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
      if(!match_rgroups.find(str_key.ptr())) {
         match_rgroups.insert(str_key.ptr());
      }
      match_rgroups.at(str_key.ptr()).push(cur_rg_idx);
   }
//   {
//      printf("***************\nmatch keys\n");
//      for (int i = match_rgroups.begin(); i != match_rgroups.end(); i = match_rgroups.next(i)) {
//         Array<int>& value = match_rgroups.value(i);
//         printf("key = %s val = ", match_rgroups.key(i));
//         for (int j = 0; j < value.size(); ++j) {
//            printf("%d, ", value[j] );
//         }
//         printf("\n");
//      }
//      FileOutput fo("res/fullscaf.mol");
//      MolfileSaver ms(fo);
//      ms.saveQueryMolecule(_fullScaffold);
//   }
   
   /*
    * Loop through all rgroups and seek for matchings
    */
   bool match_not_found;
   rg_map.clear_resize(n_rgroups);
   int map_rg_idx;
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
         Array<int>& match_r = match_rgroups.at(str_key.ptr());
         /*
          * Remove matches
          */
         if(match_r.size() == 0 ) {
            match_not_found = true;
         } else {
            map_rg_idx = match_r.pop();
         }
      } else {
         match_not_found = true;
      }
      /*
       * Add rgroup to full scaffold
       */
      if(match_not_found) {
         ++new_rg_idx;
         map_rg_idx = new_rg_idx;
         _addFullRGroup(att_ord, att_idx, mol_set, map, new_rg_idx);
      }
      /*
       * Set up out map for rgroup
       */
      rg_map[rg_idx] = map_rg_idx;
   }
//   {
//      FileOutput fo("res/fullscaf.mol");
//      MolfileSaver ms(fo);
//      ms.saveQueryMolecule(_fullScaffold);
//   }
}

/*
 */
void IndigoDeconvolution::_addFullRGroup(Array<int>& att_orders, Array<int>& att_indexes, Molecule& qmol, Array<int>& map, int new_rg_idx) {
   /*
    * If not found then add Rsite to the full scaffold
    */
   int att_order, att_idx, att_self, new_atom_idx;
   
   if (att_indexes.size() > 0) {
      new_atom_idx = _fullScaffold.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));

      /*
       * Copy coordinates
       */
      Vec3f& atom_xyz = qmol.getAtomXyz(att_indexes[0]);
      _fullScaffold.setAtomXyz(new_atom_idx, atom_xyz.x, atom_xyz.y, atom_xyz.z);

      /*
       * Add Rsites
       */
      _fullScaffold.allowRGroupOnRSite(new_atom_idx, new_rg_idx);
   }

   /*
    * Add all bonds
    */
   for (int point_att = 0; point_att < att_indexes.size(); ++point_att) {
      att_order = att_orders[point_att];
      att_idx = att_indexes[point_att];
      att_self = map[att_order];

      if (_fullScaffold.findEdgeIndex(new_atom_idx, att_self) == -1) {
         int edge_idx = qmol.findEdgeIndex(att_order, att_idx);
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
      deco->save_ap_bond_orders = self.deco_save_ap_bond_orders;
      deco->ignore_errors = self.deco_ignore_errors;
      int i;

      for (i = 0; i < mol_array.objects.size(); i++)
      {
         IndigoObject &obj = *mol_array.objects[i];
            /*
             * Add molecule
             */
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
      AutoPtr<IndigoObject> mol_ptr;

      if (obj.type == IndigoObject::DECONVOLUTION) {
         /*
          * Create query scaffold
          */
         IndigoDeconvolution& deco = (IndigoDeconvolution &) obj;
         mol_ptr.reset(new IndigoQueryMolecule());
         IndigoQueryMolecule& qmol = (IndigoQueryMolecule&) mol_ptr.ref();
         qmol.qmol.clone(deco.getDecomposedScaffold(), 0, 0);
      } else if (obj.type == IndigoObject::DECONVOLUTION_ELEM) {
         /*
          * Create simple scaffold with rsites
          */
         IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;
         IndigoDeconvolution::DecompositionEnumerator& deco_enum = elem.deco_enum;

         if(deco_enum.contexts.size() == 0) {
            throw IndigoError("indigoDecomposedMoleculeScaffold(): no embeddings were found for the molecule %d", elem.idx);
         }
         IndigoDeconvolution::EmbContext& emb_context = deco_enum.contexts[0];

         mol_ptr.reset(new IndigoMolecule());
         IndigoMolecule& mol = (IndigoMolecule&) mol_ptr.ref();

         mol.mol.clone(emb_context.mol_scaffold, 0, 0);
      } else {
         throw IndigoError("indigoDecomposedMoleculeScaffold(): not applicable to %s", obj.debugInfo());
      }
      int obj_idx = self.addObject(mol_ptr.release());
      /*
       * Call layout
       */
      indigoLayout(obj_idx);
      return obj_idx;
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
      IndigoDeconvolution::DecompositionEnumerator& deco_enum = elem.deco_enum;

      if(deco_enum.contexts.size() == 0) {
         throw IndigoError("indigoDecomposedMoleculeHighlighted(): no embeddings were found for the molecule %d", elem.idx);
      }
      IndigoDeconvolution::EmbContext& emb_context = deco_enum.contexts[0];

      AutoPtr<IndigoMolecule> mol;
      mol.create();

      mol->mol.clone_KeepIndices(emb_context.mol_out, 0);
      mol->copyProperties(elem.properties);

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
      IndigoDeconvolution::DecompositionEnumerator& deco_enum = elem.deco_enum;

      if(deco_enum.contexts.size() == 0) {
         throw IndigoError("indigoDecomposedMoleculeSubstituents(): no embeddings were found for the molecule %d", elem.idx);
      }
      IndigoDeconvolution::EmbContext& emb_context = deco_enum.contexts[0];

      Molecule* qmol = &emb_context.rgroup_mol;
      return self.addObject(new IndigoRGroupsIter(qmol));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDecomposedMoleculeWithRGroups (int decomp) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);
      AutoPtr<IndigoMolecule> mol_ptr;

      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
         throw IndigoError("indigoDecomposedMoleculeWithRGroups(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&)obj;
      IndigoDeconvolution::DecompositionEnumerator& deco_enum = elem.deco_enum;

      if(deco_enum.contexts.size() == 0) {
         throw IndigoError("indigoDecomposedMoleculeWithRGroups(): no embeddings were found for the molecule %d", elem.idx);
      }
      IndigoDeconvolution::EmbContext& emb_context = deco_enum.contexts[0];

      mol_ptr.reset(new IndigoMolecule());
      mol_ptr->mol.clone(emb_context.rgroup_mol,0, 0);
      mol_ptr->copyProperties(elem.properties);

      int obj_idx = self.addObject(mol_ptr.release());
      /*
       * Call layout
       */
//      indigoLayout(obj_idx);
      return obj_idx;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCreateDecomposer(int scaffold) {
   INDIGO_BEGIN
   {
      AutoPtr<IndigoDeconvolution> deco(new IndigoDeconvolution(self.deconvolution_aromatization));
      deco->save_ap_bond_orders = self.deco_save_ap_bond_orders;
      deco->ignore_errors = self.deco_ignore_errors;

      QueryMolecule& scaf = self.getObject(scaffold).getQueryMolecule();

      deco->setScaffold(scaf);
      return self.addObject(deco.release());
   }
   INDIGO_END(-1)
}
CEXPORT int indigoDecomposeMolecule(int decomp, int mol) {
   INDIGO_BEGIN
   {
      IndigoObject& obj = self.getObject(decomp);

      if (obj.type != IndigoObject::DECONVOLUTION)
         throw IndigoError("indigoDecomposeMolecule(): not applicable to %s", obj.debugInfo());

      IndigoDeconvolution& deco = (IndigoDeconvolution &)obj;

      AutoPtr<IndigoDeconvolutionElem> deco_elem;
      deco_elem.reset(new IndigoDeconvolutionElem(self.getObject(mol).getMolecule()));

      deco.makeRGroup(deco_elem.ref());

      return self.addObject(deco_elem.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateDecompositions(int deco_item) {
//   INDIGO_BEGIN
//   {
//      IndigoObject& obj = self.getObject(deco_item);
//      AutoPtr<IndigoMolecule> mol_ptr;
//
//      if (obj.type != IndigoObject::DECONVOLUTION_ELEM)
//         throw IndigoError("indigoIterateDecompositions(): not applicable to %s", obj.debugInfo());
//
//      IndigoDeconvolutionElem& elem = (IndigoDeconvolutionElem&) obj;
//
//      if (elem.mol_out.vertexCount() == 0) {
//         throw IndigoError("indigoIterateDecompositions(): no embeddings were found for the molecule %d", elem.idx);
//      }
//
//      mol_ptr.reset(new IndigoMolecule());
//      mol_ptr->mol.clone(elem.rgroup_mol, 0, 0);
//      mol_ptr->copyProperties(elem.properties);
//
//      int obj_idx = self.addObject(mol_ptr.release());
//      /*
//       * Call layout
//       */
//      //      indigoLayout(obj_idx);
//      return obj_idx;
//   }
//   INDIGO_END(-1)
}
CEXPORT int indigoAddDecomposition(int decomp, int q_match) {
}

IndigoDeconvolution::EmbContext::EmbContext(){
}

bool IndigoDeconvolution::_matchAtoms (Graph &g1, Graph &g2, const int *, int sub_idx, int super_idx, void* userdata){
   QueryMolecule::Atom* q_atom = &((BaseMolecule &)g1).asQueryMolecule().getAtom(sub_idx);
   BaseMolecule &target = (BaseMolecule &)g2;
   DecompositionEnumerator& deco_enum = *(DecompositionEnumerator*)userdata;

   return MoleculeSubstructureMatcher::matchQueryAtom(q_atom, target, super_idx, deco_enum.fmcache.get(), 0xFFFFFFFF);
}


bool IndigoDeconvolution::_matchBonds (Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void* userdata){
   DecompositionEnumerator& deco_enum = *(DecompositionEnumerator*)userdata;

   QueryMolecule &query = (QueryMolecule &)subgraph;
   BaseMolecule &target  = (BaseMolecule &)supergraph;
   QueryMolecule::Bond &sub_bond = query.getBond(sub_idx);

   if (!MoleculeSubstructureMatcher::matchQueryBond(&sub_bond, target, sub_idx, super_idx, deco_enum.am.get(), 0xFFFFFFFF))
      return false;

   return true;
}

void IndigoDeconvolution::_removeAtom (Graph &subgraph, int sub_idx, void *userdata){
   DecompositionEnumerator& deco_enum = *(DecompositionEnumerator*)userdata;

   if (deco_enum.am.get() != 0)
      deco_enum.am->unfixNeighbourQueryBond(sub_idx);
}

void IndigoDeconvolution::_addBond (Graph &subgraph, Graph &supergraph,
                                    int sub_idx, int super_idx, void *userdata)
{
   DecompositionEnumerator& deco_enum = *(DecompositionEnumerator*)userdata;
   BaseMolecule &target = (BaseMolecule &)supergraph;
   
   if (deco_enum.am.get() != 0)
      deco_enum.am->fixQueryBond(sub_idx, target.getBondOrder(super_idx) == BOND_AROMATIC);
}

void IndigoDeconvolution::_makeInvertMap(Array<int>& map, Array<int>& invmap) {
   for(int i = 0;i < map.size();i++){
      if(map[i] != -1){
         invmap[map[i]] = i;
      }
   }
}
