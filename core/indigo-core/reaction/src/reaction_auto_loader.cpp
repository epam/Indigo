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
#include "molecule/parse_utils.h"
#include "molecule/rdf_loader.h"
#include "reaction/icr_loader.h"
#include "reaction/icr_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/pathway_reaction_builder.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cdxml_loader.h"
#include "reaction/reaction_cml_loader.h"
#include "reaction/reaction_json_loader.h"
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
    dearomatize_on_load = false;
    treat_stereo_as = 0;
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
    auto rptr = loadReaction(reaction.isQueryReaction());
    reaction.clone(*rptr);
    reaction.original_format = rptr->original_format;
    for (int i = 0; i < rptr->reactionBlocksCount(); i++)
        reaction.addReactionBlock().copy(rptr->reactionBlock(i));
}

std::unique_ptr<BaseReaction> ReactionAutoLoader::loadReaction(bool query)
{
    auto reaction = _loadReaction(query);
    if (!query && dearomatize_on_load)
        reaction->dearomatize(arom_options);
    return reaction;
}

std::unique_ptr<BaseReaction> ReactionAutoLoader::_loadReaction(bool query)
{
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
            base64_data.copy(base64_str.data(), static_cast<int>(base64_str.size()));
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
            return loader2.loadReaction(query);
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
            auto reaction = std::make_unique<Reaction>();
            loader.loadReaction(*reaction);
            return reaction;
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
            {
                auto reaction = std::make_unique<QueryReaction>();
                loader.loadQueryReaction(*reaction);
                return reaction;
            }
            else
            {
                auto reaction = std::make_unique<Reaction>();
                loader.loadReaction(*reaction);
                return reaction;
            }
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
            auto reaction = std::make_unique<Reaction>();
            loader.loadReaction(*reaction);
            return reaction;
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
                auto reaction = std::make_unique<Reaction>();
                loader.loadReaction(*reaction);
                return reaction;
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

            if (query)
            {
                auto reaction = std::make_unique<QueryReaction>();
                loader.loadReaction(*reaction);
                return reaction;
            }
            else
            {
                auto reaction = std::make_unique<Reaction>();
                loader.loadReaction(*reaction);
                return reaction;
            }
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
                _scanner->seek(pos, SEEK_SET);
                bool is_pathway = _scanner->findWord("multi-tailed-arrow");
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
                            ReactionJsonLoader loader(data, layout_options);
                            loader.stereochemistry_options = stereochemistry_options;
                            loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
                            loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
                            loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
                            std::unique_ptr<BaseReaction> reaction;
                            if (is_pathway)
                            {
                                auto pwr = std::make_unique<PathwayReaction>();
                                loader.loadReaction(*pwr);
                                if (pwr->reactionsCount() == 0) // something went wrong
                                {
                                    reaction = std::make_unique<Reaction>();
                                    reaction->clone(*pwr);
                                }
                                else
                                    reaction = std::move(pwr);
                            }
                            else if (query)
                            {
                                reaction = std::make_unique<QueryReaction>();
                                loader.loadReaction(*reaction);
                            }
                            else
                            {
                                reaction = std::make_unique<Reaction>();
                                loader.loadReaction(*reaction);
                            }
                            return reaction;
                        }
                    }
                    return nullptr;
                }
            }
        }
        _scanner->seek(pos, SEEK_SET);
    }

    // check for RDF format
    {
        long long pos = _scanner->tell();
        Array<char> firstLine;
        const char* delimeters{};
        _scanner->readWord(firstLine, delimeters);
        _scanner->seek(pos, SEEK_SET);
        if (!strcmp(firstLine.ptr(), "$RDFILE"))
        {
            if (query)
                throw Error("RDF queries not supported");
            std::deque<Reaction> reactions;
            RdfLoader rdf_loader(*_scanner);
            while (!rdf_loader.isEOF())
            {
                rdf_loader.readNext();
                BufferScanner reaction_scanner(rdf_loader.data);
                RxnfileLoader loader(reaction_scanner);
                loader.stereochemistry_options = stereochemistry_options;
                loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
                loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
                loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
                loader.treat_stereo_as = treat_stereo_as;
                loader.ignore_bad_valence = ignore_bad_valence;
                reactions.emplace_back();
                loader.loadReaction(reactions.back(), rdf_loader.properties);
            }

            PathwayReactionBuilder builder;
            return builder.buildPathwayReaction(reactions, layout_options);
        }
    }

    // check for SMILES format
    if (Scanner::isSingleLine(*_scanner))
    {
        long long pos = _scanner->tell();
        RSmilesLoader loader(*_scanner);

        loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
        loader.ignore_cistrans_errors = ignore_cistrans_errors;
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_bad_valence = ignore_bad_valence;

        if (query)
        {
            // Try to load query as SMILES, if error occured - try to load as SMARTS
            auto reaction = std::make_unique<QueryReaction>();
            try
            {
                loader.loadQueryReaction(*reaction);
            }
            catch (Exception&)
            {
                loader.smarts_mode = true;
                _scanner->seek(pos, SEEK_SET);
                loader.loadQueryReaction(*reaction);
            }
            return reaction;
        }
        else
        {
            auto reaction = std::make_unique<Reaction>();
            loader.loadReaction(*reaction);
            return reaction;
        }
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
        {
            auto reaction = std::make_unique<QueryReaction>();
            loader.loadQueryReaction(*reaction);
            return reaction;
        }
        else
        {
            auto reaction = std::make_unique<Reaction>();
            loader.loadReaction(*reaction);
            return reaction;
        }
    }
}
