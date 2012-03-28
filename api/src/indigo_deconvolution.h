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

#ifndef __indigo_deconvolution__
#define __indigo_deconvolution__

#include "indigo_internal.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class IndigoDeconvolutionElem;
class IndigoDecompositionMatch;

class DLLEXPORT IndigoDeconvolution : public IndigoObject {
private:
   enum {
      SHIFT_IDX = 2
   };
public:
   IndigoDeconvolution();
   virtual ~IndigoDeconvolution(){}

   void addMolecule(Molecule& mol, RedBlackStringObjMap< Array<char> >* props, int idx);

   void setScaffold (QueryMolecule& scaffold);
   void makeRGroups (QueryMolecule& scaffold);
   void makeRGroup (IndigoDeconvolutionElem& elem, bool all_matches, bool change_scaffold);

   QueryMolecule& getDecomposedScaffold() { return _fullScaffold; }
   ObjArray<IndigoDeconvolutionElem>& getItems () {return _deconvolutionElems;}
   /*
    * Save AP as sepearate atoms
    */
   bool save_ap_bond_orders;
   /*
    * Ignore match errors
    */
   bool ignore_errors;
   /*
    * Aromatize
    */
   bool aromatize;

   int (*cbEmbedding) (const int *sub_vert_map, const int *sub_edge_map, const void* info, void* userdata);
   void *embeddingUserdata;

public:
   
   class DecompositionEnumerator {
   public:
      DecompositionEnumerator():all_matches(false){}
      ~DecompositionEnumerator(){}

      AutoPtr<AromaticityMatcher> am;
      AutoPtr<MoleculeSubstructureMatcher::FragmentMatchCache> fmcache;
       
      ObjArray<IndigoDecompositionMatch> contexts;
      bool all_matches;
   private:
      DecompositionEnumerator(const DecompositionEnumerator&); //no implicit copy
   };

   void addCompleteRGroup(IndigoDecompositionMatch& emb_context, bool change_scaffold, Array<int>* rg_map);
   void createRgroups(IndigoDecompositionMatch& emb_context, bool change_scaffold);

private:
   void _parseOptions(const char* options);
   
   void _addFullRGroup(Array<int>& att_order, Array<int>& att_idx, Molecule& qmol, Array<int>& map, int new_rg_idx);

   static int _rGroupsEmbedding(Graph &g1, Graph &g2, int *core1, int *core2, void *userdata);

   static bool _matchAtoms (Graph &g1, Graph &g2, const int *, int sub_idx, int super_idx, void* userdata);
   static bool _matchBonds (Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void* userdata);
   static void _addBond (Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void *userdata);
   static void _removeAtom (Graph &subgraph, int sub_idx, void *userdata);
   void _makeInvertMap(Array<int>& map, Array<int>& invmap);


   QueryMolecule _scaffold;
   QueryMolecule _fullScaffold;
   ObjArray<IndigoDeconvolutionElem> _deconvolutionElems;

   DEF_ERROR("R-Group deconvolution");
};

class DLLEXPORT IndigoDeconvolutionElem : public IndigoObject
{
public:
   IndigoDeconvolutionElem (Molecule& mol);
   IndigoDeconvolutionElem (Molecule& mol, RedBlackStringObjMap< Array<char> >* props);
   IndigoDeconvolutionElem (Molecule& mol, int* index);
   IndigoDeconvolutionElem (IndigoDeconvolutionElem& elem);

   virtual ~IndigoDeconvolutionElem ();

   virtual int getIndex () {return idx;}
   virtual RedBlackStringObjMap< Array<char> > * getProperties() {return &properties;}
   IndigoDeconvolution* deco;
   int idx;

   Molecule mol_in;
   IndigoDeconvolution::DecompositionEnumerator deco_enum;

   RedBlackStringObjMap< Array<char> > properties;

   void _copyProperties(RedBlackStringObjMap< Array<char> >* props);
};

class DLLEXPORT IndigoDecompositionMatch : public IndigoObject {
public:
   IndigoDecompositionMatch();
   Array<int> visitedAtoms;
   Array<int> scaffoldBonds;
   Array<int> scaffoldAtoms;
   Array<int> lastMapping;
   Array<int> lastInvMapping;
   ObjArray< Array<int> > attachmentOrder;
   ObjArray< Array<int> > attachmentIndex;

   int getRgroupNumber() const {
      return attachmentIndex.size() - 1;
   }

   void renumber(Array<int>& map, Array<int>& inv_map);
   void copy(IndigoDecompositionMatch& other);

   Molecule mol_out;
   Molecule rgroup_mol;
   Molecule mol_scaffold;
private:
   IndigoDecompositionMatch(const IndigoDecompositionMatch&); //no implicit copy
};

class DLLEXPORT IndigoDeconvolutionIter : public IndigoObject {
public:

   IndigoDeconvolutionIter(ObjArray<IndigoDeconvolutionElem>& items);
   virtual ~IndigoDeconvolutionIter();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   int _index;
   ObjArray<IndigoDeconvolutionElem>& _items;
};
class DLLEXPORT IndigoDecompositionMatchIter : public IndigoObject {
public:

   IndigoDecompositionMatchIter(ObjArray<IndigoDecompositionMatch>& matches);
   virtual ~IndigoDecompositionMatchIter(){}

   virtual IndigoObject * next ();
   virtual bool hasNext ();
   virtual int getIndex() {return _index;}

protected:
   int _index;
   ObjArray<IndigoDecompositionMatch>& _matches;
};



#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
