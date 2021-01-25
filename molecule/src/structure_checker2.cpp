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

#include "molecule/structure_checker2.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/base_reaction.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <string>

using namespace indigo;

StructureChecker2::StructureChecker2()
{
}

bool StructureChecker2::CheckResult::isEmpty() const
{
    return messages.size() == 0;
}

static void message(StructureChecker2::CheckResult& result, StructureChecker2::CheckMessageCode code, int index, std::vector<int> ids,
                    const StructureChecker2::CheckResult& subresult)
{
    StructureChecker2::CheckMessage m = {code, index, ids, subresult};
    result.messages.push_back(m);
}

static void message(StructureChecker2::CheckResult& result, StructureChecker2::CheckMessageCode code, int index,
                    const StructureChecker2::CheckResult& subresult)
{
    message(result, code, index, std::vector<int>(), subresult);
}

static void message(StructureChecker2::CheckResult& result, StructureChecker2::CheckMessageCode code, const std::vector<int>& ids)
{
    message(result, code, -1, ids, StructureChecker2::CheckResult());
}

static void message(StructureChecker2::CheckResult& result, StructureChecker2::CheckMessageCode code, const std::unordered_set<int>& ids)
{
    std::vector<int> v;
    std::copy(ids.begin(), ids.end(), std::back_inserter(v));
    message(result, code, v);
}

static void message(StructureChecker2::CheckResult& result, StructureChecker2::CheckMessageCode code)
{
    message(result, code, -1, std::vector<int>(), StructureChecker2::CheckResult());
}

/**************************************************/
static void filter_atoms(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, StructureChecker2::CheckResult& result,
                         StructureChecker2::CheckMessageCode msg, const std::function<bool(BaseMolecule&, int)>& filter, bool default_filter = true)
{
    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids), [&mol, &filter, default_filter](int idx) {
        return (!default_filter || !mol.isPseudoAtom(idx) && !mol.isRSite(idx) && !mol.isTemplateAtom(idx)) && filter(mol, idx);
    });
    if (ids.size())
    {
        message(result, msg, ids);
    }
}
//

static void check_load(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker2::CheckResult& result)
{
    if (mol.vertexCount() == 0)
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_EMPTY);
    }
}

#define FILTER_ATOMS(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER, false);
#define FILTER_ATOMS_DEFAULT(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER);

static void check_valence(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{

    if (mol.isQueryMolecule())
    {
        message(
            result,
            StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_QUERY); // 'Structure contains query features, so valency could not be checked'
    }
    else if (mol.hasRGroups())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_RGROUP); // 'Structure contains RGroup components, so valency could
    }
    else if (!mol.isQueryMolecule() && mol.asMolecule().getIgnoreBadValenceFlag())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_IGNORE_VALENCE_ERROR);
    }
    else
    {
        FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE,
                     [](BaseMolecule& mol, int idx) { return mol.getAtomValence_NoThrow(idx, -1) == -1; });
    }
}

static void check_radical(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{
    if (mol.hasPseudoAtoms())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_RADICAL_NOT_CHECKED_PSEUDO);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_RADICAL,
                             [](BaseMolecule& mol, int idx) { return mol.getAtomRadical_NoThrow(idx, -1) > 0; });
    }
}
static void check_pseudoatom(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                             StructureChecker2::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_PSEUDOATOM, [](BaseMolecule& mol, int idx) { return mol.isPseudoAtom(idx); });
}
static void check_stereo(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (!mol.isQueryMolecule())
    {
        std::unique_ptr<Molecule> target(new Molecule);
        auto saved_valence_flag = mol.asMolecule().getIgnoreBadValenceFlag();
        mol.asMolecule().setIgnoreBadValenceFlag(true);
        target->clone_KeepIndices(mol);

        for (const auto i : target->vertices())
        {
            if (!target->stereocenters.exists(i) && target->stereocenters.isPossibleStereocenter(i))
            {
                try
                {
                    target->stereocenters.add(i, MoleculeStereocenters::ATOM_ABS, 0, false);
                }
                catch (Exception& e)
                {
                    e.code();
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

        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_3D_STEREO, [](BaseMolecule& mol, int idx) {
            bool stereo_3d = true;
            if (BaseMolecule::hasZCoord(mol) && mol.stereocenters.exists(idx))
            {
                const Vertex& vertex = mol.getVertex(idx);
                for (auto j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                    if (mol.getBondDirection2(idx, vertex.neiVertex(j)) > 0)
                        stereo_3d = false;
            }
            return stereo_3d;
        });

        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_WRONG_STEREO, [&target](BaseMolecule& mol, int idx) {
            return mol.stereocenters.exists(idx) && target->stereocenters.exists(idx) && mol.stereocenters.getType(idx) != target->stereocenters.getType(idx) ||
                   mol.stereocenters.exists(idx) && !target->stereocenters.exists(idx);
        });
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO,
                             [&target](BaseMolecule& mol, int idx) { return !mol.stereocenters.exists(idx) && target->stereocenters.exists(idx); });

        mol.asMolecule().setIgnoreBadValenceFlag(saved_valence_flag);
    }
}
static void check_query(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker2::CheckResult& result)
{
    if (mol.isQueryMolecule())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY);
    }
    FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY_ATOM,
                         [](BaseMolecule& mol, int idx) { return mol.reaction_atom_exact_change[idx] || mol.reaction_atom_inversion[idx]; });

    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids),
                 [&mol](int idx) { return idx >= 0 && idx < mol.reaction_bond_reacting_center.size() && mol.reaction_bond_reacting_center[idx] != 0; });
    if (ids.size())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY_BOND, ids);
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
                               StructureChecker2::CheckResult& result)
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
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_OVERLAP_ATOM, ids);
    }
}
static void check_overlap_bond(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                               StructureChecker2::CheckResult& result)
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
            message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_OVERLAP_BOND, ids);
        }
    }
}

static void check_rgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (!mol.isQueryMolecule() && mol.hasRGroups())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_RGROUP);
    }
}

static void check_sgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (mol.sgroups.getSGroupCount() > 0)
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_SGROUP);
    }
}

static void check_tgroup(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (mol.tgroups.getTGroupCount() > 0)
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_TGROUP);
    }
}

static void check_chirality(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                            StructureChecker2::CheckResult& result)
{
    if (mol.isChiral())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_CHIRALITY);
    }
}

static void check_chiral_flag(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                              StructureChecker2::CheckResult& result)
{
    if (mol.getChiralFlag() > 0 && mol.stereocenters.size() == 0)
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_CHIRAL_FLAG);
    }
}

static void check_3d_coord(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                           StructureChecker2::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_3D_COORD, [](BaseMolecule& mol, int idx) { return fabs(mol.getAtomXyz(idx).z) > 0.001; });
}

static void check_charge(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (std::accumulate(selected_atoms.begin(), selected_atoms.end(), 0, [&mol](int sum, int idx) { return sum + mol.getAtomCharge(idx); }))
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_CHARGE);
    }
}

static void check_salt(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker2::CheckResult& result)
{
    // not impl
    message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_SALT_NOT_IMPL);
}

static void check_ambigous_h(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                             StructureChecker2::CheckResult& result)
{
    if (mol.isQueryMolecule())
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H_NOT_CHECKED_QUERY);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H, [](BaseMolecule& mol, int idx) {
            return mol.asMolecule().getImplicitH_NoThrow(idx, -1) == -1 && mol.getAtomAromaticity(idx) == ATOM_AROMATIC;
        });
    }
}

static void check_coord(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker2::CheckResult& result)
{
    if (mol.vertexCount() > 1 && !BaseMolecule::hasCoord(mol))
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_ZERO_COORD);
    }
}

static void check_v3000(BaseMolecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker2::CheckResult& result)
{
    if (mol.hasHighlighting() || (!mol.stereocenters.haveAllAbsAny() && !mol.stereocenters.haveAllAndAny()) || mol.vertexCount() > 999 || mol.edgeCount() > 999)
    {
        message(result, StructureChecker2::CheckMessageCode::CHECK_MSG_V3000);
    }
}
#undef FILTER_ATOMS
#undef FILTER_ATOMS_DEFAULT

static void (*check_type_checkers[])(BaseMolecule&, const std::unordered_set<int>&, const std::unordered_set<int>&, StructureChecker2::CheckResult&) = {
    &check_load,         &check_valence, &check_radical,    &check_pseudoatom, &check_stereo,    &check_query,       &check_overlap_atom,
    &check_overlap_bond, &check_rgroup,  &check_sgroup,     &check_tgroup,     &check_chirality, &check_chiral_flag, &check_3d_coord,
    &check_charge,       &check_salt,    &check_ambigous_h, &check_coord,      &check_v3000};

StructureChecker2::CheckResult StructureChecker2::checkMolecule(const BaseMolecule& bmol, int check_types, const std::vector<int>& selected_atoms,
                                                                const std::vector<int>& selected_bonds)
{
    StructureChecker2::CheckResult result;

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

    Molecule& mol = ((BaseMolecule&)bmol).isQueryMolecule() ? (Molecule&)((BaseMolecule&)bmol).asQueryMolecule() : ((BaseMolecule&)bmol).asMolecule();
    for (int i = 0; i < sizeof(check_type_checkers) / sizeof(*check_type_checkers); i++)
    {
        if (check_types & (1 << i))
        {
            check_type_checkers[i](mol, sel_atoms, sel_bonds, result);
        }
    }
    return result;
}

StructureChecker2::CheckResult StructureChecker2::checkReaction(const BaseReaction& reaction, int check_types)
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
            message(r, StructureChecker2::CheckMessageCode::CHECK_MSG_REACTION, i, res);                                                                       \
        }                                                                                                                                                      \
    }
    CHECK_REACTION_COMPONENT(reactant)
    CHECK_REACTION_COMPONENT(product)
    CHECK_REACTION_COMPONENT(catalyst)
#undef CHECK_REACTION_COMPONENT
    return r;
}

static const std::string message_list[] = {"",
                                           "Error at loading structure, wrong format found",
                                           "Structure contains atoms with unusuall valence",
                                           "Structure contains query features, so valency could not be checked",
                                           "Structure contains RGroup components, so valency could not be checked",
                                           "IGNORE_BAD_VALENCE flag is active",
                                           "Structure contains radicals",
                                           "Structure contains pseudoatoms, so radicals could not be checked",
                                           "Structure contains pseudoatoms",
                                           "Structure contains wrong chiral flag",
                                           "Structure contains incorrect stereochemistry",
                                           "Structure contains stereocenters defined by 3D coordinates",
                                           "Structure contains stereocenters with undefined stereo configuration",
                                           "CHECK_MSG_IGNORE_STEREO_ERROR",
                                           "Structure contains query features",
                                           "Structure contains query features for atoms",
                                           "Structure contains query features for bonds",
                                           "CHECK_MSG_IGNORE_QUERY_FEATURE",
                                           "Structure contains overlapping atoms",
                                           "Structure contains overlapping bonds.",
                                           "Structure contains R-groups",
                                           "Structure contains S-groups",
                                           "Structure contains SCSR templates",
                                           "Structure has non-zero charge",
                                           "Structure contains charged fragments (possible salt)",
                                           "Input structure is empty",
                                           "Structure contains ambiguous hydrogens",
                                           "Structure contains query features, so ambiguous H could not be checked",
                                           "Structure contains 3D coordinates",
                                           "Structure has no atoms coordinates",
                                           "Reaction component check result",
                                           "Structure contains chirality",
                                           "Not implemented yet: check salt",
                                           "Structure supports only Molfile V3000"

};

std::string StructureChecker2::CheckMessage::message()
{
    return message_list[(size_t)code];
}
