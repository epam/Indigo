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

// CDX/CDXML side of haptic bonds (#3233, ticket #3843). The format carries the
// group as a node of its own - NodeType="MultiAttachment" whose Attachments
// property lists the member atoms - and the bond to the metal as an ordinary
// bond to that node, which is the vendor's own description of ferrocene.
// VariableAttachment (#3731) has the same shape and the other meaning, so the
// two are told apart here as well.

#include <gtest/gtest.h>

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/molecule.h>
#include <molecule/molecule_cdxml_loader.h>
#include <molecule/molecule_cdxml_saver.h>

#include "common.h"

using namespace indigo;

class IndigoCoreHapticCdxmlTest : public IndigoCoreTest
{
protected:
    static void loadCdxml(const std::string& text, Molecule& mol)
    {
        BufferScanner scanner(text.c_str());
        MoleculeCdxmlLoader loader(scanner);
        loader.loadMolecule(mol);
    }

    static void loadCdx(const std::string& binary, Molecule& mol)
    {
        BufferScanner scanner(binary.c_str(), static_cast<int>(binary.size()));
        // The reader starts at the first object, past the document header the
        // saver wrote - the same step MoleculeAutoLoader takes.
        ASSERT_TRUE(scanner.startsWith(kCDX_HeaderString));
        scanner.seek(kCDX_HeaderLength, SEEK_CUR);

        MoleculeCdxmlLoader loader(scanner, true);
        loader.loadMolecule(mol);
    }

    static std::string save(Molecule& mol, bool binary = false)
    {
        Array<char> buffer;
        ArrayOutput output(buffer);
        MoleculeCdxmlSaver saver(output, binary);
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

    // A cyclopentadienyl ring, an iron, and an attachment node standing for the
    // whole ring - the construct the CDX specification illustrates with ferrocene.
    // `node_type` picks the meaning of the member set: every atom at once
    // (MultiAttachment) or any one of them (VariableAttachment).
    static std::string ring_and_metal(const char* node_type = "MultiAttachment", const char* bond_attrs = "", const char* attachments = "10 11 12 13 14",
                                      bool with_bond = true)
    {
        std::string text = R"(<?xml version="1.0" encoding="UTF-8"?>
<CDXML BondLength="30.000000"><page HeightPages="1" WidthPages="1"><fragment id="1">
<n id="10" p="100.00 100.00"/><n id="11" p="128.53 120.73"/><n id="12" p="117.63 154.27"/>
<n id="13" p="82.37 154.27"/><n id="14" p="71.47 120.73"/>
<n id="20" p="100.00 130.00" NodeType=")";
        text += node_type;
        text += R"(" Attachments=")";
        text += attachments;
        text += R"("/>
<n id="30" p="180.00 130.00" Element="26"/>
<b id="40" B="10" E="11" Order="2"/><b id="41" B="11" E="12"/><b id="42" B="12" E="13" Order="2"/>
<b id="43" B="13" E="14"/><b id="44" B="14" E="10" Order="2"/>
)";
        if (with_bond)
        {
            text += R"(<b id="50" B="30" E="20" )";
            text += bond_attrs;
            text += "/>";
        }
        text += R"(
</fragment></page></CDXML>)";
        return text;
    }
};

TEST_F(IndigoCoreHapticCdxmlTest, MultiAttachmentNodeBecomesAGroup)
{
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    // The node is not an atom: five carbons and the iron, and no bond joins them.
    ASSERT_EQ(6, mol.vertexCount());
    EXPECT_EQ(5, mol.edgeCount());

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    const std::vector<int>& atoms = mol.attachment_groups.group(mol.attachment_groups.begin()).atoms();
    EXPECT_EQ((std::vector<int>{0, 1, 2, 3, 4}), atoms);

    ASSERT_EQ(1, mol.haptic_bonds.count());
    const HapticBond& bond = mol.haptic_bonds.at(mol.haptic_bonds.begin());
    EXPECT_EQ(_BOND_HAPTIC, bond.type());
    EXPECT_FALSE(bond.begin().isGroup());
    EXPECT_EQ(5, bond.begin().index()); // the iron
    EXPECT_TRUE(bond.end().isGroup());
}

TEST_F(IndigoCoreHapticCdxmlTest, HapticBondHoldsTheComplexTogether)
{
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);

    EXPECT_EQ(2, mol.countComponents());          // the ring and the metal, by edges alone
    EXPECT_EQ(1, mol.countComponents(neighbors)); // one complex, as the file drew it
}

TEST_F(IndigoCoreHapticCdxmlTest, VariableAttachmentNodeIsNotReadAsHaptic)
{
    Molecule mol;
    loadCdxml(ring_and_metal("VariableAttachment"), mol);

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    ASSERT_EQ(1, mol.haptic_bonds.count());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, mol.haptic_bonds.at(mol.haptic_bonds.begin()).type());
}

TEST_F(IndigoCoreHapticCdxmlTest, BondOrderOnTheHapticBondIsIgnored)
{
    // ChemDraw draws a metal-ligand bond with any order it likes; what makes the
    // bond haptic is the node it ends at, never its order.
    Molecule mol;
    loadCdxml(ring_and_metal("MultiAttachment", "Order=\"dative\""), mol);

    EXPECT_EQ(5, mol.edgeCount());
    ASSERT_EQ(1, mol.haptic_bonds.count());
    EXPECT_EQ(_BOND_HAPTIC, mol.haptic_bonds.at(mol.haptic_bonds.begin()).type());
}

TEST_F(IndigoCoreHapticCdxmlTest, FerroceneKeepsBothRings)
{
    // The specification's own example: two multicenter attachment nodes, one per
    // ring, both bonded to the same iron.
    const char* ferrocene = R"(<?xml version="1.0" encoding="UTF-8"?>
<CDXML BondLength="30.000000"><page HeightPages="1" WidthPages="1"><fragment id="1">
<n id="10" p="100.00 60.00"/><n id="11" p="128.53 80.73"/><n id="12" p="117.63 114.27"/>
<n id="13" p="82.37 114.27"/><n id="14" p="71.47 80.73"/>
<n id="20" p="100.00 90.00" NodeType="MultiAttachment" Attachments="10 11 12 13 14"/>
<n id="110" p="100.00 200.00"/><n id="111" p="128.53 220.73"/><n id="112" p="117.63 254.27"/>
<n id="113" p="82.37 254.27"/><n id="114" p="71.47 220.73"/>
<n id="120" p="100.00 230.00" NodeType="MultiAttachment" Attachments="110 111 112 113 114"/>
<n id="30" p="180.00 160.00" Element="26"/>
<b id="40" B="10" E="11" Order="2"/><b id="41" B="11" E="12"/><b id="42" B="12" E="13" Order="2"/>
<b id="43" B="13" E="14"/><b id="44" B="14" E="10" Order="2"/>
<b id="140" B="110" E="111" Order="2"/><b id="141" B="111" E="112"/><b id="142" B="112" E="113" Order="2"/>
<b id="143" B="113" E="114"/><b id="144" B="114" E="110" Order="2"/>
<b id="50" B="30" E="20"/><b id="150" B="30" E="120"/>
</fragment></page></CDXML>)";

    Molecule mol;
    loadCdxml(ferrocene, mol);

    EXPECT_EQ(11, mol.vertexCount());
    EXPECT_EQ(2, mol.attachment_groups.groupCount());
    EXPECT_EQ(2, mol.haptic_bonds.count());

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);
    EXPECT_EQ(1, mol.countComponents(neighbors));
}

TEST_F(IndigoCoreHapticCdxmlTest, GroupIsSavedAsAnAttachmentNode)
{
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    // Where the node stands and how its bond is written is held by the reference
    // of formats/haptic_cdxml.py; here only that the group became one node.
    const std::string saved = save(mol);
    EXPECT_EQ(1, countOccurrences(saved, "NodeType=\"MultiAttachment\""));
    EXPECT_EQ(1, countOccurrences(saved, "Attachments=\"5 6 7 8 9\""));
}

TEST_F(IndigoCoreHapticCdxmlTest, RoundTripKeepsTheGroupAndTheBond)
{
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    Molecule reloaded;
    loadCdxml(save(mol), reloaded);

    ASSERT_EQ(1, reloaded.attachment_groups.groupCount());
    EXPECT_EQ(5, static_cast<int>(reloaded.attachment_groups.group(reloaded.attachment_groups.begin()).atoms().size()));
    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    EXPECT_EQ(_BOND_HAPTIC, reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin()).type());
    EXPECT_EQ(6, reloaded.vertexCount());
    EXPECT_EQ(5, reloaded.edgeCount());
}

TEST_F(IndigoCoreHapticCdxmlTest, RoundTripKeepsVariableAttachmentApart)
{
    Molecule mol;
    loadCdxml(ring_and_metal("VariableAttachment"), mol);

    const std::string saved = save(mol);
    EXPECT_EQ(1, countOccurrences(saved, "NodeType=\"VariableAttachment\""));

    Molecule reloaded;
    loadCdxml(saved, reloaded);
    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin()).type());
}

TEST_F(IndigoCoreHapticCdxmlTest, BinaryCdxRoundTrip)
{
    // The binary form of the Attachments property - a UINT16 count and then the
    // ids - is written and read by cases of its own, so the whole trip is made
    // here rather than trusting the XML one.
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    Molecule reloaded;
    loadCdx(save(mol, true), reloaded);

    EXPECT_EQ(6, reloaded.vertexCount());
    EXPECT_EQ(5, reloaded.edgeCount());
    ASSERT_EQ(1, reloaded.attachment_groups.groupCount());
    EXPECT_EQ((std::vector<int>{0, 1, 2, 3, 4}), reloaded.attachment_groups.group(reloaded.attachment_groups.begin()).atoms());
    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    EXPECT_EQ(_BOND_HAPTIC, reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin()).type());
}

TEST_F(IndigoCoreHapticCdxmlTest, BinaryCdxKeepsVariableAttachmentApart)
{
    Molecule mol;
    loadCdxml(ring_and_metal("VariableAttachment"), mol);

    Molecule reloaded;
    loadCdx(save(mol, true), reloaded);

    ASSERT_EQ(1, reloaded.haptic_bonds.count());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, reloaded.haptic_bonds.at(reloaded.haptic_bonds.begin()).type());
}

TEST_F(IndigoCoreHapticCdxmlTest, AttachmentNodeWithoutMembersIsRejected)
{
    const char* text = R"(<?xml version="1.0" encoding="UTF-8"?>
<CDXML BondLength="30.000000"><page HeightPages="1" WidthPages="1"><fragment id="1">
<n id="10" p="100.00 100.00"/>
<n id="20" p="100.00 130.00" NodeType="MultiAttachment"/>
<n id="30" p="180.00 130.00" Element="26"/>
<b id="50" B="30" E="20"/>
</fragment></page></CDXML>)";

    Molecule mol;
    EXPECT_THROW(loadCdxml(text, mol), Exception);
}

TEST_F(IndigoCoreHapticCdxmlTest, AttachmentsThatIsNotAListOfIdsIsRejected)
{
    Molecule mol;
    EXPECT_THROW(loadCdxml(ring_and_metal("MultiAttachment", "", "10 abc"), mol), Exception);

    Molecule trailing;
    EXPECT_THROW(loadCdxml(ring_and_metal("MultiAttachment", "", "10 11x"), trailing), Exception);
}

TEST_F(IndigoCoreHapticCdxmlTest, GroupWithNoBondSurvivesTheRoundTrip)
{
    // A lone attachment node declares a set of atoms and nothing more. The format
    // holds it, so saving must not quietly drop what loading accepted.
    Molecule mol;
    loadCdxml(ring_and_metal("MultiAttachment", "", "10 11 12 13 14", false), mol);

    ASSERT_EQ(1, mol.attachment_groups.groupCount());
    ASSERT_EQ(0, mol.haptic_bonds.count());

    const std::string saved = save(mol);
    EXPECT_EQ(1, countOccurrences(saved, "NodeType=\"MultiAttachment\""));

    Molecule reloaded;
    loadCdxml(saved, reloaded);
    EXPECT_EQ(1, reloaded.attachment_groups.groupCount());
    EXPECT_EQ(0, reloaded.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticCdxmlTest, AttachmentNodePointingAtANonAtomIsRejected)
{
    // The second attachment node is not an atom, so it cannot be a member of the
    // first one: the file says something the format cannot mean.
    const char* text = R"(<?xml version="1.0" encoding="UTF-8"?>
<CDXML BondLength="30.000000"><page HeightPages="1" WidthPages="1"><fragment id="1">
<n id="10" p="100.00 100.00"/><n id="11" p="128.53 120.73"/>
<n id="21" p="110.00 110.00" NodeType="MultiAttachment" Attachments="10 11"/>
<n id="20" p="100.00 130.00" NodeType="MultiAttachment" Attachments="10 21"/>
<n id="30" p="180.00 130.00" Element="26"/>
<b id="40" B="10" E="11"/><b id="50" B="30" E="20"/>
</fragment></page></CDXML>)";

    Molecule mol;
    EXPECT_THROW(loadCdxml(text, mol), Exception);
}

TEST_F(IndigoCoreHapticCdxmlTest, AtomToAtomHapticBondIsDropped)
{
    // CDX has no node to hang such a bond on, so it goes the way of every other
    // feature the target format cannot express - and takes nothing else with it.
    Molecule mol;
    mol.addAtom(ELEM_Fe);
    mol.addAtom(ELEM_C);
    mol.setAtomXyz(0, Vec3f(0, 0, 0));
    mol.setAtomXyz(1, Vec3f(1, 0, 0));
    mol.addHapticBond(HapticBond::Endpoint::atom(0), HapticBond::Endpoint::atom(1));

    const std::string saved = save(mol);
    EXPECT_EQ(0, countOccurrences(saved, "NodeType=\"MultiAttachment\""));
    EXPECT_EQ(0, countOccurrences(saved, "Attachments="));
    EXPECT_EQ(2, countOccurrences(saved, "<n id="));
}

TEST_F(IndigoCoreHapticCdxmlTest, TwoBondsToOneGroupShareItsNode)
{
    // A ligand held by two metals is still one set of atoms: the group must reach
    // the file as a single node, or the two bonds would name different groups.
    Molecule mol;
    loadCdxml(ring_and_metal(), mol);

    const int second_metal = mol.addAtom(ELEM_Fe);
    mol.setAtomXyz(second_metal, Vec3f(-5, 0, 0));
    const int group = mol.attachment_groups.begin();
    mol.addHapticBond(HapticBond::Endpoint::atom(second_metal), HapticBond::Endpoint::group(group));

    const std::string saved = save(mol);
    EXPECT_EQ(1, countOccurrences(saved, "NodeType=\"MultiAttachment\""));
    EXPECT_EQ(1, countOccurrences(saved, "Attachments=\"5 6 7 8 9\""));
}
