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

#ifndef __base_reaction_h__
#define __base_reaction_h__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#include "molecule/base_molecule.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_array.h"

namespace indigo {

class Reaction;
class QueryReaction;

class DLLEXPORT BaseReaction {
public:
   enum
   {
      REACTANT = 1,
      PRODUCT = 2,
      CATALYST = 4
   };

   BaseReaction();
   virtual ~BaseReaction ();

   // 'neu' means 'new' in German
   virtual BaseReaction * neu () = 0;

   int begin() const                   { return _nextElement(REACTANT | PRODUCT | CATALYST, -1); }
   int next(int index) const           { return _nextElement(REACTANT | PRODUCT | CATALYST, index); }
   int end() const                     { return _indexes.size(); }
   int count() const                   { return end(); }

   int reactantBegin() const           { return _nextElement(REACTANT, -1); }
   int reactantNext(int index) const   { return _nextElement(REACTANT, index); }
   int reactantEnd() const             { return _indexes.size(); }

   int productBegin() const            { return _nextElement(PRODUCT, -1); }
   int productNext(int index) const    { return _nextElement(PRODUCT, index); }
   int productEnd() const              { return _indexes.size(); }

   int catalystBegin() const            { return _nextElement(CATALYST, -1); }
   int catalystNext(int index) const    { return _nextElement(CATALYST, index); }
   int catalystEnd() const              { return _indexes.size(); }

   int sideBegin (int side) const            { return _nextElement(side, -1); }
   int sideNext (int side, int index) const    { return _nextElement(side, index); }
   int sideEnd () const              { return _indexes.size(); }

   int getSideType(int index) const {return _indexes[index]; }

   int reactantsCount() const { return _reactantCount; }
   int productsCount() const { return _productCount; }
   int catalystCount() const { return _catalystCount; }
   
   virtual void clear();

   virtual void aromatize() = 0;
   virtual void dearomatize() = 0;

   // poor man's dynamic casting
   virtual Reaction & asReaction ();
   virtual QueryReaction & asQueryReaction ();
   virtual bool isQueryReaction ();

   BaseMolecule & getBaseMolecule(int index)  { return *_allMolecules.at(index); }

   int getAAM(int index, int atom);
   int getReactingCenter(int index, int bond);
   int getInversion (int index, int atom);

   Array<int> & getAAMArray(int index);
   Array<int> & getReactingCenterArray(int index);
   Array<int> & getInversionArray (int index);

   void clearAAM ();

   int addReactant ();
   int addProduct ();
   int addCatalyst ();

   int addReactantCopy (BaseMolecule &mol, Array<int>* mapping, Array<int> *inv_mapping);
   int addProductCopy (BaseMolecule &mol, Array<int>* mapping, Array<int> *inv_mapping);
   int addCatalystCopy (BaseMolecule &mol, Array<int>* mapping, Array<int> *inv_mapping);

   int findAtomByAAM (int mol_idx, int aam);
   int findAamNumber (BaseMolecule *mol, int atom_number);
   int findReactingCenter (BaseMolecule *mol, int bond_number);

   //int findMoleculeSideIdx(const Molecule* qmol) const;
   //int findMoleculeIdx(const Molecule* qmol) const;
   //int findInversionNumber(const Molecule* qmol, int atom_number) const;
   //int findExactChange(const Molecule* qmol, int atom_number) const;

   //bool isAllConnected() const;
   void markStereocenterBonds();

   static bool haveCoord (BaseReaction &reaction);

   void clone (BaseReaction& other, ObjArray< Array<int> >* mappings, ObjArray< Array<int> >* inv_mappings);

   Array<char> name;

   DEF_ERROR("reaction");

protected:

   virtual int _addBaseMolecule (int side) = 0;
   
   virtual void _addedBaseMolecule (int idx, int side, BaseMolecule &mol);

   PtrArray<BaseMolecule> _allMolecules;
   ObjArray< Array<int> > _atomAtomMapping;
   ObjArray< Array<int> > _reactingCenters;
   ObjArray< Array<int> > _inversionNumbers;

   Array<int> _indexes;

   int _reactantCount;
   int _productCount;
   int _catalystCount;

   int _nextElement(int type, int index) const;

   virtual void _clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings);

private:
   BaseReaction(const BaseReaction&);//no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
