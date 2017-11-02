/****************************************************************************
 * Copyright (C) 2011 EPAM Systems
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

#ifndef __cml_loader__
#define __cml_loader__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"
#include "molecule/base_molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_stereocenter_options.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

class TiXmlHandle;
class TiXmlElement;
class TiXmlNode;
	
namespace indigo
{

class Scanner;
class Molecule;
class QueryMolecule;

class CmlLoader
{
public:

   DECL_ERROR;

   CmlLoader (Scanner &scanner);
   CmlLoader (TiXmlHandle &handle);

   void loadMolecule (Molecule &mol);
   void loadQueryMolecule (QueryMolecule &mol);

   StereocentersOptions stereochemistry_options;
   bool ignore_bad_valence;

protected:
   Scanner *_scanner;
   TiXmlHandle *_handle;
   TiXmlNode *_molecule;

   void _loadMolecule ();
   void _loadMoleculeElement (TiXmlHandle &handle);
   void _loadSGroupElement (TiXmlElement *elem, std::unordered_map<std::string, int> &atoms_id, int parent);
   void _loadRgroupElement (TiXmlHandle &handle);
   bool _findMolecule (TiXmlNode *node);
   void _parseRlogicRange (const char *str, Array<int> &ranges);
   void _appendQueryAtom (const char *atom_label, AutoPtr<QueryMolecule::Atom> &atom);

   Molecule      *_mol;
   BaseMolecule  *_bmol;
   QueryMolecule *_qmol;

private:
   CmlLoader (const CmlLoader &); // no implicit copy
};


}

#endif
