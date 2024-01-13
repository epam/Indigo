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

#include "reaction/crf_saver.h"
#include "base_cpp/output.h"
#include "lzw/lzw_encoder.h"
#include "molecule/cmf_saver.h"
#include "molecule/cmf_symbol_codes.h"
#include "reaction/crf_common.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(CrfSaver, "CRF saver");

void CrfSaver::_init()
{
    xyz_output = 0;
    save_bond_dirs = false;
    save_highlighting = false;
    save_mapping = false;
}

CrfSaver::CrfSaver(LzwDict& dict, Output& output) : _output(output)
{
    if (!dict.isInitialized())
        dict.init(CMF_ALPHABET_SIZE, CMF_BIT_CODE_SIZE);

    _encoder.create(dict, output);
    _init();
}

CrfSaver::CrfSaver(Output& output) : _output(output)
{
    _init();
}

void CrfSaver::saveReaction(Reaction& reaction)
{
    _writeReactionInfo(reaction);

    int i;

    _atom_stereo_flags = 0;
    _bond_rc_flags = 0;
    _aam = 0;

    for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i))
        _writeReactionMolecule(reaction, i);

    for (i = reaction.productBegin(); i < reaction.productEnd(); i = reaction.productNext(i))
        _writeReactionMolecule(reaction, i);

    if (reaction.catalystCount() > 0)
    {
        for (i = reaction.catalystBegin(); i < reaction.catalystEnd(); i = reaction.catalystNext(i))
            _writeReactionMolecule(reaction, i);
    }

    if (_encoder.get() != 0)
        _encoder->finish();
}

void CrfSaver::_writeReactionMolecule(Reaction& reaction, int i)
{
    _atom_stereo_flags = reaction.getInversionArray(i).ptr();
    _bond_rc_flags = reaction.getReactingCenterArray(i).ptr();
    _aam = reaction.getAAMArray(i).ptr();
    _writeMolecule(reaction.getMolecule(i));
}

void CrfSaver::_writeMolecule(Molecule& molecule)
{
    Obj<CmfSaver> saver;
    int i;

    if (_encoder.get() != 0)
        saver.create(_encoder.ref());
    else
        saver.create(_output);

    QS_DEF(Array<int>, atom_flags);
    QS_DEF(Array<int>, bond_flags);

    if (_atom_stereo_flags != 0)
    {
        atom_flags.clear_resize(molecule.vertexEnd());
        atom_flags.zerofill();

        for (i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
            if (_atom_stereo_flags[i] == STEREO_RETAINS)
                atom_flags[i] = 1;
            else if (_atom_stereo_flags[i] == STEREO_INVERTS)
                atom_flags[i] = 2;
        saver->atom_flags = atom_flags.ptr();
    }

    if (_bond_rc_flags != 0)
    {
        bond_flags.clear_resize(molecule.edgeEnd());
        bond_flags.zerofill();

        for (i = molecule.edgeBegin(); i != molecule.edgeEnd(); i = molecule.edgeNext(i))
        {
            if (_bond_rc_flags[i] & RC_UNCHANGED)
                bond_flags[i] |= 1;
            if (_bond_rc_flags[i] & RC_MADE_OR_BROKEN)
                bond_flags[i] |= 2;
            if (_bond_rc_flags[i] & RC_ORDER_CHANGED)
                bond_flags[i] |= 4;
        }
        saver->bond_flags = bond_flags.ptr();
    }

    saver->save_bond_dirs = save_bond_dirs;
    saver->save_highlighting = save_highlighting;
    saver->save_mapping = save_mapping;

    saver->saveMolecule(molecule);

    if (_aam != 0)
        _writeAam(_aam, saver->getAtomSequence());

    if (xyz_output != 0)
    {
        if (xyz_output == &_output && _encoder.get() != 0)
            _encoder->finish();

        saver->saveXyz(*xyz_output);

        if (xyz_output == &_output && _encoder.get() != 0)
            _encoder->start();
    }
}

void CrfSaver::_writeReactionInfo(Reaction& reaction)
{
    _output.writePackedUInt(reaction.reactantsCount());
    _output.writePackedUInt(reaction.productsCount());

    byte features = CrfFeatureFlags::CRF_AAM;
    if (reaction.catalystCount() > 0)
        features |= CrfFeatureFlags::CRF_CATALYST;

    _output.writeByte(features);
    if (reaction.catalystCount() > 0)
        _output.writePackedUInt(reaction.catalystCount());
}

void CrfSaver::_writeAam(const int* aam, const Array<int>& sequence)
{
    int i;

    for (i = 0; i < sequence.size(); i++)
    {
        int value = aam[sequence[i]] + 1;

        if (value < 1 || value >= CMF_ALPHABET_SIZE)
            throw Error("bad AAM value: %d", value);

        if (_encoder.get() != 0)
            _encoder->send(value);
        else
            _output.writeByte(static_cast<byte>(value));
    }
}
