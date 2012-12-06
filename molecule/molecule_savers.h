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

#ifndef __molecule_savers_h__
#define __molecule_savers_h__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#include "molecule/base_molecule.h"

namespace indigo {

class MoleculeSavers
{
public:
   static int getHCount (BaseMolecule &mol, int index, int atom_number, int atom_charge);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
