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

#ifndef __molecule_auto_loader__
#define __molecule_auto_loader__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/red_black.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Scanner;
class Molecule;
class QueryMolecule;
class BaseMolecule;

class DLLEXPORT MoleculeAutoLoader
{
public:
   MoleculeAutoLoader (Scanner &scanner);
   MoleculeAutoLoader (const Array<char> &arr);
   MoleculeAutoLoader (const char *str);

   ~MoleculeAutoLoader ();

   void loadMolecule (Molecule &mol);
   void loadQueryMolecule (QueryMolecule &qmol);

   bool ignore_stereocenter_errors;
   bool ignore_closing_bond_direction_mismatch;
   bool ignore_noncritical_query_features;
   bool treat_x_as_pseudoatom;
   bool skip_3d_chirality;

   // Loaded properties
   TL_CP_DECL(RedBlackStringObjMap< Array<char> >, properties);

   DECL_ERROR;

   static bool tryMDLCT (Scanner &scanner, Array<char> &outbuf);

protected:
   Scanner *_scanner;
   bool     _own_scanner;

   void _init ();
   bool _isSingleLine ();
   void _loadMolecule (BaseMolecule &mol, bool query);
private:
   MoleculeAutoLoader (const MoleculeAutoLoader &); // no implicit copy

};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
