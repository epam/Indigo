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

#include "molecule/molecule_savers.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace indigo;

int MoleculeSavers::getHCount(BaseMolecule& mol, int index, int atom_number, int atom_charge)
{
    int hydrogens_count = -1;
    if (!mol.isRSite(index) && !mol.isPseudoAtom(index) && !mol.isTemplateAtom(index))
    {
        if (!mol.isQueryMolecule())
        {
            if (mol.getAtomAromaticity(index) == ATOM_AROMATIC && ((atom_number != ELEM_C && atom_number != ELEM_O) || atom_charge != 0))
                hydrogens_count = mol.asMolecule().getImplicitH_NoThrow(index, -1);
        }
        else
        {
            QueryMolecule::Atom& atom = mol.asQueryMolecule().getAtom(index);

            if (!atom.sureValue(QueryMolecule::ATOM_TOTAL_H, hydrogens_count))
            {
                // Try to check if there are only one constraint
                QueryMolecule::Atom* constraint = atom.sureConstraint(QueryMolecule::ATOM_TOTAL_H);
                if (constraint != NULL)
                    hydrogens_count = constraint->value_min;
                else
                    hydrogens_count = -1;
            }
        }
    }
    return hydrogens_count;
}

bool MoleculeSavers::getRingBondCountFlagValue(QueryMolecule& qmol, int idx, int& value)
{
    QueryMolecule::Atom& atom = qmol.getAtom(idx);
    int rbc;
    if (atom.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
    {
        if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS, rbc))
        {
            value = rbc;
            if (value == 0)
                value = -1;
            return true;
        }
        int rbc_values[1] = {4};
        if (atom.sureValueBelongs(QueryMolecule::ATOM_RING_BONDS, rbc_values, 1))
        {
            value = 4;
            return true;
        }
    }
    else if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbc))
    {
        value = -2;
        return true;
    }
    return false;
}

bool MoleculeSavers::getSubstitutionCountFlagValue(QueryMolecule& qmol, int idx, int& value)
{
    QueryMolecule::Atom& atom = qmol.getAtom(idx);
    int v;
    if (atom.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
    {
        if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS, v))
        {
            value = v;
            if (value == 0)
                value = -1;
            return true;
        }
        int values[1] = {6};
        if (atom.sureValueBelongs(QueryMolecule::ATOM_SUBSTITUENTS, values, 1))
        {
            value = 6;
            return true;
        }
    }
    else if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN, v))
    {
        value = -2;
        return true;
    }
    return false;
}
