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

// The KET side of haptic bonds (#3233, format #3838): a molecule node declares
// its attachment groups, and every haptic bond is a connection of the root.
// The wire contract is Task/3233/KET-CONTRACT.md; its two subtleties are pinned
// here — a haptic connection is keyed by "type" while the monomer ones use
// "connectionType", and every id is a string while the member atoms are numbers.

#include <gtest/gtest.h>

#include <base_cpp/output.h>
#include <layout/metalayout.h>
#include <molecule/molecule.h>
#include <molecule/molecule_json_loader.h>
#include <molecule/molecule_json_saver.h>
#include <reaction/reaction.h>
#include <reaction/reaction_json_loader.h>

#include "common.h"

using namespace indigo;

class IndigoCoreHapticKetTest : public IndigoCoreTest
{
protected:
    static void loadKet(const char* json, Molecule& mol)
    {
        rapidjson::Document data;
        ASSERT_FALSE(data.Parse(json).HasParseError());
        MoleculeJsonLoader loader(data);
        loader.loadMolecule(mol);
    }

    static std::string loadKetError(const char* json)
    {
        Molecule mol;
        try
        {
            rapidjson::Document data;
            if (data.Parse(json).HasParseError())
                return "the fixture itself is not valid JSON";
            MoleculeJsonLoader loader(data);
            loader.loadMolecule(mol);
        }
        catch (Exception& e)
        {
            return e.message();
        }
        return "";
    }

    static std::string saveKet(BaseMolecule& mol)
    {
        Array<char> buffer;
        ArrayOutput output(buffer);
        MoleculeJsonSaver saver(output);
        saver.saveMolecule(mol);
        return std::string(buffer.ptr(), static_cast<size_t>(buffer.size()));
    }

    static void loadKetReaction(const char* json, Reaction& rxn)
    {
        rapidjson::Document data;
        ASSERT_FALSE(data.Parse(json).HasParseError());
        LayoutOptions options;
        ReactionJsonLoader loader(data, options);
        loader.loadReaction(rxn);
    }

    static size_t countOccurrences(const std::string& text, const std::string& part)
    {
        size_t count = 0;
        for (size_t pos = text.find(part); pos != std::string::npos; pos = text.find(part, pos + part.size()))
            count++;
        return count;
    }

    // Ferrocene: two cyclopentadienyl rings, each an attachment group bonded to the
    // iron between them. Coordinates are given so that saving needs no layout.
    static void makeFerrocene(Molecule& mol)
    {
        static const float ring_x[5] = {0.0f, 0.95f, 0.59f, -0.59f, -0.95f};
        static const float ring_y[5] = {1.0f, 0.31f, -0.81f, -0.81f, 0.31f};

        for (int ring = 0; ring < 2; ring++)
        {
            const float shift = ring == 0 ? 2.5f : -2.5f;
            for (int i = 0; i < 5; i++)
                mol.setAtomXyz(mol.addAtom(ELEM_C), Vec3f(ring_x[i], shift + ring_y[i] * 0.5f, 0));
            for (int i = 0; i < 5; i++)
                mol.addBond(ring * 5 + i, ring * 5 + (i + 1) % 5, BOND_SINGLE);
        }

        const int metal = mol.addAtom(ELEM_Fe);
        mol.setAtomXyz(metal, Vec3f(0, 0, 0));

        for (int ring = 0; ring < 2; ring++)
        {
            const int group = mol.attachment_groups.addGroup();
            for (int i = 0; i < 5; i++)
                mol.attachment_groups.group(group).addAtom(ring * 5 + i);
            mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));
        }
    }
};

namespace
{
    const char* KET_RING_AND_METAL = R"({
        "root": {
            "nodes": [{"$ref": "mol0"}, {"$ref": "mol1"}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"attachmentGroupId": "7", "moleculeId": "mol1"}}
            ]
        },
        "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [0, 0, 0]}]},
        "mol1": {
            "type": "molecule",
            "atoms": [
                {"label": "C", "location": [1, 0, 0]}, {"label": "C", "location": [2, 0, 0]},
                {"label": "C", "location": [3, 0, 0]}, {"label": "C", "location": [4, 0, 0]},
                {"label": "C", "location": [5, 0, 0]}
            ],
            "bonds": [
                {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]}, {"type": 1, "atoms": [2, 3]},
                {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 0]}
            ],
            "attachmentGroups": [{"id": "7", "atoms": [0, 1, 2, 3, 4]}]
        }
    })";

    const char* KET_BONDED_PAIR_WITH_HAPTIC = R"({
        "root": {
            "nodes": [{"$ref": "mol0"}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"atomId": "1", "moleculeId": "mol0"}}
            ]
        },
        "mol0": {
            "type": "molecule",
            "atoms": [{"label": "C", "location": [0, 0, 0]}, {"label": "Fe", "location": [1, 0, 0]}],
            "bonds": [{"type": 1, "atoms": [0, 1]}]
        }
    })";

    // The two examples attached to ketcher#8390, with the molecule node that carries
    // the group added to root.nodes: the attached files list mol0 alone, which the
    // author confirmed to be a mistake in the example (KET-CONTRACT.md §3.3).
    const char* KET_REFERENCE_GROUP_TO_ATOM = R"({
        "ket_version": "2.0.0",
        "root": {
            "nodes": [{"$ref": "mol0"}, {"$ref": "mol1"}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"attachmentGroupId": "0", "moleculeId": "mol1"}}
            ],
            "templates": []
        },
        "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [3.383987, -5.449992, 0]}], "bonds": []},
        "mol1": {
            "type": "molecule",
            "atoms": [
                {"label": "C", "location": [3.383987, -5.449992, 0]}, {"label": "C", "location": [3.383987, -6.450007, 0]},
                {"label": "C", "location": [4.25, -6.950014, 0]},     {"label": "C", "location": [5.116012, -6.450007, 0]},
                {"label": "C", "location": [5.116012, -5.449992, 0]}, {"label": "C", "location": [4.25, -4.949985, 0]}
            ],
            "bonds": [
                {"type": 1, "atoms": [5, 0]}, {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]},
                {"type": 1, "atoms": [2, 3]}, {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 5]}
            ],
            "attachmentGroups": [{"id": "0", "atoms": [0, 1, 2, 3, 4, 5]}]
        }
    })";

    const char* KET_REFERENCE_ATOM_TO_ATOM = R"({
        "ket_version": "2.0.0",
        "root": {
            "nodes": [{"$ref": "mol0"}, {"$ref": "mol1"}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"atomId": "0", "moleculeId": "mol1"}}
            ],
            "templates": []
        },
        "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [3.383987, -5.449992, 0]}], "bonds": []},
        "mol1": {"type": "molecule", "atoms": [{"label": "C", "location": [3.383987, -6.450007, 0]}], "bonds": []}
    })";

    // Ferrocene as the reactant of a one-arrow reaction. Its three graph fragments -
    // the iron and the two rings - are held together only by the haptic bonds, so the
    // reaction loader has to count components with the attachment groups mixed in.
    const char* KET_REACTION_WITH_FERROCENE = R"({
        "root": {
            "nodes": [{"$ref": "mol0"}, {"$ref": "mol1"}, {"$ref": "mol2"}, {"$ref": "mol3"},
                      {"type": "arrow",
                       "data": {"mode": "open-angle", "pos": [{"x": 5, "y": 0, "z": 0}, {"x": 7, "y": 0, "z": 0}]}}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"attachmentGroupId": "0", "moleculeId": "mol1"}},
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"attachmentGroupId": "0", "moleculeId": "mol2"}}
            ]
        },
        "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [0, 0, 0]}], "bonds": []},
        "mol1": {
            "type": "molecule",
            "atoms": [
                {"label": "C", "location": [0.0, 2.5, 0]},   {"label": "C", "location": [0.95, 2.65, 0]},
                {"label": "C", "location": [0.59, 2.09, 0]}, {"label": "C", "location": [-0.59, 2.09, 0]},
                {"label": "C", "location": [-0.95, 2.65, 0]}
            ],
            "bonds": [
                {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]}, {"type": 1, "atoms": [2, 3]},
                {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 0]}
            ],
            "attachmentGroups": [{"id": "0", "atoms": [0, 1, 2, 3, 4]}]
        },
        "mol2": {
            "type": "molecule",
            "atoms": [
                {"label": "C", "location": [0.0, -2.5, 0]},   {"label": "C", "location": [0.95, -2.35, 0]},
                {"label": "C", "location": [0.59, -2.91, 0]}, {"label": "C", "location": [-0.59, -2.91, 0]},
                {"label": "C", "location": [-0.95, -2.35, 0]}
            ],
            "bonds": [
                {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]}, {"type": 1, "atoms": [2, 3]},
                {"type": 1, "atoms": [3, 4]}, {"type": 1, "atoms": [4, 0]}
            ],
            "attachmentGroups": [{"id": "0", "atoms": [0, 1, 2, 3, 4]}]
        },
        "mol3": {"type": "molecule", "atoms": [{"label": "O", "location": [10, 0, 0]}], "bonds": []}
    })";

    // The group example exactly as attached: mol1 is referenced by the connection but
    // absent from root.nodes.
    const char* KET_REFERENCE_GROUP_TO_ATOM_AS_ATTACHED = R"({
        "ket_version": "2.0.0",
        "root": {
            "nodes": [{"$ref": "mol0"}],
            "connections": [
                {"type": "haptic",
                 "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
                 "endpoint2": {"attachmentGroupId": "0", "moleculeId": "mol1"}}
            ],
            "templates": []
        },
        "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [3.383987, -5.449992, 0]}], "bonds": []},
        "mol1": {
            "type": "molecule",
            "atoms": [{"label": "C", "location": [3.383987, -5.449992, 0]}],
            "bonds": [],
            "attachmentGroups": [{"id": "0", "atoms": [0]}]
        }
    })";
}

// ---- loading ---------------------------------------------------------------

// The group is declared with atom indices local to its mol node; after the merge
// they must address the atoms of the whole molecule.
TEST_F(IndigoCoreHapticKetTest, GroupIsRenumberedByTheMerge)
{
    Molecule mol;
    loadKet(KET_RING_AND_METAL, mol);

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    const AttachmentGroup& ag = mol.attachment_groups.group(mol.attachment_groups.begin());
    EXPECT_EQ(std::vector<int>({1, 2, 3, 4, 5}), ag.atoms()); // mol0's iron took index 0

    ASSERT_EQ(1, mol.haptic_bonds.count());
    const HapticBond& bond = mol.haptic_bonds.at(mol.haptic_bonds.begin());
    EXPECT_EQ(_BOND_HAPTIC, bond.type());
    EXPECT_FALSE(bond.begin().isGroup());
    EXPECT_EQ(0, bond.begin().index()); // the iron
    EXPECT_TRUE(bond.end().isGroup());
    EXPECT_EQ(5, mol.edgeCount()); // the ring only: a haptic bond is not an edge
}

// KET contract §5.3: endpoints may sit in the same molecule and may already be
// bonded. The ordinary bond between them stays exactly what it was.
TEST_F(IndigoCoreHapticKetTest, HapticOnAnAlreadyBondedPairLeavesThatBondAlone)
{
    Molecule mol;
    loadKet(KET_BONDED_PAIR_WITH_HAPTIC, mol);

    ASSERT_EQ(1, mol.edgeCount());
    EXPECT_EQ(BOND_SINGLE, mol.getBondOrder(mol.edgeBegin()));
    EXPECT_EQ(1, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticKetTest, UnknownGroupIdIsRejected)
{
    std::string json(KET_RING_AND_METAL);
    const size_t pos = json.find("\"attachmentGroupId\": \"7\"");
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, 24, "\"attachmentGroupId\": \"9\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("unknown attachment group"));
}

TEST_F(IndigoCoreHapticKetTest, UnknownAtomIdIsRejected)
{
    std::string json(KET_RING_AND_METAL);
    const std::string endpoint = "\"atomId\": \"0\", \"moleculeId\": \"mol0\"";
    const size_t pos = json.find(endpoint);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, endpoint.size(), "\"atomId\": \"4\", \"moleculeId\": \"mol0\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("non-existent atom"));
}

// Rejected by the core, on the single path that creates a haptic bond.
// A non-numeric id used to reach atoi(), which reads "abc" as 0 - the endpoint bound
// itself to atom 0 instead of the load failing.
TEST_F(IndigoCoreHapticKetTest, NonNumericAtomIdIsRejected)
{
    std::string json(KET_BONDED_PAIR_WITH_HAPTIC);
    const std::string endpoint = "\"atomId\": \"1\"";
    const size_t pos = json.find(endpoint);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, endpoint.size(), "\"atomId\": \"abc\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("non-existent atom"));
}

// 2^32 + 1. While the accumulator was a `long` this wrapped to 1 on LLP64 (Win64,
// where long is 32 bits), so an out-of-range id bound itself to the second atom.
TEST_F(IndigoCoreHapticKetTest, OutOfRangeAtomIdDoesNotWrapAround)
{
    std::string json(KET_BONDED_PAIR_WITH_HAPTIC);
    const std::string endpoint = "\"atomId\": \"1\"";
    const size_t pos = json.find(endpoint);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, endpoint.size(), "\"atomId\": \"4294967297\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("non-existent atom"));
}

// The other trap of the same parser: extract_id()'s stoi() throws a std::exception,
// not an Indigo one, on a malformed suffix.
TEST_F(IndigoCoreHapticKetTest, NonNumericMoleculeIdIsRejected)
{
    std::string json(KET_RING_AND_METAL);
    const std::string endpoint = "\"moleculeId\": \"mol1\"";
    const size_t pos = json.find(endpoint);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, endpoint.size(), "\"moleculeId\": \"molXYZ\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("unknown molecule"));
}

// A guard, not a regression test: the duplicate was rejected before this change too,
// only after the group had already been added.
TEST_F(IndigoCoreHapticKetTest, DuplicateGroupIdIsRejected)
{
    std::string json(KET_RING_AND_METAL);
    const std::string groups = "\"attachmentGroups\": [{\"id\": \"7\", \"atoms\": [0, 1, 2, 3, 4]}]";
    const size_t pos = json.find(groups);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, groups.size(), "\"attachmentGroups\": [{\"id\": \"7\", \"atoms\": [0, 1]}, {\"id\": \"7\", \"atoms\": [2, 3]}]");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("Duplicate attachment group id"));
}

TEST_F(IndigoCoreHapticKetTest, ConnectionBetweenTwoGroupsIsRejected)
{
    std::string json(KET_RING_AND_METAL);
    const std::string endpoint = "\"atomId\": \"0\", \"moleculeId\": \"mol0\"";
    const size_t pos = json.find(endpoint);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, endpoint.size(), "\"attachmentGroupId\": \"7\", \"moleculeId\": \"mol1\"");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("two attachment groups"));
}

TEST_F(IndigoCoreHapticKetTest, MoleculeNodeOfGroupMissingFromRootIsRejected)
{
    EXPECT_NE(std::string::npos, loadKetError(KET_REFERENCE_GROUP_TO_ATOM_AS_ATTACHED).find("unknown molecule"));
}

TEST_F(IndigoCoreHapticKetTest, AttachmentGroupsOfTheWrongJsonTypeAreRejected)
{
    std::string json(KET_RING_AND_METAL);
    const std::string groups = "\"attachmentGroups\": [{\"id\": \"7\", \"atoms\": [0, 1, 2, 3, 4]}]";
    const size_t pos = json.find(groups);
    ASSERT_NE(std::string::npos, pos);
    json.replace(pos, groups.size(), "\"attachmentGroups\": {\"id\": \"7\"}");

    EXPECT_NE(std::string::npos, loadKetError(json.c_str()).find("must be an array"));
}

// ---- saving and round trip -------------------------------------------------

TEST_F(IndigoCoreHapticKetTest, ReferenceGroupExampleSurvivesARoundTrip)
{
    Molecule loaded;
    loadKet(KET_REFERENCE_GROUP_TO_ATOM, loaded);
    ASSERT_EQ(1, loaded.attachment_groups.groupCount());

    Molecule reloaded;
    loadKet(saveKet(loaded).c_str(), reloaded);

    EXPECT_EQ(loaded.vertexCount(), reloaded.vertexCount());
    EXPECT_EQ(loaded.edgeCount(), reloaded.edgeCount());
    ASSERT_EQ(1, reloaded.attachment_groups.groupCount());
    ASSERT_EQ(1, reloaded.haptic_bonds.count());

    // Atom indices are the saver's to choose, so the members are checked by element.
    const AttachmentGroup& ag = reloaded.attachment_groups.group(reloaded.attachment_groups.begin());
    EXPECT_EQ(6u, ag.atoms().size());
    for (int atom : ag.atoms())
        EXPECT_EQ(ELEM_C, reloaded.getAtomNumber(atom));

    const HapticBond& bond = reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin());
    const HapticBond::Endpoint& atom_end = bond.begin().isGroup() ? bond.end() : bond.begin();
    EXPECT_TRUE(bond.begin().isGroup() != bond.end().isGroup());
    EXPECT_EQ(ELEM_Fe, reloaded.getAtomNumber(atom_end.index()));
}

TEST_F(IndigoCoreHapticKetTest, ReferenceAtomExampleSurvivesARoundTrip)
{
    Molecule loaded;
    loadKet(KET_REFERENCE_ATOM_TO_ATOM, loaded);
    ASSERT_EQ(1, loaded.haptic_bonds.count());

    Molecule reloaded;
    loadKet(saveKet(loaded).c_str(), reloaded);

    EXPECT_EQ(2, reloaded.vertexCount());
    EXPECT_EQ(0, reloaded.edgeCount());
    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    const HapticBond& bond = reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin());
    EXPECT_FALSE(bond.begin().isGroup());
    EXPECT_FALSE(bond.end().isGroup());
}

// The KET contract keeps haptic bonds in root.connections, and the model keeps them
// out of the graph — so nothing of them reaches the "bonds" array.
TEST_F(IndigoCoreHapticKetTest, SaveWritesHapticBondAsAConnection)
{
    Molecule mol;
    loadKet(KET_REFERENCE_ATOM_TO_ATOM, mol);
    const std::string saved = saveKet(mol);

    EXPECT_EQ(1u, countOccurrences(saved, "\"type\":\"haptic\""));
    EXPECT_NE(std::string::npos, saved.find("\"bonds\":[]"));
}

TEST_F(IndigoCoreHapticKetTest, SaveDeclaresGroupsOnTheMoleculeThatOwnsThem)
{
    Molecule mol;
    loadKet(KET_REFERENCE_GROUP_TO_ATOM, mol);
    const std::string saved = saveKet(mol);

    EXPECT_NE(std::string::npos, saved.find("\"attachmentGroups\":[{\"id\":\"0\",\"atoms\":["));
    EXPECT_NE(std::string::npos, saved.find("\"attachmentGroupId\":\"0\""));
    EXPECT_EQ(1u, countOccurrences(saved, "\"type\":\"haptic\""));
}

// The rings and the metal are three graph components; the haptic bonds are what
// makes them one molecule, and the saver splits by exactly those sets.
TEST_F(IndigoCoreHapticKetTest, SaveKeepsFerroceneInOneNode)
{
    Molecule mol;
    makeFerrocene(mol);
    const std::string saved = saveKet(mol);

    EXPECT_EQ(1u, countOccurrences(saved, "\"type\":\"molecule\""));
    EXPECT_EQ(2u, countOccurrences(saved, "\"type\":\"haptic\""));
    EXPECT_NE(std::string::npos, saved.find("\"attachmentGroupId\":\"0\""));
    EXPECT_NE(std::string::npos, saved.find("\"attachmentGroupId\":\"1\""));

    Molecule reloaded;
    loadKet(saved.c_str(), reloaded);
    EXPECT_EQ(2, reloaded.attachment_groups.groupCount());
    EXPECT_EQ(2, reloaded.haptic_bonds.count());
    for (int i = reloaded.attachment_groups.begin(); i != reloaded.attachment_groups.end(); i = reloaded.attachment_groups.next(i))
        EXPECT_EQ(5u, reloaded.attachment_groups.group(i).atoms().size());
}

// A group without bonds is valid on its own (KET-CONTRACT.md §5.5) and must not
// drag an empty "connections" array into the file.
TEST_F(IndigoCoreHapticKetTest, GroupWithoutBondsIsSavedAndReloaded)
{
    Molecule mol;
    makeFerrocene(mol);
    const int lone = mol.attachment_groups.addGroup();
    mol.attachment_groups.group(lone).addAtom(0);

    Molecule reloaded;
    loadKet(saveKet(mol).c_str(), reloaded);
    EXPECT_EQ(3, reloaded.attachment_groups.groupCount());
    EXPECT_EQ(2, reloaded.haptic_bonds.count());
}

// A molecule without haptic bonds must be written exactly as before.
TEST_F(IndigoCoreHapticKetTest, SaveOfAPlainMoleculeHasNoConnections)
{
    Molecule mol;
    loadMolecule(BENZENE, mol);
    const std::string saved = saveKet(mol);

    EXPECT_EQ(std::string::npos, saved.find("\"connections\""));
    EXPECT_EQ(std::string::npos, saved.find("\"attachmentGroups\""));
}

// #3731 reuses the same bond with the other type, and the KET contract has no
// representation for it — it must not leave as a haptic one.
TEST_F(IndigoCoreHapticKetTest, VariableAttachmentBondIsNotWrittenAsHaptic)
{
    Molecule mol;
    makeFerrocene(mol);
    const int group = mol.attachment_groups.begin();
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(10), _BOND_VARIABLE_ATTACHMENT); // 10 = the iron

    const std::string saved = saveKet(mol);
    EXPECT_EQ(2u, countOccurrences(saved, "\"type\":\"haptic\""));
}

// ---- reaction KET ----------------------------------------------------------

// Splitting a reaction into fragments is graph connectivity, and a haptic bond is
// not an edge: without the attachment groups in the external-neighbour sets the
// ferrocene reactant arrives as three separate reactants.
TEST_F(IndigoCoreHapticKetTest, FerroceneReactantStaysOneComponent)
{
    Reaction rxn;
    loadKetReaction(KET_REACTION_WITH_FERROCENE, rxn);

    ASSERT_EQ(1, rxn.reactantsCount());
    EXPECT_EQ(1, rxn.productsCount());
    // Without the groups the rings are components of their own, and the sides of
    // the arrow they sit on turn them into catalysts rather than a second reactant.
    EXPECT_EQ(0, rxn.catalystCount());

    BaseMolecule& reactant = rxn.getBaseMolecule(rxn.reactantBegin());
    EXPECT_EQ(11, reactant.vertexCount()); // two rings plus the iron
    EXPECT_EQ(2, reactant.attachment_groups.groupCount());
    EXPECT_EQ(2, reactant.haptic_bonds.count());
}
