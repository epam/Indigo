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

// Laying out haptic bonds (#3233 requirement 4, ticket #3844). A haptic bond is
// not an edge, so its two ends are separate components and the ordinary layout
// puts them in a grid; the acceptance criteria below are the geometry that must
// come out instead. They are the A1-A6 of Task/3233/IMPLEMENTATION-PLAN.md 6.2.4.

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include <layout/molecule_layout.h>
#include <molecule/elements.h>
#include <molecule/molecule.h>

#include "common.h"

using namespace indigo;

class IndigoCoreHapticLayoutTest : public IndigoCoreTest
{
protected:
    // The layout places a lone cycle as a regular polygon, so the numbers the
    // criteria speak of are exact rather than approximate; a thousandth of a bond
    // length is far below anything a chemist can see and far above float noise.
    static constexpr float TOLERANCE = 1e-3f;

    static int addRing(Molecule& mol, int size)
    {
        const int base = mol.vertexCount();
        for (int i = 0; i < size; i++)
            mol.addAtom(ELEM_C);
        for (int i = 0; i < size; i++)
            mol.addBond(base + i, base + (i + 1) % size, BOND_SINGLE);
        return base;
    }

    static int addGroup(Molecule& mol, const std::vector<int>& atoms)
    {
        const int group = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(group).setAtoms(atoms);
        return group;
    }

    static void makeLayout(Molecule& mol, float multiplier = 1.5f)
    {
        MoleculeLayout layout(mol);
        layout.bond_length = 1.f;
        layout.haptic_bond_multiplier = multiplier;
        layout.make();
    }

    static Vec2f pos(Molecule& mol, int atom)
    {
        Vec2f flat;
        Vec2f::projectZ(flat, mol.getAtomXyz(atom));
        return flat;
    }

    static Vec2f groupCentre(Molecule& mol, int group)
    {
        Vec2f flat;
        Vec2f::projectZ(flat, mol.attachmentGroupCentre(group));
        return flat;
    }

    // How far the partner stands from the outline of the ligand along the bond:
    // the length of the bond less the reach of the group in that direction.
    static float outlineGap(Molecule& mol, int group, int partner)
    {
        const Vec2f centre = groupCentre(mol, group);
        Vec2f direction;
        direction.diff(pos(mol, partner), centre);
        const float length = direction.length();
        direction.normalize();

        float reach = 0.f;
        for (int atom : mol.attachment_groups.group(group).atoms())
        {
            Vec2f arm;
            arm.diff(pos(mol, atom), centre);
            reach = std::max(reach, Vec2f::dot(arm, direction));
        }
        return length - reach;
    }

    static float angleDegrees(const Vec2f& first, const Vec2f& second)
    {
        Vec2f a(first), b(second);
        a.normalize();
        b.normalize();
        return _2FLOAT(RAD2DEG(acosf(std::max(-1.f, std::min(1.f, Vec2f::dot(a, b))))));
    }

    // The angle the bond leaves an atom at, measured against each of the two ring
    // bonds of that atom. Both must be the bisector of the external angle.
    static void expectBisector(Molecule& mol, int atom, int neighbour_one, int neighbour_two, int partner, float expected)
    {
        Vec2f to_partner, to_one, to_two;
        to_partner.diff(pos(mol, partner), pos(mol, atom));
        to_one.diff(pos(mol, neighbour_one), pos(mol, atom));
        to_two.diff(pos(mol, neighbour_two), pos(mol, atom));

        EXPECT_NEAR(expected, angleDegrees(to_partner, to_one), 1.f);
        EXPECT_NEAR(expected, angleDegrees(to_partner, to_two), 1.f);
    }
};

// A2: the partner stands 1.5 bond lengths from the centre of the group, and the
// distance is measured from that centre and not from a member atom (N5).
TEST_F(IndigoCoreHapticLayoutTest, GroupToAtomBondIsOneAndAHalfBondLengths)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    const int group = addGroup(mol, {ring, ring + 1, ring + 2, ring + 3, ring + 4, ring + 5});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), TOLERANCE);
}

// A1: what the ticket actually asks for — the partner must clear the outline of
// the ligand. On a hexagon of side 1 the reach of the ring is 1.0 towards a
// vertex and 0.866 towards the middle of a bond, so the gap the 1.5 leaves is
// between 0.5 and 0.634 depending on where the bond points.
TEST_F(IndigoCoreHapticLayoutTest, GroupToAtomLeavesHalfABondLengthToTheOutline)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    const int group = addGroup(mol, {ring, ring + 1, ring + 2, ring + 3, ring + 4, ring + 5});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    const float gap = outlineGap(mol, group, metal);
    EXPECT_GE(gap, 0.5f - TOLERANCE);
    EXPECT_LE(gap, 0.635f);
}

// A3: requirement 4 of #3233 says the multiplier is for group-to-atom bonds only.
TEST_F(IndigoCoreHapticLayoutTest, AtomToAtomBondKeepsTheOrdinaryLength)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    mol.addHapticBond(HapticBond::Endpoint::atom(ring), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    EXPECT_NEAR(1.f, Vec2f::dist(pos(mol, ring), pos(mol, metal)), TOLERANCE);
}

// A4, six-membered ring: the bond leaves the atom along the bisector of the
// external angle. The internal angle of a hexagon is 120 degrees, so the bond
// stands at 120 to each of the two ring bonds — the number the chemist named.
TEST_F(IndigoCoreHapticLayoutTest, TheBondLeavesAHexagonAtomAlongTheBisector)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    mol.addHapticBond(HapticBond::Endpoint::atom(ring), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    expectBisector(mol, ring, ring + 1, ring + 5, metal, 120.f);
}

// A4, five-membered ring: the same rule, another number. The internal angle of a
// cyclopentadienyl ring is 108 degrees, so the bond stands at 126 — which is why
// "120 degrees" could not be written into the code as a constant.
TEST_F(IndigoCoreHapticLayoutTest, TheBondLeavesACyclopentadienylAtomAt126Degrees)
{
    Molecule mol;
    const int ring = addRing(mol, 5);
    const int metal = mol.addAtom(ELEM_Fe);
    mol.addHapticBond(HapticBond::Endpoint::atom(ring), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    expectBisector(mol, ring, ring + 1, ring + 4, metal, 126.f);
}

// A5: the anchor is the centre of the participating atoms, not of the ring the
// atoms belong to. The set is deliberately asymmetric — on a symmetric one the
// bounding box gives the same answer and the defect stays invisible.
TEST_F(IndigoCoreHapticLayoutTest, ThePartialGroupAnchorsAtTheCentreOfItsOwnAtoms)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    const int group = addGroup(mol, {ring, ring + 1, ring + 2});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), TOLERANCE);

    std::vector<Vec2f> ring_atoms;
    for (int i = 0; i < 6; i++)
        ring_atoms.push_back(pos(mol, ring + i));
    const Vec2f ring_centre = AttachmentGroup::centreOf(ring_atoms);

    EXPECT_GT(Vec2f::dist(ring_centre, groupCentre(mol, group)), 0.4f) << "three atoms of a hexagon do not centre on the ring";
    EXPECT_GT(std::fabs(Vec2f::dist(ring_centre, pos(mol, metal)) - 1.5f), 0.1f) << "the length is measured from the group, not from the ring";
}

// A6: the multiplier is a parameter, so that variable attachment (#3731) can ask
// for another value without touching this code.
TEST_F(IndigoCoreHapticLayoutTest, TheMultiplierIsAParameter)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    const int group = addGroup(mol, {ring, ring + 1, ring + 2, ring + 3, ring + 4, ring + 5});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol, 2.f);

    EXPECT_NEAR(2.f, Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), TOLERANCE);
}

// Ferrocene: two rings and one metal, three components and two haptic bonds. Both
// bonds must hold their length and the rings must end up on opposite sides of the
// metal — the sandwich the specification draws.
TEST_F(IndigoCoreHapticLayoutTest, FerroceneComesOutAsASandwich)
{
    Molecule mol;
    const int first = addRing(mol, 5);
    const int second = addRing(mol, 5);
    const int metal = mol.addAtom(ELEM_Fe);

    const int first_group = addGroup(mol, {first, first + 1, first + 2, first + 3, first + 4});
    const int second_group = addGroup(mol, {second, second + 1, second + 2, second + 3, second + 4});
    mol.addHapticBond(HapticBond::Endpoint::group(first_group), HapticBond::Endpoint::atom(metal));
    mol.addHapticBond(HapticBond::Endpoint::group(second_group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    const Vec2f metal_pos = pos(mol, metal);
    const Vec2f first_centre = groupCentre(mol, first_group);
    const Vec2f second_centre = groupCentre(mol, second_group);

    EXPECT_NEAR(1.5f, Vec2f::dist(first_centre, metal_pos), TOLERANCE);
    EXPECT_NEAR(1.5f, Vec2f::dist(second_centre, metal_pos), TOLERANCE);

    Vec2f to_first, to_second;
    to_first.diff(first_centre, metal_pos);
    to_second.diff(second_centre, metal_pos);
    to_first.normalize();
    to_second.normalize();
    EXPECT_LT(Vec2f::dot(to_first, to_second), -0.99f) << "the rings face each other across the metal";
}

// A variable attachment bond (#3731) shares the container with haptic bonds and
// has geometry of its own; the layout must leave it to whoever implements it,
// the way the render and the KET saver already do.
TEST_F(IndigoCoreHapticLayoutTest, VariableAttachmentIsNotLaidOutAsHaptic)
{
    Molecule mol;
    const int ring = addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    const int group = addGroup(mol, {ring, ring + 1, ring + 2, ring + 3, ring + 4, ring + 5});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal), _BOND_VARIABLE_ATTACHMENT);

    makeLayout(mol);

    EXPECT_GT(Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), 1.5f + TOLERANCE) << "the grid placed it, not the haptic rule";
}

// The anchor is an ordinary vertex bonded to the partner, so the layout first
// places it as a substituent; it belongs at the centre of the ligand it stands
// for, which is where the haptic bond is drawn from.
TEST_F(IndigoCoreHapticLayoutTest, TheAnchorEndsUpAtTheCentreOfItsGroup)
{
    Molecule mol;
    const int ring = addRing(mol, 5);
    const int metal = mol.addAtom(ELEM_Fe);
    const int star = mol.addAtom(ELEM_PSEUDO);
    mol.setPseudoAtom(star, "*");
    mol.addBond(metal, star, BOND_SINGLE);

    const int group = addGroup(mol, {ring, ring + 1, ring + 2, ring + 3, ring + 4});
    mol.attachment_groups.group(group).setAnchorAtom(star);
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    EXPECT_NEAR(0.f, Vec2f::dist(pos(mol, star), groupCentre(mol, group)), TOLERANCE);
    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), TOLERANCE);
}

// ---- several bonds holding one ligand --------------------------------------

// An eta-2 end is a line, and a bond leaving it along that line would run over the
// ligand's own bond. It leaves across instead - the right angle the chemist asks
// for, and the reason the direction cannot simply be "away from the ring".
TEST_F(IndigoCoreHapticLayoutTest, TheBondCrossesATwoAtomGroupAtARightAngle)
{
    Molecule mol;
    const int ring = addRing(mol, 8);
    const int metal = mol.addAtom(ELEM_Ni);
    const int group = addGroup(mol, {ring, ring + 1});
    mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    Vec2f axis, bond;
    axis.diff(pos(mol, ring + 1), pos(mol, ring));
    bond.diff(pos(mol, metal), groupCentre(mol, group));

    EXPECT_NEAR(90.f, angleDegrees(axis, bond), 1.f);
    EXPECT_NEAR(1.5f, bond.length(), TOLERANCE);
}

// A chelating ligand holds the metal twice - two double bonds of one ring, as in
// bis(1,5-cyclooctadiene)nickel. Placing it by the first bond alone leaves the
// second at whatever length happens to come out; both have to reach 1.5.
TEST_F(IndigoCoreHapticLayoutTest, AChelatingLigandSatisfiesBothOfItsBonds)
{
    Molecule mol;
    const int ring = addRing(mol, 8);
    const int metal = mol.addAtom(ELEM_Ni);
    const int first = addGroup(mol, {ring, ring + 1});
    const int second = addGroup(mol, {ring + 4, ring + 5});
    mol.addHapticBond(HapticBond::Endpoint::group(first), HapticBond::Endpoint::atom(metal));
    mol.addHapticBond(HapticBond::Endpoint::group(second), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, first), pos(mol, metal)), TOLERANCE);
    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, second), pos(mol, metal)), TOLERANCE);
}

// A bridging ligand is held by two different metals, as the cyclooctatetraene of
// tris(cyclooctatetraene)triiron is. The ring has to end up between them, at the
// right distance from each.
TEST_F(IndigoCoreHapticLayoutTest, ABridgingLigandReachesBothMetals)
{
    Molecule mol;
    const int ring = addRing(mol, 8);
    const int first_metal = mol.addAtom(ELEM_Fe);
    const int second_metal = mol.addAtom(ELEM_Fe);
    mol.addBond(first_metal, second_metal, BOND_SINGLE);

    const int first = addGroup(mol, {ring, ring + 1, ring + 2});
    const int second = addGroup(mol, {ring + 4, ring + 5, ring + 6});
    mol.addHapticBond(HapticBond::Endpoint::group(first), HapticBond::Endpoint::atom(first_metal));
    mol.addHapticBond(HapticBond::Endpoint::group(second), HapticBond::Endpoint::atom(second_metal));

    makeLayout(mol);

    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, first), pos(mol, first_metal)), TOLERANCE);
    EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, second), pos(mol, second_metal)), TOLERANCE);
}

// Two ligands on one metal, each holding it twice: the metal cannot be reached by
// translating it towards one of them, and the exact answer is where the circles of
// both bonds meet.
TEST_F(IndigoCoreHapticLayoutTest, AMetalBetweenTwoChelatingLigandsReachesAllFourBonds)
{
    Molecule mol;
    const int first_ring = addRing(mol, 8);
    const int second_ring = addRing(mol, 8);
    const int metal = mol.addAtom(ELEM_Ni);

    const int groups[4] = {addGroup(mol, {first_ring, first_ring + 1}), addGroup(mol, {first_ring + 4, first_ring + 5}),
                           addGroup(mol, {second_ring, second_ring + 1}), addGroup(mol, {second_ring + 4, second_ring + 5})};
    for (int group : groups)
        mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));

    makeLayout(mol);

    for (int group : groups)
        EXPECT_NEAR(1.5f, Vec2f::dist(groupCentre(mol, group), pos(mol, metal)), TOLERANCE) << "group " << group;
}
