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

#include "mango_matchers.h"

#include "base_cpp/crc32.h"
#include "base_cpp/scanner.h"
#include "bingo_context.h"
#include "bingo_error.h"
#include "mango_index.h"
#include "graph/filter.h"
#include "graph/subgraph_hash.h"
#include "molecule/cmf_loader.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_substructure_matcher.h"

MangoExact::MangoExact(BingoContext& context) : _context(context)
{
    _flags = 0;
    _rms_threshold = 0;
}

void MangoExact::loadQuery(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);

    _context.setLoaderSettings(loader);
    loader.loadMolecule(_query);
    Molecule::checkForConsistency(_query);

    _initQuery(_query);

    calculateHash(_query, _query_hash);
}

void MangoExact::calculateHash(Molecule& mol, Hash& hash)
{
    hash.clear();

    QS_DEF(Molecule, mol_without_h);
    QS_DEF(ArrayInt, vertices);
    int i;

    vertices.clear();

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.getAtomNumber(i) != ELEM_H)
            vertices.push(i);

    mol_without_h.makeSubmolecule(mol, vertices, 0);

    // Decompose into connected components
    int n_comp = mol_without_h.countComponents();
    QS_DEF(Molecule, component);
    QS_DEF(ArrayInt, vertex_codes);

    for (int i = 0; i < n_comp; i++)
    {
        Filter filter(mol_without_h.getDecomposition().ptr(), Filter::EQ, i);
        component.makeSubmolecule(mol_without_h, filter, 0, 0);

        SubgraphHash hh(component);

        vertex_codes.clear_resize(component.vertexEnd());
        for (int v = component.vertexBegin(); v != component.vertexEnd(); v = component.vertexNext(v))
            vertex_codes[v] = component.atomCode(v);
        hh.vertex_codes = &vertex_codes;
        hh.max_iterations = (component.edgeCount() + 1) / 2;

        dword component_hash = hh.getHash();

        // Find component hash in all hashes
        bool found = false;

        for (int j = 0; j < hash.size(); j++)
            if (hash[j].hash == component_hash)
            {
                hash[j].count++;
                found = true;
                break;
            }

        if (!found)
        {
            HashElement& hash_element = hash.push();
            hash_element.count = 1;
            hash_element.hash = component_hash;
        }
    }
}

void MangoExact::loadQuery(const ArrayChar& buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

void MangoExact::loadQuery(const char* buf)
{
    BufferScanner scanner(buf);

    loadQuery(scanner);
}

const MangoExact::Hash& MangoExact::getQueryHash() const
{
    return _query_hash;
}

void MangoExact::setParameters(const char* conditions)
{
    MoleculeExactMatcher::parseConditions(conditions, _flags, _rms_threshold);
}

void MangoExact::loadTarget(Scanner& scanner)
{
    MoleculeAutoLoader loader(scanner);
    _context.setLoaderSettings(loader);
    loader.loadMolecule(_target);
    Molecule::checkForConsistency(_target);
    _initTarget(_target, false);
}

void MangoExact::loadTarget(const ArrayChar& target_buf)
{
    BufferScanner scanner(target_buf);

    loadTarget(scanner);
}

void MangoExact::loadTarget(const char* target)
{
    BufferScanner scanner(target);

    loadTarget(scanner);
}

bool MangoExact::matchLoadedTarget()
{
    MoleculeExactMatcher matcher(_query, _target);

    matcher.flags = _flags;
    matcher.rms_threshold = _rms_threshold;

    return matcher.find();
}

void MangoExact::_initQuery(Molecule& query)
{
    int i;
    MoleculeAromatizer::aromatizeBonds(query, AromaticityOptions::BASIC);

    if (_flags & MoleculeExactMatcher::CONDITION_STEREO)
    {
        for (i = query.edgeBegin(); i != query.edgeEnd(); i = query.edgeNext(i))
            if (query.getEdgeTopology(i) == TOPOLOGY_RING)
                query.cis_trans.setParity(i, 0);
    }
}

void MangoExact::_initTarget(Molecule& target, bool from_database)
{
    if (!from_database)
        MoleculeAromatizer::aromatizeBonds(target, AromaticityOptions::BASIC);
}

bool MangoExact::matchBinary(Scanner& scanner, Scanner* xyz_scanner)
{
    CmfLoader loader(_context.cmf_dict, scanner);

    loader.loadMolecule(_target);
    if (xyz_scanner != 0)
        loader.loadXyz(*xyz_scanner);

    _initTarget(_target, true);
    /*
     * Set up timeout for matching
     */

    MoleculeExactMatcher matcher(_query, _target);

    matcher.flags = _flags;
    matcher.rms_threshold = _rms_threshold;

    return matcher.find();
}

bool MangoExact::matchBinary(const ArrayChar& target_buf, const ArrayChar* xyz_buf)
{
    BufferScanner scanner(target_buf);

    if (xyz_buf == 0)
        return matchBinary(scanner, 0);

    BufferScanner xyz_scanner(*xyz_buf);
    return matchBinary(scanner, &xyz_scanner);
}

bool MangoExact::needCoords() const
{
    return (_flags & MoleculeExactMatcher::CONDITION_3D) != 0;
}

bool MangoExact::needComponentMatching() const
{
    return (_flags & MoleculeExactMatcher::CONDITION_FRAGMENTS) == 0;
}

bool MangoExact::parse(const char* params)
{
    if (params == 0)
    {
        setParameters("");
        return true;
    }

    QS_DEF(ArrayChar, params_upper);

    params_upper.upper(params);

    if (strstr(params_upper.ptr(), "TAU") != NULL)
        return false;

    setParameters(params);
    return true;
}
