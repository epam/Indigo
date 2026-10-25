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

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "base_c/defs.h"
#include "molecule/molecule_cip_calculator.h"

namespace indigo
{
    class BaseMolecule;

    class DLLEXPORT MoleculeCIPCalc
    {
    public:
        // Calculate CIP labels for the molecule according to issue #3853 requirements
        // Returns true if more than 0 CIP labels were calculated
        static bool calcCIPLabels(BaseMolecule& mol);

    private:
        // Structure to represent an expanded atom for Rule 1.2 (sphere expansion)
        struct ExpandedAtom
        {
            int atom_idx;
            int atomic_number;
            int mass_number;
            int bond_order; // Bond order to parent (for phantom atoms from multiple bonds)
            bool is_duplicate;
            int duplicate_distance;
            std::vector<ExpandedAtom> children;

            ExpandedAtom() : atom_idx(-1), atomic_number(0), mass_number(0), bond_order(1), is_duplicate(false), duplicate_distance(-1)
            {
            }
        };

        struct CandidateAtom
        {
            int atom_idx;
            int first_neighbor_direction;
            std::vector<int> neighbors; // neighbors in anti-clockwise orders. first neighbor bond direction in first_neighbor_direction, second neighbor bond
                                        // direction is opposit to first

            CandidateAtom(int atom, int dir, std::vector<int>&& neibs) : atom_idx(atom), first_neighbor_direction(dir), neighbors(std::move(neibs))
            {
            }
        };
        // Check if an atom is appropriate for CIP stereochemistry
        // Appropriate atoms: C, N, Si, P, S, As, Se, Te and their isoelectronic equivalents
        static bool _isAppropriateAtom(BaseMolecule& mol, int atom_idx, int& isoelectronic_equivalent);

        // Context for CIP comparison to track Rule 5 state
        struct ComparisonContext
        {
            CIPDesc ref_cip1;
            CIPDesc ref_cip2;
            bool rule_5_used;

            ComparisonContext() : ref_cip1(CIPDesc::NONE), ref_cip2(CIPDesc::NONE), rule_5_used(false)
            {
            }
        };

        // Get neighbors of an atom excluding specific atoms
        static std::vector<int> _getNeighbors(BaseMolecule& mol, int atom_idx);

        // Compare two substituents using CIP priority rules
        // Returns: -1 if sub1 has higher priority, 1 if sub2 has higher priority, 0 if equal
        static int _compareCIPPriority(BaseMolecule& mol, int atom_idx, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels,
                                       std::map<int, CIPDesc>& bond_labels, ComparisonContext* context = nullptr);

        // Apply Rule 1.1: Atomic number comparison (immediate neighbors)
        static int _applyRule1_1(BaseMolecule& mol, int sub1, int sub2);

        // Apply Rule 1.2: Sphere expansion - compare substituent trees recursively
        static int _applyRule1_2(BaseMolecule& mol, int center_atom, int sub1, int sub2, ExpandedAtom& tree1, ExpandedAtom& tree2);

        // Build expanded tree for a substituent (handles multiple bonds as phantom atoms)
        static void _buildExpandedTree(BaseMolecule& mol, int atom_idx, int from_atom, ExpandedAtom& node, std::map<int, int>& visited, int distance);

        // Compare two expanded trees recursively
        static int _compareExpandedTrees(const ExpandedAtom& tree1, const ExpandedAtom& tree2);

        // Apply Rule 2: Mass number (isotope) comparison
        static int _applyRule2(const ExpandedAtom& tree1, const ExpandedAtom& tree2);

        // Apply Rule 3: Stereochemical descriptors (Z > E > nonstereogenic)
        static int _applyRule3(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& bond_labels);

        // Apply Rule 4.1: Chiral > pseudoasymmetric > nonstereogenic
        static int _applyRule4_1(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels);

        // Apply Rule 4.2: Like descriptor pairs rank higher than unlike pairs
        static int _applyRule4_2(BaseMolecule& mol, int center_atom, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels,
                                 std::map<int, CIPDesc>& bond_labels);

        // Apply Rule 4.3: Chiral > pseudoasymmetric > nonstereogenic
        static int _applyRule4_3(BaseMolecule& mol, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels);

        // Apply Rule 5: Seqcis precedes seqtrans - matching reference descriptors
        static int _applyRule5(BaseMolecule& mol, int center_atom, int sub1, int sub2, std::map<int, CIPDesc>& atom_labels, std::map<int, CIPDesc>& bond_labels,
                               ComparisonContext* context);

        // Collect CIP descriptors along a substituent path (for Rule 4.2)
        static void _collectDescriptorsAlongPath(BaseMolecule& mol, int start_atom, int from_atom, std::map<int, CIPDesc>& atom_labels,
                                                 std::map<int, CIPDesc>& bond_labels, std::vector<CIPDesc>& descriptors);

        // Calculate R/S labels for atoms (with iterative refinement)
        static void _calcAtomLabels(BaseMolecule& mol, std::map<int, CIPDesc>& atom_labels, std::map<int, CIPDesc>& bond_labels,
                                    std::vector<CandidateAtom>& atom_candidates);

        // Calculate E/Z labels for double bonds
        static void _calcBondLabels(BaseMolecule& mol, std::map<int, CIPDesc>& atom_labels, std::map<int, CIPDesc>& bond_labels, Array<int>& markers,
                                    BaseMolecule& coordinate_mol);

        // Collect chiral centers candidates when no coordinates in molecule
        static void _atomCandidatesFromStereocenters(BaseMolecule& mol, Array<int>& markers, std::vector<CandidateAtom>& atom_candidates);

        // Collect chiral centers candidates based on coordinates
        static void _collectAtomCandidates(BaseMolecule& mol, BaseMolecule& unfolded_mol, Array<int>& markers, std::vector<CandidateAtom>& atom_candidates);
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
