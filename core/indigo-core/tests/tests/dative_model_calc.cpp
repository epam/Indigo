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

// Core of the dative-bond valence model (ticket #3617, phase 3): requirements 1-6.
// The reference numbers are the worked examples of the specification, issue body as
// of 2026-08-04.

#include <gtest/gtest.h>

#include <string>

#include <base_cpp/scanner.h>
#include <molecule/dative_model.h>
#include <molecule/elements.h>
#include <molecule/molecule.h>
#include <molecule/molecule_arom.h>
#include <molecule/molecule_dearom.h>
#include <molecule/molfile_loader.h>
#include <molecule/smiles_loader.h>

#include "common.h"

using namespace indigo;

class IndigoCoreDativeModelTest : public IndigoCoreTest
{
protected:
    static void loadV3000(Molecule& mol, const std::string& molfile)
    {
        BufferScanner scanner(molfile.c_str());
        MolfileLoader loader(scanner);
        loader.loadMolecule(mol);
    }

    static std::string wrap(const std::string& atoms, const std::string& bonds, int atom_count, int bond_count)
    {
        return "\n\n\n"
               "  0  0  0     0  0            999 V3000\n"
               "M  V30 BEGIN CTAB\n"
               "M  V30 COUNTS " +
               std::to_string(atom_count) + " " + std::to_string(bond_count) +
               " 0 0 0\n"
               "M  V30 BEGIN ATOM\n" +
               atoms +
               "M  V30 END ATOM\n"
               "M  V30 BEGIN BOND\n" +
               bonds +
               "M  V30 END BOND\n"
               "M  V30 END CTAB\n"
               "M  END\n";
    }

    static DativeModel::AtomResult computeFor(Molecule& mol, int atom_idx)
    {
        DativeModel model(mol);
        DativeModel::AtomResult result;
        EXPECT_TRUE(model.compute(atom_idx, result));
        return result;
    }
};

// Specification example 1: Ni(2+) on its own — 4 donor or 5 acceptor bonds.
TEST_F(IndigoCoreDativeModelTest, example1_nickel_dication)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Ni 0 0 0 0 CHG=2\n", "", 1, 0)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(8, r.el);
    EXPECT_EQ(9, r.orb);
    EXPECT_EQ(4, r.max_donor);
    EXPECT_EQ(5, r.max_acceptor);
    EXPECT_FALSE(r.valence_error);
}

// Specification example 2: carbon with three bonds and a monoradical — every orbital is
// taken, so it can be neither donor nor acceptor. Both gates fail on `n > 0`, not on
// `Or >= n`, which is what makes this example distinct from the hypervalent one below.
TEST_F(IndigoCoreDativeModelTest, example2_carbon_with_monoradical)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 C 0 0 0 0 RAD=2\n"
                                        "M  V30 2 C 1.5 0 0 0\n"
                                        "M  V30 3 C -1.5 0 0 0\n"
                                        "M  V30 4 C 0 1.5 0 0\n",
                                        "M  V30 1 1 1 2\n"
                                        "M  V30 2 1 1 3\n"
                                        "M  V30 3 1 1 4\n",
                                        4, 3)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(0, r.el);
    EXPECT_EQ(0, r.orb);
    EXPECT_EQ(0, r.max_donor);
    EXPECT_EQ(0, r.max_acceptor);
}

// Specification example 4: the pyrrole nitrogen. The example is stated on the
// dearomatized structure (two single bonds, bonds = 2), so it doubles as the reference
// case for requirement 1.1 — loaded here as aromatic, and the model must still see 2.
//
// The ticket prints the acceptor gate as `1 >= 0 > 0` while its own Or is 2; the printed
// left-hand side was not updated when the example was recomputed on 2026-08-04. The
// conclusion is unaffected (the gate fails on `0 > 0`), and the computed values are what
// is pinned here — see DESIGN.md section 6.2.
TEST_F(IndigoCoreDativeModelTest, example4_pyrrole_nitrogen)
{
    Molecule mol;
    ASSERT_NO_THROW(loadMolecule("c1cc[nH]c1", mol));

    int nitrogen = -1;
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.getAtomNumber(i) == ELEM_N)
            nitrogen = i;
    ASSERT_NE(-1, nitrogen);

    const auto r = computeFor(mol, nitrogen);
    EXPECT_EQ(3, r.el) << "bonds must be 2, as on the dearomatized structure";
    EXPECT_EQ(2, r.orb);
    EXPECT_EQ(1, r.max_donor);
    EXPECT_EQ(0, r.max_acceptor) << "no arrangement of the electrons leaves an empty orbital";
}

// Specification example 3: Cl(3+) with one ordinary bond — donor or acceptor of one.
TEST_F(IndigoCoreDativeModelTest, example3_chlorine_trication)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Cl 0 0 0 0 CHG=3\n"
                                        "M  V30 2 C 1.5 0 0 0\n",
                                        "M  V30 1 1 1 2\n", 2, 1)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(3, r.el);
    EXPECT_EQ(3, r.orb);
    EXPECT_EQ(1, r.max_donor);
    EXPECT_EQ(1, r.max_acceptor);
}

// Specification example 5: Pr(1-) with four bonds and a diradical — acceptor of ten,
// donor of none. Exercises the f-block row of the Or0 table (16 orbitals).
TEST_F(IndigoCoreDativeModelTest, example5_praseodymium)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Pr 0 0 0 0 CHG=-1 RAD=3\n"
                                        "M  V30 2 C 1.5 0 0 0\n"
                                        "M  V30 3 C -1.5 0 0 0\n"
                                        "M  V30 4 C 0 1.5 0 0\n"
                                        "M  V30 5 C 0 -1.5 0 0\n",
                                        "M  V30 1 1 1 2\n"
                                        "M  V30 2 1 1 3\n"
                                        "M  V30 3 1 1 4\n"
                                        "M  V30 4 1 1 5\n",
                                        5, 4)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(0, r.el);
    EXPECT_EQ(10, r.orb);
    EXPECT_EQ(0, r.max_donor) << "all electrons are tied up in bonds and radicals";
    EXPECT_EQ(10, r.max_acceptor);
}

// Requirement 3: a donor and an acceptor bond on the same atom cancel each other.
TEST_F(IndigoCoreDativeModelTest, requirement3_donor_and_acceptor_cancel_out)
{
    // Atom 1 donates to atom 2 and accepts from atom 3.
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 N 0 0 0 0\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n"
                                        "M  V30 3 Fe -1.5 0 0 0\n",
                                        "M  V30 1 9 1 2\n"  // 1 -> 2 : atom 1 is donor
                                        "M  V30 2 9 3 1\n", // 3 -> 1 : atom 1 is acceptor
                                        3, 2)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(0, r.donor_bonds) << "one donor and one acceptor bond must cancel";
    EXPECT_EQ(0, r.acceptor_bonds);
    EXPECT_FALSE(r.valence_error);
}

// Direction matters: beg is the donor side, end the acceptor side.
TEST_F(IndigoCoreDativeModelTest, bond_direction_selects_donor_and_acceptor)
{
    Molecule donor_side;
    ASSERT_NO_THROW(loadV3000(donor_side, wrap("M  V30 1 N 0 0 0 0\n"
                                               "M  V30 2 Fe 1.5 0 0 0\n",
                                               "M  V30 1 9 1 2\n", 2, 1)));
    const auto donating = computeFor(donor_side, 0);
    EXPECT_EQ(1, donating.donor_bonds);
    EXPECT_EQ(0, donating.acceptor_bonds);

    Molecule acceptor_side;
    ASSERT_NO_THROW(loadV3000(acceptor_side, wrap("M  V30 1 N 0 0 0 0\n"
                                                  "M  V30 2 Fe 1.5 0 0 0\n",
                                                  "M  V30 1 9 2 1\n", 2, 1)));
    const auto accepting = computeFor(acceptor_side, 0);
    EXPECT_EQ(0, accepting.donor_bonds);
    EXPECT_EQ(1, accepting.acceptor_bonds);
}

// Requirement 6: more dative bonds than the atom can hold is a valence error.
// Fluorine has no free orbital, so it can never accept.
TEST_F(IndigoCoreDativeModelTest, requirement6_fluorine_cannot_accept)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 F 0 0 0 0\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n",
                                        "M  V30 1 9 2 1\n", 2, 1)));

    const auto r = computeFor(mol, 0);
    EXPECT_EQ(0, r.max_acceptor) << "fluorine has no empty orbital to offer";
    EXPECT_EQ(1, r.acceptor_bonds);
    EXPECT_TRUE(r.valence_error);
}

// Q2, won't-fix: a hypervalent p-block atom gets a negative Or, so both gates fail and
// it can hold no dative bond. The behaviour is accepted; pin it so it cannot drift.
TEST_F(IndigoCoreDativeModelTest, hypervalent_atom_supports_no_dative_bond)
{
    // [SiF5]- style: silicon with five ordinary bonds plus one dative bond.
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Si 0 0 0 0\n"
                                        "M  V30 2 F 1.5 0 0 0\n"
                                        "M  V30 3 F -1.5 0 0 0\n"
                                        "M  V30 4 F 0 1.5 0 0\n"
                                        "M  V30 5 F 0 -1.5 0 0\n"
                                        "M  V30 6 F 1.0 1.0 0 0\n"
                                        "M  V30 7 Fe -1.0 -1.0 0 0\n",
                                        "M  V30 1 1 1 2\n"
                                        "M  V30 2 1 1 3\n"
                                        "M  V30 3 1 1 4\n"
                                        "M  V30 4 1 1 5\n"
                                        "M  V30 5 1 1 6\n"
                                        "M  V30 6 9 1 7\n",
                                        7, 6)));

    const auto r = computeFor(mol, 0);
    EXPECT_LT(r.orb, 0) << "five bonds exceed the four orbitals of the p-block model";
    EXPECT_EQ(0, r.max_donor);
    EXPECT_EQ(0, r.max_acceptor);
    EXPECT_TRUE(r.valence_error);
}

// Requirement 1.1 — the property the dearomatize step buys us: a molecule and its
// Kekule form must give identical results. Comparing one molecule against a
// dearomatized clone of itself keeps the atom indices aligned, which two separately
// parsed SMILES would not.
TEST_F(IndigoCoreDativeModelTest, aromatic_and_kekule_forms_agree)
{
    const char* aromatics[] = {
        "c1ccncc1",   // pyridine — nitrogen keeps a double bond in Kekule form
        "c1cc[nH]c1", // pyrrole  — nitrogen donates its lone pair to the ring
        "c1ccoc1",    // furan
        "c1ccsc1",    // thiophene
        "c1ccccc1",   // benzene — control, no heteroatom
    };

    for (const char* smiles : aromatics)
    {
        Molecule aromatic;
        ASSERT_NO_THROW(loadMolecule(smiles, aromatic)) << smiles;

        Molecule kekule;
        kekule.clone(aromatic);
        AromaticityOptions options;
        options.unique_dearomatization = false;
        ASSERT_NO_THROW(MoleculeDearomatizer::dearomatizeMolecule(kekule, options)) << smiles;

        DativeModel aromatic_model(aromatic);
        DativeModel kekule_model(kekule);

        for (int i = aromatic.vertexBegin(); i != aromatic.vertexEnd(); i = aromatic.vertexNext(i))
        {
            DativeModel::AtomResult from_aromatic, from_kekule;
            ASSERT_TRUE(aromatic_model.compute(i, from_aromatic)) << smiles << " atom " << i;
            ASSERT_TRUE(kekule_model.compute(i, from_kekule)) << smiles << " atom " << i;

            EXPECT_EQ(from_kekule.el, from_aromatic.el) << smiles << " atom " << i << ": El differs between representations";
            EXPECT_EQ(from_kekule.orb, from_aromatic.orb) << smiles << " atom " << i << ": Or differs between representations";
            EXPECT_EQ(from_kekule.max_donor, from_aromatic.max_donor) << smiles << " atom " << i;
            EXPECT_EQ(from_kekule.max_acceptor, from_aromatic.max_acceptor) << smiles << " atom " << i;
        }
    }
}

// The gate lives in Molecule, not in the model, so it is tested through behaviour --
// see atoms_without_dative_bonds_are_left_alone and hydrogens_elsewhere_are_left_alone
// below. What belongs here is the model's own contract: it answers for the atom it is
// given, including atoms its caller would never ask about.
TEST_F(IndigoCoreDativeModelTest, compute_answers_for_any_atom_it_is_given)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 N 0 0 0 0\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n"
                                        "M  V30 3 C -1.5 0 0 0\n",
                                        "M  V30 1 9 1 2\n"
                                        "M  V30 2 1 1 3\n",
                                        3, 2)));

    DativeModel model(mol);

    DativeModel::AtomResult carbon;
    ASSERT_TRUE(model.compute(2, carbon)) << "the carbon has no dative bond, but is still computable";
    EXPECT_EQ(0, carbon.donor_bonds);
    EXPECT_EQ(0, carbon.acceptor_bonds);
    EXPECT_FALSE(carbon.valence_error);
}

// The tests above exercise the model directly. The ones below exercise the path a user
// actually travels: Molecule::getAtomValence and everything layered on top of it.

class IndigoCoreDativeValenceIntegrationTest : public IndigoCoreDativeModelTest
{
protected:
    // Fluorine accepting a dative bond: it has no empty orbital to offer, so requirement 6
    // must report it. Atom 0 is the fluorine, atom 1 the iron, atom 2 a carbon that has no
    // dative bond of its own.
    static void loadAcceptingFluorine(Molecule& mol)
    {
        ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 F 0 0 0 0\n"
                                            "M  V30 2 Fe 1.5 0 0 0\n"
                                            "M  V30 3 C -1.5 0 0 0\n",
                                            "M  V30 1 9 2 1\n", 3, 1)));
    }
};

// Requirement 6 reaches the caller through the existing channel, so nothing downstream
// needed changing to see it.
TEST_F(IndigoCoreDativeValenceIntegrationTest, valence_error_is_thrown_by_getAtomValence)
{
    Molecule mol;
    loadAcceptingFluorine(mol);

    EXPECT_THROW(mol.getAtomValence(0), Element::Error);
}

// The no-throw entry point is what StructureChecker's CHECK_MSG_VALENCE calls, so a
// dative valence error surfaces there as -1 like any other valence error.
TEST_F(IndigoCoreDativeValenceIntegrationTest, valence_error_is_visible_to_no_throw_callers)
{
    Molecule mol;
    loadAcceptingFluorine(mol);

    EXPECT_EQ(-1, mol.getAtomValence_NoThrow(0, -1));
}

// Requirement 9 was dropped from the specification, so no contract governs the
// interaction with the ignore-bad-valence flag. Treating a dative valence error like
// every other valence error is the consistent choice; pin it.
TEST_F(IndigoCoreDativeValenceIntegrationTest, valence_error_is_suppressed_by_the_ignore_flag)
{
    Molecule mol;
    loadAcceptingFluorine(mol);
    mol.setIgnoreBadValenceFlag(true);

    EXPECT_NO_THROW(mol.getAtomValence(0));
}

// The isolation invariant at the integration level: an atom that carries no dative bond
// keeps its behaviour even when it shares a molecule with one that does.
TEST_F(IndigoCoreDativeValenceIntegrationTest, atoms_without_dative_bonds_are_left_alone)
{
    Molecule mol;
    loadAcceptingFluorine(mol);

    EXPECT_EQ(4, mol.getAtomValence(2)) << "the carbon is unrelated to the dative bond";
}

// Requirement 7, and the step the ticket exists to produce: chlorine(3+) with one
// ordinary bond has no hydrogen today, and gains one as soon as a dative bond is drawn
// on it. El_remaining = 3 - 2 = 1, Or_remaining = 3 - 1 = 2, and 1 <= 2, so the count is
// El_remaining itself.
TEST_F(IndigoCoreDativeValenceIntegrationTest, donor_dative_bond_adds_a_hydrogen)
{
    Molecule without_dative;
    ASSERT_NO_THROW(loadV3000(without_dative, wrap("M  V30 1 Cl 0 0 0 0 CHG=3\n"
                                                   "M  V30 2 C 1.5 0 0 0\n",
                                                   "M  V30 1 1 1 2\n", 2, 1)));
    ASSERT_EQ(0, without_dative.getImplicitH(0)) << "baseline: no hydrogen without a dative bond";

    Molecule with_dative;
    ASSERT_NO_THROW(loadV3000(with_dative, wrap("M  V30 1 Cl 0 0 0 0 CHG=3\n"
                                                "M  V30 2 C 1.5 0 0 0\n"
                                                "M  V30 3 Fe -1.5 0 0 0\n",
                                                "M  V30 1 1 1 2\n"
                                                "M  V30 2 9 1 3\n", // chlorine donates
                                                3, 2)));

    EXPECT_EQ(1, with_dative.getImplicitH(0));
}

// The other branch of the requirement 7 formula. As an acceptor the chlorine keeps its
// electrons: El_remaining = 3, Or_remaining = 2, so 2*Or > El > Or and the count is
// 2*Or_remaining - El_remaining = 1.
TEST_F(IndigoCoreDativeValenceIntegrationTest, acceptor_dative_bond_uses_the_second_branch)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Cl 0 0 0 0 CHG=3\n"
                                        "M  V30 2 C 1.5 0 0 0\n"
                                        "M  V30 3 Fe -1.5 0 0 0\n",
                                        "M  V30 1 1 1 2\n"
                                        "M  V30 2 9 3 1\n", // chlorine accepts
                                        3, 2)));

    EXPECT_EQ(1, mol.getImplicitH(0));
}

// Requirement 8: a transition metal is not drawn with hydrogens, dative bonds or not.
TEST_F(IndigoCoreDativeValenceIntegrationTest, transition_metals_get_no_hydrogens)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 N 0 0 0 0\n"
                                        "M  V30 2 Ni 1.5 0 0 0 CHG=2\n",
                                        "M  V30 1 9 1 2\n", 2, 1)));

    EXPECT_EQ(0, mol.getImplicitH(1));
}

// The same rule for noble gases, the ticket's other named example.
TEST_F(IndigoCoreDativeValenceIntegrationTest, noble_gases_get_no_hydrogens)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 Xe 0 0 0 0\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n",
                                        "M  V30 1 9 1 2\n", 2, 1)));

    EXPECT_EQ(0, mol.getImplicitH(0));
}

// The isolation invariant for requirement 7: an atom with no dative bond of its own keeps
// the hydrogen count it had before the feature existed.
TEST_F(IndigoCoreDativeValenceIntegrationTest, hydrogens_elsewhere_are_left_alone)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 N 0 0 0 0\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n"
                                        "M  V30 3 C -1.5 0 0 0\n"
                                        "M  V30 4 C -3.0 0 0 0\n",
                                        "M  V30 1 9 1 2\n"
                                        "M  V30 2 1 3 4\n",
                                        4, 2)));

    EXPECT_EQ(3, mol.getImplicitH(2)) << "a methyl carbon is unaffected by the dative bond elsewhere";
}

// An explicitly specified valence must not mask requirement 6: the error is a property of
// the bonding around the atom, not of the valence number stored on it.
TEST_F(IndigoCoreDativeValenceIntegrationTest, specified_valence_does_not_mask_the_error)
{
    Molecule mol;
    ASSERT_NO_THROW(loadV3000(mol, wrap("M  V30 1 F 0 0 0 0 VAL=1\n"
                                        "M  V30 2 Fe 1.5 0 0 0\n",
                                        "M  V30 1 9 2 1\n", 2, 1)));

    EXPECT_THROW(mol.getAtomValence(0), Element::Error);
}
