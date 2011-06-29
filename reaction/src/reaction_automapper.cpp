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

#include "reaction/reaction_automapper.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "molecule/molecule_arom.h"
#include "base_cpp/red_black.h" 
#include "molecule/elements.h"
#include "base_cpp/auto_ptr.h"

using namespace indigo;

ReactionAutomapper::ReactionAutomapper(BaseReaction& reaction):
_reaction(reaction){
}

void ReactionAutomapper::automap(int mode) {
   AutoPtr<BaseReaction> reaction_copy;

   QS_DEF(ObjArray< Array<int> >, mappings);
   QS_DEF(Array<int>, mol_mapping);

   if (mode != AAM_REGEN_DISCARD) 
      _checkAtomMapping(true, false, false);
   
   reaction_copy.reset(_reaction.neu());
   reaction_copy->clone(_reaction, &mol_mapping, &mappings, 0);
   reaction_copy->aromatize();

   _createReactionMap(mode, reaction_copy.ref());
   _setupReactionMap(mode, reaction_copy.ref(), mol_mapping, mappings);

   _considerDissociation();
   _considerDimerization();


   _checkAtomMapping(false, true, false);


   
}

void ReactionAutomapper::_initMappings(int mode, BaseReaction& reaction){

   int i,j;

   if(mode == AAM_REGEN_ALTER || mode ==  AAM_REGEN_DISCARD){
      int current_map = 0;
      for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i)){
         for (j = 0; j < reaction.getAAMArray(i).size(); j++){
            ++current_map;
            reaction.getAAMArray(i).at(j) = current_map;
         }
      }
      _usedVertices.resize(current_map+1);
      _usedVertices.zerofill();
   }

   if(mode == AAM_REGEN_KEEP){
      RedBlackSet<int> used_maps;
      int max_value = 0;
      for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i)){
         for (j = 0; j < reaction.getAAMArray(i).size(); j++){
            used_maps.find_or_insert(reaction.getAAM(i, j));
            if(reaction.getAAM(i, j) > max_value)
               max_value = _reaction.getAAM(i, j);
         }
      }
      int new_size = used_maps.size();
      int current_map = 0;
      for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i)){
         for (j = 0; j < reaction.getAAMArray(i).size(); j++){
            if(reaction.getAAM(i, j) == 0 ){
               while(new_size == used_maps.size()){
                  ++current_map;
                  used_maps.find_or_insert(current_map);
               }
               new_size = used_maps.size();
               reaction.getAAMArray(i).at(j) = current_map;
            }
         }
      }
      if(current_map > max_value)
         max_value = current_map;
      _usedVertices.resize(max_value + 1);
      _usedVertices.zerofill();
   }

   for (i = reaction.productBegin(); i < reaction.productEnd(); i = reaction.productNext(i)){
      reaction.getAAMArray(i).zerofill();
   }
}

void ReactionAutomapper::_createReactionMap(int mode, BaseReaction& reaction){
   int i, j;
   
   QS_DEF(ObjArray< Array<int> >, permulations);
   QS_DEF(Array<int>, product_mapping_tmp);
   QS_DEF(Array<int>, reactant_indexes);
   

   ReactionMapMatchingData react_map_match(reaction);
   react_map_match.createAtomMatchingData();

   _initMappings(mode, reaction);

   reactant_indexes.resize(_reaction.reactantsCount());
   j = 0;
   for(i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i)) {
      reactant_indexes[j] = i;
      ++j;
   }

   _permutation(reactant_indexes, permulations);

   for(int product = reaction.productBegin(); product < reaction.productEnd(); product = reaction.productNext(product)){
      product_mapping_tmp.clear_resize(reaction.getAAMArray(product).size());

      _maxMapUsed = 0;
      _maxVertUsed = 0;
      _maxCompleteMap = 0;

      for(int pmt = 0; pmt < permulations.size(); pmt++) {
         _handleWithProduct(permulations[pmt], product_mapping_tmp, reaction, mode, product, react_map_match);
      }
      _usedVertices.zerofill();
      for(int k = reaction.productBegin(); k <= product; k = reaction.productNext(k)){
         for (int j = 0; j < reaction.getAAMArray(k).size(); j++){
            int m = reaction.getAAM(k ,j);
            if(m > 0)
               _usedVertices[m] = 1;
         }
      }
      _cleanReactants(reaction);
   }

}

void ReactionAutomapper::_cleanReactants(BaseReaction& reaction) {
   for(int react = reaction.reactantBegin(); react < reaction.reactantEnd(); react = reaction.reactantNext(react)) {
      BaseMolecule& rmol = reaction.getBaseMolecule(react);
      for(int vert = rmol.vertexBegin(); vert < rmol.vertexEnd();) {
         if(_usedVertices[reaction.getAAM(react, vert)]) {
            int next_vert = rmol.vertexNext(vert);
            rmol.removeVertex(vert);
            vert = next_vert;
            continue;
         }
         vert = rmol.vertexNext(vert);
      }
   }
}

void ReactionAutomapper::_handleWithProduct(const Array<int>& reactant_cons, Array<int>& product_mapping_tmp, BaseReaction& reaction, int mode, int product, ReactionMapMatchingData& react_map_match) {
   
   QS_DEF(Array<int>, matching_map);
   QS_DEF(Array<int>, rsub_map_in);
   QS_DEF(Array<int>, rsub_map_out);


   BaseMolecule& product_cut = reaction.getBaseMolecule(product);
   //delete hydrogens 
   for(int k = product_cut.vertexBegin(); k < product_cut.vertexEnd(); k = product_cut.vertexNext(k))
      if(product_cut.getAtomNumber(k) == ELEM_H)
         product_cut.removeVertex(k);

   product_mapping_tmp.zerofill();

   int map_used = 0;
   int map_complete = 0;
   _usedVertices[0] = 0;

   for(int i = 0; i < reactant_cons.size(); i++){
      int react = reactant_cons.at(i);

      int react_vsize = reaction.getBaseMolecule(react).vertexEnd();
      rsub_map_in.resize(react_vsize);
      for(int k = 0; k < react_vsize; k++)
         rsub_map_in[k] = SubstructureMcs::UNMAPPED;


      bool map_exc = false;
      if(mode != AAM_REGEN_DISCARD){
         for(int m = product_cut.vertexBegin(); m < product_cut.vertexEnd(); m = product_cut.vertexNext(m)){
            react_map_match.getAtomMap(product, react, m, &matching_map);

            for(int k = 0; k < matching_map.size();k++) {
               rsub_map_in[matching_map[k]] = m;
               map_exc = true;
               break;
            }
         }
      }

      if(!map_exc) 
         rsub_map_in.clear();

      RSubstructureMcs react_sub_mcs(reaction, react, product);
      bool find_sub = react_sub_mcs.searchSubstructureReact(_reaction.getBaseMolecule(react), &rsub_map_in, &rsub_map_out);
            
      if (!find_sub) {
         react_sub_mcs.searchMaxCommonSubReact(&rsub_map_in, &rsub_map_out);
      }

      bool cur_used = false;
      for (int j = 0; j < rsub_map_out.size(); j++) {
         int v = rsub_map_out.at(j);
         if (v >= 0) {
            cur_used = true;
            ++map_used;
            product_mapping_tmp[v] = reaction.getAAM(react, j);
            if (_usedVertices[product_mapping_tmp[v]] == 0)
               ++_usedVertices[0];
            product_cut.removeVertex(v);
         }
      }
      if(!cur_used)
         ++map_complete;

      if(product_cut.vertexCount() == 0) {
         map_complete += reactant_cons.size() - i - 1;
         break;
      }
   }
   _chooseBestMapping(reaction, product_mapping_tmp, product, map_used, map_complete);
}

void ReactionAutomapper::_chooseBestMapping(BaseReaction& reaction, Array<int>& product_mapping,  int product, int map_used, int map_complete) {
   bool map_u = map_used > _maxMapUsed;
   bool map_c = (map_used == _maxMapUsed) && (map_complete > _maxCompleteMap);
   bool map_v = (map_used == _maxMapUsed) && (map_complete == _maxCompleteMap) && (_usedVertices[0] > _maxVertUsed); 
   if(map_u || map_c || map_v){
      _maxMapUsed = map_used;
      _maxVertUsed = _usedVertices[0];
      _maxCompleteMap = map_complete;
      reaction.getAAMArray(product).copy(product_mapping);
   }
}


bool ReactionAutomapper::_checkAtomMapping(bool change_rc, bool change_aam, bool change_rc_null) {

   ReactionMapMatchingData map_match(_reaction);
   map_match.createBondMatchingData();
   
   QS_DEF(ObjArray< Array<int> >, bond_centers);
   QS_DEF(Array<int>, mapping);
   QS_DEF(Array<int>, v_mapping);
   QS_DEF(Array<int>, null_map);
   AutoPtr<BaseReaction> reaction_copy_ptr;
   QS_DEF(ObjArray< Array<int> > , react_invmap);

   null_map.clear();
   bool unchanged = true;
   
   bond_centers.clear();
   for(int i = 0; i < _reaction.end(); ++i)
      bond_centers.push();
   for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
      bond_centers[i].resize(_reaction.getBaseMolecule(i).edgeEnd());
      bond_centers[i].zerofill();
   }

   reaction_copy_ptr.reset(_reaction.neu());

   BaseReaction &reaction_copy = reaction_copy_ptr.ref();

   reaction_copy.clone(_reaction, 0, 0, &react_invmap);
   reaction_copy.aromatize();

   for (int mol_idx = _reaction.begin(); mol_idx != _reaction.end(); mol_idx = _reaction.next(mol_idx)) {
      BaseMolecule& rmol = _reaction.getBaseMolecule(mol_idx);
      for (int vert = rmol.vertexBegin(); vert < rmol.vertexEnd(); vert = rmol.vertexNext(vert)) {
         if (_reaction.getAAM(mol_idx, vert) > 0 && map_match.beginAtomMap(mol_idx, vert) >= map_match.endAtomMap()) {
            _reaction.getAAMArray(mol_idx).at(vert) = 0;
         }
      }
   }


   for (int mol_idx = _reaction.begin(); mol_idx != _reaction.end(); mol_idx = _reaction.next(mol_idx)) {
      BaseMolecule& rmol = _reaction.getBaseMolecule(mol_idx);
      for (int bond_idx = rmol.edgeBegin(); bond_idx < rmol.edgeEnd(); bond_idx = rmol.edgeNext(bond_idx)) {
         for(int opp_idx = map_match.beginMap(mol_idx); opp_idx < map_match.endMap(); opp_idx = map_match.nextMap(mol_idx, opp_idx)) {
            BaseMolecule& pmol = _reaction.getBaseMolecule(opp_idx);
            map_match.getBondMap(mol_idx, opp_idx, bond_idx, &mapping);

            if(mapping.size() == 0) {
               int ve_beg = rmol.getEdge(bond_idx).beg;
               int ve_end = rmol.getEdge(bond_idx).end;

               map_match.getAtomMap(mol_idx, opp_idx, ve_beg, &v_mapping);

               if(v_mapping.size() > 0) {
                  bool change_broken = true;
                  for(int v_map = 0; v_map < v_mapping.size(); ++v_map) {
                     const Vertex& end_vert = pmol.getVertex(v_mapping[v_map]);
                     for (int nei_vert = end_vert.neiBegin(); nei_vert < end_vert.neiEnd(); nei_vert = end_vert.neiNext(nei_vert)) {
                        int end_nei_vert = end_vert.neiVertex(nei_vert);
                        if (_reaction.getAAM(opp_idx, end_nei_vert) == 0 && MaxCommonSubmolecule::matchAtoms(rmol, pmol, 0, ve_end, end_nei_vert, 0)) {
                           change_broken = false;
                           break;
                        }
                     }
                  }
                  if (change_broken)
                     bond_centers[mol_idx][bond_idx] |= RC_MADE_OR_BROKEN;
               }

               map_match.getAtomMap(mol_idx, opp_idx, ve_end, &v_mapping);

               if (v_mapping.size() > 0) {
                  bool change_broken = true;
                  for (int v_map = 0; v_map < v_mapping.size(); ++v_map) {
                     const Vertex& beg_vert = pmol.getVertex(v_mapping[v_map]);
                     for (int nei_vert = beg_vert.neiBegin(); nei_vert < beg_vert.neiEnd(); nei_vert = beg_vert.neiNext(nei_vert)) {
                        int beg_nei_vert = beg_vert.neiVertex(nei_vert);
                        if (_reaction.getAAM(opp_idx, beg_nei_vert) == 0 && MaxCommonSubmolecule::matchAtoms(rmol, pmol, 0, ve_beg, beg_nei_vert, 0)) {
                           change_broken = false;
                           break;
                        }
                     }
                  }
                  if (change_broken)
                     bond_centers[mol_idx][bond_idx] |= RC_MADE_OR_BROKEN;
               }

            } else {
               for(int i = 0; i < mapping.size(); ++i) {

                  const Edge &r_edge = rmol.getEdge(bond_idx);
                  const Edge &p_edge = pmol.getEdge(mapping[i]);
                  
                  BaseMolecule& cr_mol = reaction_copy.getBaseMolecule(mol_idx);
                  BaseMolecule& cp_mol = reaction_copy.getBaseMolecule(opp_idx);
                  
                  int mol_edge_idx = cr_mol.findEdgeIndex(react_invmap.at(mol_idx)[r_edge.beg], react_invmap.at(mol_idx)[r_edge.end]);
                  int opp_edge_idx = cp_mol.findEdgeIndex(react_invmap.at(opp_idx)[p_edge.beg], react_invmap.at(opp_idx)[p_edge.end]);
                     
                  bool react_arom = cr_mol.getBondOrder(mol_edge_idx) == BOND_AROMATIC;
                  bool prod_arom = cp_mol.getBondOrder(opp_edge_idx) == BOND_AROMATIC;

                  if (change_aam && (react_arom || prod_arom) ) {
                      bond_centers[mol_idx][bond_idx] |= _reaction.getReactingCenter(mol_idx, bond_idx);
                      continue;
                  }

                  bool bond_cond_simple = MaxCommonSubmolecule::matchBonds(pmol, rmol, mapping[i], bond_idx, 0);
                  
                  if (bond_cond_simple || (react_arom && prod_arom)) 
                     bond_centers[mol_idx][bond_idx] |= RC_UNCHANGED;
                  else 
                     bond_centers[mol_idx][bond_idx] |= RC_ORDER_CHANGED;
                  
                  if(bond_cond_simple && (react_arom != prod_arom))
                      bond_centers[mol_idx][bond_idx] |= RC_ORDER_CHANGED;
                     
               }
            }
         }
      }
   }

   for (int mol_idx = _reaction.begin(); mol_idx != _reaction.end(); mol_idx = _reaction.next(mol_idx)) {
      BaseMolecule& rmol = _reaction.getBaseMolecule(mol_idx);
      for (int bond_idx = rmol.edgeBegin(); bond_idx < rmol.edgeEnd(); bond_idx = rmol.edgeNext(bond_idx)) {
         int rc_bond = _reaction.getReactingCenter(mol_idx, bond_idx);
         bool aam_bond = ((_reaction.getAAM(mol_idx, rmol.getEdge(bond_idx).beg) > 0) && (_reaction.getAAM(mol_idx, rmol.getEdge(bond_idx).end) > 0)) || change_rc_null;
         if (aam_bond && ((bond_centers[mol_idx][bond_idx] & ~rc_bond) || rc_bond == 0)) {
            if (!change_rc && !change_aam) {
               return false;
            } else if(change_rc) {
               _reaction.getReactingCenterArray(mol_idx).at(bond_idx) = bond_centers[mol_idx][bond_idx];
               unchanged = false;
            } else if(change_aam && rc_bond && _reaction.getSideType(mol_idx)== Reaction::REACTANT) {
               //only rc != 0 0 can match on every type of the bond
               //only reactants rules (lazy users)
               null_map.push(_reaction.getAAM(mol_idx, rmol.getEdge(bond_idx).beg));
               null_map.push(_reaction.getAAM(mol_idx, rmol.getEdge(bond_idx).end));
               unchanged = false;
            }
         }
      }
   }
   //erase all wrong map
   if(change_aam) {
      for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
         BaseMolecule& rmol = _reaction.getBaseMolecule(i);
         for(int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom)) {
            for(int j = 0; j < null_map.size(); ++j) {
               if(null_map[j] == _reaction.getAAM(i, atom))
                  _reaction.getAAMArray(i).at(atom) = 0;
            }
         }
      }
      null_map.clear();
      for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
         BaseMolecule& rmol = _reaction.getBaseMolecule(i);
         for(int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom)) {
            const Vertex& vertex = rmol.getVertex(atom);
            if(_reaction.getAAM(i ,atom) > 0 && vertex.degree() > 0) {
               bool has_aam = false;
               for(int nei = vertex.neiBegin(); nei < vertex.neiEnd(); nei = vertex.neiNext(nei)) {
                  int nei_atom = vertex.neiVertex(nei);
                  if(_reaction.getAAM(i, nei_atom) > 0) {
                     has_aam = true;
                     break;
                  }
               }
               if(!has_aam)
                  null_map.push(_reaction.getAAM(i, atom));
            }
         }
      }
      for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
         BaseMolecule& rmol = _reaction.getBaseMolecule(i);
         for(int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom)) {
            for(int j = 0; j < null_map.size(); ++j) {
               if(null_map[j] == _reaction.getAAM(i, atom))
                  _reaction.getAAMArray(i).at(atom) = 0;
            }
         }
      }

   }

   return unchanged;

}

void ReactionAutomapper::_setupReactionMap(int mode, BaseReaction& reaction_copy, Array<int> &mol_mapping, ObjArray< Array<int> >& mappings){
   int i,j,v;
   if(mode == AAM_REGEN_KEEP)
      _usedVertices.zerofill();
   for (i = _reaction.productBegin(); i < _reaction.productEnd(); i = _reaction.productNext(i)){
      int ii = mol_mapping[i];
      for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
         v = reaction_copy.getAAM(ii, mappings[i][j]);
         if(mode == AAM_REGEN_DISCARD)
            _reaction.getAAMArray(i)[j] = v;

         if(mode == AAM_REGEN_ALTER)
            _reaction.getAAMArray(i)[j] = v;

         if(mode == AAM_REGEN_KEEP && _reaction.getAAM(i, j) == 0){
            _reaction.getAAMArray(i)[j] = v;
            _usedVertices[v] = 1;
         }
      }
   }
   for (i = _reaction.reactantBegin(); i < _reaction.reactantEnd(); i = _reaction.reactantNext(i)){
      int ii = mol_mapping[i];
      for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
         v = reaction_copy.getAAM(ii, mappings[i][j]);
         if(mode == AAM_REGEN_DISCARD)
            _reaction.getAAMArray(i)[j] = v*_usedVertices[v];
         if(mode == AAM_REGEN_ALTER)
            _reaction.getAAMArray(i)[j] = v*_usedVertices[v];
         if(mode == AAM_REGEN_KEEP && _reaction.getAAM(i, j) == 0)
            _reaction.getAAMArray(i)[j] = v*_usedVertices[v];
      }
   }
}

void ReactionAutomapper::_considerDissociation(){
   Molecule null_map_cut;
   Molecule full_map_cut;
   QS_DEF(Array<int>,map);
   int i, j, mcv, mcvsum;

   for (i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)){
      mcvsum = 0;
      mcv = 0;
      for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
         if(_reaction.getAAM(i, j) == 0)
            mcvsum++;
         else
            mcv++;
      }
      if(mcvsum < mcv || mcv <= _MIN_VERTEX_SUB)
         continue;
      
      full_map_cut.clone(_reaction.getBaseMolecule(i), 0, 0);
      MoleculeAromatizer::aromatizeBonds(full_map_cut);

      for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
         if(_reaction.getAAM(i, j) == 0)
            full_map_cut.removeVertex(j);
      }
      if(full_map_cut.vertexCount() == 0)
         continue;
      while(mcvsum >= mcv){
         null_map_cut.clone(_reaction.getBaseMolecule(i), 0, 0);
         MoleculeAromatizer::aromatizeBonds(null_map_cut);
         for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
            if(_reaction.getAAM(i, j) > 0 || _reaction.getBaseMolecule(i).getAtomNumber(j) == ELEM_H)
               null_map_cut.removeVertex(j);
         }
         if(null_map_cut.vertexCount() == 0)
            break;
         SubstructureMcs sss(full_map_cut, null_map_cut);
         sss.cbMatchVertex = MaxCommonSubmolecule::matchAtoms;
         sss.cbMatchEdge = MaxCommonSubmolecule::matchBonds;

         map.clear();
         if(!sss.searchSubstructure(&map))
            break;
         for(j = 0; j < map.size(); j++){
            if(map[j] >= 0 && map[j] < _reaction.getAAMArray(i).size()){
               _reaction.getAAMArray(i)[map[j]] = _reaction.getAAM(i, j);
            }
         }
         mcvsum = 0;
         for (j = 0; j < _reaction.getAAMArray(i).size(); j++){
            if(_reaction.getAAM(i, j) == 0)
               mcvsum++;
         }
      }
   }

}
void ReactionAutomapper::_considerDimerization() {
   QS_DEF(ObjArray< Array<int> >, inv_mappings);
   QS_DEF(Array<int>, sub_map);
   QS_DEF(Array<int>, max_sub_map);
   AutoPtr<BaseReaction> reaction_copy_ptr;

   bool way_exit = true, map_changed = false;
   int map_found, max_found , max_react_index = -1;
   reaction_copy_ptr.reset(_reaction.neu());

   BaseReaction &reaction_copy = reaction_copy_ptr.ref();

   reaction_copy.clone(_reaction, 0, 0, &inv_mappings);

   for(int prod = reaction_copy.productBegin(); prod < reaction_copy.productEnd(); prod = reaction_copy.productNext(prod)) {
      BaseMolecule& pmol = reaction_copy.getBaseMolecule(prod);
      pmol.aromatize();
      way_exit = true;
      while(way_exit) {
         for(int vert = pmol.vertexBegin(); vert < pmol.vertexEnd(); vert = pmol.vertexNext(vert)) {
            if((reaction_copy.getAAM(prod, vert) > 0) || (pmol.getAtomNumber(vert) == ELEM_H))
               pmol.removeVertex(vert);
         }
         for(int edg = pmol.edgeBegin(); edg < pmol.edgeEnd(); edg = pmol.edgeNext(edg)) {
            if(reaction_copy.getReactingCenter(prod, edg) == RC_MADE_OR_BROKEN)
               pmol.removeEdge(edg);
         }
         _removeSmallComponents(pmol);
         if(pmol.vertexCount() < _MIN_VERTEX_SUB)
            way_exit = false;

         max_found = _MIN_VERTEX_SUB;
         for(int react = reaction_copy.reactantBegin(); react < reaction_copy.reactantEnd() && way_exit; react = reaction_copy.reactantNext(react)) {
            
            map_found = _validMapFound(reaction_copy, react, prod, sub_map);
            
            if(map_found > max_found){
               max_found = map_found;
               max_sub_map.copy(sub_map);
               max_react_index = react;
            }
         }
         if(max_found > _MIN_VERTEX_SUB) {
            for(int i = 0; i < max_sub_map.size(); ++i) {
               if(max_sub_map[i] >= 0) {
                  reaction_copy.getAAMArray(prod).at(max_sub_map[i]) = reaction_copy.getAAM(max_react_index, i);
                  map_changed = true;
               }
            }
         } else {
            way_exit = false;
         }
      }
   }

   if(map_changed) {
      for(int rindex = _reaction.productBegin(); rindex < _reaction.productEnd(); rindex = _reaction.productNext(rindex)) {
         BaseMolecule& rmol = _reaction.getBaseMolecule(rindex);
         for(int vert = rmol.vertexBegin(); vert < rmol.vertexEnd(); vert = rmol.vertexNext(vert)) {
            int copy_aam = reaction_copy.getAAM(rindex, inv_mappings[rindex].at(vert));
            if(_reaction.getAAM(rindex, vert) == 0 && copy_aam  > 0)
               _reaction.getAAMArray(rindex).at(vert) = copy_aam ;
         }
      }
   }
}

int ReactionAutomapper::_validMapFound(BaseReaction& reaction, int react, int prod, Array<int>& sub_map) const {

   BaseMolecule &react_copy = reaction.getBaseMolecule(react);
   int result = 0;
   react_copy.clone(_reaction.getBaseMolecule(react), 0, 0);
   react_copy.aromatize();

   for(int vert = react_copy.vertexBegin(); vert < react_copy.vertexEnd(); vert = react_copy.vertexNext(vert)) {
      if((reaction.getAAM(react, vert) == 0) || (react_copy.getAtomNumber(vert) == ELEM_H))
         react_copy.removeVertex(vert);
   }
   for(int edg = react_copy.edgeBegin(); edg < react_copy.edgeEnd(); edg = react_copy.edgeNext(edg)) {
      if(reaction.getReactingCenter(react, edg) == RC_MADE_OR_BROKEN)
         react_copy.removeEdge(edg);
   }
   _removeSmallComponents(react_copy);

   if(react_copy.vertexCount() < _MIN_VERTEX_SUB)
      return result;

   RSubstructureMcs rsub_mcs(reaction, react, prod);
   rsub_mcs.cbMatchVertex = RSubstructureMcs::atomConditionReact;
   rsub_mcs.cbMatchEdge = RSubstructureMcs::bondConditionReact;
   rsub_mcs.userdata = &rsub_mcs;

   if(rsub_mcs.searchSubstructure(&sub_map)) 
      result = __min(react_copy.vertexCount(), reaction.getBaseMolecule(prod).vertexCount());

   return result;
}

void ReactionAutomapper::_removeSmallComponents(BaseMolecule& mol) const {

   int ncomp = mol.countComponents();
   const Array<int> &decomposition = mol.getDecomposition();

   QS_DEF(Array<int>, vertices_to_remove);
   vertices_to_remove.clear();
   
   for(int comp_idx = 0; comp_idx < ncomp; ++comp_idx) {
      if(mol.countComponentVertices(comp_idx) < _MIN_VERTEX_SUB) {
         for(int j = mol.vertexBegin(); j < mol.vertexEnd(); j = mol.vertexNext(j))
            if(decomposition[j] == comp_idx)
               vertices_to_remove.push(j);
      }
   }
   for (int i = 0; i < vertices_to_remove.size(); ++i) {
      mol.removeVertex(vertices_to_remove[i]);
   }

}

 

//all transpositions for numbers from 0 to n-1
void ReactionAutomapper::_permutation(Array<int>& s_array, ObjArray< Array<int> > &p_array) {
   
   p_array.clear();
   QS_DEF(Array<int>, per);
   QS_DEF(Array<int>, obr);
   int n = s_array.size();
   int i, j, k, tmp, min = -1, raz;
   bool flag;
   per.resize(n);
   obr.resize(n);
   for (i = 0; i < n; i++) {
      per[i] = i+1;
   }
   while (1) {
      /*
       * Break to escape permutations out of memory error
       */
      if(p_array.size() > MAX_PERMUTATIONS_NUMBER)
         break;
      Array<int>& new_array = p_array.push();
      new_array.resize(n);
      for (k = 0; k < n; k++) {
         new_array.at(k) = s_array.at(per[k]-1);
      }
      flag = false;
      for (i = n - 2; i >= 0; i--) {
         if (per[i] < per[i + 1]) {
            flag = true;
            break;
         }
      }
      if (flag == false) {
         break;
      }
      raz = per[i+1];
      for (j = i+1; j < n; j++) {
         if (((per[j] - per[i]) < raz) && (per[i] < per[j])) {
            min = j;
         }
      }
      tmp = per[i];
      per[i] = per[min];
      per[min] = tmp;
      for (j = i + 1; j < n; j++) {
         obr[j] = per[j];
      }
      j = i + 1;
      for (k = n-1; k >= i+1; k--) {
         per[j] = obr[k];
         j++;
      }
   }
}




ReactionMapMatchingData::ReactionMapMatchingData(BaseReaction &r) :
_reaction(r) {
}

void ReactionMapMatchingData::createAtomMatchingData(){

   _vertexMatchingArray.clear();
   for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
      for(int j = 0; j < _reaction.getBaseMolecule(i).vertexEnd(); j++) {
         _vertexMatchingArray.push();
      }
   }


   for(int react = _reaction.reactantBegin(); react < _reaction.reactantEnd(); react = _reaction.reactantNext(react)) {
      BaseMolecule& rmol = _reaction.getBaseMolecule(react);
      for(int rvert = rmol.vertexBegin(); rvert < rmol.vertexEnd(); rvert = rmol.vertexNext(rvert)) {
         if(_reaction.getAAM(react, rvert) > 0){
            for(int prod = _reaction.productBegin(); prod < _reaction.productEnd(); prod = _reaction.productNext(prod)) {
               BaseMolecule& pmol = _reaction.getBaseMolecule(prod);
               for(int pvert = pmol.vertexBegin(); pvert < pmol.vertexEnd(); pvert = pmol.vertexNext(pvert)) {
                  if(_reaction.getAAM(react, rvert) == _reaction.getAAM(prod, pvert)){
                     int v_id1 = _getVertexId(react, rvert);
                     int v_id2 = _getVertexId(prod, pvert);
                     _vertexMatchingArray[v_id1].push(v_id2);
                     _vertexMatchingArray[v_id2].push(v_id1);
                  }
               }
            }
         }
      }
   }
   
}

void ReactionMapMatchingData::createBondMatchingData() {

   QS_DEF(Array<int>, matchingMap1);
   QS_DEF(Array<int>, matchingMap2);
   int v1,v2;

   createAtomMatchingData();

   _edgeMatchingArray.clear();
   for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
      for(int j = 0; j < _reaction.getBaseMolecule(i).edgeEnd(); j++) {
         _edgeMatchingArray.push();
      }
   }


   for(int react = _reaction.reactantBegin(); react < _reaction.reactantEnd(); react = _reaction.reactantNext(react)) {
      BaseMolecule& rmol = _reaction.getBaseMolecule(react);
      for(int redge = rmol.edgeBegin(); redge < rmol.edgeEnd(); redge = rmol.edgeNext(redge)){
         for(int prod = _reaction.productBegin(); prod < _reaction.productEnd(); prod = _reaction.productNext(prod)) {
            v1 = rmol.getEdge(redge).beg;
            v2 = rmol.getEdge(redge).end;
            getAtomMap(react, prod, v1, &matchingMap1);
            getAtomMap(react, prod, v2, &matchingMap2);
            for(int m = 0; m < matchingMap1.size(); m++){
               for(int n = 0; n < matchingMap2.size(); n++){
                  int pedge = _reaction.getBaseMolecule(prod).findEdgeIndex(matchingMap1[m],matchingMap2[n]);
                  if(pedge >= 0){
                     int e_id1 = _getEdgeId(react,redge);
                     int e_id2 = _getEdgeId(prod, pedge);
                     _edgeMatchingArray[e_id1].push(e_id2);
                     _edgeMatchingArray[e_id2].push(e_id1);
                  }
               }
            }
         }
      }
   }
}


int ReactionMapMatchingData::endMap() const { 
   return _reaction.sideEnd(); 
}

int ReactionMapMatchingData::nextMap(int mol_idx, int opposite_idx) const {
   int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT:Reaction::REACTANT;
   return _reaction.sideNext(side_type, opposite_idx);
}

int ReactionMapMatchingData::endAtomMap() const { 
   return _reaction.sideEnd(); 
}

int ReactionMapMatchingData::nextAtomMap(int mol_idx, int opposite_idx, int atom_idx) const {
   int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT:Reaction::REACTANT;
   int rm_idx = _reaction.sideNext(side_type, opposite_idx);
   for(;rm_idx < _reaction.sideEnd(); rm_idx = _reaction.sideNext(side_type, rm_idx)) {
      if(getAtomMap(mol_idx, rm_idx, atom_idx, 0))
         break;
   }
   return rm_idx;
}

bool ReactionMapMatchingData::getAtomMap(int mol_idx, int opposite_idx, int atom_idx, Array<int>* mapping) const {
   bool result = false;
   int vertex_id = _getVertexId(mol_idx, atom_idx);
   int first_r_vertex = _getVertexId(opposite_idx, 0);
   int last_r_vertex = _getVertexId(opposite_idx+1, 0);
   if(mapping)
      mapping->clear();
   for(int i = 0; i < _vertexMatchingArray[vertex_id].size(); i++){
      int m_vertex = _vertexMatchingArray[vertex_id].at(i);
      if(m_vertex >= first_r_vertex && m_vertex < last_r_vertex) {
         if(mapping)
            mapping->push(m_vertex - first_r_vertex);
         else
            return true;
         result = true;
      }
   }
   return result;
}

int ReactionMapMatchingData::endBondMap() const { 
   return _reaction.sideEnd(); 
}

int ReactionMapMatchingData::nextBondMap(int mol_idx, int opposite_idx, int bond_idx) const {
   int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT:Reaction::REACTANT;
   int rm_idx = _reaction.sideNext(side_type, opposite_idx);
   for(;rm_idx < _reaction.sideEnd(); rm_idx = _reaction.sideNext(side_type, rm_idx)) {
      if(getBondMap(mol_idx, rm_idx, bond_idx, 0))
         break;
   }
   return rm_idx;
}

bool ReactionMapMatchingData::getBondMap(int mol_idx, int opposite_idx, int bond_idx, Array<int>* mapping) const {
   bool result = false;
   int edge = _getEdgeId(mol_idx, bond_idx);
   int first_r_edge = _getEdgeId(opposite_idx,0);
   int last_r_edge = _getEdgeId(opposite_idx+1,0);
   if(mapping)
      mapping->clear();
   for(int i = 0; i < _edgeMatchingArray[edge].size(); i++){
      int m_edge = _edgeMatchingArray[edge].at(i);
      if(m_edge >= first_r_edge && m_edge < last_r_edge) {
         if(mapping)
            mapping->push(m_edge - first_r_edge);
         else
            return true;
         
         result = true;
      }
   }
   return result;
}


int ReactionMapMatchingData::_getVertexId(int mol_idx, int vert) const {
   int result = 0;
   for(int i = _reaction.begin(); i < mol_idx; i = _reaction.next(i)) {
      result += _reaction.getBaseMolecule(i).vertexEnd();
   }
   result += vert;
   return result;
}

int ReactionMapMatchingData::_getEdgeId(int mol_idx, int edge) const {
   int result = 0;
   for(int i = _reaction.begin(); i < mol_idx; i = _reaction.next(i)) {
      result += _reaction.getBaseMolecule(i).edgeEnd();
   }
   result += edge;
   return result;
}

RSubstructureMcs::RSubstructureMcs(BaseReaction &reaction,  int sub_num, int super_num):
SubstructureMcs(reaction.getBaseMolecule(sub_num), reaction.getBaseMolecule(super_num)),
_reaction(reaction), 
_subReactNumber(sub_num), 
_superProductNumber(super_num) 
{}


bool RSubstructureMcs::searchSubstructureReact(BaseMolecule& init_rmol, const Array<int>* in_map, Array<int> *out_map){

   QS_DEF(ObjArray< Array<int> >, tmp_maps);
   QS_DEF(ObjArray<EmbeddingEnumerator>, emb_enums);
   QS_DEF(Array<int>, in_map_cut);
   QS_DEF(Array<int>, results);

   emb_enums.clear();
   tmp_maps.clear();
   int map_result = 0;
   results.resize(4);

   BaseMolecule& mol_react = _reaction.getBaseMolecule(_subReactNumber);

   int react_vsize = mol_react.vertexCount();

   if(react_vsize < 2) {
      mol_react.clone(init_rmol, 0, 0);
      react_vsize = mol_react.vertexCount();
      mol_react.aromatize();
   }

   if(_super->vertexCount() < 2 || _sub->vertexCount() < 2)
      return false;

   for(int i = 0; i < 4; ++i) {
      EmbeddingEnumerator& emb_enum = emb_enums.push(*_super);
      emb_enum.setSubgraph(*_sub);
      emb_enum.cb_match_vertex = RSubstructureMcs::atomConditionReact;
      emb_enum.cb_embedding = _embedding;
      emb_enum.userdata = this;
      if(i & 1)
         emb_enum.cb_match_edge = bondConditionReact;
      else
         emb_enum.cb_match_edge = bondConditionReactStrict;
      tmp_maps.push().clear();
      results[i] = -1;
   }  


   Array<int>* in_map_c = 0;
   
   if(react_vsize > 0 && in_map != 0 && in_map->size() > 0) {
      in_map_c = &in_map_cut;

      in_map_cut.clear();
      in_map_cut.resize(mol_react.vertexEnd());

      for(int k = 0; k < in_map_cut.size(); k++)
         in_map_cut[k] = SubstructureMcs::UNMAPPED;
      
      for(int k = mol_react.vertexBegin(); k < mol_react.vertexEnd(); k = mol_react.vertexNext(k)) {
         in_map_cut[k] = in_map->at(k);
      }
   }

   //first more strict rules then more weak

   results[0] = _searchSubstructure(emb_enums[0], in_map_c, &tmp_maps[0]);
   results[1] = _searchSubstructure(emb_enums[1], in_map_c, &tmp_maps[1]);

   mol_react.clone(init_rmol, 0, 0);
   mol_react.aromatize();

   if(mol_react.vertexCount() > react_vsize) {
      results[2] = _searchSubstructure(emb_enums[2], in_map, &tmp_maps[2]);
      results[3] = _searchSubstructure(emb_enums[3], in_map, &tmp_maps[3]);
   }

   map_result = 3;
   for(int i = 2; i >= 0; --i) {
      if(results[i] >= results[map_result])
         map_result = i;
   }
   if(results[map_result] < 2)
      return false;

   if(out_map != 0)
      out_map->copy(tmp_maps[map_result]);
   return true;
}

bool RSubstructureMcs::searchMaxCommonSubReact(const Array<int>* in_map, Array<int> *out_map) {

   if(out_map != 0)
      out_map->clear();

   if(_super->vertexCount() < 2 || _sub->vertexCount() < 2)
      return false;

   Molecule *sub_molecule;
   Molecule *super_molecule;
   if(_invert) {
      sub_molecule = (Molecule*)_super;
      super_molecule = (Molecule*)_sub;
   } else {
      sub_molecule = (Molecule*)_sub;
      super_molecule = (Molecule*)_super;
   }
      
   MaxCommonSubmolecule mcs(*sub_molecule, *super_molecule);
   mcs.parametersForExact.maxIteration = MAX_ITERATION_NUMBER; 
   mcs.conditionVerticesColor = atomConditionReact;
   mcs.conditionEdgeWeight = bondConditionReact;
   mcs.cbSolutionTerm = cbMcsSolutionTerm;  
   mcs.userdata = this;
   mcs.parametersForApproximate.randomize = true;

   if(in_map != 0) 
      mcs.incomingMap.copy(*in_map);

   mcs.findExactMCS();
   if (mcs.parametersForExact.isStopped) {
      mcs.findApproximateMCS();
   }

   mcs.getMaxSolutionMap(out_map, 0);
   return true;
}

int RSubstructureMcs::findReactionCenter(BaseMolecule& mol, int bondNum) const {
   int edgeInd = bondNum;
   if(_sub == &mol){
      if(_invert){
         return _reaction.getReactingCenter(_superProductNumber, edgeInd);
      }else{
         return _reaction.getReactingCenter(_subReactNumber, edgeInd);
      }
   }
   if(_super == &mol){
      if(_invert){
         return _reaction.getReactingCenter(_subReactNumber, edgeInd);
      }else{
         return _reaction.getReactingCenter(_superProductNumber, edgeInd);
      }
   }
   return -2;
}

void RSubstructureMcs::getReactingCenters(BaseMolecule& mol1, BaseMolecule& mol2, int bond1, int bond2, int& rc_reactant, int& rc_product) const {
   if(_sub == &mol1 && _super == &mol2){
      if(_invert) {
         rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond2);
         rc_product = _reaction.getReactingCenter(_superProductNumber, bond1);
      }else{
         rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond1);
         rc_product = _reaction.getReactingCenter(_superProductNumber, bond2);
      }
   }
   if(_sub == &mol2 && _super == &mol1){
      if(_invert) {
         rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond1);
         rc_product = _reaction.getReactingCenter(_superProductNumber, bond2);
      }else{
         rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond2);
         rc_product = _reaction.getReactingCenter(_superProductNumber, bond1);
      }
   }
}

bool RSubstructureMcs::atomConditionReact (Graph &g1, Graph &g2, const int *core_sub, int i, int j, void* userdata) {
   return MaxCommonSubmolecule::matchAtoms(g1, g2, 0, i, j, 0);
}
bool RSubstructureMcs::bondConditionReact (Graph &g1, Graph &g2, int i, int j, void* userdata){

   Molecule &mol1 = (Molecule &)g1;
   Molecule &mol2 = (Molecule &)g2;
   RSubstructureMcs &rsm = *(RSubstructureMcs *)userdata;

   int rc_reactant;
   int rc_product;

   rsm.getReactingCenters(mol1, mol2, i, j, rc_reactant, rc_product);

   //not consider
   if(rc_reactant == RC_MADE_OR_BROKEN || rc_product == RC_MADE_OR_BROKEN)
      return false;

   //aromatic
   if(mol1.getBondOrder(i) == BOND_AROMATIC || mol2.getBondOrder(j) == BOND_AROMATIC)
      return true;

   //not change 
   if(rc_reactant == RC_UNMARKED && rc_product == RC_UNMARKED)
      return mol1.getBondOrder(i) == mol2.getBondOrder(j);

   //not change reactants
   if((rc_reactant == RC_NOT_CENTER || rc_reactant == RC_UNCHANGED || rc_reactant == RC_UNCHANGED + RC_MADE_OR_BROKEN))
      return mol1.getBondOrder(i)  ==  mol2.getBondOrder(j);

   //change reactants
   if((rc_reactant == RC_ORDER_CHANGED || rc_reactant == RC_ORDER_CHANGED+RC_MADE_OR_BROKEN))
      return mol1.getBondOrder(i) != mol2.getBondOrder(j);

   //not change products
   if((rc_product == RC_NOT_CENTER || rc_product == RC_UNCHANGED || rc_product == RC_UNCHANGED + RC_MADE_OR_BROKEN))
      return mol1.getBondOrder(i)  ==  mol2.getBondOrder(j);

   //change products
   if((rc_product == RC_ORDER_CHANGED || rc_product == RC_ORDER_CHANGED+RC_MADE_OR_BROKEN))
      return mol1.getBondOrder(i) != mol2.getBondOrder(j);

   //can change
   return true;
}

bool RSubstructureMcs::bondConditionReactStrict (Graph &g1, Graph &g2, int i, int j, void* userdata) {

   Molecule &mol1 = (Molecule &)g1;
   Molecule &mol2 = (Molecule &)g2;

   RSubstructureMcs &rsm = *(RSubstructureMcs *)userdata;

   int rc_reactant;
   int rc_product;

   rsm.getReactingCenters(mol1, mol2, i, j, rc_reactant, rc_product);

   //not consider
   if((rc_reactant & RC_MADE_OR_BROKEN) || (rc_product & RC_MADE_OR_BROKEN))
      return false;

   //aromatic
   if(mol1.getBondOrder(i) == BOND_AROMATIC || mol2.getBondOrder(j) == BOND_AROMATIC)
      return true;

   //not change 
   if(rc_reactant == RC_UNMARKED && rc_product == RC_UNMARKED)
      return mol1.getBondOrder(i) == mol2.getBondOrder(j);

   //not change reactants
   if(rc_reactant == RC_NOT_CENTER || rc_reactant == RC_UNCHANGED)
      return mol1.getBondOrder(i)  ==  mol2.getBondOrder(j);

   //change reactants
   if(rc_reactant == RC_ORDER_CHANGED)
      return mol1.getBondOrder(i) != mol2.getBondOrder(j);

   //not change products
   if((rc_product == RC_NOT_CENTER || rc_product == RC_UNCHANGED))
      return mol1.getBondOrder(i)  ==  mol2.getBondOrder(j);

   //change products
   if(rc_product == RC_ORDER_CHANGED)
      return mol1.getBondOrder(i) != mol2.getBondOrder(j);

   //can change
   return true;
}

int RSubstructureMcs::cbMcsSolutionTerm(Array<int>& a1, Array<int>& a2, void* context) {
   int result = MaxCommonSubgraph::ringsSolutionTerm(a1, a2, context);
   if(result == 0) {
      const RSubstructureMcs* r_sub_mcs = (const RSubstructureMcs *)context;
      BaseReaction& reaction = r_sub_mcs->getReaction();
      int react = r_sub_mcs->getReactNumber();
      int prod = r_sub_mcs->getProductNumber();
      int e_size1 = a1[1];
      int e_size2 = a2[1];
      int r1_rc= 0, r2_rc = 0;
      for(int i = 0; i < e_size1; ++i) {
         int e_map = a1.at(2+a1[0]+i);
         if(e_map >= 0) {
            if((reaction.getReactingCenter(react, i) & RC_MADE_OR_BROKEN))
               ++r1_rc;
            if((reaction.getReactingCenter(prod, e_map) & RC_MADE_OR_BROKEN))
               ++r1_rc;
         }
      }
      for(int i = 0; i < e_size2; ++i) {
         int e_map = a2.at(2+a2[0]+i);
         if(e_map >= 0) {
            if((reaction.getReactingCenter(react, i) & RC_MADE_OR_BROKEN))
               ++r2_rc;
            if((reaction.getReactingCenter(prod, e_map) & RC_MADE_OR_BROKEN))
               ++r2_rc;
         }
      }
      result = r1_rc - r2_rc;
   }
   return result;
}


int RSubstructureMcs::_searchSubstructure(EmbeddingEnumerator& emb_enum, const Array<int>* in_map, Array<int> *out_map) {
   

   int result = 0;
   if(in_map != 0) {
      for(int i = 0; i < in_map->size(); i++) {
         if(in_map->at(i) >= 0 && !_invert) {
            if(!emb_enum.fix(i, in_map->at(i))){
               result = -1;
               break;
            }
         }

         if(in_map->at(i) >= 0 && _invert) {
            if(!emb_enum.fix(in_map->at(i), i)){
               result = -1;
               break;
            }
         }
      }
   }

   if(result == -1)
      return result;

   int proc = emb_enum.process();

   if(proc == 1)
      return -1;

   int ncomp = _sub->countComponents();
   const Array<int> &decomposition = _sub->getDecomposition();

   int j, max_index = 0;

   for (j = 1; j < ncomp; j++)
      if (_sub->countComponentVertices(j) > _sub->countComponentVertices(max_index))
         max_index = j;
   
   if(out_map != 0){
      if(!_invert){
         out_map->clear_resize(_sub->vertexEnd());
         for(int i = 0;i < out_map->size();i++)
            out_map->at(i) = SubstructureMcs::UNMAPPED;
         for(int i = _sub->vertexBegin(); i < _sub->vertexEnd(); i = _sub->vertexNext(i)) {
            out_map->at(i) = emb_enum.getSubgraphMapping()[i];
            if(max_index != decomposition[i])
               out_map->at(i) = SubstructureMcs::UNMAPPED;

            if(out_map->at(i) >= 0)
               ++result;
         }
      }else{
         out_map->clear_resize(_super->vertexEnd());
         for(int i = 0;i < out_map->size();i++)
            out_map->at(i) = SubstructureMcs::UNMAPPED;
         for(int i = _super->vertexBegin(); i < _super->vertexEnd(); i = _super->vertexNext(i)) {
            out_map->at(i) = emb_enum.getSupergraphMapping()[i];
            if((out_map->at(i) >= 0) && (max_index != decomposition[out_map->at(i)]))
               out_map->at(i) = SubstructureMcs::UNMAPPED;
            if(out_map->at(i) >= 0)
               ++result;
         }
      }
   }
   return result;
}
