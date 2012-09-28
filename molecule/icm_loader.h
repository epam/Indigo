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

#ifndef __icm_loader__
#define __icm_loader__

#include "base_cpp/exception.h"

namespace indigo {

class Scanner;
class Molecule;

class IcmLoader
{
public:

   // external dictionary, internal decoder
   explicit IcmLoader (Scanner &scanner);

   void loadMolecule (Molecule &mol);

   DECL_ERROR;

protected:
   Scanner &_scanner;

private:
   IcmLoader (const IcmLoader &); // no implicit copy
};

}

#endif
