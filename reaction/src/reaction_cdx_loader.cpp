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

#include "reaction/reaction_cdx_loader.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule_cdx_loader.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(ReactionCdxLoader, "reaction CDX loader");

ReactionCdxLoader::ReactionCdxLoader(Scanner& scanner) : _scanner(scanner)
{
    ignore_bad_valence = false;
}

ReactionCdxLoader::~ReactionCdxLoader()
{
}

void ReactionCdxLoader::loadReaction(Reaction& rxn)
{
    rxn.clear();

    QS_DEF(Array<char>, buf);
    _scanner.readAll(buf);
    buf.push(0);
}
