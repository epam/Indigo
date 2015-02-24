/****************************************************************************
 * Copyright (C) 2015 GGA Software Services LLC
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

#include "indigo_tautomer_enumerator.h"
#include "indigo_molecule.h"

CEXPORT int indigoTautomerEnumerate(int molecule)
{
   INDIGO_BEGIN
   {
   Molecule &mol = self.getObject(molecule).getMolecule();

   return self.addObject(new IndigoTautomerIter(mol));
   }
   INDIGO_END(-1)
}

IndigoTautomerIter::IndigoTautomerIter(Molecule &molecule) :
IndigoObject(TAUTOMER_ITER),
_enumerator(molecule)
{
   _layer = 0;
}

const char * IndigoTautomerIter::debugInfo()
{
   return "<tautomer iterator>";
}

IndigoTautomerIter::~IndigoTautomerIter()
{
}

IndigoObject * IndigoTautomerIter::next()
{
   if (_layer < _enumerator.size())
      return new IndigoMoleculeTautomer(_enumerator, _layer++);
   return NULL;
}

bool IndigoTautomerIter::hasNext()
{
   return _layer < _enumerator.size() - 1;
}

IndigoMoleculeTautomer::IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int layer) :
IndigoObject(TAUTOMER_MOLECULE)
{
   enumerator.constructMolecule(_molInstance, layer);
}

const char * IndigoMoleculeTautomer::debugInfo()
{
   return "<molecule tautomer>";
}

IndigoMoleculeTautomer::~IndigoMoleculeTautomer()
{
}

IndigoObject *IndigoMoleculeTautomer::clone()
{
   return IndigoMolecule::cloneFrom(*this);
}

Molecule & IndigoMoleculeTautomer::getMolecule()
{
   return _molInstance;
}

RedBlackStringObjMap< Array<char> > * IndigoMoleculeTautomer::getProperties()
{
   return 0;
}
