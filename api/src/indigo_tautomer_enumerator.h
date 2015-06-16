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

#ifndef __indigo_tautomer_enumerator__
#define __indigo_tautomer_enumerator__

#include "indigo_internal.h"
#include "molecule/molecule_tautomer_enumerator.h"
#include "base_cpp/properties_map.h"

class IndigoMoleculeTautomer : public IndigoObject
{
public:
   IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int position);
   virtual ~IndigoMoleculeTautomer();

   virtual Molecule & getMolecule();
   virtual IndigoObject * clone();

   virtual const char * debugInfo();
   
   virtual PropertiesMap& getProperties() { return _properties;}

private:
   Molecule _molInstance;
   indigo::PropertiesMap _properties;
};

class IndigoTautomerIter : public IndigoObject
{
public:
   IndigoTautomerIter(Molecule &molecule, const char *options);
   virtual ~IndigoTautomerIter();

   virtual IndigoObject * next();
   virtual bool hasNext();

   virtual const char * debugInfo();

protected:

   TautomerEnumerator _enumerator;
   int _currentPosition;
   bool _complete;
};

#endif /* __indigo_tautomer_enumerator__ */
