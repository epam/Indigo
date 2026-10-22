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
#include <base_cpp/tree.h>
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
    const PolymerFields GENERIC = {SGroup::SG_TYPE_GEN, NO_SUBTYPE, NO_CONNECTIVITY, NOT_NUMBERED};

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

    PolymerFields fieldsOf(SGroup& sgroup)
    {
        return {sgroup.sgroup_type, sgroup.sgroup_subtype.value(), sgroup.connectivity, sgroup.component_order};
    }

    // The fields of every S-group that has atoms, sorted: a copy is free to number the groups differently.
    std::vector<PolymerFields> polymerFields(Molecule& mol)
    {
        std::vector<PolymerFields> fields;
        for (int sgroup_idx = mol.sgroups.begin(); sgroup_idx != mol.sgroups.end(); sgroup_idx = mol.sgroups.next(sgroup_idx))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(sgroup_idx);
            if (sgroup.atoms.size() > 0)
                fields.push_back(fieldsOf(sgroup));
        }
        return sorted(fields);
    }

    using ParentLink = std::pair<PolymerFields, PolymerFields>; // {child, parent}

    // Every nested group with its parent, by fields and sorted, as the savers resolve the parents.
    std::vector<ParentLink> parentLinks(Molecule& mol)
    {
        std::vector<ParentLink> links;
        const std::vector<int> parents = mol.sgroups.resolveParents();
        for (int sgroup_idx = mol.sgroups.begin(); sgroup_idx != mol.sgroups.end(); sgroup_idx = mol.sgroups.next(sgroup_idx))
        {
            if (parents[sgroup_idx] >= 0)
                links.push_back({fieldsOf(mol.sgroups.getSGroup(sgroup_idx)), fieldsOf(mol.sgroups.getSGroup(parents[sgroup_idx]))});
        }
        std::sort(links.begin(), links.end());
        return links;
    }

    std::vector<ParentLink> formulationParentLinks()
    {
        std::vector<ParentLink> links = {{FIRST_COMPONENT, FORMULATION},
                                         {SECOND_COMPONENT, FORMULATION},
                                         {BLOCK_COPOLYMER, FIRST_COMPONENT},
                                         {HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER},
                                         {HEAD_TO_HEAD_UNIT, BLOCK_COPOLYMER}};
        std::sort(links.begin(), links.end());
        return links;
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

// ---- parent links ---------------------------------------------------------

TEST(SGroupParentLinks, MergeIntoAMoleculeWithAnSGroupKeepsThem)
{
    Molecule source;
    buildPolymerFormulation(source);
    ASSERT_EQ(formulationParentLinks(), parentLinks(source));

    Molecule merged;
    merged.addAtom(ELEM_N);
    addSGroup(merged, GENERIC, {0}, NO_PARENT);
    merged.mergeWithMolecule(source, nullptr);

    ASSERT_EQ(1 + source.sgroups.getSGroupCount(), merged.sgroups.getSGroupCount());
    EXPECT_EQ(formulationParentLinks(), parentLinks(merged));
}

// A molfile reader links a group both ways: by pool position and by the index of the parent.
TEST(SGroupParentLinks, AChildCopiedWithoutItsParentHasNone)
{
    const int parent_index = 5;
    Molecule source;
    source.addAtom(ELEM_C);
    source.addAtom(ELEM_O);
    const int parent = addSGroup(source, GENERIC, {0}, NO_PARENT);
    source.sgroups.getSGroup(parent).index = parent_index;
    const int child_idx = addSGroup(source, GENERIC, {1}, parent);
    source.sgroups.getSGroup(child_idx).parent_group = parent_index;

    Molecule oxygen;
    Array<int> oxygen_atoms;
    oxygen_atoms.push(1);
    oxygen.makeSubmolecule(source, oxygen_atoms, nullptr);

    ASSERT_EQ(1, oxygen.sgroups.getSGroupCount());
    const SGroup& child = oxygen.sgroups.getSGroup(oxygen.sgroups.begin());
    EXPECT_EQ(NO_PARENT, oxygen.sgroups.resolveParents()[oxygen.sgroups.begin()]);
    EXPECT_EQ(0, child.parent_group.value_or(0));
}

// A child with atoms of its own goes with its atoms: it is not copied, empty, along with its parent.
TEST(SGroupParentLinks, AChildWithAtomsElsewhereStaysOutOfItsParentsCopy)
{
    Molecule source;
    source.addAtom(ELEM_C);
    source.addAtom(ELEM_O);
    const int parent = addSGroup(source, GENERIC, {0}, NO_PARENT);
    addSGroup(source, GENERIC, {1}, parent);

    Molecule carbon;
    Array<int> carbon_atoms;
    carbon_atoms.push(0);
    carbon.makeSubmolecule(source, carbon_atoms, nullptr);

    ASSERT_EQ(1, carbon.sgroups.getSGroupCount());
    EXPECT_EQ(1, carbon.sgroups.getSGroup(carbon.sgroups.begin()).atoms.size());
}

// The CML reader names a parent only by its one-based position, and a merge moves the groups to new positions.
TEST(SGroupParentLinks, ALinkByPositionSurvivesAMerge)
{
    Molecule source;
    for (int atom_idx = 0; atom_idx < 2; atom_idx++)
        source.addAtom(ELEM_C);
    source.addBond(0, 1, BOND_SINGLE);
    addSGroup(source, BLOCK_COPOLYMER, {0, 1}, NO_PARENT);
    const int unit = addSGroup(source, HEAD_TO_TAIL_UNIT, {1}, NO_PARENT);
    source.sgroups.getSGroup(unit).parent_group = 1;
    ASSERT_EQ(std::vector<ParentLink>({{HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER}}), parentLinks(source));

    Molecule merged;
    merged.addAtom(ELEM_N);
    addSGroup(merged, GENERIC, {0}, NO_PARENT);
    merged.mergeWithMolecule(source, nullptr);

    ASSERT_EQ(3, merged.sgroups.getSGroupCount());
    EXPECT_EQ(std::vector<ParentLink>({{HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER}}), parentLinks(merged));
}

// The C API sets a parent by its index only.
TEST(SGroupParentLinks, ALinkByIndexSurvivesAMerge)
{
    const int copolymer_index = 5;
    Molecule source;
    for (int atom_idx = 0; atom_idx < 2; atom_idx++)
        source.addAtom(ELEM_C);
    source.addBond(0, 1, BOND_SINGLE);
    const int copolymer = addSGroup(source, BLOCK_COPOLYMER, {0, 1}, NO_PARENT);
    source.sgroups.getSGroup(copolymer).index = copolymer_index;
    const int unit = addSGroup(source, HEAD_TO_TAIL_UNIT, {1}, NO_PARENT);
    source.sgroups.getSGroup(unit).parent_group = copolymer_index;
    ASSERT_EQ(std::vector<ParentLink>({{HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER}}), parentLinks(source));

    Molecule merged;
    merged.addAtom(ELEM_N);
    addSGroup(merged, GENERIC, {0}, NO_PARENT);
    merged.mergeWithMolecule(source, nullptr);

    ASSERT_EQ(3, merged.sgroups.getSGroupCount());
    EXPECT_EQ(std::vector<ParentLink>({{HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER}}), parentLinks(merged));
}

TEST(SGroupParentLinks, RemovingAParentMovesItsChildrenUp)
{
    Molecule mol;
    buildPolymerFormulation(mol);
    int copolymer = -1;
    for (int sgroup_idx = mol.sgroups.begin(); sgroup_idx != mol.sgroups.end(); sgroup_idx = mol.sgroups.next(sgroup_idx))
    {
        if (mol.sgroups.getSGroup(sgroup_idx).sgroup_type == SGroup::SG_TYPE_COP)
            copolymer = sgroup_idx;
    }
    ASSERT_GE(copolymer, 0);

    mol.removeSGroup(copolymer);

    std::vector<ParentLink> expected = {
        {FIRST_COMPONENT, FORMULATION}, {SECOND_COMPONENT, FORMULATION}, {HEAD_TO_TAIL_UNIT, FIRST_COMPONENT}, {HEAD_TO_HEAD_UNIT, FIRST_COMPONENT}};
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(expected, parentLinks(mol));
}

TEST(SGroupParentLinks, AGroupInTheSlotOfARemovedParentIsNoParent)
{
    MoleculeSGroups sgroups;
    const int parent = sgroups.addSGroup(SGroup::SG_TYPE_GEN);
    const int child = sgroups.addSGroup(SGroup::SG_TYPE_GEN);
    sgroups.getSGroup(child).parent_idx = parent;

    sgroups.remove(parent);
    const int newcomer = sgroups.addSGroup(SGroup::SG_TYPE_GEN);

    // Without reuse the newcomer could not be mistaken for the parent.
    ASSERT_EQ(parent, newcomer);
    EXPECT_EQ(NO_PARENT, sgroups.resolveParents()[child]);
}

// A group without an index names no parent by it: removing one must not hand its parent to every root group.
TEST(SGroupParentLinks, RemovingAGroupWithoutAnIndexReparentsNobody)
{
    Molecule mol;
    for (int atom_idx = 0; atom_idx < 3; atom_idx++)
        mol.addAtom(ELEM_C);
    addSGroup(mol, BLOCK_COPOLYMER, {0, 1}, NO_PARENT);
    const int unit = addSGroup(mol, HEAD_TO_TAIL_UNIT, {1}, NO_PARENT);
    mol.sgroups.getSGroup(unit).parent_group = 1; // the copolymer, by one-based position
    addSGroup(mol, GENERIC, {2}, NO_PARENT);
    ASSERT_EQ(std::vector<ParentLink>({{HEAD_TO_TAIL_UNIT, BLOCK_COPOLYMER}}), parentLinks(mol));

    mol.removeSGroup(unit);

    EXPECT_EQ(std::vector<ParentLink>(), parentLinks(mol));
}

// ---- tree of S-groups -----------------------------------------------------

namespace
{
    void collectLabels(Tree& node, std::vector<int>& labels)
    {
        for (int i = 0; i < node.children().size(); i++)
        {
            labels.push_back(node.children()[i].label);
            collectLabels(node.children()[i], labels);
        }
    }
} // namespace

TEST(SGroupTree, AChildListedBeforeItsParentGivesOneNodePerGroup)
{
    MoleculeSGroups sgroups;
    const int child = sgroups.addSGroup(SGroup::SG_TYPE_SRU);
    const int parent = sgroups.addSGroup(SGroup::SG_TYPE_GEN);
    sgroups.getSGroup(child).parent_idx = parent;

    Tree tree;
    sgroups.buildTree(tree);

    std::vector<int> labels;
    collectLabels(tree, labels);
    std::sort(labels.begin(), labels.end());
    EXPECT_EQ(std::vector<int>({child, parent}), labels);
    ASSERT_EQ(1, tree.children().size());
    Tree& parent_node = tree.children()[0];
    EXPECT_EQ(parent, parent_node.label);
    ASSERT_EQ(1, parent_node.children().size());
    EXPECT_EQ(child, parent_node.children()[0].label);
}

TEST(SGroupTree, ACycleOfParentsGivesOneNodePerGroup)
{
    MoleculeSGroups sgroups;
    const int first = sgroups.addSGroup(SGroup::SG_TYPE_GEN);
    const int second = sgroups.addSGroup(SGroup::SG_TYPE_GEN);
    sgroups.getSGroup(first).parent_idx = second;
    sgroups.getSGroup(second).parent_idx = first;

    Tree tree;
    sgroups.buildTree(tree);

    std::vector<int> labels;
    collectLabels(tree, labels);
    std::sort(labels.begin(), labels.end());
    EXPECT_EQ(std::vector<int>({first, second}), labels);
}
