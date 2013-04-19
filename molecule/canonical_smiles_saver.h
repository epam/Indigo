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

#ifndef __canonical_smiles_saver__
#define __canonical_smiles_saver__

#include "base_cpp/exception.h"

namespace indigo {

class Molecule;
class Output;

class DLLEXPORT CanonicalSmilesSaver
{
public:

   explicit CanonicalSmilesSaver (Output &output);
   ~CanonicalSmilesSaver ();

   bool find_invalid_stereo;

   void saveMolecule (Molecule &mol) const;

   DECL_ERROR;

protected:

   Output &_output;
};

}

#endif
