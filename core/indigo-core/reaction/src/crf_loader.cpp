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

#include "reaction/crf_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/cmf_loader.h"
#include "reaction/crf_common.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(CrfLoader, "CRF loader");

void CrfLoader::_init()
{
    xyz_scanner = 0;
    version = 2;
}

CrfLoader::CrfLoader(LzwDict& dict, Scanner& scanner) : _scanner(scanner)
{
    _dict = &dict;
    _decoder.create(dict, scanner);
    _init();
}

CrfLoader::CrfLoader(Scanner& scanner) : _scanner(scanner)
{
    _dict = 0;
    _init();
}

void CrfLoader::loadReaction(Reaction& reaction)
{
    int i;
    int nreactants = _scanner.readPackedUInt();
    int nproducts = _scanner.readPackedUInt();
    byte flags = _scanner.readByte();
    int ncatalyst = 0;
    if (flags & CrfFeatureFlags::CRF_CATALYST)
        ncatalyst = _scanner.readPackedUInt();

    reaction.clear();

    _bond_rc_flags = 0;
    _atom_stereo_flags = 0;
    _aam = 0;

    bool have_aam = (flags != 0);

    for (i = 0; i < nreactants; i++)
    {
        int index = reaction.addReactant();
        _loadReactionMolecule(reaction, index, have_aam);
    }

    for (i = 0; i < nproducts; i++)
    {
        int index = reaction.addProduct();
        _loadReactionMolecule(reaction, index, have_aam);
    }

    for (i = 0; i < ncatalyst; i++)
    {
        int index = reaction.addCatalyst();
        _loadReactionMolecule(reaction, index, have_aam);
    }
}

void CrfLoader::_loadReactionMolecule(Reaction& reaction, int index, bool have_aam)
{
    _bond_rc_flags = &reaction.getReactingCenterArray(index);
    _atom_stereo_flags = &reaction.getInversionArray(index);
    if (have_aam)
        _aam = &reaction.getAAMArray(index);

    _loadMolecule(reaction.getMolecule(index));
}

void CrfLoader::_loadMolecule(Molecule& molecule)
{
    Obj<CmfLoader> loader;
    int i;

    if (_decoder.get() != 0)
        loader.create(_decoder.ref());
    else
        loader.create(_scanner);

    QS_DEF(Array<int>, atom_flags);
    QS_DEF(Array<int>, bond_flags);

    loader->atom_flags = &atom_flags;
    loader->bond_flags = &bond_flags;
    loader->version = version;
    loader->loadMolecule(molecule);
    bool has_mapping = loader->has_mapping;

    if (_atom_stereo_flags != 0)
    {
        _atom_stereo_flags->clear_resize(molecule.vertexCount());
        _atom_stereo_flags->zerofill();
        for (i = 0; i < molecule.vertexCount(); i++)
        {
            int idx = i;
            if (has_mapping)
                idx = loader->inv_atom_mapping_to_restore[i];
            if (atom_flags[i] & 1)
                _atom_stereo_flags->at(idx) |= STEREO_RETAINS;
            if (atom_flags[i] & 2)
                _atom_stereo_flags->at(idx) |= STEREO_INVERTS;
        }
    }

    if (_bond_rc_flags != 0)
    {
        _bond_rc_flags->clear_resize(molecule.edgeCount());
        _bond_rc_flags->zerofill();

        for (i = 0; i < molecule.edgeCount(); i++)
        {
            int idx = i;
            if (has_mapping)
                idx = loader->inv_bond_mapping_to_restore[i];

            if (bond_flags[i] & 1)
                _bond_rc_flags->at(idx) |= RC_UNCHANGED;
            if (bond_flags[i] & 2)
                _bond_rc_flags->at(idx) |= RC_MADE_OR_BROKEN;
            if (bond_flags[i] & 4)
                _bond_rc_flags->at(idx) |= RC_ORDER_CHANGED;
        }
    }

    if (_aam != 0)
    {
        _aam->clear_resize(molecule.vertexCount());
        _aam->zerofill();
        for (i = 0; i < molecule.vertexCount(); i++)
        {
            int value;

            if (_decoder.get() != 0)
                value = _decoder->get();
            else
                value = _scanner.readByte();

            int idx = i;
            if (has_mapping)
                idx = loader->inv_atom_mapping_to_restore[i];
            _aam->at(idx) = value - 1;
        }
    }

    if (xyz_scanner != 0)
        loader->loadXyz(*xyz_scanner);
}
