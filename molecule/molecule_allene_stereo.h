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

#ifndef __molecule_allene_stereo__
#define __molecule_allene_stereo__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif


namespace indigo {

class BaseMolecule;

class DLLEXPORT MoleculeAlleneStereo
{
public:
   MoleculeAlleneStereo ();

   void clear ();

protected:
   BaseMolecule & _getMolecule();
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
