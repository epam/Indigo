/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __molecule_json_saver_h__
#define __molecule_json_saver_h__

namespace indigo {

class Molecule;
class QueryMolecule;
class Output;

class DLLEXPORT MoleculeJsonSaver
{
public:
   explicit MoleculeJsonSaver (Output &output);

   void saveMolecule (Molecule &mol);
   void saveQueryMolecule (QueryMolecule &qmol);

   DECL_ERROR;

protected:
   Molecule *_mol;
   Output   &_output;

private:
   MoleculeJsonSaver (const MoleculeJsonSaver &); // no implicit copy
};

}

#endif

