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

#include "bingo_context.h"
#include "layout/reaction_layout.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_automapper.h"
#include "reaction/rxnfile_saver.h"
#include "ringo_matchers.h"

using namespace indigo;

IMPL_ERROR(RingoAAM, "ringo AAM");

RingoAAM::RingoAAM(BingoContext& context) : _context(context)
{
}

void RingoAAM::loadReaction(const Array<char>& buf)
{
    BufferScanner scanner(buf);
    loadReaction(scanner);
}

void RingoAAM::loadReaction(const char* str)
{
    BufferScanner scanner(str);
    loadReaction(scanner);
}

void RingoAAM::loadReaction(Scanner& scanner)
{
    ReactionAutoLoader rxd(scanner);
    _context.setLoaderSettings(rxd);
    rxd.loadReaction(_reaction);
}

void RingoAAM::parse(const char* mode)
{
    if (strcasecmp(mode, "CLEAR") == 0)
    {
        _reaction.clearAAM();
        return;
    }
    ReactionAutomapper ram(_reaction);
    if (strcasecmp(mode, "DISCARD") == 0)
    {
        ram.automap(ReactionAutomapper::AAM_REGEN_DISCARD);
    }
    else if (strcasecmp(mode, "ALTER") == 0)
    {
        ram.automap(ReactionAutomapper::AAM_REGEN_ALTER);
    }
    else if (strcasecmp(mode, "KEEP") == 0)
    {
        ram.automap(ReactionAutomapper::AAM_REGEN_KEEP);
    }
    else
        throw Error("unknown mode: %s", mode);
}

void RingoAAM::getResult(Array<char>& buf)
{
    ArrayOutput output_r(buf);
    RxnfileSaver rcs(output_r);

    if (!Reaction::haveCoord(_reaction))
    {
        ReactionLayout layout(_reaction);

        layout.make();
        _reaction.markStereocenterBonds();
    }
    _context.setSaverSettings(rcs);

    rcs.saveReaction(_reaction);
}
