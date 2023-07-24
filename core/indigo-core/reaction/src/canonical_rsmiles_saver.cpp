/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "reaction/canonical_rsmiles_saver.h"

#include "base_cpp/output.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/smiles_saver.h"
#include "reaction/reaction.h"
#include "reaction/rsmiles_saver.h"

using namespace indigo;

IMPL_ERROR(CanonicalRSmilesSaver, "canonical SMILES saver for reactions");

CanonicalRSmilesSaver::CanonicalRSmilesSaver(Output& output) : RSmilesSaver(output)
{
}

CanonicalRSmilesSaver::~CanonicalRSmilesSaver()
{
}

void CanonicalRSmilesSaver::saveReaction(Reaction& reaction_)
{
    int j;

    QS_DEF(Reaction, reaction);
    reaction.clear();

    reaction.name.copy(reaction_.name);

    if (reaction_.reactantsCount())
    {
        j = reaction.addReactant();
        Molecule& mol = reaction.getMolecule(j);
        for (auto i : reaction_.reactants)
        {
            mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
        }
    }

    if (reaction_.catalystCount())
    {
        j = reaction.addCatalyst();
        Molecule& mol = reaction.getMolecule(j);
        for (auto i : reaction_.catalysts)
        {
            mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
        }
    }

    if (reaction_.productsCount())
    {
        j = reaction.addProduct();
        Molecule& mol = reaction.getMolecule(j);
        for (auto i : reaction_.products)
        {
            mol.mergeWithMolecule(reaction_.getMolecule(i), 0);
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

void CanonicalRSmilesSaver::_writeMolecule(int i, CanonicalSmilesSaver& saver)
{
    int j;

    saver.write_extra_info = false;
    saver.chemaxon = false;
    saver.separate_rsites = false;
    saver.rsite_indices_as_aam = false;

    saver.smarts_mode = smarts_mode;
    saver.inside_rsmiles = true;

    if (_rxn != 0)
        saver.saveMolecule(_rxn->getMolecule(i));
    // else
    //   saver.saveQueryMolecule(_qrxn->getQueryMolecule(i));

    _ncomp.push(saver.writtenComponents());

    const Array<int>& atoms = saver.writtenAtoms();

    for (j = 0; j < atoms.size(); j++)
    {
        _Idx& idx = _written_atoms.push();

        idx.mol = i;
        idx.idx = atoms[j];
    }

    const Array<int>& bonds = saver.writtenBonds();

    for (j = 0; j < bonds.size(); j++)
    {
        _Idx& idx = _written_bonds.push();

        idx.mol = i;
        idx.idx = bonds[j];
    }
}
