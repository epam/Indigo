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

#include "molecule/molecule_attachments_search.h"
#include "molecule/molecule_rgroups_composition.h"

class DLLEXPORT IndigoCompositionIter : public IndigoObject {
public:
    IndigoCompositionIter(BaseMolecule& mol)
        : IndigoObject(COMPOSITION_ITER),
          it(MoleculeRGroupsComposition::combinations(mol)->iterator()) {}
    virtual ~IndigoCompositionIter() {}

    virtual IndigoObject* next() {
        if (hasNext()) {
            AutoPtr<BaseMolecule> mol(it.ref().next());
            if (mol.ref().isQueryMolecule()) {
                AutoPtr<IndigoQueryMolecule> result(new IndigoQueryMolecule());
                result.ref().qmol.clone(mol.ref(), nullptr, nullptr);
                return result.release();
            } else {
                AutoPtr<IndigoMolecule> result(new IndigoMolecule());
                result.ref().mol.clone(mol.ref(), nullptr, nullptr);
                return result.release();
            }
        } else {
            return nullptr;
        }
    }
    virtual bool hasNext() {
        return it->hasNext();
    }

protected:
    AutoPtr<Iterator<BaseMolecule*>> it;
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