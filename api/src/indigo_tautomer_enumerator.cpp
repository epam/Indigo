/****************************************************************************
 * Copyright (C) 2015 EPAM Systems
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

CEXPORT int indigoIterateTautomers(int molecule, const char *options)
{
   INDIGO_BEGIN
   {
      Molecule &mol = self.getObject(molecule).getMolecule();

      TautomerMethod method;
      if(strncasecmp(options, "INCHI", 5) == 0)
         method = INCHI;
      else if(strncasecmp(options, "RSMARTS", 7) == 0)
         method = RSMARTS;
      else
         method = RSMARTS;
      return self.addObject(new IndigoTautomerIter(mol, method));
   }
   INDIGO_END(-1)
}

IndigoTautomerIter::IndigoTautomerIter(Molecule &molecule, TautomerMethod method) :
IndigoObject(TAUTOMER_ITER),
_enumerator(molecule, method),
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

int IndigoTautomerIter::getIndex()
{
   return _currentPosition > 0? _currentPosition: -_currentPosition;
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

IndigoMoleculeTautomer::IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int index) :
IndigoObject(TAUTOMER_MOLECULE),
_index(index)
{
   enumerator.constructMolecule(_molInstance, index);
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

int IndigoMoleculeTautomer::getIndex()
{
   return (_index > 0? _index: -_index) - 1;
}

Molecule & IndigoMoleculeTautomer::getMolecule()
{
   return _molInstance;
}

