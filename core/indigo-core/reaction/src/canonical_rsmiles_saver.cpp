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

SmilesSaver& CanonicalRSmilesSaver::_addMoleculeSaver()
{
    auto saver = std::make_unique<CanonicalSmilesSaver>(_output);
    saver->write_extra_info = false;
    saver->chemaxon = false;
    saver->separate_rsites = false;
    saver->rsite_indices_as_aam = false;
    saver->smarts_mode = smarts_mode;
    saver->inside_rsmiles = true;
    saver->ignore_hydrogens = !_qrxn;
    if (_smiles_savers.size())
    {
        // If there are already some savers, we need to copy aam from the last one.
        CanonicalSmilesSaver& last_saver = static_cast<CanonicalSmilesSaver&>(*_smiles_savers.back());
        saver->copyAAM(last_saver);
    }
    _smiles_savers.emplace_back(std::move(saver));
    return *_smiles_savers.back();
}

void CanonicalRSmilesSaver::saveReaction(Reaction& reaction)
{
    int j;

    _reaction.clear();

    _reaction.name.copy(reaction.name);

    if (reaction.reactantsCount())
    {
        j = _reaction.addReactant();
        Molecule& mol = _reaction.getMolecule(j);
        for (auto i : reaction.reactants)
        {
            mol.mergeWithMolecule(reaction.getMolecule(i), 0);
        }
    }

    if (reaction.catalystCount())
    {
        j = _reaction.addCatalyst();
        Molecule& mol = _reaction.getMolecule(j);
        for (auto i : reaction.catalysts)
        {
            mol.mergeWithMolecule(reaction.getMolecule(i), 0);
        }
    }

    if (reaction.productsCount())
    {
        j = _reaction.addProduct();
        Molecule& mol = _reaction.getMolecule(j);
        for (auto i : reaction.products)
        {
            mol.mergeWithMolecule(reaction.getMolecule(i), 0);
        }
    }

    _brxn = &_reaction;
    _qrxn = 0;
    _rxn = &_reaction;
    RSmilesSaver::_saveReaction();
}
