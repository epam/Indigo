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

CEXPORT int indigoTautomerEnumerate(int molecule, const char *options)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      return self.addObject(new IndigoTautomerIter(mol, options));
   }
   INDIGO_END(-1)
}

IndigoTautomerIter::IndigoTautomerIter(Molecule &molecule, const char *options) :
IndigoObject(TAUTOMER_ITER),
_enumerator(molecule, options),
_complete(false)
{
   bool needAromatize = molecule.isAromatized();
   if(needAromatize)
      _currentPosition = _enumerator.beginAromatized();
   else
      _currentPosition = _enumerator.beginNotAromatized();
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
   if (hasNext())
   {
      AutoPtr<IndigoMoleculeTautomer> result(new IndigoMoleculeTautomer(_enumerator, _currentPosition));
      _currentPosition = _enumerator.next(_currentPosition);
      return result.release();
   }
   return NULL;
}

bool IndigoTautomerIter::hasNext()
{
   return _enumerator.isValid(_currentPosition);
}

IndigoMoleculeTautomer::IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int position) :
IndigoObject(TAUTOMER_MOLECULE)
{
   enumerator.constructMolecule(_molInstance, position);
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
