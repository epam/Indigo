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

#include "molecule/structure_checker.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/base_reaction.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace indigo;

StructureChecker::StructureChecker()
{
}

bool StructureChecker::CheckResult::isEmpty() const
{
    return messages.size() == 0;
}

static void message(StructureChecker::CheckResult& result, StructureChecker::CheckMessageCode code, int index, std::vector<int> ids,
                    const StructureChecker::CheckResult& subresult)
{
    result.messages.push_back(StructureChecker::CheckMessage(code, index, ids, subresult));
}

static void message(StructureChecker::CheckResult& result, StructureChecker::CheckMessageCode code, int index, const StructureChecker::CheckResult& subresult)
{
    message(result, code, index, std::vector<int>(), subresult);
}

static void message(StructureChecker::CheckResult& result, StructureChecker::CheckMessageCode code, const std::vector<int>& ids)
{
    message(result, code, -1, ids, StructureChecker::CheckResult());
}

static void message(StructureChecker::CheckResult& result, StructureChecker::CheckMessageCode code, const std::unordered_set<int>& ids)
{
    std::vector<int> v;
    std::copy(ids.begin(), ids.end(), std::back_inserter(v));
    message(result, code, v);
}

static void message(StructureChecker::CheckResult& result, StructureChecker::CheckMessageCode code)
{
    message(result, code, -1, std::vector<int>(), StructureChecker::CheckResult());
}

/**************************************************/
static void filter_atoms(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, StructureChecker::CheckResult& result,
                         StructureChecker::CheckMessageCode msg, const std::function<bool(BaseMolecule&, int)>& filter, bool default_filter = true)
{
    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids), [&mol, &filter, default_filter](int idx) {
        return (!default_filter || (!mol.isPseudoAtom(idx) && !mol.isRSite(idx) && !mol.isTemplateAtom(idx))) && filter(mol, idx);
    });
    if (ids.size())
    {
        message(result, msg, ids);
    }
}

static bool isQueryMolecule(BaseMolecule& mol)
{
    bool r = mol.isQueryMolecule();
    if (!r)
    {
        for (auto idx : mol.vertices())
        {
            r = r || (mol.reaction_atom_exact_change[idx] || mol.reaction_atom_inversion[idx]);
        }
        for (auto idx : mol.edges())
        {
            r = r || mol.reaction_bond_reacting_center[idx];
        }
    }
    return r;
}

static bool hasRGroups(BaseMolecule& mol)
{
    return !isQueryMolecule(mol) && (mol.countRSites() || mol.attachmentPointCount() || mol.rgroups.getRGroupCount());
}

static bool hasPseudoAtoms(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (mol.isPseudoAtom(i))
        {
            return true;
        }
    }
    return false;
}

//

static void check_none(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker::CheckResult& result)
{
}

static void check_load(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker::CheckResult& result)
{
    if (mol.vertexCount() == 0)
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_EMPTY);
    }
}

#define FILTER_ATOMS(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER, false);
#define FILTER_ATOMS_DEFAULT(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER);

static void check_valence(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker::CheckResult& result)
{

    if (isQueryMolecule(mol))
    {
        message(result,
                StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_QUERY); // 'Structure contains query features, so valency could not be
                                                                                          // checked'
    }
    else if (hasRGroups(mol))
    {
        message(result,
                StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_RGROUP); // 'Structure contains RGroup components, so valency could
    }
    else if (!isQueryMolecule(mol) && mol.asMolecule().getIgnoreBadValenceFlag())
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_IGNORE_VALENCE_ERROR);
    }
    else
    {
        FILTER_ATOMS(StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE,
                     [](BaseMolecule& mol, int idx) { return mol.getAtomValence_NoThrow(idx, -1) == -1; });
    }
}

static void check_radical(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker::CheckResult& result)
{
    if (hasPseudoAtoms(mol))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL_NOT_CHECKED_PSEUDO);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL,
                             [](BaseMolecule& mol, int idx) { return mol.getAtomRadical_NoThrow(idx, -1) > 0; });
    }
}
static void check_pseudoatom(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                             StructureChecker::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker::CheckMessageCode::CHECK_MSG_PSEUDOATOM, [](BaseMolecule& mol, int idx) { return mol.isPseudoAtom(idx); });
}
static void check_stereo(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker::CheckResult& result)
{
    if (!isQueryMolecule(mol))
    {
        std::unique_ptr<Molecule> target = std::make_unique<Molecule>();
        auto saved_valence_flag = mol.asMolecule().getIgnoreBadValenceFlag();
        mol.asMolecule().setIgnoreBadValenceFlag(true);
        target->clone_KeepIndices(mol);

        for (const auto i : target->vertices())
        {
            if (!target->stereocenters.exists(i) && target->isPossibleStereocenter(i))
            {
                try
                {
                    target->addStereocenters(i, MoleculeStereocenters::ATOM_ABS, 0, false);
                }
                catch (Exception&)
                {
                    // Just ignore this stereo center
                }
            }
        }

        MoleculeAutomorphismSearch as;

        as.detect_invalid_cistrans_bonds = true;
        as.detect_invalid_stereocenters = true;
        as.find_canonical_ordering = false;
        as.process(*target);

        for (const auto i : target->vertices())
        {
            if (target->stereocenters.exists(i) && as.invalidStereocenter(i))
            {
                target->stereocenters.remove(i);
            }
        }

        FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_3D_STEREO, [](BaseMolecule& mol, int idx) {
            bool stereo_3d = false;
            if (BaseMolecule::hasZCoord(mol) && mol.stereocenters.exists(idx))
            {
                const Vertex& vertex = mol.getVertex(idx);
                for (auto j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                    if (mol.getBondDirection2(idx, vertex.neiVertex(j)) > 0)
                        stereo_3d = true;
            }
            return stereo_3d;
        });

        FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_WRONG_STEREO, [&target](BaseMolecule& mol, int idx) {
            return (mol.stereocenters.exists(idx) && target->stereocenters.exists(idx) &&
                    mol.stereocenters.getType(idx) != target->stereocenters.getType(idx)) ||
                   (mol.stereocenters.exists(idx) && !target->stereocenters.exists(idx));
        });
        FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO,
                             [&target](BaseMolecule& mol, int idx) { return !mol.stereocenters.exists(idx) && target->stereocenters.exists(idx); });

        mol.asMolecule().setIgnoreBadValenceFlag(saved_valence_flag);
    }
    else
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO);
    }
}
static void check_query(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker::CheckResult& result)
{
    if (isQueryMolecule(mol))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_QUERY);
    }
    FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_QUERY_ATOM,
                         [](BaseMolecule& mol, int idx) { return mol.reaction_atom_exact_change[idx] || mol.reaction_atom_inversion[idx]; });

    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids),
                 [&mol](int idx) { return idx >= 0 && idx < mol.reaction_bond_reacting_center.size() && mol.reaction_bond_reacting_center[idx] != 0; });
    if (ids.size())
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_QUERY_BOND, ids);
    }
}

static float calc_mean_dist(BaseMolecule& mol)
{
    float mean_dist = 0.0;
    for (const auto i : mol.edges())
    {
        const Edge& edge = mol.getEdge(i);
        Vec3f& a = mol.getAtomXyz(edge.beg);
        Vec3f& b = mol.getAtomXyz(edge.end);
        mean_dist += Vec3f::dist(a, b);
    }
    if (mol.edgeCount() > 0)
        mean_dist = mean_dist / mol.edgeCount();
    return mean_dist;
}

static void check_overlap_atom(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                               StructureChecker::CheckResult& result)
{
    auto mean_dist = calc_mean_dist(mol);
    std::unordered_set<int> ids;
    std::for_each(selected_atoms.begin(), selected_atoms.end(), [&mol, &ids, mean_dist](int idx) {
        Vec3f& a = mol.getAtomXyz(idx);
        for (const auto i : mol.vertices())
        {
            if (i != idx)
            {
                Vec3f& b = mol.getAtomXyz(i);
                if ((mean_dist > 0.0) && (Vec3f::dist(a, b) < 0.25 * mean_dist))
                {
                    ids.insert(idx);
                    ids.insert(i);
                }
            }
        }
    });
    if (ids.size())
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_OVERLAP_ATOM, ids);
    }
}
static void check_overlap_bond(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                               StructureChecker::CheckResult& result)
{
    if (BaseMolecule::hasCoord(mol))
    {
        auto mean_dist = calc_mean_dist(mol);
        std::unordered_set<int> ids;
        std::for_each(selected_bonds.begin(), selected_bonds.end(), [&mol, &ids, mean_dist](int idx) {
            const Edge& e1 = mol.getEdge(idx);
            Vec2f a1, b1, a2, b2;
            Vec2f::projectZ(a1, mol.getAtomXyz(e1.beg));
            Vec2f::projectZ(b1, mol.getAtomXyz(e1.end));

            for (const auto i : mol.edges())
            {
                if ((i != idx))
                {
                    const Edge& e2 = mol.getEdge(i);
                    Vec2f::projectZ(a2, mol.getAtomXyz(e2.beg));
                    Vec2f::projectZ(b2, mol.getAtomXyz(e2.end));
                    if ((Vec2f::dist(a1, a2) < 0.01 * mean_dist) || (Vec2f::dist(b1, b2) < 0.01 * mean_dist) || (Vec2f::dist(a1, b2) < 0.01 * mean_dist) ||
                        (Vec2f::dist(b1, a2) < 0.01 * mean_dist))
                        continue;

                    if (Vec2f::segmentsIntersect(a1, b1, a2, b2))
                    {
                        ids.insert(idx);
                        ids.insert(i);
                    }
                }
            }
        });
        if (ids.size())
        {
            message(result, StructureChecker::CheckMessageCode::CHECK_MSG_OVERLAP_BOND, ids);
        }
    }
}

static void check_rgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker::CheckResult& result)
{
    if (!isQueryMolecule(mol) && hasRGroups(mol))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_RGROUP);
    }
}

static void check_sgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker::CheckResult& result)
{
    if (mol.sgroups.getSGroupCount() > 0)
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_SGROUP);
    }
}

static void check_tgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker::CheckResult& result)
{
    if (mol.tgroups.getTGroupCount() > 0)
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_TGROUP);
    }
}

static void check_chirality(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                            StructureChecker::CheckResult& result)
{
    if (mol.isChiral())
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_CHIRALITY);
    }
}

static void check_chiral_flag(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                              StructureChecker::CheckResult& result)
{
    if (mol.getChiralFlag() > 0 && mol.stereocenters.size() == 0)
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_CHIRAL_FLAG);
    }
}

static void check_3d_coord(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                           StructureChecker::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker::CheckMessageCode::CHECK_MSG_3D_COORD, [](BaseMolecule& mol, int idx) { return fabs(mol.getAtomXyz(idx).z) > 0.001; });
}

static void check_charge(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker::CheckResult& result)
{
    if (std::accumulate(selected_atoms.begin(), selected_atoms.end(), 0, [&mol](int sum, int idx) { return sum + mol.getAtomCharge(idx); }))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_CHARGE);
    }
}

static void check_salt(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker::CheckResult& result)
{
    // not impl
    message(result, StructureChecker::CheckMessageCode::CHECK_MSG_SALT_NOT_IMPL);
}

static void check_ambiguous_h(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                              StructureChecker::CheckResult& result)
{
    if (isQueryMolecule(mol))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H_NOT_CHECKED_QUERY);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H, [](BaseMolecule& mol, int idx) {
            return mol.asMolecule().getImplicitH_NoThrow(idx, -1) == -1 && mol.getAtomAromaticity(idx) == ATOM_AROMATIC;
        });
    }
}

static void check_coord(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker::CheckResult& result)
{
    if (mol.vertexCount() > 1 && !BaseMolecule::hasCoord(mol))
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_ZERO_COORD);
    }
}

static void check_v3000(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker::CheckResult& result)
{
    if (mol.hasHighlighting() || (!mol.stereocenters.haveAllAbsAny() && !mol.stereocenters.haveAllAndAny()) || mol.vertexCount() > 999 || mol.edgeCount() > 999)
    {
        message(result, StructureChecker::CheckMessageCode::CHECK_MSG_V3000);
    }
}
#undef FILTER_ATOMS
#undef FILTER_ATOMS_DEFAULT

/// <summary>
/// Textual Check Type values for the corresponding CheckTypeCode
/// are intended for use mainly in host language calls (Java, Python etc.) as a comma-separated list
///
/// The check_type_map maps Textual Check Type values to the corresponding CheckTypeCode codes
/// and CheckMessageCode codes to the corresponding text messages.
///
/// checkMolecule(molecule, "load, valence, radical, atoms 1 2 3, bonds 4 5 6, tgroup"
/// is equivalent to
/// checkMolecule(molecule, check_types=[CHECK_LOAD, CHECK_VALECE, CHECK_RADICAL, CHECK_TGROUP], selected_atoms=[1,2,3], selected_bonds=[4,5,6]);
/// </summary>
typedef void (*Checker)(BaseMolecule&, const std::unordered_set<int>&, const std::unordered_set<int>&, StructureChecker::CheckResult&);
struct CheckType
{
    StructureChecker::CheckTypeCode code;
    Checker checker;
    std::vector<std::pair<StructureChecker::CheckMessageCode, std::string>> messages;
};
static const std::unordered_map<std::string, CheckType> check_type_map = {
    {"",
     {StructureChecker::CheckTypeCode::CHECK_NONE,
      &check_none,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_NONE, ""},
       {StructureChecker::CheckMessageCode::CHECK_MSG_LOAD, "Error at loading structure, wrong format found"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_REACTION, "Reaction component check result"}}}},

    {"load", {StructureChecker::CheckTypeCode::CHECK_LOAD, &check_load, {{StructureChecker::CheckMessageCode::CHECK_MSG_EMPTY, "Input structure is empty"}}}},

    {"valence",
     {StructureChecker::CheckTypeCode::CHECK_VALENCE,
      &check_valence,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE, "Structure contains atoms with unusual valence"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_QUERY, "Structure contains query features, so valency could not be checked"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_RGROUP, "Structure contains RGroup components, so valency could not be checked"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_IGNORE_VALENCE_ERROR, "IGNORE_BAD_VALENCE flag is active, so valency could not be checked"}}}},

    {"radicals",
     {StructureChecker::CheckTypeCode::CHECK_RADICAL,
      &check_radical,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL, "Structure contains radicals"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL_NOT_CHECKED_PSEUDO, "Structure contains pseudoatoms, so radicals could not be checked"}}}},

    {"pseudoatoms",
     {StructureChecker::CheckTypeCode::CHECK_PSEUDOATOM,
      &check_pseudoatom,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_PSEUDOATOM, "Structure contains pseudoatoms"}}}},

    {"stereo",
     {StructureChecker::CheckTypeCode::CHECK_STEREO,
      &check_stereo,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_3D_STEREO, "Structure contains stereocenters defined by 3D coordinates"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_WRONG_STEREO, "Structure contains incorrect stereochemistry"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO, "Structure contains stereocenters with undefined stereo configuration"}}}},

    {"query",
     {StructureChecker::CheckTypeCode::CHECK_QUERY,
      &check_query,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_QUERY, "Structure contains query features"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_QUERY_ATOM, "Structure contains query features for atoms"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_QUERY_BOND, "Structure contains query features for bonds"}}}},

    {"overlapping_atoms",
     {StructureChecker::CheckTypeCode::CHECK_OVERLAP_ATOM,
      &check_overlap_atom,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_OVERLAP_ATOM, "Structure contains overlapping atoms"}}}},

    {"overlapping_bonds",
     {StructureChecker::CheckTypeCode::CHECK_OVERLAP_BOND,
      &check_overlap_bond,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_OVERLAP_BOND, "Structure contains overlapping bonds."}}}},

    {"rgroups",
     {StructureChecker::CheckTypeCode::CHECK_RGROUP, &check_rgroup, {{StructureChecker::CheckMessageCode::CHECK_MSG_RGROUP, "Structure contains R-groups"}}}},

    {"sgroups",
     {StructureChecker::CheckTypeCode::CHECK_SGROUP, &check_sgroup, {{StructureChecker::CheckMessageCode::CHECK_MSG_SGROUP, "Structure contains S-groups"}}}},

    {"tgroups",
     {StructureChecker::CheckTypeCode::CHECK_TGROUP,
      &check_tgroup,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_TGROUP, "Structure contains SCSR templates"}}}},

    {"chiral",
     {StructureChecker::CheckTypeCode::CHECK_CHIRALITY,
      &check_chirality,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_CHIRALITY, "Structure contains chirality"}}}},

    {"chiral_flag",
     {StructureChecker::CheckTypeCode::CHECK_CHIRAL_FLAG,
      &check_chiral_flag,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_CHIRAL_FLAG, "Structure contains wrong chiral flag"}}}},

    {"3d",
     {StructureChecker::CheckTypeCode::CHECK_3D_COORD,
      &check_3d_coord,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_3D_COORD, "Structure contains 3D coordinates"}}}},

    {"charge",
     {StructureChecker::CheckTypeCode::CHECK_CHARGE, &check_charge, {{StructureChecker::CheckMessageCode::CHECK_MSG_CHARGE, "Structure has non-zero charge"}}}},

    //    {"salt",
    //     {StructureChecker::CheckTypeCode::CHECK_SALT,
    //      &check_salt,
    //      {{StructureChecker::CheckMessageCode::CHECK_MSG_SALT, "Structure contains charged fragments (possible salt)"},
    //       {StructureChecker::CheckMessageCode::CHECK_MSG_SALT_NOT_IMPL, "Not implemented yet: check salt"}}}},

    {"ambiguous_h",
     {StructureChecker::CheckTypeCode::CHECK_AMBIGUOUS_H,
      &check_ambiguous_h,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H, "Structure contains ambiguous hydrogens"},
       {StructureChecker::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H_NOT_CHECKED_QUERY,
        "Structure contains query features, so ambiguous H could not be checked"}}}},

    {"coord",
     {StructureChecker::CheckTypeCode::CHECK_COORD,
      &check_coord,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_ZERO_COORD, "Structure has no atoms coordinates"}}}},

    {"v3000",
     {StructureChecker::CheckTypeCode::CHECK_V3000,
      &check_v3000,
      {{StructureChecker::CheckMessageCode::CHECK_MSG_V3000, "Structure supports only Molfile V3000"}}}}};

static const struct CheckNamesMap
{
    std::vector<StructureChecker::CheckTypeCode> all;
    std::unordered_map<int, std::string> types;
    std::unordered_map<int, Checker> checkers;
    std::unordered_map<int, std::string> messages;
    std::unordered_map<int, StructureChecker::CheckTypeCode> code2type;
    CheckNamesMap()
    {
        std::for_each(check_type_map.begin(), check_type_map.end(), [this](std::pair<std::string, CheckType> t) {
            this->all.push_back(t.second.code);
            this->types.insert(std::pair<int, std::string>((int)t.second.code, t.first));
            this->checkers.insert(std::pair<int, Checker>((int)t.second.code, t.second.checker));
            std::for_each(t.second.messages.begin(), t.second.messages.end(), [this, t](std::pair<StructureChecker::CheckMessageCode, std::string> it) {
                this->messages.insert(std::pair<int, std::string>((int)it.first, it.second));
                this->code2type.insert(std::pair<int, StructureChecker::CheckTypeCode>((int)it.first, t.second.code));
            });
        });
    }
} check_names_map;

StructureChecker::CheckTypeCode StructureChecker::getCheckType(const std::string& type)
{
    auto code = check_type_map.find(type);
    return code == check_type_map.end() ? StructureChecker::CheckTypeCode::CHECK_NONE : code->second.code;
}

std::string StructureChecker::getCheckType(StructureChecker::CheckTypeCode code)
{
    return check_names_map.types.at((int)code);
}

std::string StructureChecker::getCheckMessage(StructureChecker::CheckMessageCode code)
{
    return check_names_map.messages.at((int)code);
}

StructureChecker::CheckTypeCode StructureChecker::getCheckTypeByMsgCode(StructureChecker::CheckMessageCode code)
{
    return check_names_map.code2type.at((int)code);
}

struct CheckParams
{
    std::vector<StructureChecker::CheckTypeCode> check_types;
    std::vector<int> selected_atoms;
    std::vector<int> selected_bonds;
};

static CheckParams check_params_from_string(const std::string& params)
{
    CheckParams r;
    if (!params.empty())
    {
        std::smatch sm1;
        std::unordered_set<int> ct;
        std::string s = params;
        std::regex rx1(R"(\b(\w+)\b)", std::regex_constants::icase);
        while (std::regex_search(s, sm1, rx1))
        {
            auto code = StructureChecker::getCheckType(sm1[1]);
            if (code != StructureChecker::CheckTypeCode::CHECK_NONE)
            {
                ct.insert((int)code);
            }
            s = sm1.suffix();
        }
        std::vector<StructureChecker::CheckTypeCode> chk;
        std::for_each(ct.begin(), ct.end(), [&r](int v) { r.check_types.push_back((StructureChecker::CheckTypeCode)v); });

        std::smatch sm2;
        s = params;
        std::regex rx2(R"(\b(atoms|bonds)\b((?:\W+\b\d+\b\W*?)+))", std::regex_constants::icase);
        std::regex rx3(R"(\b(\d+)\b)");
        std::smatch sm3;
        while (std::regex_search(s, sm2, rx2))
        {
            std::vector<int>& vec = std::tolower(sm2[1].str()[0]) == 'a' ? r.selected_atoms : r.selected_bonds;
            std::string a = sm2[2];
            while (std::regex_search(a, sm3, rx3))
            {
                vec.push_back(atoi(sm3[1].str().c_str()));
                a = sm3.suffix();
            }
            s = sm2.suffix();
        }
    }
    return r;
}

StructureChecker::CheckResult StructureChecker::checkMolecule(const BaseMolecule& item, const std::string& check_types_and_selections)
{
    auto pars = check_params_from_string(check_types_and_selections);
    return checkMolecule(item, pars.check_types, pars.selected_atoms, pars.selected_bonds);
}
StructureChecker::CheckResult StructureChecker::checkMolecule(const BaseMolecule& item, const std::string& check_types, const std::vector<int>& selected_atoms,
                                                              const std::vector<int>& selected_bonds)
{
    auto pars = check_params_from_string(check_types);
    return checkMolecule(item, pars.check_types, selected_atoms, selected_bonds);
}

StructureChecker::CheckResult StructureChecker::checkMolecule(const BaseMolecule& bmol, const std::vector<CheckTypeCode>& check_types,
                                                              const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds)
{
    StructureChecker::CheckResult result;

    auto num_atoms = bmol.vertexEnd();
    auto num_bonds = bmol.edgeEnd();

    std::unordered_set<int> sel_atoms(num_atoms);
    std::unordered_set<int> sel_bonds(num_bonds);

    if (selected_atoms.size() == 0 && selected_bonds.size() == 0)
    {
        for (const auto i : ((BaseMolecule&)bmol).vertices())
        {
            sel_atoms.insert(i);
        }

        for (const auto i : ((BaseMolecule&)bmol).edges())
        {
            sel_bonds.insert(i);
        }
    }
    else
    {
        std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::inserter(sel_atoms, sel_atoms.begin()),
                     [num_atoms](int i) { return i >= 0 && i < num_atoms; });
        std::copy_if(selected_bonds.begin(), selected_bonds.end(), std::inserter(sel_bonds, sel_bonds.begin()),
                     [num_bonds](int i) { return i >= 0 && i < num_bonds; });
    }
    std::transform(sel_bonds.begin(), sel_bonds.end(), std::inserter(sel_atoms, sel_atoms.begin()), [&bmol](int i) { return bmol.getEdge(i).beg; });
    std::transform(sel_bonds.begin(), sel_bonds.end(), std::inserter(sel_atoms, sel_atoms.begin()), [&bmol](int i) { return bmol.getEdge(i).end; });

    const auto& ct = check_types.size() ? check_types : check_names_map.all;
    std::set<CheckTypeCode> ct_uniq;
    std::copy(ct.begin(), ct.end(), std::inserter(ct_uniq, ct_uniq.end()));

    const auto& checkers = check_names_map.checkers;
    std::for_each(ct_uniq.begin(), ct_uniq.end(), [&checkers, &bmol, &sel_atoms, &sel_bonds, &result](CheckTypeCode code) {
        checkers.at((int)code)((BaseMolecule&)bmol, sel_atoms, sel_bonds, result);
    });
    return result;
}

StructureChecker::CheckResult StructureChecker::checkReaction(const BaseReaction& reaction, const std::string& check_types)
{
    auto pars = check_params_from_string(check_types);
    return checkReaction(reaction, pars.check_types);
}

StructureChecker::CheckResult StructureChecker::checkReaction(const BaseReaction& reaction, const std::vector<CheckTypeCode>& check_types)
{
    CheckResult r;
    bool query = ((BaseReaction&)reaction).isQueryReaction();
    BaseReaction* brxn = query ? (BaseReaction*)&((BaseReaction&)reaction).asQueryReaction() : (BaseReaction*)&((BaseReaction&)reaction).asReaction();

#define CHECK_REACTION_COMPONENT(KIND)                                                                                                                         \
    for (auto i = brxn->KIND##Begin(); i < brxn->KIND##End(); i = brxn->KIND##Next(i))                                                                         \
    {                                                                                                                                                          \
        CheckResult res = checkMolecule(brxn->getBaseMolecule(i), check_types);                                                                                \
        if (!res.isEmpty())                                                                                                                                    \
        {                                                                                                                                                      \
            message(r, StructureChecker::CheckMessageCode::CHECK_MSG_REACTION, i, res);                                                                        \
        }                                                                                                                                                      \
    }
    CHECK_REACTION_COMPONENT(reactant)
    CHECK_REACTION_COMPONENT(product)
    CHECK_REACTION_COMPONENT(catalyst)
#undef CHECK_REACTION_COMPONENT
    return r;
}

StructureChecker::CheckMessage::CheckMessage()
{
}

StructureChecker::CheckMessage::CheckMessage(StructureChecker::CheckMessageCode _code, int _index, const std::vector<int>& _ids,
                                             const StructureChecker::CheckResult& _subresult)
    : code(_code), index(_index), ids(_ids), subresult(_subresult)
{
    std::sort(ids.begin(), ids.end());
}

std::string StructureChecker::CheckMessage::message()
{
    return StructureChecker::getCheckMessage(code);
}
