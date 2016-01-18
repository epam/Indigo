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

#include "indigo_internal.h"
#include "indigo_molecule.h"

#include "molecule/molecule_rgroups_composition.h"

class DLLEXPORT IndigoCompositionIter : public IndigoObject {
public:
   IndigoCompositionIter(BaseMolecule& mol)
      : IndigoObject(COMPOSITION_ITER),
      _composition(MoleculeRGroupsComposition(mol)),
      _it(_composition.begin()), _end(_composition.end())
   {}
   virtual ~IndigoCompositionIter() {}

   virtual IndigoObject* next() {
      if (!_hasNext) { return nullptr; }

      AutoPtr<IndigoMolecule> result(new IndigoMolecule());
      _it.dump(result.ref().mol);
      _hasNext = _it.next();
      return result.release();
   }
   virtual bool hasNext() {
      return _hasNext;
   }

protected:
   MoleculeRGroupsComposition _composition;
   MoleculeRGroupsComposition::MoleculeIter _it;
   MoleculeRGroupsComposition::MoleculeIter _end;

   bool _hasNext = true;
};

CEXPORT int indigoRGroupComposition(int molecule, const char* options)
{
   INDIGO_BEGIN
   {
      BaseMolecule& target = self.getObject(molecule).getBaseMolecule();
      return self.addObject(new IndigoCompositionIter(target));
   }
   INDIGO_END(-1);
}