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

#include <memory>

#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "reaction/reaction.h"
#include "reaction/reaction_automapper.h"

using namespace indigo;

IMPL_ERROR(Reaction, "reaction");

Reaction::Reaction()
{
}

Reaction::~Reaction()
{
}

void Reaction::clear()
{
    BaseReaction::clear();
}

Molecule& Reaction::getMolecule(int index)
{
    return getBaseMolecule(index).asMolecule();
}

int Reaction::_addBaseMolecule(int side)
{
    int idx = _allMolecules.add(new Molecule());
    _addedBaseMolecule(idx, side, *_allMolecules[idx]);
    return idx;
}

void Reaction::saveBondOrders(Reaction& reaction, ObjArray<Array<int>>& bond_types)
{

    while (bond_types.size() < reaction.end())
        bond_types.push();

    for (int i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
    {
        Molecule::saveBondOrders(reaction.getMolecule(i), bond_types[i]);
    }
}

void Reaction::loadBondOrders(Reaction& reaction, ObjArray<Array<int>>& bond_types)
{
    for (int i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
    {
        Molecule::loadBondOrders(reaction.getMolecule(i), bond_types[i]);
    }
}

bool Reaction::aromatize(const AromaticityOptions& options)
{
    bool arom_found = false;
    for (int i = begin(); i < end(); i = next(i))
    {
        arom_found |= MoleculeAromatizer::aromatizeBonds(*(Molecule*)_allMolecules[i], options);
    }
    return arom_found;
}

Reaction& Reaction::asReaction()
{
    return *this;
}

std::unique_ptr<BaseReaction> Reaction::getBaseReaction(int index)
{
    std::unique_ptr<BaseReaction> reaction(neu());
    if (_reactionBlocks.size())
    {
        auto& rb = _reactionBlocks[index];
        for (auto ridx : rb.reactants)
            reaction->addReactantCopy(getBaseMolecule(ridx), 0, 0);

        for (auto pidx : rb.products)
            reaction->addProductCopy(getBaseMolecule(pidx), 0, 0);

        return reaction;
    }
    reaction->clone(*this);
    return reaction;
}

BaseReaction* Reaction::neu()
{
    return new Reaction();
}

void Reaction::checkForConsistency(Reaction& rxn)
{
    for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
        Molecule::checkForConsistency(rxn.getMolecule(i));
}
