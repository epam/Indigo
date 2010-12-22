/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_decomposition.h"
#include "indigo_molecule.h"

IndigoDecomposition::IndigoDecomposition (BaseMolecule &mol_) :
IndigoObject(DECOMPOSITION),
mol(mol_),
decomposer(mol_)
{
  decomposer.decompose(0, 0);
}

IndigoDecomposition & IndigoDecomposition::cast (IndigoObject &obj)
{
   if (obj.type != DECOMPOSITION)
      throw IndigoError("%s is not a decomposition", obj.debugInfo());
   return (IndigoDecomposition &)obj;
}

CEXPORT int indigoDecomposition (int mol)
{
   INDIGO_BEGIN
   {
      BaseMolecule &bm = self.getObject(mol).getBaseMolecule();

      return self.addObject(new IndigoDecomposition(bm));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCountComponents (int decomposition)
{
   INDIGO_BEGIN
   {
      IndigoDecomposition &id = IndigoDecomposition::cast(self.getObject(decomposition));

      return id.decomposer.getComponentsCount();
   }
   INDIGO_END(-1)
}

IndigoComponentsIterator::IndigoComponentsIterator (IndigoDecomposition &decomp) :
IndigoObject(COMPONENTS_ITERATOR),
_decomp(decomp)
{
   _idx = -1;
}

IndigoObject * IndigoComponentsIterator::next ()
{
   if (!hasNext())
      return 0;

   _idx++;

   return new IndigoMoleculeComponent(_decomp, _idx);
}

bool IndigoComponentsIterator::hasNext ()
{
   return _idx + 1 < _decomp.decomposer.getComponentsCount();
}


IndigoMoleculeComponent::IndigoMoleculeComponent (IndigoDecomposition &decomp, int idx) :
IndigoObject(MOLECULE_COMPONENT),
_decomp(decomp),
_idx(idx)
{
}

int IndigoMoleculeComponent::getIndex ()
{
   return _idx;
}

void IndigoMoleculeComponent::_validateMol ()
{
   if (_mol.get() != 0)
      return;

   if (_decomp.mol.isQueryMolecule())
      _mol.reset(new QueryMolecule());
   else
      _mol.reset(new Molecule());
      
   _decomp.decomposer.buildComponentMolecule(_idx, _mol.ref(), 0, 0);
}

BaseMolecule & IndigoMoleculeComponent::getBaseMolecule ()
{
   _validateMol();
   return _mol.ref();
}

QueryMolecule & IndigoMoleculeComponent::getQueryMolecule ()
{
   _validateMol();
   return _mol.ref().asQueryMolecule();
}

Molecule & IndigoMoleculeComponent::getMolecule ()
{
   _validateMol();
   return _mol.ref().asMolecule();
}

GraphHighlighting * IndigoMoleculeComponent::getMoleculeHighlighting()
{
   return 0;
}

CEXPORT int indigoIterateComponents (int decomposition)
{
   INDIGO_BEGIN
   {
      IndigoDecomposition &id = IndigoDecomposition::cast(self.getObject(decomposition));

      return self.addObject(new IndigoComponentsIterator(id));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoComponentIndex (int decomposition, int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));
      IndigoDecomposition &id = IndigoDecomposition::cast(self.getObject(decomposition));

      if (ia.mol != &id.mol)
         throw IndigoError("indigoComponentIndex(): molecules do not match");

      return id.decomposer.getComponent(ia.idx);
   }
   INDIGO_END(-1)
}
