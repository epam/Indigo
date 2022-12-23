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
#include "reaction/reaction_cdxml_loader.h"
#include "reaction/reaction_cml_loader.h"
#include "reaction/reaction_json_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"

#include <string>

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
    dearomatize_on_load = false;
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

void ReactionAutoLoader::loadQueryReaction(QueryReaction& qreaction)
{
    loadReaction(qreaction);
}

void ReactionAutoLoader::loadReaction(BaseReaction& reaction)
{
    _loadReaction(reaction);
    if (!reaction.isQueryReaction() && dearomatize_on_load)
        reaction.dearomatize(arom_options);
}

void ReactionAutoLoader::_loadReaction(BaseReaction& reaction)
{
    bool query = reaction.isQueryReaction();
    auto local_scanner = _scanner;
    // chack for base64
    uint8_t base64_id[] = "base64::";
    std::unique_ptr<BufferScanner> base64_scanner;
    Array<char> base64_data;
    if (local_scanner->length() >= (sizeof(base64_id) - 1))
    {
        byte id[sizeof(base64_id) - 1];
        long long pos = local_scanner->tell();
        local_scanner->readCharsFix(sizeof(base64_id) - 1, (char*)id);
        bool is_base64 = (std::equal(std::begin(id), std::end(id), std::begin(base64_id)));
        if (!is_base64)
            local_scanner->seek(pos, SEEK_SET);

        std::string base64_str;
        local_scanner->readAll(base64_str);
        base64_str.erase(std::remove_if(base64_str.begin(), base64_str.end(), [](char c) { return c == '\n' || c == '\r'; }), base64_str.end());
        if (validate_base64(base64_str))
        {
            base64_data.copy(base64_str.data(), base64_str.size());
            base64_scanner = std::make_unique<BufferScanner>(base64_data, true);
            local_scanner = base64_scanner.get();
        }
        local_scanner->seek(pos, SEEK_SET);
        _scanner->seek(pos, SEEK_SET);
    }

    // check fir GZip format
    if (local_scanner->length() >= 2)
    {
        byte id[2];
        long long pos = local_scanner->tell();

        local_scanner->readCharsFix(2, (char*)id);
        local_scanner->seek(pos, SEEK_SET);

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
            loader2.loadReaction(reaction);
            return;
        }
    }

    {
        if (local_scanner->findWord(kCDX_HeaderString))
        {
            local_scanner->seek(kCDX_HeaderLength, SEEK_CUR);
            ReactionCdxmlLoader loader(*local_scanner, true);
            loader.stereochemistry_options = stereochemistry_options;
            if (query)
                throw Error("CDX queries not supported yet");
            loader.loadReaction(reaction);
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

    // check for CDXML format
    {
        long long pos = _scanner->tell();
        _scanner->skipSpace();
        if (_scanner->lookNext() == '<' && _scanner->findWord("<CDXML"))
        {
            _scanner->seek(pos, SEEK_SET);
            ReactionCdxmlLoader loader(*_scanner);
            loader.stereochemistry_options = stereochemistry_options;
            loader.loadReaction(reaction);
            return;
        }
        _scanner->seek(pos, SEEK_SET);
    }

    // check for JSON-KET format

    {
        long long pos = _scanner->tell();
        bool hasbom = false;
        if (_scanner->length() >= 3)
        {
            unsigned char bom[3];
            _scanner->readCharsFix(3, (char*)bom);
            if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
                hasbom = true;
            else
                _scanner->seek(pos, SEEK_SET);
        }

        if (_scanner->lookNext() == '{')
        {
            if (_scanner->findWord("arrow"))
            {
                using namespace rapidjson;
                _scanner->seek(pos, SEEK_SET);
                {
                    Array<char> buf;
                    _scanner->readAll(buf);
                    buf.push(0);
                    unsigned char* ptr = (unsigned char*)buf.ptr();
                    // skip utf8 BOM
                    if (hasbom)
                        ptr += 3;
                    Document data;
                    if (!data.Parse((char*)ptr).HasParseError())
                    {
                        if (data.HasMember("root") && data["root"].HasMember("nodes"))
                        {
                            ReactionJsonLoader loader(data);
                            loader.stereochemistry_options = stereochemistry_options;
                            loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
                            loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
                            loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
                            loader.loadReaction(reaction);
                        }
                    }
                    return;
                }
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
