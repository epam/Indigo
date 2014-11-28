/****************************************************************************
 * Copyright (C) 2009-2014 GGA Software Services LLC
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

#include "reaction/canonical_rsmiles_saver.h"

#include "base_cpp/output.h"
#include "molecule/smiles_saver.h"
#include "reaction/reaction.h"
#include "reaction/rsmiles_saver.h"
#include "molecule/canonical_smiles_saver.h"

using namespace indigo;

IMPL_ERROR(CanonicalRSmilesSaver, "canonical SMILES saver for reactions");

CanonicalRSmilesSaver::CanonicalRSmilesSaver (Output &output)
: RSmilesSaver(output)
{
}

CanonicalRSmilesSaver::~CanonicalRSmilesSaver ()
{
}

void CanonicalRSmilesSaver::saveReaction(Reaction &reaction_)
{
   int i, j;

   QS_DEF(Reaction, reaction);
   reaction.clear();

   reaction.name.copy(reaction_.name);

   i = reaction_.reactantBegin();
   if (i != reaction_.reactantEnd()) {
      j = reaction.addReactantCopy(reaction_.getMolecule(i), 0, 0);
      Molecule &mol = reaction.getMolecule(j);
      Array<int> &aamArray = reaction.getAAMArray(j);
      aamArray.copy(reaction_.getAAMArray(i));

      for (i = reaction_.reactantNext(i); i != reaction_.reactantEnd(); i = reaction_.reactantNext(i))
      {
         Array<int> &aamArray_ = reaction_.getAAMArray(i);
         mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
         aamArray.reserve(aamArray.size() + aamArray_.size());
         for (int i = 0; i < aamArray_.size(); ++i) {
            aamArray.push();
            aamArray.at(aamArray.size() - 1) = aamArray_.at(i);
         }
      }
   }

   i = reaction_.catalystBegin();
   if (i != reaction_.catalystEnd()) {
      j = reaction.addCatalystCopy(reaction_.getMolecule(i), 0, 0);
      Molecule &mol = reaction.getMolecule(j);
      Array<int> &aamArray = reaction.getAAMArray(j);
      aamArray.copy(reaction_.getAAMArray(i));

      for (i = reaction_.catalystNext(i); i != reaction_.catalystEnd(); i = reaction_.catalystNext(i))
      {
         Array<int> &aamArray_ = reaction_.getAAMArray(i);
         mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
         aamArray.reserve(aamArray.size() + aamArray_.size());
         for (int i = 0; i < aamArray_.size(); ++i) {
            aamArray.push();
            aamArray.at(aamArray.size() - 1) = aamArray_.at(i);
         }
      }
   }

   i = reaction_.productBegin();
   if (i != reaction_.productEnd()) {
      j = reaction.addProductCopy(reaction_.getMolecule(i), 0, 0);
      Molecule &mol = reaction.getMolecule(j);
      Array<int> &aamArray = reaction.getAAMArray(j);
      aamArray.copy(reaction_.getAAMArray(i));

      for (i = reaction_.productNext(i); i != reaction_.productEnd(); i = reaction_.productNext(i))
      {
         Array<int> &aamArray_ = reaction_.getAAMArray(i);
         mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
         aamArray.reserve(aamArray.size() + aamArray_.size());
         for (int i = 0; i < aamArray_.size(); ++i) {
            aamArray.push();
            aamArray.at(aamArray.size() - 1) = aamArray_.at(i);
         }
      }
   }

   _brxn = &reaction;
   _qrxn = 0;
   _rxn = &reaction;
   _saveReaction();
}

void CanonicalRSmilesSaver::_saveReaction()
{
   _written_atoms.clear();
   _written_bonds.clear();
   _ncomp.clear();
   _comma = false;

   CanonicalSmilesSaver moleculeSaver(_output);

   // Invariant: there are only one molecule for each part of the reaction. Thus we can use _brxn->xxxBegin instead of the loop.
   // The only exception is catalyst. There could be zero catalysts.
   if (_brxn->reactantsCount())
      _writeMolecule(_brxn->reactantBegin(), moleculeSaver);
   _output.writeString(">");
   if (_brxn->catalystCount())
      _writeMolecule(_brxn->catalystBegin(), moleculeSaver);
   _output.writeString(">");
   if (_brxn->productsCount())
      _writeMolecule(_brxn->productBegin(), moleculeSaver);

   _writeFragmentsInfo();
   _writeStereogroups();
   _writeRadicals();
   _writePseudoAtoms();
   _writeHighlighting();

   if (_comma)
      _output.writeChar('|');
}

void CanonicalRSmilesSaver::_writeMolecule(int i, CanonicalSmilesSaver &saver)
{
   int j;

   saver.initial_atom_atom_mapping = &_brxn->getAAMArray(i);
   saver.smarts_mode = smarts_mode;

   if (_rxn != 0)
      saver.saveMolecule(_rxn->getMolecule(i));
   //else
   //   saver.saveQueryMolecule(_qrxn->getQueryMolecule(i));

   _ncomp.push(saver.writtenComponents());

   const Array<int> &atoms = saver.writtenAtoms();

   for (j = 0; j < atoms.size(); j++)
   {
      _Idx &idx = _written_atoms.push();

      idx.mol = i;
      idx.idx = atoms[j];
   }

   const Array<int> &bonds = saver.writtenBonds();

   for (j = 0; j < bonds.size(); j++)
   {
      _Idx &idx = _written_bonds.push();

      idx.mol = i;
      idx.idx = bonds[j];
   }
}
