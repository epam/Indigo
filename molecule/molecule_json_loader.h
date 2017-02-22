/****************************************************************************
 * Copyright (C) 2015 EPAM Systems
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

#ifndef __molecule_json_loader__
#define __molecule_json_loader__

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include "base_cpp/non_copyable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

class Scanner;
class Molecule;

/*
 * Loader for JSON format
 */

class DLLEXPORT MoleculeJsonLoader: public NonCopyable
{
public:

   DECL_ERROR;

   explicit MoleculeJsonLoader (Scanner &scanner);
   void loadMolecule (Molecule &mol);
   
private:
   Scanner& _scanner;
};


}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
