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

using MoleculeIter = MoleculeRGroupsComposition::MoleculeIter;

class DLLEXPORT IndigoCompositionElem : public IndigoObject {
public:
   IndigoCompositionElem()
     : IndigoObject(COMPOSITION_ELEM) {}
   virtual ~IndigoCompositionElem() {}

   Molecule        molecule;
   MoleculeRGroups variants[RGCOMP_OPT_COUNT];
};

CEXPORT int indigoGetFragmentedMolecule(int elem, const char* options)
{
   INDIGO_BEGIN
   {
      if (!strcmp(options, "")) {
         options = MoleculeIter::OPTION(RGCOMP_OPT::ERASE);
      }

      IndigoObject &obj = self.getObject(elem);
      IndigoCompositionElem& elem = dynamic_cast<IndigoCompositionElem&>(obj);

      MoleculeRGroups *rgroups = nullptr;
      RGCOMP_OPT OPTS[RGCOMP_OPT_COUNT] = RGCOMP_OPT_ENUM;
      for (auto i = 0; i < RGCOMP_OPT_COUNT; i++) {
         if (!strcmp(options, MoleculeIter::OPTION(OPTS[i]))) {
            rgroups = &elem.variants[i];
            break;
         }
      }
      if (rgroups == nullptr) {
         throw IndigoError("indigoGetFragmentedMolecule(): weird options \"%s\"", options);
      }

      AutoPtr<IndigoMolecule> result(new IndigoMolecule());
      result.ref().mol.clone(elem.molecule, nullptr, nullptr);
      result.ref().mol.rgroups.copyRGroupsFromMolecule(*rgroups);
      return self.addObject(result.release());
   }
   INDIGO_END(-1);
}

class DLLEXPORT IndigoCompositionIter : public IndigoObject {
public:
   IndigoCompositionIter(BaseMolecule& mol)
      : IndigoObject(COMPOSITION_ITER), _composition(mol),
      _it(_composition.begin()), _end(_composition.end()) {}
   virtual ~IndigoCompositionIter() {}

   virtual IndigoObject* next() {
      if (!_hasNext) { return nullptr; }
      AutoPtr<IndigoCompositionElem> result(new IndigoCompositionElem());

      _it.dump(result.ref().molecule);

      RGCOMP_OPT OPTS[RGCOMP_OPT_COUNT] = RGCOMP_OPT_ENUM;
      for (auto i = 0; i < RGCOMP_OPT_COUNT; i++) {
         result.ref().variants[i].copyRGroupsFromMolecule(
            *_it.modifyRGroups(MoleculeIter::OPTION(OPTS[i])));
      }

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