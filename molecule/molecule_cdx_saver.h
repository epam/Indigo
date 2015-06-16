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

#ifndef __molecule_cdx_saver_h__
#define __molecule_cdx_saver_h__

namespace indigo {

class Molecule;
class Output;

class DLLEXPORT MoleculeCdxSaver
{
public:
   explicit MoleculeCdxSaver (Output &output);

   void saveMolecule (Molecule &mol);

   DECL_ERROR;

protected:
   Molecule *_mol;
   Output   &_output;

private:
   MoleculeCdxSaver (const MoleculeCdxSaver &); // no implicit copy
};

}

#endif

