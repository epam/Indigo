/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __canonical_smiles_saver__
#define __canonical_smiles_saver__

#include "molecule/smiles_saver.h"
#include "base_cpp/exception.h"

namespace indigo {

class DLLEXPORT CanonicalSmilesSaver : public SmilesSaver
{
public:

   explicit CanonicalSmilesSaver (Output &output);
   ~CanonicalSmilesSaver ();

   bool find_invalid_stereo;
   const Array<int> *initial_atom_atom_mapping;

   void saveMolecule (Molecule &mol);

   DECL_ERROR;

protected:
   typedef RedBlackMap<int, int> MapIntInt;

   TL_CP_DECL(Array<int>, _actual_atom_atom_mapping);
   TL_CP_DECL(MapIntInt, _initial_to_actual);
   int _aam_counter;
};

}

#endif
