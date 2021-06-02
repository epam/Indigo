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

#include "ringo_index.h"

#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/output.h"
#include "bingo_context.h"
#include "mango_index.h"
#include "ringo_matchers.h"
#include "reaction/crf_saver.h"
#include "reaction/reaction.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_automapper.h"
#include "reaction/reaction_fingerprint.h"

void RingoIndex::prepare(Scanner& rxnfile, Output& output, OsLock* lock_for_exclusive_access)
{
    QS_DEF(Reaction, reaction);

    ReactionAutoLoader rrd(rxnfile);
    _context->setLoaderSettings(rrd);
    rrd.loadReaction(reaction);

    // Skip all SGroups
    for (int mol_idx = reaction.begin(); mol_idx != reaction.end(); mol_idx = reaction.next(mol_idx))
        reaction.getBaseMolecule(mol_idx).clearSGroups();

    Reaction::checkForConsistency(reaction);

    ReactionAutomapper ram(reaction);
    ram.correctReactingCenters(true);

    reaction.aromatize(AromaticityOptions::BASIC);

    _hash = RingoExact::calculateHash(reaction);
    {
        ArrayOutput out(_hash_str);
        out.printf("%02X", _hash);
        _hash_str.push(0);
    }

    if (!skip_calculate_fp)
    {
        ReactionFingerprintBuilder builder(reaction, _context->fp_parameters);

        builder.process();
        _fp.copy(builder.get(), _context->fp_parameters.fingerprintSizeExtOrdSim() * 2);
    }

    ArrayOutput output_crf(_crf);
    {
        // CrfSaver modifies _context->cmf_dict and
        // requires exclusive access for this
        OsLockerNullable locker(lock_for_exclusive_access);
        CrfSaver saver(_context->cmf_dict, output_crf);
        saver.saveReaction(reaction);
    }

    output.writeArray(_crf);
}

const byte* RingoIndex::getFingerprint()
{
    return _fp.ptr();
}

const ArrayChar& RingoIndex::getCrf()
{
    return _crf;
}

dword RingoIndex::getHash()
{
    return _hash;
}

const char* RingoIndex::getHashStr()
{
    return _hash_str.ptr();
}

void RingoIndex::clear()
{
    _fp.clear();
    _crf.clear();
}
