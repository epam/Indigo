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

#ifndef __reaction_h__
#define __reaction_h__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "molecule/molecule.h"
#include "reaction/base_reaction.h"

namespace indigo {

// Stereo changes during reaction
enum
{
   STEREO_UNMARKED = 0,
   STEREO_INVERTS  = 1,
   STEREO_RETAINS  = 2
};


// Reacting centers
enum
{
   RC_NOT_CENTER     = -1,
   RC_UNMARKED       =  0,
   RC_CENTER         =  1,
   RC_UNCHANGED      =  2,
   RC_MADE_OR_BROKEN =  4,
   RC_ORDER_CHANGED  =  8,
   RC_TOTAL          = 16
};

class DLLEXPORT Reaction : public BaseReaction {
public:

   Reaction ();
   virtual ~Reaction ();

   virtual void clear();

   virtual BaseReaction * neu ();

   Molecule & getMolecule (int index);

   virtual bool aromatize();
   virtual bool dearomatize();

   virtual Reaction & asReaction ();

   /*
   void dearomatizeBonds();
   void aromatizeQueryBonds();
   bool isAllConnected() const;*/

   static void saveBondOrders (Reaction& reaction, ObjArray< Array<int> > &bond_types);
   static void loadBondOrders (Reaction& reaction, ObjArray< Array<int> > &bond_types);

   static void checkForConsistency (Reaction &rxn);

   DEF_ERROR("reaction");

protected:

   virtual int _addBaseMolecule (int side);

private:
   Reaction (const Reaction&);//no implicit copy
};

}

#endif
