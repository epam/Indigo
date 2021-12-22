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

#include "base_cpp/crc32.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "graph/subgraph_hash.h"
#include "molecule/elements.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/crf_loader.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_exact_matcher.h"
#include "ringo_matchers.h"

IMPL_ERROR(RingoExact, "Ringo exact");

RingoExact::RingoExact(BingoContext& context) : _context(context)
{
    _flags = 0;
}

void RingoExact::loadQuery(Scanner& scanner)
{
    ReactionAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadReaction(_query);
    Reaction::checkForConsistency(_query);

    _initQuery(_query);

    _query_hash = calculateHash(_query);
}

dword RingoExact::calculateHash(Reaction& rxn)
{
    QS_DEF(Molecule, mol_without_h);
    QS_DEF(Array<int>, vertices);
    int i, j;
    dword hash = 0;

    for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
    {
        Molecule& mol = rxn.getMolecule(j);

        vertices.clear();

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (mol.getAtomNumber(i) != ELEM_H)
                vertices.push(i);

        mol_without_h.makeSubmolecule(mol, vertices, 0);
        SubgraphHash hh(mol_without_h);
        hash += hh.getHash();
    }

    return hash;
}

void RingoExact::loadQuery(const Array<char>& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void RingoExact::loadQuery(const char* buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

dword RingoExact::getQueryHash()
{
    return _query_hash;
}

const char* RingoExact::getQueryHashStr()
{
    ArrayOutput out(_query_hash_str);
    out.printf("%02X", getQueryHash());
    _query_hash_str.push(0);
    return _query_hash_str.ptr();
}

void RingoExact::setParameters(const char* flags)
{
    // TODO: merge Indigo code into Bingo and stop this endless copy-paste

    if (flags == 0)
        flags = "";

    static struct
    {
        const char* token;
        int value;
    } token_list[] = {{"ELE", MoleculeExactMatcher::CONDITION_ELECTRONS},
                      {"MAS", MoleculeExactMatcher::CONDITION_ISOTOPE},
                      {"STE", MoleculeExactMatcher::CONDITION_STEREO},
                      {"AAM", ReactionExactMatcher::CONDITION_AAM},
                      {"RCT", ReactionExactMatcher::CONDITION_REACTING_CENTERS}};

    int i, res = 0, count = 0;
    bool had_none = false;
    bool had_all = false;

    BufferScanner scanner(flags);

    QS_DEF(Array<char>, word);
    while (1)
    {
        scanner.skipSpace();
        if (scanner.isEOF())
            break;
        scanner.readWord(word, 0);

        if (strcasecmp(word.ptr(), "NONE") == 0)
        {
            if (had_all)
                throw Error("NONE conflicts with ALL");
            had_none = true;
            count++;
            continue;
        }
        if (strcasecmp(word.ptr(), "ALL") == 0)
        {
            if (had_none)
                throw Error("ALL conflicts with NONE");
            had_all = true;
            res = MoleculeExactMatcher::CONDITION_ALL | ReactionExactMatcher::CONDITION_ALL;
            count++;
            continue;
        }

        for (i = 0; i < NELEM(token_list); i++)
        {
            if (strcasecmp(token_list[i].token, word.ptr()) == 0)
            {
                if (had_all)
                    throw Error("only negative flags are allowed together with ALL");
                res |= token_list[i].value;
                break;
            }
            if (word[0] == '-' && strcasecmp(token_list[i].token, word.ptr() + 1) == 0)
            {
                res &= ~token_list[i].value;
                break;
            }
        }
        if (i == NELEM(token_list))
            throw Error("unknown token %s", word.ptr());
        count++;
    }

    if (had_none && count > 1)
        throw Error("no flags are allowed together with NONE");

    if (count == 0)
        res = MoleculeExactMatcher::CONDITION_ALL | ReactionExactMatcher::CONDITION_ALL;

    _flags = res;
}

void RingoExact::loadTarget(Scanner& scanner)
{
    ReactionAutoLoader loader(scanner);

    _context.setLoaderSettings(loader);
    loader.loadReaction(_target);
    Reaction::checkForConsistency(_target);
    _initTarget(_target, false);
}

void RingoExact::loadTarget(const Array<char>& target_buf)
{
    BufferScanner scanner(target_buf);

    loadTarget(scanner);
}

void RingoExact::loadTarget(const char* target)
{
    BufferScanner scanner(target);

    loadTarget(scanner);
}

bool RingoExact::matchLoadedTarget()
{
    ReactionExactMatcher matcher(_query, _target);

    matcher.flags = _flags;

    return matcher.find();
}

void RingoExact::_initQuery(Reaction& query)
{
    int i, j;
    query.aromatize(AromaticityOptions::BASIC);

    if (_flags & MoleculeExactMatcher::CONDITION_STEREO)
    {
        for (j = query.begin(); j != query.end(); j = query.next(j))
        {
            Molecule& mol = query.getMolecule(j);
            for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
                if (mol.getEdgeTopology(i) == TOPOLOGY_RING)
                    mol.cis_trans.setParity(i, 0);
        }
    }
}

void RingoExact::_initTarget(Reaction& target, bool from_database)
{
    if (!from_database)
        target.aromatize(AromaticityOptions::BASIC);
}

bool RingoExact::matchBinary(Scanner& scanner)
{
    CrfLoader loader(_context.cmf_dict, scanner);

    loader.loadReaction(_target);

    _initTarget(_target, true);

    ReactionExactMatcher matcher(_query, _target);

    matcher.flags = _flags;

    return matcher.find();
}

bool RingoExact::matchBinary(const Array<char>& target_buf)
{
    BufferScanner scanner(target_buf);

    return matchBinary(scanner);
}
