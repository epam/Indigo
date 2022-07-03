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

#include "reaction/reaction_fingerprint.h"
#include "base_c/bitarray.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/base_reaction.h"

using namespace indigo;

IMPL_ERROR(ReactionFingerprintBuilder, "fingerprint builder");

CP_DEF(ReactionFingerprintBuilder);

ReactionFingerprintBuilder::ReactionFingerprintBuilder(BaseReaction& reaction, const MoleculeFingerprintParameters& parameters)
    : _reaction(reaction), _parameters(parameters), CP_INIT, TL_CP_GET(_fingerprint)
{
    query = false;
    skip_sim = false;
    skip_ord = false;
    skip_ext = false;
}

void ReactionFingerprintBuilder::process()
{
    int i, one_fp_size = _parameters.fingerprintSizeExtOrdSim();

    _fingerprint.clear_resize(one_fp_size * 2);
    _fingerprint.zerofill();

    for (i = _reaction.reactantBegin(); i < _reaction.reactantEnd(); i = _reaction.reactantNext(i))
    {
        MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

        builder.query = query;
        builder.skip_tau = true;
        builder.skip_sim = skip_sim;
        builder.skip_ord = skip_ord;
        builder.skip_ext = skip_ext;
        builder.skip_any_atoms = true;
        builder.skip_any_bonds = true;
        builder.skip_any_atoms_bonds = true;
        builder.process();
        bitOr(get(), builder.get(), _parameters.fingerprintSizeExtOrd());
        bitOr(getSim(), builder.getSim(), _parameters.fingerprintSizeSim());
    }
    for (i = _reaction.productBegin(); i < _reaction.productEnd(); i = _reaction.productNext(i))
    {
        MoleculeFingerprintBuilder builder(_reaction.getBaseMolecule(i), _parameters);

        builder.query = query;
        builder.skip_tau = true;
        builder.skip_sim = skip_sim;
        builder.skip_ord = skip_ord;
        builder.skip_ext = skip_ext;
        builder.skip_any_atoms = true;
        builder.skip_any_bonds = true;
        builder.skip_any_atoms_bonds = true;
        builder.process();
        bitOr(get() + _parameters.fingerprintSizeExtOrd(), builder.get(), _parameters.fingerprintSizeExtOrd());
        bitOr(getSim() + +_parameters.fingerprintSizeSim(), builder.getSim(), _parameters.fingerprintSizeSim());
    }
}

byte* ReactionFingerprintBuilder::get()
{
    return _fingerprint.ptr();
}

byte* ReactionFingerprintBuilder::getSim()
{
    return _fingerprint.ptr() + _parameters.fingerprintSizeExtOrd() * 2;
}

void ReactionFingerprintBuilder::parseFingerprintType(const char* type, bool query)
{
    this->query = query;

    if (type == 0 || *type == 0 || strcasecmp(type, "sim") == 0)
    {
        // similarity
        this->skip_ext = true;
        this->skip_ord = true;
    }
    else if (strcasecmp(type, "sub") == 0)
        // substructure
        this->skip_sim = true;
    else if (strcasecmp(type, "full") == 0)
    {
        if (query)
            throw Error("there can not be 'full' fingerprint of a query reaction");
        // full (non-query) fingerprint, do not skip anything
    }
    else
        throw Error("unknown molecule fingerprint type: %s", type);
}
