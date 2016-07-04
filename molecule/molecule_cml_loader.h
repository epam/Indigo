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

#ifndef __molecule_cml_loader__
#define __molecule_cml_loader__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"
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

class MoleculeCmlLoader
{
public:

   DECL_ERROR;

   MoleculeCmlLoader (Scanner &scanner);
   MoleculeCmlLoader (TiXmlHandle &handle);

   void loadMolecule (Molecule &mol);

   StereocentersOptions stereochemistry_options;

protected:
   Scanner *_scanner;
   TiXmlHandle *_handle;
   TiXmlNode *_molecule;

   void _loadMolecule (TiXmlHandle &handle, Molecule &mol);
   void _loadSGroup (TiXmlElement *elem, Molecule &mol, std::unordered_map<std::string, int> &atoms_id, int parent);
   void _loadRgroup (TiXmlHandle &handle, Molecule &mol);
   bool _findMolecule (TiXmlNode *node);
   void _parseRlogicRange (const char *str, Array<int> &ranges);

private:
   MoleculeCmlLoader (const MoleculeCmlLoader &); // no implicit copy
};


}

#endif
