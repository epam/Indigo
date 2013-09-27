/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

#ifndef __molecule_cml_loader__
#define __molecule_cml_loader__

#include "base_cpp/exception.h"

class TiXmlHandle;

namespace indigo
{

class Scanner;
class Molecule;

class MoleculeCmlLoader
{
public:

   DECL_ERROR;

   MoleculeCmlLoader (Scanner &scanner);
   MoleculeCmlLoader (TiXmlHandle &handle);

   void loadMolecule (Molecule &mol);

   bool ignore_stereochemistry_errors;

protected:
   Scanner *_scanner;
   TiXmlHandle *_handle;

   void _loadMolecule (TiXmlHandle &handle, Molecule &mol);

private:
   MoleculeCmlLoader (const MoleculeCmlLoader &); // no implicit copy
};


}

#endif
