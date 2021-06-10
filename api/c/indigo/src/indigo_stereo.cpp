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

#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/reaction.h"

CEXPORT int indigoStereocenterType(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.allene_stereo.isCenter(ia.idx))
            return INDIGO_ALLENE;

        switch (ia.mol.stereocenters.getType(ia.idx))
        {
        case MoleculeStereocenters::ATOM_ABS:
            return INDIGO_ABS;
        case MoleculeStereocenters::ATOM_OR:
            return INDIGO_OR;
        case MoleculeStereocenters::ATOM_AND:
            return INDIGO_AND;
        case MoleculeStereocenters::ATOM_ANY:
            return INDIGO_EITHER;
        default:
            return 0;
        }
    }
    INDIGO_END(-1);
}

static int mapStereocenterType(int api_stereocenter_type)
{
    switch (api_stereocenter_type)
    {
    case INDIGO_ABS:
        return MoleculeStereocenters::ATOM_ABS;
    case INDIGO_OR:
        return MoleculeStereocenters::ATOM_OR;
    case INDIGO_AND:
        return MoleculeStereocenters::ATOM_AND;
    case INDIGO_EITHER:
        return MoleculeStereocenters::ATOM_ANY;
    default:
        throw IndigoError("Unknown stereocenter type");
    }
}

CEXPORT int indigoStereocenterGroup(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.stereocenters.getType(ia.idx) == 0)
            throw IndigoError("Atom is not a stereocenter");

        return ia.mol.stereocenters.getGroup(ia.idx);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetStereocenterGroup(int atom, int group)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.stereocenters.getType(ia.idx) == 0)
            throw IndigoError("Atom is not a stereocenter");

        ia.mol.stereocenters.setGroup(ia.idx, group);
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoChangeStereocenterType(int atom, int type)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        if (ia.mol.stereocenters.getType(ia.idx) == 0)
            throw IndigoError("Atom is not a stereocenter");

        int group = ia.mol.stereocenters.getGroup(ia.idx);
        ia.mol.stereocenters.setType(ia.idx, mapStereocenterType(type), group);
        if (ia.mol.have_xyz)
            ia.mol.stereocenters.markBond(ia.idx);

        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddStereocenter(int atom, int type, int v1, int v2, int v3, int v4)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));

        int core_type;
        switch (type)
        {
        case INDIGO_ABS:
            core_type = MoleculeStereocenters::ATOM_ABS;
            break;
        case INDIGO_OR:
            core_type = MoleculeStereocenters::ATOM_OR;
            break;
        case INDIGO_AND:
            core_type = MoleculeStereocenters::ATOM_AND;
            break;
        case INDIGO_EITHER:
            core_type = MoleculeStereocenters::ATOM_ANY;
            break;
        default:
            throw IndigoError("Unknown stereocenter type");
        }

        int pyramid[4] = {v1, v2, v3, v4};
        ia.mol.stereocenters.add(ia.idx, core_type, 0, pyramid);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const int* indigoStereocenterPyramid(int atom)
{
    INDIGO_BEGIN
    {
        IndigoAtom& ia = IndigoAtom::cast(self.getObject(atom));
        int* pyramid = ia.mol.stereocenters.getPyramid(ia.idx);
        if (pyramid == 0)
            throw IndigoError("No stereocenter at the atom %d", atom);

        return ia.mol.stereocenters.getPyramid(ia.idx);
    }
    INDIGO_END(NULL);
}

CEXPORT int indigoCountStereocenters(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return mol.stereocenters.size();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearAlleneCenters(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        mol.allene_stereo.clear();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountAlleneCenters(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();

        return mol.allene_stereo.size();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoBondStereo(int bond)
{
    INDIGO_BEGIN
    {
        IndigoBond& ib = IndigoBond::cast(self.getObject(bond));
        BaseMolecule& mol = ib.mol;

        int dir = mol.getBondDirection(ib.idx);

        if (dir == BOND_UP)
            return INDIGO_UP;
        if (dir == BOND_DOWN)
            return INDIGO_DOWN;
        if (dir == BOND_EITHER)
            return INDIGO_EITHER;

        int parity = mol.cis_trans.getParity(ib.idx);

        if (parity == MoleculeCisTrans::CIS)
            return INDIGO_CIS;
        if (parity == MoleculeCisTrans::TRANS)
            return INDIGO_TRANS;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoInvertStereo(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(self.getObject(item));

            if (ia.mol.stereocenters.getType(ia.idx) > 0)
            {
                int *pyramid = ia.mol.stereocenters.getPyramid(ia.idx);
                if (pyramid == 0)
                    throw IndigoError("indigoInvertStereo: internal");
                std::swap(pyramid[0], pyramid[1]);
            }
            else if (ia.mol.allene_stereo.isCenter(ia.idx))
                ia.mol.allene_stereo.invert(ia.idx);
            else
                throw IndigoError("indigoInvertStereo: not a stereo atom");
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(self.getObject(item));

            int parity = ib.mol.cis_trans.getParity(ib.idx);
            if (parity == 0)
                throw IndigoError("indigoInvertStereo: not a stereobond");

            if (parity == MoleculeCisTrans::CIS)
                ib.mol.cis_trans.setParity(ib.idx, MoleculeCisTrans::TRANS);
            else
                ib.mol.cis_trans.setParity(ib.idx, MoleculeCisTrans::CIS);
        }
        else
            throw IndigoError("indigoInvertStereo(): %s given", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetStereo(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(self.getObject(item));

            if (ia.mol.stereocenters.getType(ia.idx) != 0)
                ia.mol.stereocenters.remove(ia.idx);
            if (ia.mol.allene_stereo.isCenter(ia.idx))
                ia.mol.allene_stereo.reset(ia.idx);
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(self.getObject(item));

            ib.mol.setBondDirection(ib.idx, 0);
            ib.mol.cis_trans.setParity(ib.idx, 0);
        }
        else
            throw IndigoError("indigoResetStereo(): %s given", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearStereocenters(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (IndigoBaseMolecule::is(obj))
        {
            obj.getBaseMolecule().stereocenters.clear();
            obj.getBaseMolecule().clearBondDirections();
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            int i;

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            {
                rxn.getBaseMolecule(i).stereocenters.clear();
                rxn.getBaseMolecule(i).clearBondDirections();
            }
        }
        else
            throw IndigoError("only molecules and reactions have stereocenters");
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearCisTrans(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (IndigoBaseMolecule::is(obj))
            obj.getBaseMolecule().cis_trans.clear();
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            int i;

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                rxn.getBaseMolecule(i).cis_trans.clear();
        }
        else
            throw IndigoError("only molecules and reactions have cis-trans");
        return 1;
    }
    INDIGO_END(-1);
}

static int _resetSymmetric(Molecule& mol, bool cistrans, bool stereo)
{
    MoleculeAutomorphismSearch am;
    int sum = 0;

    if (cistrans)
        am.detect_invalid_cistrans_bonds = true;
    if (stereo)
        am.detect_invalid_stereocenters = true;
    am.allow_undefined = true;
    am.process(mol);

    if (cistrans)
    {
        for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        {
            if (mol.cis_trans.getParity(i) == 0)
                continue;

            if (am.invalidCisTransBond(i))
            {
                mol.cis_trans.setParity(i, 0);
                sum++;
            }
        }
    }

    if (stereo)
    {
        QS_DEF(ArrayInt, to_remove);
        to_remove.clear();
        for (int i = mol.stereocenters.begin(); i != mol.stereocenters.end(); i = mol.stereocenters.next(i))
        {
            int atom_index = mol.stereocenters.getAtomIndex(i);
            if (am.invalidStereocenter(atom_index))
            {
                to_remove.push(atom_index);
                sum++;
            }
        }
        for (int i = 0; i < to_remove.size(); i++)
            mol.stereocenters.remove(to_remove[i]);

        if (to_remove.size() > 0)
        {
            mol.clearBondDirections();
            mol.stereocenters.markBonds();
        }
    }
    return sum;
}

static int _markEitherCisTrans(Molecule& mol)
{
    MoleculeAutomorphismSearch am;
    int i, sum = 0;

    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        int substituents[4];

        if (mol.cis_trans.getParity(i) != 0)
            continue;

        if (!MoleculeCisTrans::isGeomStereoBond(mol, i, substituents, false))
            continue;

        if (mol.getEdgeTopology(i) == TOPOLOGY_RING)
        {
            // Mark all bonds in large cycles
            if (mol.edgeSmallestRingSize(i) >= 8)
                mol.cis_trans.ignore(i);
            continue;
        }

        am.possible_cis_trans_to_check.push(i);
    }

    am.allow_undefined = true;
    am.process(mol);

    for (i = 0; i < am.possible_cis_trans_to_check.size(); i++)
    {
        int bond = am.possible_cis_trans_to_check[i];
        mol.cis_trans.ignore(bond);
        sum++;
    }

    return sum;
}

CEXPORT int indigoResetSymmetricCisTrans(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
            return _resetSymmetric(obj.getMolecule(), true, false);
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            int i, sum = 0;

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                sum += _resetSymmetric(rxn.getMolecule(i), true, false);
            return sum;
        }
        throw IndigoError("only molecules and reactions have cis-trans");
    }
    INDIGO_END(-1);
}

CEXPORT int indigoResetSymmetricStereocenters(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
            return _resetSymmetric(obj.getMolecule(), false, true);
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            int i, sum = 0;

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                sum += _resetSymmetric(rxn.getMolecule(i), false, true);
            return sum;
        }
        throw IndigoError("only molecules and reactions have cis-trans");
    }
    INDIGO_END(-1);
}

CEXPORT int indigoMarkEitherCisTrans(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
            return _markEitherCisTrans(obj.getMolecule());
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            int i, sum = 0;

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                sum += _markEitherCisTrans(rxn.getMolecule(i));
            return sum;
        }
        throw IndigoError("only molecules and reactions have cis-trans");
    }
    INDIGO_END(-1);
}

CEXPORT int indigoMarkStereobonds(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
            obj.getMolecule().stereocenters.markBonds();
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                rxn.getMolecule(i).stereocenters.markBonds();
        }
        else
            throw IndigoError("only molecules and reactions have stereocenters");
        return 0;
    }
    INDIGO_END(-1);
}

static void _indigoValidateMoleculeChirality(Molecule& mol)
{
    if (mol.stereocenters.size() == 0)
        return;
    if (!mol.stereocenters.haveAbs())
        return;

    QS_DEF(Molecule, mirror);
    mirror.clone(mol, 0, 0);

    for (int s = mirror.stereocenters.begin(); s != mirror.stereocenters.end(); s = mirror.stereocenters.next(s))
    {
        int atom = mirror.stereocenters.getAtomIndex(s);
        if (mirror.stereocenters.getType(atom) == MoleculeStereocenters::ATOM_ABS)
            mirror.stereocenters.invertPyramid(atom);
    }

    // Check exact matching
    MoleculeExactMatcher matcher(mol, mirror);
    matcher.flags = MoleculeExactMatcher::CONDITION_ALL;

    if (!matcher.find())
        return;

    for (int s = mol.stereocenters.begin(); s != mol.stereocenters.end(); s = mol.stereocenters.next(s))
    {
        int atom = mol.stereocenters.getAtomIndex(s);
        if (mol.stereocenters.getType(atom) == MoleculeStereocenters::ATOM_ABS)
            mol.stereocenters.setType(atom, MoleculeStereocenters::ATOM_AND);
    }
}

CEXPORT int indigoValidateChirality(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();
            _indigoValidateMoleculeChirality(mol);
        }
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                _indigoValidateMoleculeChirality(rxn.getMolecule(i));
        }
        else
            throw IndigoError("only molecules and reactions have stereocenters");
        return 0;
    }
    INDIGO_END(-1);
}
