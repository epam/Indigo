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
#include "bingo_error.h"
#include "mango_matchers.h"
#include "layout/molecule_layout.h"
#include "molecule/cmf_loader.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_tautomer_matcher.h"
#include "molecule/molfile_saver.h"

IMPL_ERROR(MangoTautomer, "tautomer matcher");

MangoTautomer::MangoTautomer(BingoContext& context) : _context(context)
{
    preserve_bonds_on_highlighting = false;
}

void MangoTautomer::loadQuery(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);

    if (_params.substructure)
    {
        _query.reset(new QueryMolecule());
        loader.loadQueryMolecule((QueryMolecule&)_query.ref());
    }
    else
    {
        _query.reset(new Molecule());
        loader.loadMolecule((Molecule&)_query.ref());
    }

    _query_data_valid = false;
}

void MangoTautomer::loadQuery(const ArrayChar& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void MangoTautomer::loadQuery(const char* str)
{
    BufferScanner scanner(str);

    loadQuery(scanner);
}

const char* MangoTautomer::getQueryGross()
{
    _validateQueryData();

    return _query_gross_str.ptr();
}

void MangoTautomer::setParams(int conditions, bool force_hydrogens, bool ring_chain, bool substructure)
{
    _params.conditions = conditions;
    _params.force_hydrogens = force_hydrogens;
    _params.ring_chain = ring_chain;
    _params.substructure = substructure;
}

void MangoTautomer::setParameters(const char* conditions)
{
    TautomerMethod m = RSMARTS;
    MoleculeTautomerMatcher::parseConditions(conditions, _params.conditions, _params.force_hydrogens, _params.ring_chain, m);
}

void MangoTautomer::_validateQueryData()
{
    if (_query_data_valid)
        return;

    if (_params.substructure)
    {
        QS_DEF(QueryMolecule, aromatized_query);

        aromatized_query.clone(_query.ref(), 0, 0);
        QueryMoleculeAromatizer::aromatizeBonds(aromatized_query, AromaticityOptions::BASIC);

        MoleculeFingerprintBuilder builder(aromatized_query, _context.fp_parameters);
        builder.query = true;
        builder.skip_ord = true;
        builder.skip_sim = true;

        // Tautomer fingerprint part does already contain all necessary any-bits
        builder.skip_any_atoms = true;
        builder.skip_any_bonds = true;
        builder.skip_any_atoms_bonds = true;

        builder.process();
        _query_fp.copy(builder.get(), _context.fp_parameters.fingerprintSize());
    }
    else
    {
        QS_DEF(ArrayInt, gross);

        MoleculeGrossFormula::collect(_query.ref(), gross);
        gross[ELEM_H] = 0;
        MoleculeGrossFormula::toString(gross, _query_gross_str);
    }
    _query_data_valid = true;
}

void MangoTautomer::loadTarget(const char* target)
{
    BufferScanner scanner(target);

    loadTarget(scanner);
}

void MangoTautomer::loadTarget(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadMolecule(_target);

    _initTarget(false);
}

void MangoTautomer::loadTarget(const ArrayChar& molfile_buf)
{
    BufferScanner scanner(molfile_buf);

    loadTarget(scanner);
}

bool MangoTautomer::matchLoadedTarget()
{
    MoleculeTautomerMatcher matcher(_target, _params.substructure);
    TautomerMethod m = RSMARTS;

    matcher.setRulesList(&_context.tautomer_rules);
    matcher.setRules(_params.conditions, _params.force_hydrogens, _params.ring_chain, m);
    matcher.setQuery(_query.ref());
    matcher.highlight = true;

    return matcher.find();
}

void MangoTautomer::getHighlightedTarget(ArrayChar& molfile_buf)
{
    ArrayOutput output(molfile_buf);
    MolfileSaver saver(output);

    if (!_target.have_xyz)
    {
        MoleculeLayout ml(_target);
        ml.make();
        _target.clearBondDirections();
        _target.stereocenters.markBonds();
        _target.allene_stereo.markBonds();
    }

    if (preserve_bonds_on_highlighting)
        Molecule::loadBondOrders(_target, _target_bond_types);

    saver.saveMolecule(_target);
}

void MangoTautomer::_initTarget(bool from_database)
{
    if (preserve_bonds_on_highlighting)
        Molecule::saveBondOrders(_target, _target_bond_types);

    if (!from_database)
        MoleculeAromatizer::aromatizeBonds(_target, AromaticityOptions::BASIC);
}

bool MangoTautomer::matchBinary(const ArrayChar& target_buf)
{
    BufferScanner scanner(target_buf);

    return matchBinary(scanner);
}

bool MangoTautomer::matchBinary(Scanner& scanner)
{
    CmfLoader loader(_context.cmf_dict, scanner);

    loader.loadMolecule(_target);
    _initTarget(true);
    TautomerMethod m = RSMARTS;

    MoleculeTautomerMatcher matcher(_target, _params.substructure);

    matcher.setRulesList(&_context.tautomer_rules);
    matcher.setRules(_params.conditions, _params.force_hydrogens, _params.ring_chain, m);
    matcher.setQuery(_query.ref());

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

bool MangoTautomer::parseSub(const char* params)
{
    if (params == 0)
        return false;

    preserve_bonds_on_highlighting = false;

    BufferScanner scanner(params);

    scanner.skipSpace();

    if (scanner.isEOF())
        return false;

    QS_DEF(ArrayChar, word);
    scanner.readWord(word, 0);

    if (strcasecmp(word.ptr(), "TAU") != 0)
        return false;

    setParameters(params);
    _params.substructure = true;

    return true;
}

bool MangoTautomer::parseExact(const char* params)
{
    if (params == 0)
        return false;

    preserve_bonds_on_highlighting = false;

    BufferScanner scanner(params);

    scanner.skipSpace();

    QS_DEF(ArrayChar, word);

    scanner.readWord(word, 0);

    if (strcasecmp(word.ptr(), "TAU") != 0)
        return false;

    setParameters(params);
    _params.substructure = false;

    return true;
}

const byte* MangoTautomer::getQueryFingerprint()
{
    _validateQueryData();
    return _query_fp.ptr();
}
