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

#ifndef __reaction_enumerator_state__
#define __reaction_enumerator_state__

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "graph/embedding_enumerator.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/obj.h"

namespace indigo {

class ReactionEnumeratorContext
{
public:
   AromaticityOptions arom_options;
};

class ReactionEnumeratorState
{
public:
   DECL_ERROR;

   class ReactionMonomers
   {
   public:
      DECL_ERROR;

      TL_CP_DECL(ReusableObjArray<Molecule>, _monomers);
      TL_CP_DECL(Array<int>, _reactant_indexes);
      TL_CP_DECL(Array<int>, _deep_levels);
      TL_CP_DECL(Array<int>, _tube_indexes);

      ReactionMonomers();

      int size();
      
      void clear();

      Molecule & getMonomer( int reactant_idx, int index );

      Molecule & getMonomer( int mon_index );

      void addMonomer( int reactant_idx, Molecule &monomer, int deep_level = 0, int tube_idx = -1 );

      void removeMonomer( int idx );
   };
   
   void (*product_proc)( Molecule &product, Array<int> &monomers_indices, void *userdata );
   void *userdata;
   bool is_multistep_reaction;
   bool is_self_react;
   bool is_one_tube;
   bool is_same_keeping;
   bool is_transform;

   int max_deep_level;
   int max_product_count;
   int max_reuse_count;
   
   ReactionEnumeratorState(ReactionEnumeratorContext &context, QueryReaction &cur_reaction, QueryMolecule &cur_full_product, 
      Array<int> &cur_product_aam_array, RedBlackStringMap<int> &cur_smiles_array, 
      ReactionMonomers &cur_reaction_monomers, int &cur_product_coint, 
      ObjArray< Array<int> > &cur_tubes_monomers );
         
   ReactionEnumeratorState( ReactionEnumeratorState &cur_rpe_state );

   int buildProduct( void );

   bool performSingleTransformation( Molecule &molecule, Array<int> &forbidden_atoms, Array<int> &original_hydrogens );

private:
   ReactionEnumeratorContext &_context;

   QueryReaction &_reaction;
   int _reactant_idx;

   int &_product_count;

   ObjArray< Array<int> > &_tubes_monomers;
   Array<int> &_product_aam_array;
   RedBlackStringMap<int> &_smiles_array;
   ReactionMonomers &_reaction_monomers;

   TL_CP_DECL(Array<int>, _fragments_aam_array);
   TL_CP_DECL(QueryMolecule, _full_product);
   TL_CP_DECL(Array<int>, _product_monomers);
   TL_CP_DECL(Molecule, _fragments);
   TL_CP_DECL(Array<int>, _is_needless_atom);
   TL_CP_DECL(Array<int>, _is_needless_bond);
   TL_CP_DECL(Array<int>, _bonds_mapping_sub);
   TL_CP_DECL(Array<int>, _bonds_mapping_super);
   TL_CP_DECL(ObjArray< Array<int> >, _att_points);
   TL_CP_DECL(MoleculeSubstructureMatcher::FragmentMatchCache, _fmcache);
   TL_CP_DECL(Array<int>, _monomer_forbidden_atoms);
   TL_CP_DECL(Array<int>, _product_forbidden_atoms);

   TL_CP_DECL(Array<int>, _original_hydrogens);

   AromaticityMatcher *_am;
   EmbeddingEnumerator *_ee;
   int _tube_idx;
   int _deep_level;
   bool _is_frag_search;
   bool _is_rg_exist;

   int _findCurTube( void );

   bool _isMonomerFromCurTube( int monomer_idx );
   
   static void _foldHydrogens( BaseMolecule &molecule, Array<int> *atoms_to_keep = 0, Array<int> *original_hydrogens = 0 );

   void _productProcess( void );

   bool _nextMatchProcess( EmbeddingEnumerator &ee, const QueryMolecule &reactant, 
      const Molecule &monomer );

   int _calcMaxHCnt( QueryMolecule &molecule );

   bool _startEmbeddingEnumerator( Molecule &monomer );

   void _changeQueryNode( QueryMolecule &ee_reactant, int change_atom_idx );

   void _findFragAtoms( Array<byte> &unfrag_mon_atoms, QueryMolecule &submolecule, 
      Molecule &fragment, int *core_sub, int *core_super );

   void _cleanFragments( void );

   void _findR2PMapping( QueryMolecule &reactant, Array<int> &mapping);

   void _invertStereocenters( Molecule &molecule, int edge_idx );

   void _cistransUpdate( QueryMolecule &submolecule, Molecule &supermolecule, 
      int *frag_mapping, const Array<int> &rp_mapping, int *core_sub);

   QueryMolecule::Atom * _getReactantAtom( int atom_aam );
   
   void _buildMolProduct( QueryMolecule &product, Molecule &mol_product, 
      Molecule &uncleaned_fragments, Array<int> &all_forbidden_atoms, Array<int> &mapping_out );

   void _stereocentersUpdate( QueryMolecule &submolecule,
      Molecule &supermolecule, const Array<int> &rp_mapping,
      int *core_sub, int *core_super );

   void _completeCisTrans( Molecule &product, Molecule &uncleaned_fragments, 
                            Array<int> &frags_mapping );

   bool _checkValence( Molecule &mol, int atom_idx );

   bool _attachFragments( Molecule &ready_product_out );

   bool _checkFragment( QueryMolecule &submolecule, Molecule &monomer, 
                        Array<byte> &unfrag_mon_atoms, int *core_sub );

   void _checkFragmentNecessity ( Array<int> &is_needless_att_point );

   bool _addFragment( Molecule &fragment, QueryMolecule &submolecule, Array<int> &rp_mapping, 
      const Array<int> &sub_rg_atoms, int *core_sub, int *core_super );

   static bool _matchVertexCallback( Graph &subgraph, Graph &supergraph,
      const int *core_sub, int sub_idx, int super_idx, void *userdata );

   static bool _matchEdgeCallback( Graph &subgraph, Graph &supergraph,
      int self_idx, int other_idx, void *userdata );
   
   static bool _allowManyToOneCallback( Graph &subgraph, int sub_idx, void *userdata );

   static void _removeAtomCallback( Graph &subgraph, int sub_idx, void *userdata );

   static void _addBondCallback( Graph &subgraph, Graph &supergraph,
      int self_idx, int other_idx, void *userdata );

   static int _embeddingCallback( Graph &subgraph, Graph &supergraph,
      int *core_sub, int *core_super, void *userdata );
};

}

#endif /* __reaction_enumerator_state__ */
