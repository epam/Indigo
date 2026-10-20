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

// Characterization tests for indigo::MoleculeSGroups and
// indigo::Superatom::attachment_points.
//
// Rationale (task #3766): MoleculeSGroups::remove() and
// Superatom::attachment_points.remove() are slot-reuse paths only exercised
// indirectly by format loaders. Written against the legacy PtrPool<SGroup> /
// ObjPool<_AttachmentPoint>, they pin the reuse contract the PtrReusablePool
// migration preserves: index allocation, LIFO reuse, per-type counting,
// iteration over holes, and that a non-trivial element type with an
// Array<char> member round-trips construct/destruct through the pool.

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <utility>
#include <vector>

#include <base_cpp/exception.h>
#include <molecule/elements.h>
#include <molecule/molecule.h>
#include <molecule/molecule_sgroups.h>

using namespace indigo;

// ---- MoleculeSGroups ------------------------------------------------------

TEST(MoleculeSGroupsContract, AddAssignsIndicesAndCountsByType)
{
    MoleculeSGroups sg;
    EXPECT_EQ(0, sg.getSGroupCount());
    int a = sg.addSGroup(SGroup::SG_TYPE_GEN);
    int b = sg.addSGroup(SGroup::SG_TYPE_DAT);
    int c = sg.addSGroup(SGroup::SG_TYPE_GEN);
    EXPECT_EQ(0, a);
    EXPECT_EQ(1, b);
    EXPECT_EQ(2, c);
    EXPECT_EQ(3, sg.getSGroupCount());
    EXPECT_EQ(2, sg.getSGroupCount(SGroup::SG_TYPE_GEN));
    EXPECT_EQ(1, sg.getSGroupCount(SGroup::SG_TYPE_DAT));
}

// Core golden-master: remove() frees the slot; hasSGroup reflects it; a later
// addSGroup reuses the freed index (LIFO) with a freshly constructed SGroup
// (no stale atoms from the removed one).
TEST(MoleculeSGroupsContract, RemoveFreesSlotAndAddReusesFreshInstance)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN);         // 0
    int b = sg.addSGroup(SGroup::SG_TYPE_GEN); // 1
    sg.addSGroup(SGroup::SG_TYPE_GEN);         // 2
    sg.getSGroup(b).atoms.push(42);
    ASSERT_EQ(3, sg.getSGroupCount());

    sg.remove(b);
    EXPECT_EQ(2, sg.getSGroupCount());
    EXPECT_FALSE(sg.hasSGroup(b));

    int reused = sg.addSGroup(SGroup::SG_TYPE_GEN);
    EXPECT_EQ(b, reused); // LIFO slot reuse
    EXPECT_TRUE(sg.hasSGroup(reused));
    EXPECT_EQ(0, sg.getSGroup(reused).atoms.size()); // fresh, not stale {42}
    EXPECT_EQ(3, sg.getSGroupCount());
}

// COM, MIX, FOR and GEN are objects of one class and share a reuse bucket: what reuse() leaves
// behind on a removed component surfaces on an unrelated group.
TEST(MoleculeSGroupsContract, ReusedSlotForgetsConnectivityAndComponentOrder)
{
    MoleculeSGroups sg;
    int component = sg.addSGroup(SGroup::SG_TYPE_COM);
    int copolymer = sg.addSGroup(SGroup::SG_TYPE_COP);
    sg.getSGroup(component).component_order = 5;
    ASSERT_TRUE(sg.getSGroup(component).hasComponentOrder());
    sg.getSGroup(copolymer).connectivity = SGroup::HEAD_TO_TAIL;
    sg.getSGroup(copolymer).sgroup_subtype = SGroup::SG_SUBTYPE_BLO;
    const SGroup* removed_component = &sg.getSGroup(component);
    const SGroup* removed_copolymer = &sg.getSGroup(copolymer);

    sg.remove(component);
    sg.remove(copolymer);

    SGroup& mixture = sg.getSGroup(sg.addSGroup(SGroup::SG_TYPE_MIX));
    SGroup& fresh_copolymer = sg.getSGroup(sg.addSGroup(SGroup::SG_TYPE_COP));
    // Without reuse there is nothing to forget: the new groups must be the objects just removed.
    ASSERT_EQ(removed_component, &mixture);
    ASSERT_EQ(removed_copolymer, &fresh_copolymer);
    EXPECT_FALSE(mixture.hasComponentOrder());
    EXPECT_EQ(0, fresh_copolymer.connectivity);
    EXPECT_EQ(0, fresh_copolymer.sgroup_subtype.value());
}

TEST(MoleculeSGroupsContract, IterationSkipsRemovedSlots)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 0
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 1
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 2
    sg.remove(1);

    std::vector<int> visited;
    for (int i = sg.begin(); i != sg.end(); i = sg.next(i))
        visited.push_back(i);

    ASSERT_EQ(2u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
}

TEST(MoleculeSGroupsContract, RemoveOnFreedSlotThrows)
{
    MoleculeSGroups sg;
    int a = sg.addSGroup(SGroup::SG_TYPE_GEN);
    sg.remove(a);
    EXPECT_THROW(sg.remove(a), Exception);
}

TEST(MoleculeSGroupsContract, ClearRemovesAll)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN);
    sg.addSGroup(SGroup::SG_TYPE_DAT);
    sg.clear();
    EXPECT_EQ(0, sg.getSGroupCount());
    EXPECT_EQ(sg.begin(), sg.end());
}

// ---- Superatom::attachment_points (PtrReusablePool<_AttachmentPoint>) ------

namespace
{
    // Adds an attachment point the way the loaders do after migration:
    // push() then fill (equivalent to the legacy _AttachmentPoint(int) ctor).
    int addAp(Superatom& sup, int atom_id)
    {
        int idx = sup.attachment_points.add();
        auto& ap = sup.attachment_points[idx];
        ap.aidx = atom_id;
        ap.apid.push(0);
        return idx;
    }
} // namespace

TEST(SuperatomAttachmentPointsContract, AddRemoveReuseWithNonTrivialElement)
{
    Superatom sup;
    int a0 = 10, a1 = 11, a2 = 12;
    EXPECT_EQ(0, addAp(sup, a0));
    int i1 = addAp(sup, a1);
    EXPECT_EQ(1, i1);
    EXPECT_EQ(2, addAp(sup, a2));
    EXPECT_EQ(3, sup.attachment_points.size());

    // aidx set, one apid entry pushed.
    EXPECT_EQ(11, sup.attachment_points[i1].aidx);
    EXPECT_EQ(1, sup.attachment_points[i1].apid.size());

    sup.attachment_points.remove(i1);
    EXPECT_EQ(2, sup.attachment_points.size());
    EXPECT_FALSE(sup.attachment_points.hasElement(i1));

    int reused = addAp(sup, a1);
    EXPECT_EQ(i1, reused); // LIFO reuse
    EXPECT_EQ(11, sup.attachment_points[reused].aidx);
    EXPECT_EQ(1, sup.attachment_points[reused].apid.size()); // fresh element
}

// ---- repeat pattern, subtype and component order ---------------------------

namespace
{
    const int NO_SUBTYPE = 0;
    const int NO_CONNECTIVITY = 0;
    const int NOT_NUMBERED = 0;
    const int NOT_A_CODE = -1;
    const int NO_PARENT = -1;
    const int FIRST_COMPONENT_ORDER = 1;
    const int SECOND_COMPONENT_ORDER = 2;

    // {type, subtype, connectivity, component order} of an S-group.
    using PolymerFields = std::array<int, 4>;

    const PolymerFields FORMULATION = {SGroup::SG_TYPE_FOR, NO_SUBTYPE, NO_CONNECTIVITY, NOT_NUMBERED};
    const PolymerFields FIRST_COMPONENT = {SGroup::SG_TYPE_COM, NO_SUBTYPE, NO_CONNECTIVITY, FIRST_COMPONENT_ORDER};
    const PolymerFields SECOND_COMPONENT = {SGroup::SG_TYPE_COM, NO_SUBTYPE, NO_CONNECTIVITY, SECOND_COMPONENT_ORDER};
    const PolymerFields BLOCK_COPOLYMER = {SGroup::SG_TYPE_COP, SGroup::SG_SUBTYPE_BLO, SGroup::HEAD_TO_TAIL, NOT_NUMBERED};
    const PolymerFields HEAD_TO_TAIL_UNIT = {SGroup::SG_TYPE_SRU, NO_SUBTYPE, SGroup::HEAD_TO_TAIL, NOT_NUMBERED};
    const PolymerFields HEAD_TO_HEAD_UNIT = {SGroup::SG_TYPE_SRU, NO_SUBTYPE, SGroup::HEAD_TO_HEAD, NOT_NUMBERED};

    std::vector<PolymerFields> sorted(std::vector<PolymerFields> fields)
    {
        std::sort(fields.begin(), fields.end());
        return fields;
    }

    int addSGroup(Molecule& mol, const PolymerFields& fields, std::initializer_list<int> atoms, int parent_idx)
    {
        const int sgroup_idx = mol.sgroups.addSGroup(fields[0]);
        SGroup& sgroup = mol.sgroups.getSGroup(sgroup_idx);
        sgroup.sgroup_subtype = fields[1];
        sgroup.connectivity = fields[2];
        sgroup.component_order = fields[3];
        sgroup.parent_idx = parent_idx;
        for (int atom_idx : atoms)
            sgroup.atoms.push(atom_idx);
        return sgroup_idx;
    }

    // C-C-C-C and a separate O as a formulation of two numbered components. The first component holds a
    // head-to-tail block copolymer of two repeating units; the second is the oxygen.
    void buildPolymerFormulation(Molecule& mol)
    {
        const int chain_length = 4;
        for (int atom_idx = 0; atom_idx < chain_length; atom_idx++)
            mol.addAtom(ELEM_C);
        for (int atom_idx = 1; atom_idx < chain_length; atom_idx++)
            mol.addBond(atom_idx - 1, atom_idx, BOND_SINGLE);
        mol.addAtom(ELEM_O);

        const int formulation = addSGroup(mol, FORMULATION, {0, 1, 2, 3, 4}, NO_PARENT);
        const int first_component = addSGroup(mol, FIRST_COMPONENT, {0, 1, 2, 3}, formulation);
        addSGroup(mol, SECOND_COMPONENT, {4}, formulation);
        const int copolymer = addSGroup(mol, BLOCK_COPOLYMER, {1, 2}, first_component);
        addSGroup(mol, HEAD_TO_TAIL_UNIT, {1}, copolymer);
        addSGroup(mol, HEAD_TO_HEAD_UNIT, {2}, copolymer);
    }

    // The fields of every S-group that has atoms, sorted: a copy is free to number the groups differently.
    std::vector<PolymerFields> polymerFields(Molecule& mol)
    {
        std::vector<PolymerFields> fields;
        for (int sgroup_idx = mol.sgroups.begin(); sgroup_idx != mol.sgroups.end(); sgroup_idx = mol.sgroups.next(sgroup_idx))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(sgroup_idx);
            if (sgroup.atoms.size() > 0)
                fields.push_back({sgroup.sgroup_type, sgroup.sgroup_subtype.value(), sgroup.connectivity, sgroup.component_order});
        }
        return sorted(fields);
    }
} // namespace

TEST(SGroupPolymerFields, CloneAndMergeKeepThem)
{
    const std::vector<PolymerFields> formulation =
        sorted({FORMULATION, FIRST_COMPONENT, SECOND_COMPONENT, BLOCK_COPOLYMER, HEAD_TO_TAIL_UNIT, HEAD_TO_HEAD_UNIT});

    Molecule source;
    buildPolymerFormulation(source);
    ASSERT_EQ(formulation, polymerFields(source));

    Molecule cloned;
    cloned.clone(source);
    EXPECT_EQ(formulation, polymerFields(cloned));

    Molecule merged;
    merged.addAtom(ELEM_N);
    merged.mergeWithMolecule(source, nullptr);
    EXPECT_EQ(formulation, polymerFields(merged));
}

TEST(SGroupPolymerFields, SubmoleculeOfAComponentKeepsThem)
{
    Molecule source;
    buildPolymerFormulation(source);

    Molecule first_component;
    Array<int> first_component_atoms;
    for (int atom_idx : {0, 1, 2, 3})
        first_component_atoms.push(atom_idx);
    first_component.makeSubmolecule(source, first_component_atoms, nullptr);

    EXPECT_EQ(sorted({FORMULATION, FIRST_COMPONENT, BLOCK_COPOLYMER, HEAD_TO_TAIL_UNIT, HEAD_TO_HEAD_UNIT}), polymerFields(first_component));
}

TEST(SGroupPolymerFields, SubtypeAndConnectivityHaveMolfileCodes)
{
    const std::pair<int, const char*> subtypes[] = {{SGroup::SG_SUBTYPE_ALT, "ALT"}, {SGroup::SG_SUBTYPE_RAN, "RAN"}, {SGroup::SG_SUBTYPE_BLO, "BLO"}};
    for (const auto& [subtype, code] : subtypes)
    {
        EXPECT_STREQ(code, SGroup::subtypeToString(subtype));
        EXPECT_EQ(subtype, SGroup::getSubtype(code));
    }
    EXPECT_EQ(nullptr, SGroup::subtypeToString(NO_SUBTYPE));
    EXPECT_EQ(NOT_A_CODE, SGroup::getSubtype("BLOCK"));

    const char* const either_unknown = "EU";
    const std::pair<int, const char*> connectivities[] = {{SGroup::HEAD_TO_HEAD, "HH"}, {SGroup::HEAD_TO_TAIL, "HT"}, {SGroup::EITHER, either_unknown}};
    for (const auto& [connectivity, code] : connectivities)
    {
        EXPECT_STREQ(code, SGroup::connectivityToString(connectivity));
        EXPECT_EQ(connectivity, SGroup::getConnectivity(code));
    }
    EXPECT_STREQ(either_unknown, SGroup::connectivityToString(NO_CONNECTIVITY));
    EXPECT_EQ(NOT_A_CODE, SGroup::getConnectivity("XX"));

    for (int sg_type : {SGroup::SG_TYPE_SRU, SGroup::SG_TYPE_COP})
        EXPECT_TRUE(SGroup::typeHasConnectivity(sg_type)) << SGroup::typeToString(sg_type);
    for (int sg_type :
         {SGroup::SG_TYPE_GEN, SGroup::SG_TYPE_DAT, SGroup::SG_TYPE_SUP, SGroup::SG_TYPE_MUL, SGroup::SG_TYPE_COM, SGroup::SG_TYPE_MIX, SGroup::SG_TYPE_FOR})
        EXPECT_FALSE(SGroup::typeHasConnectivity(sg_type)) << SGroup::typeToString(sg_type);
}
