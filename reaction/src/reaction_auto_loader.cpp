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

#include "reaction/reaction_auto_loader.h"
#include "gzip/gzip_scanner.h"
#include "molecule/molecule_auto_loader.h"
#include "reaction/icr_loader.h"
#include "reaction/icr_saver.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cml_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"

using namespace indigo;

void ReactionAutoLoader::_init()
{
    treat_x_as_pseudoatom = false;
    ignore_closing_bond_direction_mismatch = false;
    stereochemistry_options.reset();
    ignore_noncritical_query_features = false;
    ignore_cistrans_errors = false;
    ignore_no_chiral_flag = false;
    ignore_bad_valence = false;
}

IMPL_ERROR(ReactionAutoLoader, "reaction auto loader");

ReactionAutoLoader::ReactionAutoLoader(Scanner& scanner)
{
    _scanner = &scanner;
    _own_scanner = false;
    _init();
}

ReactionAutoLoader::ReactionAutoLoader(const Array<char>& arr)
{
    _scanner = new BufferScanner(arr);
    _own_scanner = true;
    _init();
}

ReactionAutoLoader::ReactionAutoLoader(const char* str)
{
    _scanner = new BufferScanner(str);
    _own_scanner = true;
    _init();
}

ReactionAutoLoader::~ReactionAutoLoader()
{
    if (_own_scanner)
        delete _scanner;
}

void ReactionAutoLoader::loadReaction(Reaction& reaction)
{
    _loadReaction(reaction, false);
}

void ReactionAutoLoader::loadQueryReaction(QueryReaction& reaction)
{
    _loadReaction(reaction, true);
}

void ReactionAutoLoader::_loadReaction(BaseReaction& reaction, bool query)
{
    // check fir GZip format
    if (_scanner->length() >= 2)
    {
        byte id[2];
        long long pos = _scanner->tell();

        _scanner->readCharsFix(2, (char*)id);
        _scanner->seek(pos, SEEK_SET);

        if (id[0] == 0x1f && id[1] == 0x8b)
        {
            GZipScanner gzscanner(*_scanner);
            QS_DEF(Array<char>, buf);

            gzscanner.readAll(buf);
            ReactionAutoLoader loader2(buf);

            loader2.stereochemistry_options = stereochemistry_options;
            loader2.ignore_noncritical_query_features = ignore_noncritical_query_features;
            loader2.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
            loader2.ignore_no_chiral_flag = ignore_no_chiral_flag;
            loader2.ignore_bad_valence = ignore_bad_valence;
            if (query)
                loader2.loadQueryReaction((QueryReaction&)reaction);
            else
                loader2.loadReaction((Reaction&)reaction);
            return;
        }
    }

    // check for MDLCT format
    {
        QS_DEF(Array<char>, buf);
        if (MoleculeAutoLoader::tryMDLCT(*_scanner, buf))
        {
            BufferScanner scanner2(buf);
            RxnfileLoader loader(scanner2);
            loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
            loader.stereochemistry_options = stereochemistry_options;
            loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
            loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
            loader.ignore_bad_valence = ignore_bad_valence;
            if (query)
                loader.loadQueryReaction((QueryReaction&)reaction);
            else
                loader.loadReaction((Reaction&)reaction);
            return;
        }
    }

    // check for ICM format
    if (_scanner->length() >= 4)
    {
        char id[3];
        long long pos = _scanner->tell();

        _scanner->readCharsFix(3, id);
        _scanner->seek(pos, SEEK_SET);
        if (IcrSaver::checkVersion(id))
        {
            if (query)
                throw Error("cannot load query reaction from ICR format");

            IcrLoader loader(*_scanner);
            loader.loadReaction((Reaction&)reaction);
            return;
        }
    }

    // check for CML format
    {
        long long pos = _scanner->tell();
        _scanner->skipSpace();

        if (_scanner->lookNext() == '<')
        {
            if (_scanner->findWord("<reaction"))
            {
                ReactionCmlLoader loader(*_scanner);
                loader.stereochemistry_options = stereochemistry_options;

                if (query)
                    throw Error("CML queries not supported");
                loader.loadReaction((Reaction&)reaction);
                return;
            }
        }

        _scanner->seek(pos, SEEK_SET);
    }

    // check for SMILES format
    if (Scanner::isSingleLine(*_scanner))
    {
        RSmilesLoader loader(*_scanner);

        loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
        loader.ignore_cistrans_errors = ignore_cistrans_errors;
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_bad_valence = ignore_bad_valence;

        if (query)
            loader.loadQueryReaction((QueryReaction&)reaction);
        else
            loader.loadReaction((Reaction&)reaction);
    }

    // default is Rxnfile format
    else
    {
        RxnfileLoader loader(*_scanner);
        loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
        loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
        loader.ignore_bad_valence = ignore_bad_valence;

        if (query)
            loader.loadQueryReaction((QueryReaction&)reaction);
        else
            loader.loadReaction((Reaction&)reaction);
    }
}
