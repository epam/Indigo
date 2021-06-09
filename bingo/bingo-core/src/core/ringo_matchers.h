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

#ifndef __ringo_matchers__
#define __ringo_matchers__

#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_neighborhood_counters.h"

using namespace indigo;

namespace indigo
{
    class Scanner;
}

class BingoContext;

class RingoSubstructure
{
public:
    explicit RingoSubstructure(BingoContext& context);
    virtual ~RingoSubstructure();

    bool parse(const char* params);

    bool matchBinary(const ArrayChar& buf);
    bool matchBinary(Scanner& scanner);

    void loadTarget(const ArrayChar& buf);
    void loadTarget(Scanner& scanner);
    void loadTarget(const char* str);

    bool matchLoadedTarget();

    void loadQuery(const ArrayChar& buf);
    void loadQuery(const char* str);
    void loadQuery(Scanner& scanner);

    void loadSMARTS(const ArrayChar& buf);
    void loadSMARTS(const char* str);
    void loadSMARTS(Scanner& scanner);

    bool preserve_bonds_on_highlighting;

    void getHighlightedTarget(ArrayChar& buf);

    const byte* getQueryFingerprint();

    DECL_ERROR;

protected:
    BingoContext& _context;

    QueryReaction _query_reaction;
    Reaction _target_reaction;

    ReactionAtomNeighbourhoodCounters _nei_target_counters;
    ReactionAtomNeighbourhoodCounters _nei_query_counters;

    ObjArray<ArrayNew<int>> _target_bond_types;

    Array<byte> _query_fp;
    bool _query_data_valid;
    bool _smarts;

    void _validateQueryData();

    void _initQuery(QueryReaction& query_in, QueryReaction& query_out);
    void _initSmartsQuery(QueryReaction& query_in, QueryReaction& query_out);
    void _initTarget(bool from_database);
};

class RingoAAM
{

public:
    RingoAAM(BingoContext& context);

    void parse(const char* mode);

    void loadReaction(const ArrayChar& buf);
    void loadReaction(const char* str);
    void loadReaction(Scanner& scanner);

    void getResult(ArrayChar& buf);

    DECL_ERROR;

protected:
    BingoContext& _context;
    Reaction _reaction;
};

class RingoExact
{
public:
    RingoExact(BingoContext& context);

    void loadQuery(const ArrayChar& buf);
    void loadQuery(Scanner& scanner);
    void loadQuery(const char* buf);

    dword getQueryHash();
    const char* getQueryHashStr();

    void loadTarget(const ArrayChar& molfile_buf);
    void loadTarget(Scanner& scanner);
    void loadTarget(const char* target);

    bool matchLoadedTarget();

    bool matchBinary(Scanner& scanner);
    bool matchBinary(const ArrayChar& target_buf);

    void setParameters(const char* conditions);

    static dword calculateHash(Reaction& rxn);

    DECL_ERROR;

protected:
    BingoContext& _context;

    Reaction _query;
    Reaction _target;
    dword _query_hash;
    int _flags;
    ArrayChar _query_hash_str;

    void _initQuery(Reaction& query);
    static void _initTarget(Reaction& target, bool from_database);
};

#endif
