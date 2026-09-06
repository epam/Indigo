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

// MOL V3000 side of haptic bonds (#3233, ticket #3840): the ENDPTS/ATTACH keys the
// loader used to throw away, and the single record the saver must write them back
// in. The star atom is not absorbed - it stays an ordinary pseudo-atom, and the
// group only remembers which atom it is.

#include <gtest/gtest.h>

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/molecule.h>
#include <molecule/molfile_loader.h>
#include <molecule/molfile_saver.h>
#include <molecule/query_molecule.h>

#include "common.h"

using namespace indigo;

class IndigoCoreHapticMolfileTest : public IndigoCoreTest
{
protected:
    static void loadMolfile(const char* text, Molecule& mol)
    {
        BufferScanner scanner(text);
        MolfileLoader loader(scanner);
        loader.loadMolecule(mol);
    }

    static std::string saveMolfile(Molecule& mol, int mode = MolfileSaver::MODE_3000)
    {
        Array<char> buffer;
        ArrayOutput output(buffer);
        MolfileSaver saver(output);
        saver.mode = mode;
        saver.skip_date = true;
        saver.saveMolecule(mol);
        return {buffer.ptr(), static_cast<std::size_t>(buffer.size())};
    }

    static int countOccurrences(const std::string& text, const std::string& what)
    {
        int count = 0;
        for (std::size_t pos = text.find(what); pos != std::string::npos; pos = text.find(what, pos + what.size()))
            count++;
        return count;
    }

    // The radical form of ferrocene from the repository fixtures
    // (api/tests/integration/tests/basic/molecules/ferrocene-variant5.mol): two
    // rings, an iron, and a star atom per ring carrying the ENDPTS record.
    static const char* ferrocene()
    {
        return "\n"
               "  ACCLDraw05131421592D\n"
               "\n"
               "  0  0  0     0  0            999 V3000\n"
               "M  V30 BEGIN CTAB\n"
               "M  V30 COUNTS 13 12 0 0 0\n"
               "M  V30 BEGIN ATOM\n"
               "M  V30 1 C 11.8042 -12.5776 0 0 RAD=2\n"
               "M  V30 2 C 13.0395 -12.4112 0 0 RAD=2\n"
               "M  V30 3 C 13.8477 -12.775 0 0 RAD=2\n"
               "M  V30 4 C 13.1152 -13.1603 0 0 RAD=2\n"
               "M  V30 5 C 11.8459 -13.042 0 0 RAD=2\n"
               "M  V30 6 Fe 12.7132 -10.9095 0 0 CHG=2\n"
               "M  V30 7 C 12.4044 -8.6129 0 0 RAD=2\n"
               "M  V30 8 C 11.6245 -8.9982 0 0 RAD=2\n"
               "M  V30 9 C 12.4011 -9.3938 0 0 RAD=2\n"
               "M  V30 10 C 13.6671 -9.2514 0 0 RAD=2\n"
               "M  V30 11 C 13.6671 -8.7694 0 0 RAD=2\n"
               "M  V30 12 * 12.7132 -9.0052 0 0 CHG=-1\n"
               "M  V30 13 * 12.7132 -12.7932 0 0 CHG=-1\n"
               "M  V30 END ATOM\n"
               "M  V30 BEGIN BOND\n"
               "M  V30 1 1 1 2\n"
               "M  V30 2 1 2 3\n"
               "M  V30 3 1 3 4\n"
               "M  V30 4 1 4 5\n"
               "M  V30 5 1 5 1\n"
               "M  V30 6 1 7 11\n"
               "M  V30 7 1 8 7\n"
               "M  V30 8 1 9 8\n"
               "M  V30 9 1 10 9\n"
               "M  V30 10 1 11 10\n"
               "M  V30 11 9 6 12 ENDPTS=(5 11 7 8 9 10) ATTACH=ALL\n"
               "M  V30 12 9 6 13 ENDPTS=(5 5 4 3 2 1) ATTACH=ALL\n"
               "M  V30 END BOND\n"
               "M  V30 END CTAB\n"
               "M  END\n";
    }

    // A benzene with a substituent attached to any one of three ring atoms: the
    // Markush form of the same keys (#3731), which must not become haptic.
    static const char* variableAttachment()
    {
        return "\n"
               "  Indigo test\n"
               "\n"
               "  0  0  0     0  0            999 V3000\n"
               "M  V30 BEGIN CTAB\n"
               "M  V30 COUNTS 8 7 0 0 0\n"
               "M  V30 BEGIN ATOM\n"
               "M  V30 1 C 0.0 0.0 0 0\n"
               "M  V30 2 C 1.0 0.0 0 0\n"
               "M  V30 3 C 1.5 0.866 0 0\n"
               "M  V30 4 C 1.0 1.732 0 0\n"
               "M  V30 5 C 0.0 1.732 0 0\n"
               "M  V30 6 C -0.5 0.866 0 0\n"
               "M  V30 7 Cl 2.5 0.866 0 0\n"
               "M  V30 8 * 1.0 0.866 0 0\n"
               "M  V30 END ATOM\n"
               "M  V30 BEGIN BOND\n"
               "M  V30 1 1 1 2\n"
               "M  V30 2 2 2 3\n"
               "M  V30 3 1 3 4\n"
               "M  V30 4 2 4 5\n"
               "M  V30 5 1 5 6\n"
               "M  V30 6 2 6 1\n"
               "M  V30 7 1 7 8 ENDPTS=(3 2 3 4) ATTACH=ANY\n"
               "M  V30 END BOND\n"
               "M  V30 END CTAB\n"
               "M  END\n";
    }

    // Cyclopentadienyl ring (atoms 0..4) and an iron (5), with a haptic bond from
    // the whole ring to the metal and no star atom anywhere: the shape a molecule
    // has when it comes from a format that does not use one.
    static void makeAnchorlessComplex(Molecule& mol)
    {
        const float positions[5][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.31f, 0.95f}, {0.5f, 1.54f}, {-0.31f, 0.95f}};

        for (int i = 0; i < 5; i++)
        {
            mol.addAtom(ELEM_C);
            mol.setAtomXyz(i, positions[i][0], positions[i][1], 0.0f);
        }
        mol.addAtom(ELEM_Fe);
        mol.setAtomXyz(5, 0.5f, 3.0f, 0.0f);

        for (int i = 0; i < 5; i++)
            mol.addBond(i, (i + 1) % 5, BOND_SINGLE);

        const int group = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(group).setAtoms({0, 1, 2, 3, 4});
        mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(5));
    }
};

// ---- loading --------------------------------------------------------------

TEST_F(IndigoCoreHapticMolfileTest, EndptsBecomeGroupsAndHapticBonds)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);

    ASSERT_EQ(2, mol.attachment_groups.groupCount());
    ASSERT_EQ(2, mol.haptic_bonds.count());

    const HapticBond& first = mol.haptic_bonds.at(mol.haptic_bonds.begin());
    ASSERT_TRUE(first.begin().isGroup());
    EXPECT_FALSE(first.end().isGroup());
    EXPECT_EQ(5, first.end().index()) << "the iron is the single-atom end";
    EXPECT_EQ(_BOND_HAPTIC, first.type());

    const AttachmentGroup& group = mol.attachment_groups.group(first.begin().index());
    EXPECT_EQ(std::vector<int>({10, 6, 7, 8, 9}), group.atoms()) << "members keep the order ENDPTS lists them in";
    EXPECT_EQ(11, group.anchorAtom()) << "the star of the record, not absorbed but remembered";
}

// The star is an ordinary atom of the graph, so the charge the file put on it does
// not move anywhere and the molecule still adds up to zero.
TEST_F(IndigoCoreHapticMolfileTest, StarAtomAndItsEdgeSurviveLoading)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);

    ASSERT_EQ(13, mol.vertexCount());
    ASSERT_EQ(12, mol.edgeCount());

    ASSERT_TRUE(mol.isPseudoAtom(11));
    EXPECT_STREQ("*", mol.getPseudoAtom(11));
    EXPECT_EQ(-1, mol.getAtomCharge(11));
    EXPECT_GE(mol.findEdgeIndex(5, 11), 0) << "the record is still an edge as well";

    int total = 0;
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        total += mol.getAtomCharge(i);
    EXPECT_EQ(0, total);
}

TEST_F(IndigoCoreHapticMolfileTest, AttachAnyIsStoredWithoutBecomingHaptic)
{
    Molecule mol;
    loadMolfile(variableAttachment(), mol);

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    ASSERT_EQ(1, mol.haptic_bonds.count());

    const HapticBond& bond = mol.haptic_bonds.at(mol.haptic_bonds.begin());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, bond.type());
    EXPECT_EQ(std::vector<int>({1, 2, 3}), mol.attachment_groups.group(bond.begin().index()).atoms());
    EXPECT_EQ(6, bond.end().index()) << "the chlorine is the single-atom end";
}

// ENDPTS is a free key of any bond record, not something type 9 owns: a file that
// puts it on a single bond means the same thing.
TEST_F(IndigoCoreHapticMolfileTest, EndptsIsReadOnBondsOtherThanCoordination)
{
    Molecule mol;
    loadMolfile(variableAttachment(), mol);

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    EXPECT_EQ(BOND_SINGLE, mol.getBondOrder(mol.findEdgeIndex(6, 7)));
}

TEST_F(IndigoCoreHapticMolfileTest, EndpointOutsideTheAtomBlockIsRejected)
{
    std::string broken = ferrocene();
    broken.replace(broken.find("ENDPTS=(5 11 7 8 9 10)"), std::string("ENDPTS=(5 11 7 8 9 10)").size(), "ENDPTS=(5 11 7 8 9 99)");

    Molecule mol;
    EXPECT_THROW(loadMolfile(broken.c_str(), mol), Exception);
}

// The two ends of the record are the two sides of the association: an end that is
// also one of the attached atoms would make the bond point at itself.
TEST_F(IndigoCoreHapticMolfileTest, EndptsListingTheRecordsOwnEndIsRejected)
{
    std::string broken = ferrocene();
    broken.replace(broken.find("ENDPTS=(5 11 7 8 9 10)"), std::string("ENDPTS=(5 11 7 8 9 10)").size(), "ENDPTS=(5 11 7 8 9 6)");

    Molecule mol;
    EXPECT_THROW(loadMolfile(broken.c_str(), mol), Exception);
}

TEST_F(IndigoCoreHapticMolfileTest, UnknownAttachValueIsRejected)
{
    std::string broken = ferrocene();
    broken.replace(broken.find("ATTACH=ALL"), std::string("ATTACH=ALL").size(), "ATTACH=SOME");

    Molecule mol;
    EXPECT_THROW(loadMolfile(broken.c_str(), mol), Exception);
}

// The spoke form of the same molecule carries no ENDPTS at all: nothing must be
// inferred from a bundle of coordination bonds.
TEST_F(IndigoCoreHapticMolfileTest, BondsWithoutEndptsProduceNoGroups)
{
    std::string spokes = ferrocene();
    spokes.replace(spokes.find(" ENDPTS=(5 11 7 8 9 10) ATTACH=ALL"), std::string(" ENDPTS=(5 11 7 8 9 10) ATTACH=ALL").size(), "");
    spokes.replace(spokes.find(" ENDPTS=(5 5 4 3 2 1) ATTACH=ALL"), std::string(" ENDPTS=(5 5 4 3 2 1) ATTACH=ALL").size(), "");

    Molecule mol;
    loadMolfile(spokes.c_str(), mol);

    EXPECT_EQ(0, mol.attachment_groups.groupCount());
    EXPECT_EQ(0, mol.haptic_bonds.count());
}

// ---- saving ---------------------------------------------------------------

TEST_F(IndigoCoreHapticMolfileTest, SavedRecordCarriesTheKeysBack)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);
    const std::string saved = saveMolfile(mol);

    EXPECT_NE(std::string::npos, saved.find("M  V30 11 9 6 12 ENDPTS=(5 11 7 8 9 10) ATTACH=ALL"));
    EXPECT_NE(std::string::npos, saved.find("M  V30 12 9 6 13 ENDPTS=(5 5 4 3 2 1) ATTACH=ALL"));
    EXPECT_NE(std::string::npos, saved.find("M  V30 COUNTS 13 12 0 0 0")) << "no atom and no record was invented";
}

// One record in the file became an edge and a haptic bond; it has to come back as
// one record, not as both of them.
TEST_F(IndigoCoreHapticMolfileTest, EachHapticBondIsWrittenExactlyOnce)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);
    const std::string saved = saveMolfile(mol);

    EXPECT_EQ(2, countOccurrences(saved, "ENDPTS="));
    EXPECT_EQ(2, countOccurrences(saved, "ATTACH="));
}

TEST_F(IndigoCoreHapticMolfileTest, RoundTripIsStable)
{
    Molecule first;
    loadMolfile(ferrocene(), first);
    const std::string once = saveMolfile(first);

    Molecule second;
    loadMolfile(once.c_str(), second);
    EXPECT_EQ(once, saveMolfile(second));

    ASSERT_EQ(2, second.attachment_groups.groupCount());
    ASSERT_EQ(2, second.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticMolfileTest, VariableAttachmentRoundTripsAsAny)
{
    Molecule mol;
    loadMolfile(variableAttachment(), mol);
    const std::string saved = saveMolfile(mol);

    EXPECT_NE(std::string::npos, saved.find("ENDPTS=(3 2 3 4) ATTACH=ANY"));
    EXPECT_EQ(0, countOccurrences(saved, "ATTACH=ALL"));
}

// A group that never had a star gets one: the format has no other way to draw the
// end of the bond, and the centre of the members is where it belongs.
TEST_F(IndigoCoreHapticMolfileTest, StarIsSynthesizedForAGroupWithoutAnchor)
{
    Molecule mol;
    makeAnchorlessComplex(mol);
    const std::string saved = saveMolfile(mol);

    EXPECT_NE(std::string::npos, saved.find("M  V30 COUNTS 7 6 0 0 0")) << "one atom and one record more than the graph has";
    EXPECT_NE(std::string::npos, saved.find("M  V30 7 * 0.5 0.77 0.0 0")) << "the star sits at the centre of the ring";
    EXPECT_NE(std::string::npos, saved.find("M  V30 6 9 6 7 ENDPTS=(5 1 2 3 4 5) ATTACH=ALL"));

    Molecule reloaded;
    loadMolfile(saved.c_str(), reloaded);
    ASSERT_EQ(1, reloaded.attachment_groups.groupCount());
    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    EXPECT_EQ(6, reloaded.attachment_groups.group(reloaded.attachment_groups.begin()).anchorAtom());
}

// V2000 has no ENDPTS at all, so the haptic markup is dropped whole - and nothing
// else is: the star stays the ordinary atom it is.
TEST_F(IndigoCoreHapticMolfileTest, V2000OmitsTheHapticMarkup)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);
    const std::string saved = saveMolfile(mol, MolfileSaver::MODE_2000);

    EXPECT_EQ(0, countOccurrences(saved, "ENDPTS"));
    EXPECT_EQ(0, countOccurrences(saved, "ATTACH"));

    Molecule reloaded;
    loadMolfile(saved.c_str(), reloaded);
    EXPECT_EQ(13, reloaded.vertexCount());
    EXPECT_EQ(0, reloaded.attachment_groups.groupCount());
}

// A variable attachment is a query construct (#3731), so the query path is where
// such a file usually arrives - and it is a different loader branch and a
// different saver call than the plain molecule above.
TEST_F(IndigoCoreHapticMolfileTest, VariableAttachmentRoundTripsAsAQueryMolecule)
{
    QueryMolecule query;
    BufferScanner scanner(variableAttachment());
    MolfileLoader loader(scanner);
    loader.loadQueryMolecule(query);

    ASSERT_EQ(1, query.attachment_groups.groupCount());
    ASSERT_EQ(1, query.haptic_bonds.count());

    const HapticBond& bond = query.haptic_bonds.at(query.haptic_bonds.begin());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, bond.type());
    EXPECT_EQ(std::vector<int>({1, 2, 3}), query.attachment_groups.group(bond.begin().index()).atoms());
    EXPECT_EQ(7, query.attachment_groups.group(bond.begin().index()).anchorAtom());

    Array<char> buffer;
    ArrayOutput output(buffer);
    MolfileSaver saver(output);
    saver.mode = MolfileSaver::MODE_3000;
    saver.skip_date = true;
    saver.saveQueryMolecule(query);

    const std::string saved{buffer.ptr(), static_cast<std::size_t>(buffer.size())};
    EXPECT_NE(std::string::npos, saved.find("ENDPTS=(3 2 3 4) ATTACH=ANY"));
    EXPECT_EQ(1, countOccurrences(saved, "ENDPTS="));
}

// The group knows its star by atom index, so a copy that renumbers atoms must not
// end up writing the ENDPTS of one ring next to the star of the other.
TEST_F(IndigoCoreHapticMolfileTest, CloneWritesTheSameRecords)
{
    Molecule mol;
    loadMolfile(ferrocene(), mol);

    Molecule copy;
    copy.clone(mol);

    EXPECT_EQ(saveMolfile(mol), saveMolfile(copy));
}
