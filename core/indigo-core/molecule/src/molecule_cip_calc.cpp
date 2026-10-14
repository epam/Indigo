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

#include "molecule/molecule_cip_calc.h"

#include <algorithm>
#include <queue>
#include <set>

#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"

using namespace indigo;

// CIP rule 1, part 1: Higher atomic number precedes lower.
// If the substituent is an electron pair (see req. 3.5. & 3.7.), its priority will always be lowest -- AN=0.
constexpr int LONE_PAIR = 0;
constexpr int PYRAMID_SIZE = 4;
constexpr double TWO_PI = 2.0 * M_PI;

bool MoleculeCIPCalc::calcCIPLabels(BaseMolecule& mol)
{

    std::map<int, CIPDesc> atom_labels;
    std::map<int, CIPDesc> bond_labels;

    std::unique_ptr<BaseMolecule> unfolded_h_mol;
    Array<int> markers;
    if (mol.isQueryMolecule())
        unfolded_h_mol = std::make_unique<QueryMolecule>();
    else
        unfolded_h_mol = std::make_unique<Molecule>();

    unfolded_h_mol->clone_KeepIndices(mol);
    AromaticityOptions arom_opts;
    unfolded_h_mol->dearomatize(arom_opts);
    if (!unfolded_h_mol->isQueryMolecule()) // queries are already unfolded
        unfolded_h_mol->unfoldHydrogens(&markers, -1, true);

    std::vector<CandidateAtom> atom_candidates;
    if (mol.have_xyz)
        _collectAtomCandidates(mol, *unfolded_h_mol, markers, atom_candidates);
    else
        _atomCandidatesFromStereocenters(*unfolded_h_mol, markers, atom_candidates);

    std::set<int> digraph_seeded_stereocenters;
    std::set<int> fixed_digraph_stereocenters;
    for (const auto& candidate : atom_candidates)
    {
        const bool center_in_ring = unfolded_h_mol->vertexInRing(candidate.atom_idx);
        bool has_ring_ligand = center_in_ring;
        bool all_heavy_ligands_in_ring = true;
        bool has_heavy_ligand = false;
        bool has_multiple_bond_ligand = false;
        for (int neighbor : candidate.neighbors)
        {
            has_ring_ligand = has_ring_ligand || unfolded_h_mol->vertexInRing(neighbor);
            if (unfolded_h_mol->getAtomNumber(neighbor) != ELEM_H && markers[neighbor] != 1)
            {
                has_heavy_ligand = true;
                all_heavy_ligands_in_ring = all_heavy_ligands_in_ring && unfolded_h_mol->vertexInRing(neighbor);
            }

            const Vertex& ligand = unfolded_h_mol->getVertex(neighbor);
            for (int i = ligand.neiBegin(); i != ligand.neiEnd(); i = ligand.neiNext(i))
            {
                if (ligand.neiVertex(i) != candidate.atom_idx && unfolded_h_mol->getBondOrder(ligand.neiEdge(i)) > BOND_SINGLE)
                {
                    has_multiple_bond_ligand = true;
                    break;
                }
            }
        }

        ComparisonContext context;
        bool has_tied_ligands = false;
        for (size_t i = 0; i < candidate.neighbors.size() && !has_tied_ligands; ++i)
            for (size_t j = i + 1; j < candidate.neighbors.size(); ++j)
                if (_compareCIPPriority(*unfolded_h_mol, candidate.atom_idx, candidate.neighbors[i], candidate.neighbors[j], atom_labels, bond_labels,
                                        &context) == 0)
                {
                    has_tied_ligands = true;
                    break;
                }
        const bool coordinate_defined_ring_ligands = mol.have_xyz && has_heavy_ligand && all_heavy_ligands_in_ring;
        if (has_ring_ligand || has_tied_ligands)
            digraph_seeded_stereocenters.insert(candidate.atom_idx);
        if (has_tied_ligands || (has_ring_ligand && !center_in_ring) || (center_in_ring && (coordinate_defined_ring_ligands || has_multiple_bond_ligand)))
            fixed_digraph_stereocenters.insert(candidate.atom_idx);
    }

    std::map<int, CIPDesc> digraph_atom_labels;
    if (!digraph_seeded_stereocenters.empty() && !mol.isQueryMolecule())
    {
        // Seed ring-related and tied ligand comparisons from the established stereochemical digraph.
        std::unique_ptr<BaseMolecule> stereo_seed_mol = std::make_unique<Molecule>();
        stereo_seed_mol->clone_KeepIndices(mol);

        MoleculeCIPCalculator legacy_calculator;
        legacy_calculator.addCIPStereoDescriptors(*stereo_seed_mol);

        for (int atom_idx = stereo_seed_mol->vertexBegin(); atom_idx != stereo_seed_mol->vertexEnd(); atom_idx = stereo_seed_mol->vertexNext(atom_idx))
        {
            const CIPDesc descriptor = stereo_seed_mol->getAtomCIP(atom_idx);
            if (descriptor != CIPDesc::NONE && descriptor != CIPDesc::UNKNOWN)
                atom_labels[atom_idx] = descriptor;
            if (fixed_digraph_stereocenters.count(atom_idx) != 0 && descriptor != CIPDesc::NONE && descriptor != CIPDesc::UNKNOWN)
                digraph_atom_labels[atom_idx] = descriptor;
        }
        for (int bond_idx = stereo_seed_mol->edgeBegin(); bond_idx != stereo_seed_mol->edgeEnd(); bond_idx = stereo_seed_mol->edgeNext(bond_idx))
        {
            const CIPDesc descriptor = stereo_seed_mol->getBondCIP(bond_idx);
            if (descriptor != CIPDesc::NONE && descriptor != CIPDesc::UNKNOWN)
                bond_labels[bond_idx] = descriptor;
        }
    }

    std::map<int, CIPDesc> fixed_digraph_atom_labels;
    for (const auto& pair : digraph_atom_labels)
        fixed_digraph_atom_labels[pair.first] = pair.second;

    // Iterative refinement: loop until no more changes occur
    // const int max_iterations = 10;
    // int iteration = 0;
    bool changed = true;

    // while (changed && iteration < max_iterations)
    while (changed)
    {
        std::map<int, CIPDesc> prev_atom_labels = atom_labels;
        std::map<int, CIPDesc> prev_bond_labels = bond_labels;

        // Calculate bond labels first (needed for Rule 3)
        _calcBondLabels(*unfolded_h_mol, atom_labels, bond_labels, markers, mol);

        // Calculate atom labels (R/S)
        _calcAtomLabels(*unfolded_h_mol, atom_labels, bond_labels, atom_candidates);
        for (const auto& pair : fixed_digraph_atom_labels)
            atom_labels[pair.first] = pair.second;

        // Check if anything changed
        changed = (atom_labels != prev_atom_labels) || (bond_labels != prev_bond_labels);
        // iteration++;
    }

    // Store calculated labels in the molecule
    mol.clearCIP();

    for (const auto& pair : atom_labels)
    {
        if (pair.second != CIPDesc::NONE && pair.second != CIPDesc::UNKNOWN)
        {
            mol.setAtomCIP(pair.first, pair.second);
        }
    }

    for (const auto& pair : bond_labels)
    {
        if (pair.second != CIPDesc::NONE && pair.second != CIPDesc::UNKNOWN)
        {
            mol.setBondCIP(pair.first, pair.second);
        }
    }

    return atom_labels.size() > 0 || bond_labels.size() > 0;
}

bool MoleculeCIPCalc::_isAppropriateAtom(BaseMolecule& mol, int atom_idx, int& isoelectronic_equivalent)
{
    int atom_number = mol.getAtomNumber(atom_idx);
    if (atom_number <= 0)
        return false;

    // Appropriate atoms according to issue #3853:
    // C, N, Si, P, S, As, Se, Te and their isoelectronic equivalents
    // Isoelectronic equivalents are calculated as atomic number minus charge
    int charge = mol.getAtomCharge(atom_idx);
    isoelectronic_equivalent = atom_number - charge;

    // Check if the effective atomic number corresponds to appropriate atoms
    switch (isoelectronic_equivalent)
    {
        // 2.2 Nitrogen (N) and its isoelectric equivalents are (potential) chiral centers only if they have three single bonds, with every bond
        // participating in a ring, and no more than two of those bonds are connecting a nitrogen to an sp2 atoms.
        // sp2 atoms have only two single covalent bonds (standard single, up, down, or either) and one double covalent bond
    case ELEM_N: // Nitrogen
    {
        auto& vertex = mol.getVertex(atom_idx);
        int sp2_count = 0;
        if (vertex.degree() != 3)
            return false;
        std::vector<int> neighbors;
        for (int i : vertex.neighbors())
        {
            if (mol.getEdgeTopology(vertex.neiEdge(i)) != TOPOLOGY_RING)
                return false;
            neighbors.emplace_back(vertex.neiVertex(i));
        }
        for (int i : neighbors)
        {
            auto& neib_vertex = mol.getVertex(i);
            int single_count = 0;
            int double_count = 0;
            int other_count = 0;
            for (int ni : neib_vertex.neighbors())
            {
                int bond_order = mol.getBondOrder(neib_vertex.neiEdge(ni));
                switch (bond_order)
                {
                case BOND_SINGLE:
                    single_count++;
                    break;
                case BOND_DOUBLE:
                    double_count++;
                    break;
                default:
                    other_count++;
                }
            }
            if ((single_count == 1 || single_count == 2) && double_count == 1 && other_count == 0)
                sp2_count++;
        }
        if (sp2_count > 2)
            return false;
        else
            return true;
    }
    break;
    // 2.3. Phosphorus (P), arsenic (As) or their isoelectric equivalents are (potential) chiral centers only if they have three single bonds, with
    // every bond being connected to something other than a hydrogen OR if they have three single and one double bond.
    case ELEM_P:  // Phosphorus
    case ELEM_As: // Arsenic
    {
        auto& vertex = mol.getVertex(atom_idx);
        int single_count = 0;
        int double_count = 0;
        int other_count = 0;
        bool has_single_h_neib = false;
        for (auto i : vertex.neighbors())
        {
            int bond_idx = vertex.neiEdge(i);
            int order = mol.getBondOrder(bond_idx);
            if (order == BOND_SINGLE)
            {
                single_count++;
                if (mol.getAtomNumber(mol.getEdgeEnd(atom_idx, bond_idx)) == ELEM_H)
                    has_single_h_neib = true;
            }
            else if (order == BOND_DOUBLE)
                double_count++;
            else
                other_count++;
        }
        if ((single_count == 3 && double_count == 0 && other_count == 0 && !has_single_h_neib) || (single_count == 2 && double_count == 1 && other_count == 0))
            return true;
        else
            return false;
    }
    break;
    case ELEM_C:  // Carbon
    case ELEM_Si: // Silicon
    case ELEM_S:  // Sulfur
    case ELEM_Se: // Selenium
    case ELEM_Te: // Tellurium
        return true;
    default:
        return false;
    }
}

std::vector<int> MoleculeCIPCalc::_getNeighbors(BaseMolecule& mol, int atom_idx)
{
    std::vector<int> neighbors;
    const Vertex& vertex = mol.getVertex(atom_idx);

    for (auto nei_idx : vertex.neighbors())
    {
        neighbors.push_back(vertex.neiVertex(nei_idx));
    }

    return neighbors;
}

int MoleculeCIPCalc::_compareCIPPriority(BaseMolecule& mol, int atom_idx, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels,
                                         std::map<int, CIPDesc>& bond_labels, ComparisonContext* context)
{
    // Rule 1.1: Higher atomic number precedes lower (immediate neighbors)
    int rule1_1_result = _applyRule1_1(mol, sub1, sub2);
    if (rule1_1_result != 0)
        return rule1_1_result;

    // Rule 1.2: Sphere expansion - compare substituent trees
    ExpandedAtom tree1, tree2;
    int rule1_2_result = _applyRule1_2(mol, atom_idx, sub1, sub2, tree1, tree2);
    if (rule1_2_result != 0)
        return rule1_2_result;

    // Rule 2: Higher mass number precedes lower
    int rule2_result = _applyRule2(tree1, tree2);
    if (rule2_result != 0)
        return rule2_result;

    // Rule 3: Z > E > nonstereogenic
    int rule3_result = _applyRule3(mol, sub1, sub2, bond_labels);
    if (rule3_result != 0)
        return rule3_result;

    // Rule 4.1: Chiral > pseudoasymmetric > nonstereogenic
    int rule4_1_result = _applyRule4_1(mol, sub1, sub2, atom_labels);
    if (rule4_1_result != 0)
        return rule4_1_result;

    // Rule 4.2: Like descriptor pairs rank higher than unlike pairs
    int rule4_2_result = _applyRule4_2(mol, atom_idx, sub1, sub2, atom_labels, bond_labels);
    if (rule4_2_result != 0)
        return rule4_2_result;

    // rule 4.3: r precedes s, and m precedes p.
    int rule4_3_result = _applyRule4_3(mol, sub1, sub2, atom_labels);
    if (rule4_3_result != 0)
        return rule4_3_result;

    // Rule 5: Seqcis precedes seqtrans (matching reference descriptors)
    int rule5_result = _applyRule5(mol, atom_idx, sub1, sub2, atom_labels, bond_labels, context);
    if (rule5_result != 0)
        return rule5_result;

    return 0; // Equal priority
}

int MoleculeCIPCalc::_applyRule1_1(BaseMolecule& mol, int sub1, int sub2)
{
    int num1 = mol.getAtomNumber(sub1);
    int num2 = mol.getAtomNumber(sub2);

    if (num1 > num2)
        return -1; // sub1 has higher priority
    if (num1 < num2)
        return 1; // sub2 has higher priority

    return 0;
}

int MoleculeCIPCalc::_applyRule1_2(BaseMolecule& mol, int center_atom, int sub1, int sub2, ExpandedAtom& tree1, ExpandedAtom& tree2)
{
    std::map<int, int> visited1{{center_atom, 0}};
    std::map<int, int> visited2{{center_atom, 0}};

    _buildExpandedTree(mol, sub1, center_atom, tree1, visited1, 1);
    _buildExpandedTree(mol, sub2, center_atom, tree2, visited2, 1);
    // Compare the trees
    return _compareExpandedTrees(tree1, tree2);
}

void MoleculeCIPCalc::_buildExpandedTree(BaseMolecule& mol, int atom_idx, int from_atom, ExpandedAtom& node, std::map<int, int>& visited, int distance)
{
    struct PendingAtom
    {
        ExpandedAtom* node;
        int atom_idx;
        int from_atom;
        int distance;
    };

    auto initialize_atom = [&](ExpandedAtom& expanded, int atom, int current_distance) {
        expanded.atom_idx = atom;
        expanded.atomic_number = mol.getAtomNumber(atom);
        expanded.mass_number = mol.getAtomIsotope(atom);
        if (expanded.mass_number == 0)
            expanded.mass_number = Element::getDefaultIsotope(expanded.atomic_number);

        auto visited_atom = visited.find(atom);
        if (visited_atom != visited.end())
        {
            expanded.is_duplicate = true;
            expanded.duplicate_distance = visited_atom->second;
            return false;
        }

        visited.emplace(atom, current_distance);
        expanded.is_duplicate = false;
        expanded.duplicate_distance = -1;
        return true;
    };

    if (!initialize_atom(node, atom_idx, distance))
        return;

    std::queue<PendingAtom> pending;
    pending.push({&node, atom_idx, from_atom, distance});
    while (!pending.empty())
    {
        const PendingAtom current = pending.front();
        pending.pop();

        const Vertex& vertex = mol.getVertex(current.atom_idx);
        std::vector<size_t> expandable_children;
        for (int nei_idx = vertex.neiBegin(); nei_idx != vertex.neiEnd(); nei_idx = vertex.neiNext(nei_idx))
        {
            const int neighbor = vertex.neiVertex(nei_idx);
            if (neighbor == current.from_atom)
                continue;

            const int edge_idx = vertex.neiEdge(nei_idx);
            const int bond_order = mol.getBondOrder(edge_idx);
            if (bond_order != BOND_SINGLE && bond_order != BOND_DOUBLE && bond_order != BOND_TRIPLE)
                continue;

            ExpandedAtom real_child;
            real_child.bond_order = bond_order;
            const bool expand_real_child = initialize_atom(real_child, neighbor, current.distance + 1);
            current.node->children.push_back(std::move(real_child));
            if (expand_real_child)
                expandable_children.push_back(current.node->children.size() - 1);

            for (int i = 1; i < bond_order; ++i)
            {
                ExpandedAtom phantom;
                phantom.atom_idx = neighbor;
                phantom.atomic_number = mol.getAtomNumber(neighbor);
                phantom.mass_number = mol.getAtomIsotope(neighbor);
                if (phantom.mass_number == 0)
                    phantom.mass_number = Element::getDefaultIsotope(phantom.atomic_number);
                phantom.is_duplicate = true;
                phantom.duplicate_distance = current.distance + 1;
                current.node->children.push_back(std::move(phantom));
            }
        }

        for (size_t child_index : expandable_children)
            pending.push({&current.node->children[child_index], current.node->children[child_index].atom_idx, current.atom_idx, current.distance + 1});
    }

    auto sort_expanded_tree = [&](auto&& self, ExpandedAtom& expanded) -> void {
        for (ExpandedAtom& child : expanded.children)
            self(self, child);
        std::sort(expanded.children.begin(), expanded.children.end(),
                  [](const ExpandedAtom& first, const ExpandedAtom& second) { return MoleculeCIPCalc::_compareExpandedTrees(first, second) < 0; });
    };
    sort_expanded_tree(sort_expanded_tree, node);
}

int MoleculeCIPCalc::_compareExpandedTrees(const ExpandedAtom& tree1, const ExpandedAtom& tree2)
{
    std::vector<const ExpandedAtom*> sphere1{&tree1};
    std::vector<const ExpandedAtom*> sphere2{&tree2};

    // Rule 1.2 compares all atoms in a sphere before proceeding to the next sphere.
    while (!sphere1.empty() && !sphere2.empty())
    {
        const size_t sphere_size = std::min(sphere1.size(), sphere2.size());
        for (size_t i = 0; i < sphere_size; ++i)
        {
            if (sphere1[i]->atomic_number != sphere2[i]->atomic_number)
                return sphere1[i]->atomic_number > sphere2[i]->atomic_number ? -1 : 1;
            if (sphere1[i]->is_duplicate != sphere2[i]->is_duplicate)
                return sphere1[i]->is_duplicate ? 1 : -1;
            if (sphere1[i]->is_duplicate && sphere1[i]->duplicate_distance != sphere2[i]->duplicate_distance)
                return sphere1[i]->duplicate_distance < sphere2[i]->duplicate_distance ? -1 : 1;
        }
        if (sphere1.size() != sphere2.size())
            return sphere1.size() > sphere2.size() ? -1 : 1;

        std::vector<const ExpandedAtom*> next_sphere1;
        std::vector<const ExpandedAtom*> next_sphere2;
        for (const ExpandedAtom* atom : sphere1)
            for (const ExpandedAtom& child : atom->children)
                next_sphere1.push_back(&child);
        for (const ExpandedAtom* atom : sphere2)
            for (const ExpandedAtom& child : atom->children)
                next_sphere2.push_back(&child);
        sphere1 = std::move(next_sphere1);
        sphere2 = std::move(next_sphere2);
    }

    if (sphere1.size() != sphere2.size())
        return sphere1.size() > sphere2.size() ? -1 : 1;

    return 0;
}

int MoleculeCIPCalc::_applyRule2(const ExpandedAtom& tree1, const ExpandedAtom& tree2)
{
    std::queue<std::pair<const ExpandedAtom*, const ExpandedAtom*>> to_compare;
    to_compare.push({&tree1, &tree2});

    while (!to_compare.empty())
    {
        const auto [first, second] = to_compare.front();
        to_compare.pop();

        if (first->mass_number > second->mass_number)
            return -1;
        if (first->mass_number < second->mass_number)
            return 1;

        const size_t child_count = std::min(first->children.size(), second->children.size());
        for (size_t i = 0; i < child_count; ++i)
            to_compare.push({&first->children[i], &second->children[i]});
    }

    return 0;
}

int MoleculeCIPCalc::_applyRule3(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& bond_labels)
{
    // Find bonds involving these substituents
    CIPDesc label1 = CIPDesc::NONE;
    CIPDesc label2 = CIPDesc::NONE;

    // Check if these atoms are part of double bonds with E/Z descriptors
    for (const auto& pair : bond_labels)
    {
        int bond_idx = pair.first;
        const Edge& edge = mol.getEdge(bond_idx);

        if (edge.beg == sub1 || edge.end == sub1)
            label1 = pair.second;
        if (edge.beg == sub2 || edge.end == sub2)
            label2 = pair.second;
    }

    // Z > E > nonstereogenic
    auto get_priority = [](CIPDesc desc) -> int {
        if (desc == CIPDesc::Z)
            return 3;
        if (desc == CIPDesc::E)
            return 2;
        return 1;
    };

    int priority1 = get_priority(label1);
    int priority2 = get_priority(label2);

    if (priority1 > priority2)
        return -1;
    if (priority1 < priority2)
        return 1;

    return 0;
}

// CIP rule 4, part 1: Chiral (R/S) > pseudoasymmetric (r/s) > nonstereogenic
int MoleculeCIPCalc::_applyRule4_1(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels)
{
    CIPDesc label1 = atom_labels.count(sub1) ? atom_labels[sub1] : CIPDesc::NONE;
    CIPDesc label2 = atom_labels.count(sub2) ? atom_labels[sub2] : CIPDesc::NONE;

    auto get_priority = [](CIPDesc desc) -> int {
        if (desc == CIPDesc::R || desc == CIPDesc::S)
            return 3; // Chiral
        if (desc == CIPDesc::r || desc == CIPDesc::s)
            return 2; // Pseudoasymmetric
        return 1;     // Nonstereogenic
    };

    int priority1 = get_priority(label1);
    int priority2 = get_priority(label2);

    if (priority1 > priority2)
        return -1;
    if (priority1 < priority2)
        return 1;

    return 0;
}

// CIP rule 4, part 2: When two ligands have different descriptor pairs, then the one with the
// first chosen like descriptor pairs has priority over the one with a corresponding unlike pair.
int MoleculeCIPCalc::_applyRule4_2(BaseMolecule& mol, int center_atom, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels,
                                   std::map<int, CIPDesc>& bond_labels)
{
    // Collect all CIP descriptors along each substituent path
    std::vector<CIPDesc> descriptors1;
    std::vector<CIPDesc> descriptors2;

    _collectDescriptorsAlongPath(mol, sub1, center_atom, atom_labels, bond_labels, descriptors1);
    _collectDescriptorsAlongPath(mol, sub2, center_atom, atom_labels, bond_labels, descriptors2);

    // If one or both paths have no descriptors, no comparison possible
    if (descriptors1.empty() || descriptors2.empty())
        return 0;

    auto descriptor_sense = [](CIPDesc descriptor) {
        switch (descriptor)
        {
        case CIPDesc::R:
        case CIPDesc::m:
        case CIPDesc::Z:
            return 1;
        case CIPDesc::S:
        case CIPDesc::p:
        case CIPDesc::E:
            return -1;
        default:
            return 0;
        }
    };

    auto pair_priorities = [&](const std::vector<CIPDesc>& descriptors) {
        std::vector<int> priorities;
        for (size_t i = 0; i < descriptors.size(); ++i)
        {
            const int first_sense = descriptor_sense(descriptors[i]);
            if (first_sense == 0)
                continue;

            for (size_t j = i + 1; j < descriptors.size(); ++j)
            {
                const int second_sense = descriptor_sense(descriptors[j]);
                if (second_sense != 0)
                    priorities.push_back(first_sense == second_sense ? 1 : -1);
            }
        }
        return priorities;
    };

    const std::vector<int> priorities1 = pair_priorities(descriptors1);
    const std::vector<int> priorities2 = pair_priorities(descriptors2);
    const size_t pair_count = std::min(priorities1.size(), priorities2.size());
    for (size_t i = 0; i < pair_count; ++i)
    {
        if (priorities1[i] != priorities2[i])
            return priorities1[i] > priorities2[i] ? -1 : 1;
    }

    return 0;
}

// CIP rule 4, Part 3: r precedes s, and m precedes p.
int MoleculeCIPCalc::_applyRule4_3(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels)
{
    CIPDesc label1 = atom_labels.count(sub1) ? atom_labels[sub1] : CIPDesc::NONE;
    CIPDesc label2 = atom_labels.count(sub2) ? atom_labels[sub2] : CIPDesc::NONE;

    if ((label1 == CIPDesc::r && label2 == CIPDesc::s) || (label1 == CIPDesc::m && label2 == CIPDesc::p))
        return -1;
    else if ((label1 == CIPDesc::s && label2 == CIPDesc::r) || (label1 == CIPDesc::p && label2 == CIPDesc::m))
        return 1;

    return 0;
}

// CIP rule 5: An atom or group with the descriptor R, M and seqCis (Z) has priority over its enantiomorph S, P, or seqTrans (E).
int MoleculeCIPCalc::_applyRule5(BaseMolecule& mol, int center_atom, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels,
                                 std::map<int, CIPDesc>& bond_labels, ComparisonContext* context)
{
    CIPDesc label1 = atom_labels.count(sub1) ? atom_labels[sub1] : CIPDesc::NONE;
    CIPDesc label2 = atom_labels.count(sub2) ? atom_labels[sub2] : CIPDesc::NONE;

    if ((label1 == CIPDesc::R && label2 == CIPDesc::S) || (label1 == CIPDesc::r && label2 == CIPDesc::s) || (label1 == CIPDesc::Z && label2 == CIPDesc::E))
    {
        if (context != nullptr)
            context->rule_5_used = true;
        return -1;
    }
    else if ((label1 == CIPDesc::S && label2 == CIPDesc::R) || (label1 == CIPDesc::s && label2 == CIPDesc::r) || (label1 == CIPDesc::E && label2 == CIPDesc::Z))
    {
        if (context != nullptr)
            context->rule_5_used = true;
        return 1;
    }

    return 0;
}

void MoleculeCIPCalc::_collectDescriptorsAlongPath(BaseMolecule& mol, int start_atom, int from_atom, std::map<int, CIPDesc>& atom_labels,
                                                   std::map<int, CIPDesc>& bond_labels, std::vector<CIPDesc>& descriptors)
{
    // Use BFS to traverse the substituent and collect CIP descriptors
    std::queue<std::pair<int, int>> to_visit; // (atom_idx, depth)
    std::set<int> visited;
    std::set<int> visited_bonds;

    to_visit.push({start_atom, 0});
    visited.insert(from_atom); // Don't go back to the center atom

    while (!to_visit.empty())
    {
        auto [current_atom, depth] = to_visit.front();
        to_visit.pop();

        if (visited.count(current_atom))
            continue;

        visited.insert(current_atom);

        // Collect atom descriptor if present
        if (atom_labels.count(current_atom) && atom_labels[current_atom] > CIPDesc::UNKNOWN)
        {
            descriptors.push_back(atom_labels[current_atom]);
        }

        // Explore neighbors
        const Vertex& vertex = mol.getVertex(current_atom);
        for (int nei_idx = vertex.neiBegin(); nei_idx != vertex.neiEnd(); nei_idx = vertex.neiNext(nei_idx))
        {
            int neighbor = vertex.neiVertex(nei_idx);
            int edge_idx = vertex.neiEdge(nei_idx);

            // Collect bond descriptor if present
            if (visited_bonds.insert(edge_idx).second && bond_labels.count(edge_idx) && bond_labels[edge_idx] > CIPDesc::UNKNOWN)
            {
                descriptors.push_back(bond_labels[edge_idx]);
            }

            // Add neighbor to queue
            if (!visited.count(neighbor))
            {
                to_visit.push({neighbor, depth + 1});
            }
        }
    }
}

void MoleculeCIPCalc::_calcAtomLabels(BaseMolecule& mol, std::map<int, CIPDesc>& atom_labels, std::map<int, CIPDesc>& bond_labels,
                                      std::vector<CandidateAtom>& atom_candidates)
{

    // Iterate through all potential stereocenters
    for (auto& candidate : atom_candidates)
    {
        int atom_idx = candidate.atom_idx;

        ComparisonContext context;
        auto comp = [&](int a, int b) { return _compareCIPPriority(mol, atom_idx, a, b, atom_labels, bond_labels, &context) < 0; };

        std::set<int, decltype(comp)> neighbors(comp); // use set to sort neighbors and skip neighbors with same priority
        for (auto& neib : candidate.neighbors)
        {
            neighbors.insert(neib);
        }

        if (neighbors.size() < PYRAMID_SIZE)
            continue; // maybe still not enough info to calc correct priority

        std::map<int, int> priority_rank;
        int idx = 0;
        for (auto neib : neighbors) // neighbors is set of neighbors ordered by priority
            priority_rank[neib] = idx++;

        int inversions = 0;
        for (size_t i = 0; i < candidate.neighbors.size(); i++)
            for (size_t j = i + 1; j < candidate.neighbors.size(); j++)
                if (priority_rank[candidate.neighbors[i]] > priority_rank[candidate.neighbors[j]])
                    inversions++;

        bool is_r = ((inversions % 2) == 0) == (candidate.first_neighbor_direction == BOND_DOWN);
        atom_labels[atom_idx] = context.rule_5_used ? (is_r ? CIPDesc::r : CIPDesc::s) : (is_r ? CIPDesc::R : CIPDesc::S);
    }
}

void MoleculeCIPCalc::_calcBondLabels(BaseMolecule& mol, std::map<int, CIPDesc>& atom_labels, std::map<int, CIPDesc>& bond_labels, Array<int>& markers,
                                      BaseMolecule& coordinate_mol)
{
    std::map<int, CIPDesc> previous_bond_labels = bond_labels;
    std::map<int, CIPDesc> recalculated_bond_labels;

    // Iterate through all double bonds
    for (int bond_idx = mol.edgeBegin(); bond_idx != mol.edgeEnd(); bond_idx = mol.edgeNext(bond_idx))
    {
        // Algorithm from issue #3853 section 6 "Double bond CIP-label assignment"
        // 6.1. Double bond/geometric isomerism labels (E/Z) can be assigned only to a double bond that is between two atoms that are quadrivalent or trivalent
        // with one and only one lone pair (see req. 3.4.3.), and is not colinear (tolerance is 5°) with other bonds of those atoms.
        if (mol.getBondOrder(bond_idx) != BOND_DOUBLE)
            continue;
        if (mol.getBondTopology(bond_idx) == TOPOLOGY_RING && mol.edgeSmallestRingSize(bond_idx) <= 7)
            continue;

        // Check if this bond can have E/Z stereochemistry
        const Edge& edge = mol.getEdge(bond_idx);
        int beg_atom = edge.beg;
        int end_atom = edge.end;

        auto get_substituents = [&](int atom_idx, int double_bond_partner, std::vector<int>& substituents, int& lone_pair_count) {
            int artificial_lone_pairs = 0;
            for (int neighbor : _getNeighbors(mol, atom_idx))
            {
                if (neighbor < markers.size() && markers[neighbor] == 1 && mol.getAtomNumber(neighbor) == LONE_PAIR)
                {
                    ++artificial_lone_pairs;
                    continue;
                }
                if (neighbor != double_bond_partner)
                    substituents.push_back(neighbor);
            }
            if (substituents.size() != 1 && substituents.size() != 2)
                return false;

            const int group = Element::group(mol.getAtomNumber(atom_idx));
            const int connectivity = mol.getAtomConnectivity(atom_idx) - artificial_lone_pairs * BOND_SINGLE;
            const int vacant_orbitals =
                BaseMolecule::getVacantPiOrbitals(group, mol.getAtomCharge(atom_idx), mol.getAtomRadical(atom_idx), connectivity, &lone_pair_count);
            if (vacant_orbitals < 0)
                return false;

            return (connectivity == 4 && substituents.size() == 2 && lone_pair_count == 0) ||
                   (connectivity == 3 && substituents.size() == 1 && lone_pair_count == 1);
        };

        std::vector<int> beg_substituents;
        std::vector<int> end_substituents;
        int beg_lone_pair_count = 0;
        int end_lone_pair_count = 0;
        if (!get_substituents(beg_atom, end_atom, beg_substituents, beg_lone_pair_count) ||
            !get_substituents(end_atom, beg_atom, end_substituents, end_lone_pair_count))
            continue;

        // A lone pair is lower priority than an attached atom, so a lone-pair endpoint's
        // sole explicit substituent is necessarily the highest-priority ligand.
        auto get_high_priority_substituent = [&](const std::vector<int>& substituents, int atom_idx, int lone_pair_count) {
            if (lone_pair_count == 1)
                return substituents.front();

            const int comparison = _compareCIPPriority(mol, atom_idx, substituents[0], substituents[1], atom_labels, previous_bond_labels, nullptr);
            if (comparison == 0)
                return -1;
            return comparison < 0 ? substituents[0] : substituents[1];
        };

        const int beg_high_priority = get_high_priority_substituent(beg_substituents, beg_atom, beg_lone_pair_count);
        const int end_high_priority = get_high_priority_substituent(end_substituents, end_atom, end_lone_pair_count);
        if (beg_high_priority < 0 || end_high_priority < 0)
            continue;

        // 6.4: Determine E/Z configuration using geometric positions
        // Use the sameside function from MoleculeCisTrans to determine spatial relationship

        //  1 if substituents are on the same side
        // -1 if substituents are on opposite sides
        //  0 if indeterminate (collinear or no 3D coords)
        int side_result = 0;
        if (coordinate_mol.have_xyz)
        {
            side_result = MoleculeCisTrans::sameside(coordinate_mol.getAtomXyz(beg_atom), coordinate_mol.getAtomXyz(end_atom),
                                                     coordinate_mol.getAtomXyz(beg_high_priority), coordinate_mol.getAtomXyz(end_high_priority));
        }
        else
        {
            const int parity = mol.cis_trans.getParity(bond_idx);
            const int* substituents = mol.cis_trans.getSubstituents(bond_idx);
            if (parity == 0 || substituents == nullptr)
                continue;

            int beg_priority_slot = -1;
            int end_priority_slot = -1;
            for (int i = 0; i < 2; ++i)
                if (substituents[i] == beg_high_priority)
                    beg_priority_slot = i;
            for (int i = 2; i < 4; ++i)
                if (substituents[i] == end_high_priority)
                    end_priority_slot = i - 2;

            if (beg_priority_slot < 0 || end_priority_slot < 0)
                continue;

            const bool same_side = (parity == MoleculeCisTrans::CIS) == (beg_priority_slot == end_priority_slot);
            side_result = same_side ? 1 : -1;
        }

        // Z configuration: highest-priority groups on the same side
        // E configuration: highest-priority groups on opposite sides
        if (side_result == 1)
            recalculated_bond_labels[bond_idx] = CIPDesc::Z; // Same side
        else if (side_result == -1)                          // side_result == -1
            recalculated_bond_labels[bond_idx] = CIPDesc::E; // Opposite sides
    }

    bond_labels.swap(recalculated_bond_labels);
}

void MoleculeCIPCalc::_collectAtomCandidates(BaseMolecule& mol, BaseMolecule& unfolded_mol, Array<int>& markers, std::vector<CandidateAtom>& atom_candidates)
{
    for (int atom_idx = mol.vertexBegin(); atom_idx != mol.vertexEnd(); atom_idx = mol.vertexNext(atom_idx))
    {
        int isoelectronic_equivalent = 0;
        if (!_isAppropriateAtom(mol, atom_idx, isoelectronic_equivalent))
            continue;

        if (mol.getVertex(atom_idx).degree() < 3)
            continue;
        int lone_pair_count = 0;
        unfolded_mol.getVacantPiOrbitals(Element::group(unfolded_mol.getAtomNumber(atom_idx)), unfolded_mol.getAtomCharge(atom_idx),
                                         unfolded_mol.getAtomRadical(atom_idx), unfolded_mol.getAtomConnectivity(atom_idx), &lone_pair_count);
        int unfolded_atom_idx = -1;
        if (lone_pair_count > 0)
        {
            if (lone_pair_count > 1)
                continue;
            int lp_idx = unfolded_mol.addAtom(LONE_PAIR);
            unfolded_mol.addBond(atom_idx, lp_idx, BOND_SINGLE);
            markers.expandFill(lp_idx + 1, 0);
            markers.at(lp_idx) = 1;
            unfolded_atom_idx = lp_idx;
        }
        auto neighbors = _getNeighbors(unfolded_mol, atom_idx);
        if (neighbors.size() != PYRAMID_SIZE)
            continue;
        double angles[PYRAMID_SIZE];
        int bonds[PYRAMID_SIZE];
        // Calc angles
        bool inapropriate_atom = false;
        const Vec3f center = mol.getAtomXyz(atom_idx);
        int double_count = 0;
        std::vector<int> up_neibs;
        std::vector<int> down_neibs;
        for (int i = 0; i < neighbors.size(); i++)
        {
            int neib_idx = neighbors[i];
            int bond_idx = unfolded_mol.findEdgeIndex(atom_idx, neib_idx);
            int order = unfolded_mol.getBondOrder(bond_idx);
            switch (order)
            {
            case BOND_SINGLE:
                break;
            case BOND_DOUBLE:
                switch (isoelectronic_equivalent)
                {
                case ELEM_P:
                case ELEM_As:
                case ELEM_S:
                case ELEM_Se:
                case ELEM_Te:
                    break;
                default:
                    inapropriate_atom = true;
                    break;
                }
                double_count++;
                break;
            default:
                inapropriate_atom = true;
                break;
            }
            if (atom_idx == unfolded_mol.getEdge(bond_idx).beg)
            {
                bonds[i] = unfolded_mol.getBondDirection(bond_idx);
                switch (bonds[i])
                {
                case BOND_DIRECTION_MONO:
                    break;
                case BOND_UP:
                    up_neibs.emplace_back(i);
                    break;
                case BOND_DOWN:
                    down_neibs.emplace_back(i);
                    break;
                default:
                    inapropriate_atom = true; // 3.1.2. The chiral center cannot be at the narrow end of an either stereo-bond.
                    break;
                }
            }
            else
            {
                bonds[i] = 0; // 3.1.3. If the atom is at the wide end of a stereo-bond, that bond should be treated as a simple single bond for that atom.
            }
            if (markers[neib_idx] == 1)
            {
                if (unfolded_mol.getAtomNumber(neib_idx) == ELEM_H) // unfolded hydrogen, idx for lone pair already set
                {
                    if (unfolded_atom_idx >= 0) // already have anfolde hydrogen or lone pair
                    {
                        inapropriate_atom = true;
                        break;
                    }
                    unfolded_atom_idx = neib_idx;
                }
                continue; // angle for unfolded calculated later
            }
            // calc angle
            Vec3f neib_pos = mol.getAtomXyz(neib_idx);
            double neib_angle = std::atan2(neib_pos.y - center.y, neib_pos.x - center.x);
            if (neib_angle < 0.0f)
                neib_angle += TWO_PI;
            angles[i] = neib_angle;
        }
        switch (isoelectronic_equivalent)
        {
        case ELEM_P:
        case ELEM_As:
            if (double_count + lone_pair_count > 1)
                inapropriate_atom = true;
            break;
        case ELEM_S:
        case ELEM_Se:
        case ELEM_Te:
            if (double_count + lone_pair_count != 0 && double_count + lone_pair_count != 2)
                inapropriate_atom = true;
            break;
        default:
            if (lone_pair_count > 0) // lone pair only for P, As, S, Se, Te
                inapropriate_atom = true;
            break;
        }
        std::vector<int> neibs;
        int unfolded_i = -1;
        for (int i = 0; i < PYRAMID_SIZE; i++)
        {
            if (unfolded_atom_idx != neighbors[i])
                neibs.emplace_back(i);
            else
                unfolded_i = i;
        }
        auto stereo_count = up_neibs.size() + down_neibs.size();
        if (inapropriate_atom || stereo_count < 1 || stereo_count > 2)
            continue;
        if (unfolded_atom_idx >= 0) // need to fix unfolded hydrogen/lonepair direction
        {
            std::sort(neibs.begin(), neibs.end(), [&angles](int a, int b) { return angles[a] < angles[b]; });
            if (stereo_count == 1) // 3.3.1. If the atom has three drawn single bonds, and one of them is a stereo-bond
            {
                if (up_neibs.size() > 0) // one up
                {
                    neibs.insert(std::find(neibs.begin(), neibs.end(), up_neibs[0]), unfolded_i);
                    down_neibs.emplace_back(unfolded_i);
                    bonds[unfolded_i] = BOND_DOWN;
                }
                else // one down
                {
                    neibs.insert(std::find(neibs.begin(), neibs.end(), down_neibs[0]), unfolded_i);
                    up_neibs.emplace_back(unfolded_i);
                    bonds[unfolded_i] = BOND_UP;
                }
                stereo_count++;
            }
            else if (stereo_count == 2) // 3.3.2 two stereo-bond
            {
                // inser hydrogen into angle > PI
                if (angles[neibs[1]] - angles[neibs[0]] > M_PI)
                    neibs.insert(neibs.begin() + 1, unfolded_i);
                else if (angles[neibs[2]] - angles[neibs[1]] > M_PI)
                    neibs.insert(neibs.begin() + 2, unfolded_i);
                else if (angles[neibs[0]] - angles[neibs[2]] + TWO_PI > M_PI)
                    neibs.insert(neibs.begin(), unfolded_i);
                else
                    continue; // invalid bond configuration
            }
            else
                continue; // invalid bond configuration
        }
        else
        {
            std::sort(neibs.begin(), neibs.end(), [&angles](int a, int b) { return angles[a] < angles[b]; });
        }
        // reorder neibors to stereo, stereo, single, single if it possile
        std::vector<int> result_neibs;
        int first_neib_direction = -1;
        if (stereo_count == 1) // 3.2.2. If the atom has four drawn single bonds, and one of them is a stereo-bond
        {
            if (up_neibs.size() == 1)
            {
                first_neib_direction = BOND_UP;
                auto up = std::find(neibs.begin(), neibs.end(), up_neibs[0]);
                result_neibs.assign(up, neibs.end());
                if (up != neibs.begin())
                    result_neibs.insert(result_neibs.end(), neibs.begin(), up);
            }
            else
            {
                first_neib_direction = BOND_DOWN;
                auto down = std::find(neibs.begin(), neibs.end(), down_neibs[0]);
                result_neibs.assign(down, neibs.end());
                if (down != neibs.begin())
                    result_neibs.insert(result_neibs.end(), neibs.begin(), down);
            }
        }
        else if (up_neibs.size() == 1) //
        {
            auto up = std::find(neibs.begin(), neibs.end(), up_neibs[0]);
            auto down = std::find(neibs.begin(), neibs.end(), down_neibs[0]);
            auto dist = std::distance(up, down);
            switch (dist)
            {
            case 1:  // down after up
            case -3: // down after up, up is last
                first_neib_direction = BOND_UP;
                result_neibs.assign(up, neibs.end());
                if (up != neibs.begin())
                    result_neibs.insert(result_neibs.end(), neibs.begin(), up);
                break;
            case -1: // up after down
            case 3:  // up after down, down is last
                first_neib_direction = BOND_DOWN;
                result_neibs.assign(down, neibs.end());
                if (down != neibs.begin())
                    result_neibs.insert(result_neibs.end(), neibs.begin(), down);
                break;
            default:
                continue; // invalid bond configuration
                break;
            }
        }
        else if (up_neibs.size() == 2 || down_neibs.size() == 2)
        {
            first_neib_direction = up_neibs.size() == 2 ? BOND_UP : BOND_DOWN;
            const auto& stereo = first_neib_direction == BOND_UP ? up_neibs : down_neibs;
            auto first = std::find(neibs.begin(), neibs.end(), stereo[0]);
            auto down = std::find(neibs.begin(), neibs.end(), stereo[1]);
            auto dist = std::distance(first, down);
            switch (dist)
            {
            case 2:
            case -2:
                result_neibs.assign(first, neibs.end());
                if (first != neibs.begin())
                    result_neibs.insert(result_neibs.end(), neibs.begin(), first);
                break;
            default:
                continue; // invalid bond configuration
                break;
            }
        }
        else
            continue;
        // now result_neibs starts with two neighbors with opposite stereo bonds, first neighbor bond direction in first_neib_direction
        for (int i = 0; i < PYRAMID_SIZE; i++)
        {
            result_neibs[i] = neighbors[result_neibs[i]];
        }
        atom_candidates.emplace_back(atom_idx, first_neib_direction, std::move(result_neibs));
    }
}

void MoleculeCIPCalc::_atomCandidatesFromStereocenters(BaseMolecule& mol, Array<int>& markers, std::vector<CandidateAtom>& atom_candidates)
{
    auto is_pure_hydrogen = [&](int atom_idx) {
        if (mol.getAtomNumber(atom_idx) != ELEM_H)
            return false;

        if (mol.getAtomIsotope(atom_idx) == 0)
            return true;

        return mol.isQueryMolecule() && !mol.asQueryMolecule().getAtom(atom_idx).hasConstraint(QueryMolecule::ATOM_ISOTOPE);
    };

    for (int stereo_idx = mol.stereocenters.begin(); stereo_idx != mol.stereocenters.end(); stereo_idx = mol.stereocenters.next(stereo_idx))
    {
        const int atom_idx = mol.stereocenters.getAtomIndex(stereo_idx);
        if (mol.stereocenters.getType(atom_idx) <= MoleculeStereocenters::ATOM_ANY || !mol.stereocenters.isTetrahydral(atom_idx))
            continue;

        int pyramid[PYRAMID_SIZE];
        const int* center_pyramid = mol.stereocenters.getPyramid(atom_idx);
        if (center_pyramid == nullptr)
            continue;
        std::copy_n(center_pyramid, PYRAMID_SIZE, pyramid);
        int missing_ligand = -1;
        int missing_count = 0;
        std::set<int> unique_neighbors;
        bool valid_pyramid = true;
        for (int i = 0; i < PYRAMID_SIZE; ++i)
        {
            const int neighbor = pyramid[i];
            if (neighbor == -1)
            {
                missing_ligand = i;
                ++missing_count;
                continue;
            }

            if (mol.findEdgeIndex(atom_idx, neighbor) == -1 || !unique_neighbors.insert(neighbor).second)
            {
                valid_pyramid = false;
                break;
            }
        }
        if (!valid_pyramid || missing_count > 1)
            continue;
        if (missing_count == 1 && missing_ligand != PYRAMID_SIZE - 1)
            continue;

        const int parity = MoleculeStereocenters::getMdlParity(mol, atom_idx);
        if (parity != MoleculeStereocenters::MDL_PARITY_ODD && parity != MoleculeStereocenters::MDL_PARITY_EVEN)
            continue;

        const int pyramid_order_end = mol.vertexEnd();
        int lone_pair_idx = -1;
        if (missing_count == 1)
        {
            auto& vertex = mol.getVertex(atom_idx);
            if (vertex.degree() == PYRAMID_SIZE)
            {
                bool found_unfolded_hydrogen = false;
                for (auto i : vertex.neighbors())
                {
                    int idx = vertex.neiVertex(i);
                    if (idx < markers.size() && markers[idx] == 1 && mol.getAtomNumber(idx) == ELEM_H)
                    {
                        pyramid[missing_ligand] = idx;
                        found_unfolded_hydrogen = true;
                        break;
                    }
                }
                if (!found_unfolded_hydrogen)
                    continue;
            }
            else
            {
                int lone_pair_count = 0;
                mol.getVacantPiOrbitals(Element::group(mol.getAtomNumber(atom_idx)), mol.getAtomCharge(atom_idx), mol.getAtomRadical(atom_idx),
                                        mol.getAtomConnectivity(atom_idx), &lone_pair_count);
                if (lone_pair_count != 1)
                    continue;

                lone_pair_idx = mol.addAtom(LONE_PAIR);
                mol.addBond(atom_idx, lone_pair_idx, BOND_SINGLE);
                markers.expandFill(lone_pair_idx + 1, 0);
                markers[lone_pair_idx] = 1;
                pyramid[missing_ligand] = lone_pair_idx;
            }
        }

        if (unique_neighbors.size() + missing_count != PYRAMID_SIZE)
            continue;

        auto atom_order = [&](int neighbor) {
            if (neighbor == lone_pair_idx)
                return pyramid_order_end;

            if (is_pure_hydrogen(neighbor))
                return pyramid_order_end;

            return neighbor;
        };
        std::vector<int> neighbors(std::begin(pyramid), std::end(pyramid));
        std::sort(neighbors.begin(), neighbors.end(), [&](int first, int second) { return atom_order(first) < atom_order(second); });

        // getMdlParity already accounts for this canonical neighbor ordering.
        int direction = parity == MoleculeStereocenters::MDL_PARITY_ODD ? BOND_DOWN : BOND_UP;
        atom_candidates.emplace_back(atom_idx, direction, std::move(neighbors));
    }
}