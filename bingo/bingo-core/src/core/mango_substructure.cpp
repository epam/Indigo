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

#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "bingo_error.h"
#include "mango_index.h"
#include "mango_matchers.h"
#include "layout/molecule_layout.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"

#include "base_cpp/profiling.h"
#include "base_cpp/ptr_pool.h"

IMPL_ERROR(MangoSubstructure, "substructure");

MangoSubstructure::MangoSubstructure(BingoContext& context) : _context(context)
{
    match_3d = 0;
    rms_threshold = 0;
    preserve_bonds_on_highlighting = false;
    _use_pi_systems_matcher = false;
}

void MangoSubstructure::loadQuery(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);
    QS_DEF(QueryMolecule, source);

    _context.setLoaderSettings(loader);
    loader.loadQueryMolecule(source);

    if (!source.have_xyz && match_3d != 0)
        throw Error("cannot do 3D match without XYZ in the query");

    _initQuery(source, _query);
    _query_fp_valid = false;
    _query_extra_valid = false;
}

void MangoSubstructure::loadSMARTS(Scanner& scanner)
{
    SmilesLoader loader(scanner);
    QS_DEF(QueryMolecule, source);

    _context.setLoaderSettings(loader);
    loader.loadSMARTS(source);

    if (!source.have_xyz && match_3d != 0)
        throw Error("cannot do 3D match without XYZ in the query");

    _initSmartsQuery(source, _query);
    _query_fp_valid = false;
    _query_extra_valid = false;
    _use_pi_systems_matcher = false;
}

void MangoSubstructure::loadQuery(const Array<char>& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void MangoSubstructure::loadQuery(const char* str)
{
    BufferScanner scanner(str);

    loadQuery(scanner);
}

void MangoSubstructure::loadSMARTS(const Array<char>& buf)
{
    BufferScanner scanner(buf);

    loadSMARTS(scanner);
}

void MangoSubstructure::loadSMARTS(const char* str)
{
    BufferScanner scanner(str);

    loadSMARTS(scanner);
}

void MangoSubstructure::_validateQueryFP()
{
    if (_query_fp_valid)
        return;

    MoleculeFingerprintBuilder builder(_query, _context.fp_parameters);

    builder.query = true;
    builder.skip_sim = true;
    builder.skip_tau = true;

    // atom charges and bond types may not match in pi-systems
    if (_use_pi_systems_matcher)
    {
        builder.skip_ord = true;
        builder.skip_any_atoms = true;
        builder.skip_ext_charge = true;
    }

    builder.process();
    _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSize());

    _query_fp_valid = true;
}

void MangoSubstructure::_validateQueryExtraData()
{
    if (_query_extra_valid)
        return;

    _query_has_stereocenters = _query.stereocenters.size() > 0;
    _query_has_stereocare_bonds = _query.cis_trans.count() > 0;
    _query_extra_valid = true;
}

void MangoSubstructure::loadTarget(const Array<char>& molfile_buf)
{
    BufferScanner scanner(molfile_buf);

    loadTarget(scanner);
}

void MangoSubstructure::loadTarget(const char* target)
{
    BufferScanner scanner(target);

    loadTarget(scanner);
}

void MangoSubstructure::loadTarget(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadMolecule(_target);
    _initTarget(false);
    Molecule::checkForConsistency(_target);
}

bool MangoSubstructure::matchLoadedTarget()
{
    MoleculeSubstructureMatcher matcher(_target);

    matcher.match_3d = match_3d;
    matcher.rms_threshold = rms_threshold;
    matcher.highlight = true;
    matcher.use_pi_systems_matcher = _use_pi_systems_matcher;
    matcher.setNeiCounters(&_nei_query_counters, &_nei_target_counters);
    matcher.fmcache = &_fmcache;

    _fmcache.clear();

    matcher.setQuery(_query);

    profTimerStart(temb, "match.embedding");
    bool res = matcher.find();
    profTimerStop(temb);

    if (res)
    {
        profIncTimer("match.embedding_found", profTimerGetTime(temb));
    }
    else
    {
        profIncTimer("match.embedding_not_found", profTimerGetTime(temb));
    }
    return res;
}

void MangoSubstructure::loadBinaryTargetXyz(Scanner& xyz_scanner)
{
    cmf_loader->loadXyz(xyz_scanner);
}

void MangoSubstructure::getHighlightedTarget(Array<char>& molfile_buf)
{
    ArrayOutput output(molfile_buf);
    MolfileSaver saver(output);

    if (!_target.have_xyz)
    {
        MoleculeLayout ml(_target);
        ml.make();
        _target.clearBondDirections();
        _target.markBondsStereocenters();
        _target.markBondsAlleneStereo();
    }

    if (preserve_bonds_on_highlighting)
        Molecule::loadBondOrders(_target, _target_bond_types);

    saver.saveMolecule(_target);
}

void MangoSubstructure::getHighlightedTarget_Smiles(Array<char>& smiles_buf)
{
    ArrayOutput output(smiles_buf);
    SmilesSaver saver(output);

    if (preserve_bonds_on_highlighting)
        Molecule::loadBondOrders(_target, _target_bond_types);

    saver.saveMolecule(_target);
}

void MangoSubstructure::_correctQueryStereo(QueryMolecule& query)
{
    // Remove stereobond marks that are connected with R-groups
    for (int v = query.vertexBegin(); v != query.vertexEnd(); v = query.vertexNext(v))
    {
        if (!query.isRSite(v))
            continue;
        const Vertex& vertex = query.getVertex(v);
        for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
        {
            int edge = vertex.neiEdge(nei);
            if (query.cis_trans.getParity(edge) != 0)
                query.cis_trans.setParity(edge, 0);
        }
    }

    MoleculeRGroups& rgroups = query.rgroups;
    int n_rgroups = rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = rgroups.getRGroup(i).fragments;
        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            QueryMolecule& fragment = frags[j]->asQueryMolecule();
            _correctQueryStereo(fragment);
        }
    }
}

void MangoSubstructure::_initQuery(QueryMolecule& query_in, QueryMolecule& query_out)
{
    _correctQueryStereo(query_in);

    QueryMoleculeAromatizer::aromatizeBonds(query_in, AromaticityOptions::BASIC);
    _nei_query_counters.calculate(query_in);

    QS_DEF(Array<int>, transposition);

    _nei_query_counters.makeTranspositionForSubstructure(query_in, transposition);

    query_out.makeSubmolecule(query_in, transposition, 0);
    _nei_query_counters.calculate(query_out);
}

void MangoSubstructure::_initSmartsQuery(QueryMolecule& query_in, QueryMolecule& query_out)
{
    QS_DEF(Array<int>, transposition);

    MoleculeSubstructureMatcher::makeTransposition(query_in, transposition);
    query_out.makeSubmolecule(query_in, transposition, 0);
    _nei_query_counters.calculate(query_out);
    query_out.optimize();
}

void MangoSubstructure::_initTarget(bool from_database)
{
    if (preserve_bonds_on_highlighting)
        Molecule::saveBondOrders(_target, _target_bond_types);

    if (!from_database)
        MoleculeAromatizer::aromatizeBonds(_target, AromaticityOptions::BASIC);

    _nei_target_counters.calculate(_target);
}

bool MangoSubstructure::needCoords()
{
    return MoleculeSubstructureMatcher::needCoords(match_3d, _query);
}

bool MangoSubstructure::matchBinary(const Array<char>& target_buf, const Array<char>* xyz_buf)
{
    BufferScanner scanner(target_buf);

    if (xyz_buf == 0)
        return matchBinary(scanner, 0);

    BufferScanner xyz_scanner(*xyz_buf);

    return matchBinary(scanner, &xyz_scanner);
}

bool MangoSubstructure::matchBinary(Scanner& scanner, Scanner* xyz_scanner)
{
    _validateQueryExtraData();

    profTimerStart(tcmf, "match.cmf");

    cmf_loader.free();
    cmf_loader.create(_context.cmf_dict, scanner);

    if (!_query_has_stereocare_bonds)
        cmf_loader->skip_cistrans = true;
    if (!_query_has_stereocenters)
        cmf_loader->skip_stereocenters = true;

    cmf_loader->loadMolecule(_target);
    if (xyz_scanner != 0)
        cmf_loader->loadXyz(*xyz_scanner);

    profTimerStop(tcmf);

    profTimerStart(tinit, "match.init_target");
    _initTarget(true);
    profTimerStop(tinit);

    return matchLoadedTarget();
}

bool MangoSubstructure::parse(const char* params)
{
    match_3d = 0;
    rms_threshold = 0;
    _use_pi_systems_matcher = false;
    preserve_bonds_on_highlighting = false;

    if (params == 0)
        return true;

    BufferScanner scanner(params);

    QS_DEF(Array<char>, word);

    scanner.skipSpace();
    while (!scanner.isEOF())
    {
        scanner.skipSpace();
        scanner.readWord(word, 0);

        bool is_aff = strcasecmp(word.ptr(), "AFF") == 0;
        bool is_conf = strcasecmp(word.ptr(), "CONF") == 0;
        if (is_aff || is_conf)
        {
            if (match_3d != 0)
                return false;
            if (is_aff)
                match_3d = MoleculeSubstructureMatcher::AFFINE;
            if (is_conf)
                match_3d = MoleculeSubstructureMatcher::CONFORMATION;

            scanner.skipSpace();
            rms_threshold = scanner.readDouble();
        }
        else if (strcasecmp(word.ptr(), "RES") == 0)
            _use_pi_systems_matcher = true;
        else
            return false;
    }

    return true;
}

const byte* MangoSubstructure::getQueryFingerprint()
{
    _validateQueryFP();
    return _query_fp.ptr();
}
