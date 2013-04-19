/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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
#include "base_cpp/ptr_pool.h"

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

   int begin ();
   int end ();
   int next (int i);
   int count ();

   void remove (int i);


   int reactantBegin()         { return _nextElement(REACTANT, -1); }
   int reactantNext(int index) { return _nextElement(REACTANT, index); }
   int reactantEnd()           { return _allMolecules.end(); }

   int productBegin()          { return _nextElement(PRODUCT, -1); }
   int productNext(int index)  { return _nextElement(PRODUCT, index); }
   int productEnd()            { return _allMolecules.end(); }

   int catalystBegin()         { return _nextElement(CATALYST, -1); }
   int catalystNext(int index) { return _nextElement(CATALYST, index); }
   int catalystEnd()           { return _allMolecules.end(); }

   int sideBegin (int side)    { return _nextElement(side, -1); }
   int sideNext (int side, int index) { return _nextElement(side, index); }
   int sideEnd ()             { return _allMolecules.end(); }

   int getSideType(int index) {return _types[index]; }

   int reactantsCount() const { return _reactantCount; }
   int productsCount() const { return _productCount; }
   int catalystCount() const { return _catalystCount; }
   
   virtual void clear();

   // Returns true if some bonds were changed
   virtual bool aromatize (const AromaticityOptions &options) = 0;
   // Returns true if all bonds were dearomatized
   virtual bool dearomatize (const AromaticityOptions &options) = 0;

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

   int findMolecule (BaseMolecule *mol);

   void markStereocenterBonds();

   static bool haveCoord (BaseReaction &reaction);

   void clone (BaseReaction &other, Array<int> *mol_mapping, ObjArray< Array<int> >* mappings, ObjArray< Array<int> >* inv_mappings);

   Array<char> name;

   DECL_ERROR;

protected:

   virtual int _addBaseMolecule (int side) = 0;
   
   virtual void _addedBaseMolecule (int idx, int side, BaseMolecule &mol);

   PtrPool<BaseMolecule>  _allMolecules;
   ObjArray< Array<int> > _atomAtomMapping;
   ObjArray< Array<int> > _reactingCenters;
   ObjArray< Array<int> > _inversionNumbers;

   Array<int> _types;

   int _reactantCount;
   int _productCount;
   int _catalystCount;

   int _nextElement(int type, int index);

   virtual void _clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings);

private:
   BaseReaction(const BaseReaction&);//no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
