/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_standardize__
#define __molecule_standardize__

#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;
class StandardizeOptions;

class MoleculeStandardizer
{
public:
	MoleculeStandardizer();
   // Interface function for stadardize molecule
   static bool standardize (Molecule &mol, const StandardizeOptions &options);

protected:
   static void _stadardizeStereo(Molecule &mol);
   static void _stadardizeCharges(Molecule &mol);
   static void _centerMolecule(Molecule &mol);
   static void _removeSingleAtomFragments(Molecule &mol);
   CP_DECL;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
