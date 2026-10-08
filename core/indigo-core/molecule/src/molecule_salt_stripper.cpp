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

#include "molecule/molecule_salt_stripper.h"

#include <vector>

#include "base_cpp/scanner.h"
#include "graph/filter.h"
#include "graph/graph_decomposer.h"
#include "molecule/molecule.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

namespace
{
    // Every atom of a pattern has all its bonds counted by X, so a match is a whole
    // connected piece of the graph, never a part of a larger molecule.
    const char* const SALT_PATTERNS[] = {
        // inorganics with quaternary atom
        "[X4&!#1&!#6](~[X1&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
        "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
        "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
        "[X4&!#1&!#6](~[X2H&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
        // inorganics with tertiary atom
        "[X3&!#1&!#6](~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
        "[X3&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
        "[X3&!#1&!#6](~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
        // inorganics with secondary atom
        "[X2&!#1&!#6](~[X1&!#6])~[X1&!#6]",
        "[X2&!#1&!#6](~[X2H&!#6])~[X1&!#6]",
        // inorganics with primary atom
        "[X1&!#1&!#6]~[X1&!#6]",
        // single-element ions
        "[X0+1]",
        "[X0+2]",
        "[X0+3]",
        "[X0+4]",
        "[X0-1]",
        "[X0-2]",
    };
} // namespace

MoleculeSaltStripper::MoleculeSaltStripper(const AromaticityOptions& arom_options) : _arom_options(arom_options)
{
    for (const char* smarts : SALT_PATTERNS)
    {
        Pattern& pattern = _patterns.emplace();
        BufferScanner scanner(smarts);
        SmilesLoader loader(scanner);
        loader.loadSMARTS(pattern.query);
        pattern.counters.calculate(pattern.query);
    }
}

bool MoleculeSaltStripper::hasSalts(Molecule& mol)
{
    const int component_count = mol.moleculeComponents().getComponentsCount();
    for (int component = 0; component < component_count; component++)
        if (_isSalt(mol, component))
            return true;
    return false;
}

bool MoleculeSaltStripper::strip(Molecule& mol)
{
    const int component_count = mol.moleculeComponents().getComponentsCount();
    std::vector<bool> is_salt(component_count);
    for (int component = 0; component < component_count; component++)
        is_salt[component] = _isSalt(mol, component);

    Array<int> salt_atoms;
    const GraphDecomposer& components = mol.moleculeComponents();
    for (int atom : mol.vertices())
        if (is_salt[components.getComponent(atom)])
            salt_atoms.push(atom);

    mol.removeAtoms(salt_atoms);
    return salt_atoms.size() > 0;
}

bool MoleculeSaltStripper::_isSalt(Molecule& mol, int component)
{
    Filter filter(mol.moleculeComponents().getDecomposition().ptr(), Filter::EQ, component);
    Molecule fragment;
    fragment.makeSubmolecule(mol, filter, nullptr, nullptr);
    if (!fragment.haptic_bonds.isEmpty())
        return false;

    if (!fragment.isAromatized())
        fragment.aromatize(_arom_options);
    MoleculeAtomNeighbourhoodCounters fragment_counters;
    fragment_counters.calculate(fragment);

    for (int i = 0; i < _patterns.size(); i++)
    {
        MoleculeSubstructureMatcher matcher(fragment);
        matcher.setQuery(_patterns[i].query);
        matcher.setNeiCounters(&_patterns[i].counters, &fragment_counters);
        matcher.arom_options = _arom_options;
        if (matcher.find())
            return true;
    }
    return false;
}
