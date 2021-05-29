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

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "ringo_matchers.h"
#include "layout/reaction_layout.h"
#include "molecule/molecule_fingerprint.h"
#include "reaction/crf_loader.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/reaction_automapper.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/reaction_substructure_matcher.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rxnfile_saver.h"

IMPL_ERROR(RingoSubstructure, "reaction substructure");

RingoSubstructure::RingoSubstructure(BingoContext& context) : _context(context)
{
    preserve_bonds_on_highlighting = false;
    _smarts = false;
}

RingoSubstructure::~RingoSubstructure()
{
}

bool RingoSubstructure::parse(const char* params)
{
    preserve_bonds_on_highlighting = false;
    return true;
}

void RingoSubstructure::loadQuery(const std::string& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void RingoSubstructure::loadQuery(const char* str)
{
    BufferScanner scanner(str);

    loadQuery(scanner);
}

void RingoSubstructure::loadQuery(Scanner& scanner)
{
    QS_DEF(QueryReaction, source);

    ReactionAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadQueryReaction(source);

    _initQuery(source, _query_reaction);
    _query_data_valid = false;
    _smarts = false;
}

void RingoSubstructure::loadSMARTS(Scanner& scanner)
{
    RSmilesLoader loader(scanner);
    QS_DEF(QueryReaction, source);

    loader.smarts_mode = true;
    loader.loadQueryReaction(source);

    _initSmartsQuery(source, _query_reaction);
    _query_data_valid = false;
    _smarts = true;
}

void RingoSubstructure::loadSMARTS(const std::string& buf)
{
    BufferScanner scanner(buf);

    loadSMARTS(scanner);
}

void RingoSubstructure::loadSMARTS(const char* str)
{
    BufferScanner scanner(str);

    loadSMARTS(scanner);
}

void RingoSubstructure::_initQuery(QueryReaction& query_in, QueryReaction& query_out)
{
    query_out.makeTransposedForSubstructure(query_in);

    ReactionAutomapper ram(query_out);
    ram.correctReactingCenters(true);

    query_out.aromatize(AromaticityOptions::BASIC);

    _nei_query_counters.calculate(query_out);
}

void RingoSubstructure::_initSmartsQuery(QueryReaction& query_in, QueryReaction& query_out)
{
    query_out.makeTransposedForSubstructure(query_in);

    ReactionAutomapper ram(query_out);
    ram.correctReactingCenters(true);

    _nei_query_counters.calculate(query_out);
}

void RingoSubstructure::_validateQueryData()
{
    if (_query_data_valid)
        return;

    ReactionFingerprintBuilder builder(_query_reaction, _context.fp_parameters);

    builder.query = true;
    builder.skip_sim = true;
    builder.process();

    _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSizeExtOrdSim() * 2);

    _query_data_valid = true;
}

void RingoSubstructure::_initTarget(bool from_database)
{
    if (preserve_bonds_on_highlighting)
        Reaction::saveBondOrders(_target_reaction, _target_bond_types);

    if (!from_database)
    {
        ReactionAutomapper ram(_target_reaction);
        ram.correctReactingCenters(true);
        _target_reaction.aromatize(AromaticityOptions::BASIC);
    }
    _nei_target_counters.calculate(_target_reaction);
}

bool RingoSubstructure::matchBinary(const std::string& buf)
{
    BufferScanner scanner(buf);

    return matchBinary(scanner);
}

bool RingoSubstructure::matchBinary(Scanner& scanner)
{
    CrfLoader loader(_context.cmf_dict, scanner);

    loader.loadReaction(_target_reaction);
    _initTarget(true);

    ReactionSubstructureMatcher rsm(_target_reaction);
    rsm.setQuery(_query_reaction);
    rsm.highlight = true;
    if (_smarts)
        rsm.use_daylight_aam_mode = true;
    // rsm.setNeiCounters(&_nei_query_counters, &_nei_target_counters);

    return rsm.find();
}

void RingoSubstructure::loadTarget(const std::string& buf)
{
    BufferScanner scanner(buf);

    return loadTarget(scanner);
}

void RingoSubstructure::loadTarget(const char* str)
{
    BufferScanner scanner(str);

    return loadTarget(scanner);
}

void RingoSubstructure::loadTarget(Scanner& scanner)
{
    ReactionAutoLoader loader(scanner);

    _context.setLoaderSettings(loader);
    loader.loadReaction(_target_reaction);
    _initTarget(false);
}

bool RingoSubstructure::matchLoadedTarget()
{
    ReactionSubstructureMatcher rsm(_target_reaction);

    rsm.highlight = true;

    rsm.setQuery(_query_reaction);
    // rsm.setNeiCounters(&_nei_query_counters, &_nei_target_counters);
    if (_smarts)
        rsm.use_daylight_aam_mode = true;

    return rsm.find();
}

void RingoSubstructure::getHighlightedTarget(std::string& buf)
{
    StringOutput output(buf);
    RxnfileSaver saver(output);

    if (!Reaction::haveCoord(_target_reaction))
    {
        profTimerStart(t, "match.layout");
        ReactionLayout layout(_target_reaction);

        layout.make();
        _target_reaction.markStereocenterBonds();
    }

    if (preserve_bonds_on_highlighting)
        Reaction::loadBondOrders(_target_reaction, _target_bond_types);

    saver.saveReaction(_target_reaction);
}

const byte* RingoSubstructure::getQueryFingerprint()
{
    _validateQueryData();
    return _query_fp.ptr();
}
