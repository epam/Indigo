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

// Characterization tests for the interaction between dative/hydrogen bonds and
// valence calculation (ticket #3617, phase 1).
//
// Invariant under test: a dative bond (order 9) and a hydrogen bond (order 10)
// must not contribute to an atom's valence. Molecule::calcAtomConnectivity_noImplH
// already honours this for aliphatic atoms, but Molecule::calcAromaticAtomConnectivity
// does not, so the invariant breaks for aromatic atoms.

#include <gtest/gtest.h>

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/molecule.h>
#include <molecule/molecule_json_loader.h>
#include <molecule/molecule_json_saver.h>
#include <molecule/molecule_standardize.h>
#include <molecule/molecule_standardize_options.h>
#include <molecule/molfile_loader.h>
#include <molecule/molfile_saver.h>

#include "common.h"

using namespace indigo;

class IndigoCoreDativeValenceTest : public IndigoCoreTest
{
protected:
    // Five-membered aromatic ring (thiophene/phosphole skeleton) whose heteroatom
    // optionally carries one extra bond to an iron atom.
    //
    // hetero      — element symbol of the ring heteroatom, e.g. "S" or "P"
    // extra_order — 0 to omit the extra bond, otherwise the bond order to add
    //               (9 = coordination/dative, 10 = hydrogen)
    static std::string aromaticRingWithExtraBond(const char* hetero, int extra_order)
    {
        std::string mol = "\n\n\n"
                          "  0  0  0     0  0            999 V3000\n"
                          "M  V30 BEGIN CTAB\n";
        mol += extra_order == 0 ? "M  V30 COUNTS 5 5 0 0 0\n" : "M  V30 COUNTS 6 6 0 0 0\n";
        mol += "M  V30 BEGIN ATOM\n";
        mol += std::string("M  V30 1 ") + hetero + " 0 0 0 0\n";
        mol += "M  V30 2 C 1.2 0.8 0 0\n"
               "M  V30 3 C 2.4 0 0 0\n"
               "M  V30 4 C 2.4 -1.4 0 0\n"
               "M  V30 5 C 1.2 -2.2 0 0\n";
        if (extra_order != 0)
            mol += "M  V30 6 Fe -1.6 0 0 0\n";
        mol += "M  V30 END ATOM\n"
               "M  V30 BEGIN BOND\n"
               "M  V30 1 4 1 2\n"
               "M  V30 2 4 2 3\n"
               "M  V30 3 4 3 4\n"
               "M  V30 4 4 4 5\n"
               "M  V30 5 4 5 1\n";
        if (extra_order != 0)
            mol += "M  V30 6 " + std::to_string(extra_order) + " 1 6\n";
        mol += "M  V30 END BOND\n"
               "M  V30 END CTAB\n"
               "M  END\n";
        return mol;
    }

    static void load(Molecule& mol, const std::string& molfile)
    {
        BufferScanner scanner(molfile.c_str());
        MolfileLoader loader(scanner);
        loader.loadMolecule(mol);
    }

    static void loadKet(Molecule& mol, const char* json)
    {
        rapidjson::Document data;
        ASSERT_FALSE(data.Parse(json).HasParseError());
        MoleculeJsonLoader loader(data);
        loader.loadMolecule(mol);
    }

    static std::string saveKet(Molecule& mol)
    {
        Array<char> buffer;
        ArrayOutput output(buffer);
        MoleculeJsonSaver saver(output);
        saver.saveMolecule(mol);
        return std::string(buffer.ptr(), static_cast<size_t>(buffer.size()));
    }

    // Chlorine(3+) with one ordinary bond, donating a dative bond to iron -- the molecule
    // the requirement 7 step is stated on. `donor_first` decides which way the arrow runs.
    static std::string chlorineKet(bool donor_first)
    {
        return std::string("{\"root\":{\"nodes\":[{\"$ref\":\"mol0\"}]},\"mol0\":{\"type\":\"molecule\",\"atoms\":["
                           "{\"label\":\"Cl\",\"charge\":3,\"location\":[0,0,0]},"
                           "{\"label\":\"C\",\"location\":[1.5,0,0]},"
                           "{\"label\":\"Fe\",\"location\":[-1.5,0,0]}],\"bonds\":["
                           "{\"type\":1,\"atoms\":[0,1]},"
                           "{\"type\":9,\"atoms\":[") +
               (donor_first ? "0,2" : "2,0") + "]}]}}";
    }
};

// Baseline: the aromatic heteroatom on its own. Guards the reference values the
// dative cases below are compared against.
TEST_F(IndigoCoreDativeValenceTest, aromatic_heteroatom_baseline_valence)
{
    Molecule sulfur;
    ASSERT_NO_THROW(load(sulfur, aromaticRingWithExtraBond("S", 0)));
    EXPECT_EQ(2, sulfur.getAtomValence(0));

    Molecule phosphorus;
    ASSERT_NO_THROW(load(phosphorus, aromaticRingWithExtraBond("P", 0)));
    EXPECT_EQ(3, phosphorus.getAtomValence(0));
}

// An aliphatic atom already ignores dative bonds — the reference behaviour that
// the aromatic path is expected to match (Molecule::calcAtomConnectivity_noImplH).
TEST_F(IndigoCoreDativeValenceTest, aliphatic_atom_ignores_dative_bond)
{
    const char* without = "\n\n\n"
                          "  0  0  0     0  0            999 V3000\n"
                          "M  V30 BEGIN CTAB\n"
                          "M  V30 COUNTS 1 0 0 0 0\n"
                          "M  V30 BEGIN ATOM\n"
                          "M  V30 1 S 0 0 0 0\n"
                          "M  V30 END ATOM\n"
                          "M  V30 BEGIN BOND\n"
                          "M  V30 END BOND\n"
                          "M  V30 END CTAB\n"
                          "M  END\n";
    const char* with = "\n\n\n"
                       "  0  0  0     0  0            999 V3000\n"
                       "M  V30 BEGIN CTAB\n"
                       "M  V30 COUNTS 2 1 0 0 0\n"
                       "M  V30 BEGIN ATOM\n"
                       "M  V30 1 S 0 0 0 0\n"
                       "M  V30 2 Fe -1.6 0 0 0\n"
                       "M  V30 END ATOM\n"
                       "M  V30 BEGIN BOND\n"
                       "M  V30 1 9 1 2\n"
                       "M  V30 END BOND\n"
                       "M  V30 END CTAB\n"
                       "M  END\n";

    Molecule bare, dative;
    ASSERT_NO_THROW(load(bare, without));
    ASSERT_NO_THROW(load(dative, with));
    EXPECT_EQ(bare.getAtomValence(0), dative.getAtomValence(0));
}

// Defect #1 (ticket #3617): calcAromaticAtomConnectivity adds the raw bond order
// of dative bonds to min_conn, so the sulfur is looked up as if it had a
// connectivity of 2 + 9 = 11 and silently resolves to valence 6 instead of 2.
TEST_F(IndigoCoreDativeValenceTest, aromatic_sulfur_dative_bond_does_not_change_valence)
{
    Molecule bare, dative;
    ASSERT_NO_THROW(load(bare, aromaticRingWithExtraBond("S", 0)));
    ASSERT_NO_THROW(load(dative, aromaticRingWithExtraBond("S", 9)));

    EXPECT_EQ(bare.getAtomValence(0), dative.getAtomValence(0));
}

// Same defect, different symptom: for phosphorus no table entry matches the
// inflated min_conn, so the atom raises a spurious valence error instead of
// resolving to its normal aromatic valence.
TEST_F(IndigoCoreDativeValenceTest, aromatic_phosphorus_dative_bond_does_not_raise_valence_error)
{
    Molecule bare, dative;
    ASSERT_NO_THROW(load(bare, aromaticRingWithExtraBond("P", 0)));
    ASSERT_NO_THROW(load(dative, aromaticRingWithExtraBond("P", 9)));

    int expected = bare.getAtomValence(0);
    int actual = -1;
    ASSERT_NO_THROW(actual = dative.getAtomValence(0));
    EXPECT_EQ(expected, actual);
}

// Hydrogen bonds (order 10) are excluded from valence by the same rule.
TEST_F(IndigoCoreDativeValenceTest, aromatic_sulfur_hydrogen_bond_does_not_change_valence)
{
    Molecule bare, hbond;
    ASSERT_NO_THROW(load(bare, aromaticRingWithExtraBond("S", 0)));
    ASSERT_NO_THROW(load(hbond, aromaticRingWithExtraBond("S", 10)));

    EXPECT_EQ(bare.getAtomValence(0), hbond.getAtomValence(0));
}

// A metal-nonmetal single bond whose nonmetal is over-coordinated: the standardizer
// demotes it to a dative bond. Oxygen with three single bonds triggers that path
// (nitrogen would not — five-coordinate N is accepted as valence 4, see isNitrogenV5).
static const char* OVERCOORDINATED_OXYGEN_ON_ZINC = "\n\n\n"
                                                    "  0  0  0     0  0            999 V3000\n"
                                                    "M  V30 BEGIN CTAB\n"
                                                    "M  V30 COUNTS 4 3 0 0 0\n"
                                                    "M  V30 BEGIN ATOM\n"
                                                    "M  V30 1 O 0 0 0 0\n"
                                                    "M  V30 2 Zn 1.5 0 0 0\n"
                                                    "M  V30 3 C -1.5 0 0 0\n"
                                                    "M  V30 4 C 0 1.5 0 0\n"
                                                    "M  V30 END ATOM\n"
                                                    "M  V30 BEGIN BOND\n"
                                                    "M  V30 1 1 1 2\n"
                                                    "M  V30 2 1 1 3\n"
                                                    "M  V30 3 1 1 4\n"
                                                    "M  V30 END BOND\n"
                                                    "M  V30 END CTAB\n"
                                                    "M  END\n";

// Defect #2 (ticket #3617): _createCoordinationBonds used to store BOND_ZERO while
// _clearDativeBonds only matches _BOND_COORDINATION, so a bond created by the
// standardizer could never be cleared by it. Both now agree on order 9.
TEST_F(IndigoCoreDativeValenceTest, standardizer_creates_and_clears_the_same_representation)
{
    Molecule mol;
    ASSERT_NO_THROW(load(mol, OVERCOORDINATED_OXYGEN_ON_ZINC));

    StandardizeOptions create;
    create.create_coordination_bonds = true;
    ASSERT_NO_THROW(MoleculeStandardizer::standardize(mol, create));

    const int metal_bond = mol.findEdgeIndex(0, 1);
    ASSERT_NE(-1, metal_bond);
    EXPECT_EQ(_BOND_COORDINATION, mol.getBondOrder(metal_bond)) << "the standardizer must use the canonical dative representation";

    // Round trip: what create produced, clear must be able to remove.
    const int bonds_before = mol.edgeCount();
    StandardizeOptions clear;
    clear.clear_dative_bonds = true;
    ASSERT_NO_THROW(MoleculeStandardizer::standardize(mol, clear));
    EXPECT_EQ(bonds_before - 1, mol.edgeCount()) << "a bond created by the standardizer must be clearable by it";
}

// A copper(2+) with two dative bonds, the structure of issue3496_coordination_bond_no_val
// in formats.cpp. That test loads it as a QueryMolecule and asserts only the atom count,
// so neither the bond order, nor its direction, nor the Molecule path was covered by
// anything -- and direction is what requirements 4 and 5 are stated in terms of.
TEST_F(IndigoCoreDativeValenceTest, dative_bonds_keep_order_and_direction_on_the_molecule_path)
{
    const char* mol_text = "\n\n\n"
                           "  0  0  0     0  0            999 V3000\n"
                           "M  V30 BEGIN CTAB\n"
                           "M  V30 COUNTS 3 2 0 0 0\n"
                           "M  V30 BEGIN ATOM\n"
                           "M  V30 1 Cu 0 0 0 0 CHG=2\n"
                           "M  V30 2 N 1.5 0 0 0\n"
                           "M  V30 3 N -1.5 0 0 0\n"
                           "M  V30 END ATOM\n"
                           "M  V30 BEGIN BOND\n"
                           "M  V30 1 9 1 2\n"
                           "M  V30 2 9 1 3\n"
                           "M  V30 END BOND\n"
                           "M  V30 END CTAB\n"
                           "M  END\n";

    Molecule mol;
    ASSERT_NO_THROW(load(mol, mol_text));
    ASSERT_EQ(3, mol.vertexCount());
    ASSERT_EQ(2, mol.edgeCount());

    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        EXPECT_EQ(_BOND_COORDINATION, mol.getBondOrder(i)) << "the loader must keep order 9, not fold it into BOND_ZERO";
        EXPECT_EQ(0, mol.getEdge(i).beg) << "the copper is the donor side of both bonds as drawn";
    }

    // Neither end is over its limit here, so the model must stay silent -- this is the
    // "should not regress" case the original test was written for.
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        EXPECT_NO_THROW(mol.getAtomValence(i)) << "atom " << i;
}

// Haptic bonds (#3233) and this ticket meet in one representation: a haptic bond is
// written to V3000 as order 9 plus ENDPTS, and the loader keeps that order 9 edge in the
// graph while recording the attachment group beside it. So the dative model does see
// haptic bonds, and ferrocene -- the canonical case -- must survive it: the iron donates
// through two of them, which is well inside what its electrons and orbitals allow.
TEST_F(IndigoCoreDativeValenceTest, haptic_bonds_do_not_trip_the_dative_model)
{
    const char* ferrocene = "\n"
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

    Molecule mol;
    ASSERT_NO_THROW(load(mol, ferrocene));

    const int iron = 5; // atom 6 of the file
    ASSERT_EQ(ELEM_Fe, mol.getAtomNumber(iron));

    // Fe(2+): El = 8 - 2 = 6 and Or = 9, so up to three donor dative bonds are allowed
    // and two are drawn. Requirement 8 keeps a transition metal free of hydrogens.
    EXPECT_NO_THROW(mol.getAtomValence(iron));
    EXPECT_EQ(0, mol.getImplicitH(iron));
}

// Round trip through V3000, which is the only format that carries dative bonds today.
// V2000, CDXML and SMILES do not (defects #3-#5 of the analysis, deliberately left out of
// this ticket); pinning V3000 makes that boundary explicit rather than assumed.
TEST_F(IndigoCoreDativeValenceTest, dative_bonds_survive_a_v3000_round_trip)
{
    Molecule original;
    ASSERT_NO_THROW(load(original, aromaticRingWithExtraBond("S", _BOND_COORDINATION)));

    const int bond = original.findEdgeIndex(0, 5);
    ASSERT_NE(-1, bond);
    ASSERT_EQ(_BOND_COORDINATION, original.getBondOrder(bond));

    Array<char> buffer;
    ArrayOutput output(buffer);
    MolfileSaver saver(output);
    saver.mode = MolfileSaver::MODE_3000;
    ASSERT_NO_THROW(saver.saveMolecule(original));
    buffer.push(0);

    Molecule restored;
    ASSERT_NO_THROW(load(restored, buffer.ptr()));

    const int restored_bond = restored.findEdgeIndex(0, 5);
    ASSERT_NE(-1, restored_bond) << "the dative bond disappeared in the round trip";
    EXPECT_EQ(_BOND_COORDINATION, restored.getBondOrder(restored_bond));
    EXPECT_EQ(original.getEdge(bond).beg, restored.getEdge(restored_bond).beg) << "direction must survive too";
}

// KET is the format Ketcher speaks, and the Ketcher side of this feature
// (epam/ketcher#10427) is what the ticket exists to serve, so the contract has to hold
// there and not only in V3000. Chlorine(3+) with one ordinary bond has no hydrogen on its
// own and gains one as a donor -- the same step, reached through the other format.
TEST_F(IndigoCoreDativeValenceTest, ket_carries_dative_bonds_into_the_model)
{
    Molecule from_ket;
    ASSERT_NO_THROW(loadKet(from_ket, chlorineKet(true).c_str()));

    const int bond = from_ket.findEdgeIndex(0, 2);
    ASSERT_NE(-1, bond);
    EXPECT_EQ(_BOND_COORDINATION, from_ket.getBondOrder(bond)) << "KET must keep order 9, not fold it into a single bond";
    EXPECT_EQ(0, from_ket.getEdge(bond).beg) << "the order of \"atoms\" is the direction of the arrow";

    EXPECT_EQ(1, from_ket.getImplicitH(0));
    EXPECT_EQ(3, from_ket.getImplicitH(1)) << "the carbon is unaffected";

    // The two formats must not disagree about the same molecule.
    Molecule from_molfile;
    ASSERT_NO_THROW(load(from_molfile, "\n\n\n"
                                       "  0  0  0     0  0            999 V3000\n"
                                       "M  V30 BEGIN CTAB\n"
                                       "M  V30 COUNTS 3 2 0 0 0\n"
                                       "M  V30 BEGIN ATOM\n"
                                       "M  V30 1 Cl 0 0 0 0 CHG=3\n"
                                       "M  V30 2 C 1.5 0 0 0\n"
                                       "M  V30 3 Fe -1.5 0 0 0\n"
                                       "M  V30 END ATOM\n"
                                       "M  V30 BEGIN BOND\n"
                                       "M  V30 1 1 1 2\n"
                                       "M  V30 2 9 1 3\n"
                                       "M  V30 END BOND\n"
                                       "M  V30 END CTAB\n"
                                       "M  END\n"));
    EXPECT_EQ(from_molfile.getImplicitH(0), from_ket.getImplicitH(0));
}

// Reversing the arrow in KET has to reverse the roles, because requirements 4 and 5 are
// stated in terms of donor and acceptor rather than of the pair of atoms.
TEST_F(IndigoCoreDativeValenceTest, ket_keeps_the_direction_of_the_arrow)
{
    Molecule forward;
    ASSERT_NO_THROW(loadKet(forward, chlorineKet(true).c_str()));
    EXPECT_EQ(0, forward.getEdge(forward.findEdgeIndex(0, 2)).beg) << "chlorine donates";

    Molecule reversed;
    ASSERT_NO_THROW(loadKet(reversed, chlorineKet(false).c_str()));
    EXPECT_EQ(2, reversed.getEdge(reversed.findEdgeIndex(0, 2)).beg) << "iron donates";
}

TEST_F(IndigoCoreDativeValenceTest, ket_round_trip_keeps_the_dative_bond)
{
    Molecule original;
    ASSERT_NO_THROW(loadKet(original, chlorineKet(true).c_str()));

    const std::string saved = saveKet(original);

    // Compared without whitespace, so that turning pretty printing on or off cannot
    // decide whether this test passes.
    std::string dense;
    for (char c : saved)
        if (!isspace(static_cast<unsigned char>(c)))
            dense += c;
    EXPECT_NE(std::string::npos, dense.find("\"type\":9")) << "the saved KET no longer says the bond is dative: " << saved;

    Molecule restored;
    ASSERT_NO_THROW(loadKet(restored, saved.c_str()));

    const int bond = restored.findEdgeIndex(0, 2);
    ASSERT_NE(-1, bond) << "the dative bond disappeared in the round trip";
    EXPECT_EQ(_BOND_COORDINATION, restored.getBondOrder(bond));
    EXPECT_EQ(0, restored.getEdge(bond).beg) << "direction must survive too";
    EXPECT_EQ(1, restored.getImplicitH(0));
}
