import os
from typing import Tuple

from indigo import IndigoObject
from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        m.aromatize()
        self.assertEqual("c1ccccc1", m.smiles())

    def test_check_salt_monovalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Na+].C")
        m2 = self.indigo.loadMolecule("[Rb+].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Na+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Rb+.")

    def test_check_salt_divalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Ca+2].C")
        m2 = self.indigo.loadMolecule("[Zn+2].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Ca2+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Zn2+.")

    def test_check_salt_trivalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Al+3].C")
        m2 = self.indigo.loadMolecule("[Cr+3].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Al3+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Cr3+.")

    def test_check_salt_tetravalent_monoatomic_cation(self) -> None:
        m1 = self.indigo.loadMolecule("[Ru+4].C")
        m2 = self.indigo.loadMolecule("[Sn+4].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Ru4+.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Sn4+.")

    def test_check_salt_monovalent_monoatomic_anion(self) -> None:
        m1 = self.indigo.loadMolecule("[Cl-].C")
        m2 = self.indigo.loadMolecule("[F-].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains Cl-.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains F-.")

    def test_check_salt_divalent_monoatomic_anion(self) -> None:
        m1 = self.indigo.loadMolecule("[S-2].C")
        m2 = self.indigo.loadMolecule("[Se-2].C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains S2-.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains Se2-.")

    def test_check_salt_molecular_primary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("S=[Fe].C")
        m2 = self.indigo.loadMolecule("Cl[Ag]")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains ferrous sulfide."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains silver chloride."
        )

    def test_check_salt_molecular_secondary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("S=[Sn]=S.C")
        m2 = self.indigo.loadMolecule("O=[Mn]=O.C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains tin sulfide.")
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains manganese dioxide."
        )

    def test_check_salt_molecular_tertiary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("Cl[Fe](Cl)Cl.C")
        m2 = self.indigo.loadMolecule("OCl(=O)=O.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains ferric chloride."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains chloric acid."
        )

    def test_check_salt_molecular_quaternary_salt(self) -> None:
        m1 = self.indigo.loadMolecule("OS(=O)(=O)O.C")
        m2 = self.indigo.loadMolecule("OP(=O)(O)O.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains sulfuric acid."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains phosphoric acid."
        )

    def test_check_salt_complex_primary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[OH-].C")
        m2 = self.indigo.loadMolecule("[O-]Cl.C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains hypochlorite ion."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains hydroxide ion."
        )

    def test_check_salt_complex_secondary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[O-]I=O.C")
        m2 = self.indigo.loadMolecule("[O-]N(=O).C")
        self.assertTrue(m1.checkSalt(), f"{m1.smiles()} contains nitrite ion.")
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains iodite ion.")

    def test_check_salt_complex_tertiary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("[N+](=O)([O-])[O-].C")
        m2 = self.indigo.loadMolecule("O[Se](=O)[O-].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains hydrogenselenite ion."
        )
        self.assertTrue(m2.checkSalt(), f"{m2.smiles()} contains nitrate ion.")

    def test_check_salt_complex_quaternary_ion(self) -> None:
        m1 = self.indigo.loadMolecule("OP(=O)(O)[O-].C")
        m2 = self.indigo.loadMolecule("OS(=O)(=O)[O-].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains dihydrogenphosphate ion."
        )
        self.assertTrue(
            m2.checkSalt(), f"{m2.smiles()} contains dihydrogensulfate ion."
        )

    def test_check_salt_multiple_ions(self) -> None:
        # TODO: update for counting matches
        m1 = self.indigo.loadMolecule("[Na+].[Cl-].C")
        m2 = self.indigo.loadMolecule("[O-]S(=O)(=O)[O-].[K+].[K+].C")
        self.assertTrue(
            m1.checkSalt(), f"{m1.smiles()} contains sodium and chloride ions."
        )
        self.assertTrue(
            m2.checkSalt(),
            f"{m2.smiles()} contains potassium and sulfate ions.",
        )

    def test_check_salt_no_ions(self) -> None:
        m1 = self.indigo.loadMolecule("c1ccccc1")
        m2 = self.indigo.loadMolecule("C1=CC=C2C=CC=CC2=C1")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_salt_bonded_metal_atom(self) -> None:
        m1 = self.indigo.loadMolecule("CC[Pb](CC)(CC)CC")
        m2 = self.indigo.loadMolecule("C[Al](C)C")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_check_salt_bonded_acid_group(self) -> None:
        m1 = self.indigo.loadMolecule("C1=CC=C(C=C1)[N+](=O)[O-]")
        m2 = self.indigo.loadMolecule("C(C(=O)O)S(=O)(=O)O")
        self.assertFalse(
            m1.checkSalt(), f"{m1.smiles()} doesn`t contain any salts."
        )
        self.assertFalse(
            m2.checkSalt(), f"{m2.smiles()} doesn`t contain any salts."
        )

    def test_strip_salt_no_salts(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1")
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} doesn't contain disconnected inorganic components.",
        )

    def test_strip_salt_single_salt(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains [Cl-] anion.",
        )

    def test_strip_salt_many_salts(self) -> None:
        m = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.O.O.O.O.O.O.O.O.O.O.[Cl-].[Cl-]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains water molecules and [Cl-] anions.",
        )

    def test_strip_salt_two_organics(self) -> None:
        m = self.indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
            "[O-]S(=O)(=O)[O-]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            f"{m.smiles()} contains [O-]S(=O)(=O)[O-] anion.",
        )

    def test_strip_salt_complex_salt(self) -> None:
        m = self.indigo.loadMolecule(
            "C(C(C(C(C(C(=O)O)O)O)O)O)O.C(C(C(C(C(C(=O)O)O)O)O)O)O.[Fe]"
        )
        self.assertEqual(
            m.stripSalt().smiles(),
            "C(O)C(O)C(O)C(O)C(O)C(O)=O.C(O)C(O)C(O)C(O)C(O)C(O)=O.[Fe]",
            f"{m.smiles()} doesn't contain disconnected inorganic components.",
        )

    def test_strip_salt_only_salts(self) -> None:
        m = self.indigo.loadMolecule("[NH4+].[O-]P(=O)([O-])[O-].[Fe+2]")
        self.assertEqual(
            m.stripSalt().smiles(),
            "",
            f"{m.smiles()} contains [NH4+], [O-]P(=O)([O-])[O-] and [Fe+2]"
            " ions and no organic components.",
        )

    def test_strip_salt_options(self) -> None:
        m = self.indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
        m_strip = m.stripSalt()
        m.stripSalt(inplace=True)

        self.assertEqual(m_strip.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1")
        self.assertNotEqual(m.smiles(), "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.[Cl-]")

    def test_strip_salt_ion_between_fragment_atoms(self) -> None:
        # Ethane written around the sodium: the atoms of one component are not
        # consecutive, and the sodium, not a carbon, has to go.
        m = self.indigo.loadMolecule("C1.[Na+].C1")
        self.assertEqual(m.stripSalt().smiles(), "CC")

    def test_copy_rgroups(self) -> None:
        m_with_rg = self.indigo.loadMolecule(
            "C%91C.[*:1]%91 |$;;_R1$,RG:_R1={F%91.Cl%92.Br%93."
            "[*:1]%91.[*:1]%92.[*:1]%93 |$;;;_AP1;_AP1;_AP1$|}|"
        )
        self.assertEqual(m_with_rg.countRGroups(), 1)
        m_no_rg = self.indigo.loadMolecule("C%91C.[*:1]%91 |$;;_R1$|")
        self.assertEqual(m_no_rg.countRGroups(), 0)
        m_no_rg.copyRGroups(m_with_rg)
        self.assertEqual(m_no_rg.countRGroups(), 1)

    def test_expanded_monomers_to_atoms_group_pseudoatoms_expanded(
        self,
    ) -> None:
        # CHEMBUGS-79: expandedMonomersToAtoms expands group pseudoatoms.
        # Molfile valid for RDKit etc.; molecularWeight() works.
        # Monomer lib from project ref; basic_structure_peg4.ket in tests/data.
        project_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", "..")
        )
        ref_dir = os.path.join(
            project_root,
            "api",
            "tests",
            "integration",
            "tests",
            "formats",
            "ref",
        )
        tests_dir = os.path.join(
            project_root,
            "api",
            "tests",
            "integration",
            "tests",
            "formats",
            "serialized",
        )
        lib_path = os.path.join(ref_dir, "monomer_library.ket")
        ket_path = os.path.join(tests_dir, "basic_structure_peg4.ket")
        self.assertTrue(
            os.path.isfile(lib_path), f"Library not found: {lib_path}"
        )
        self.assertTrue(os.path.isfile(ket_path), f"KET not found: {ket_path}")
        lib = self.indigo.loadMonomerLibraryFromFile(lib_path)
        mol = self.indigo.loadMoleculeWithLibFromFile(ket_path, lib)
        expanded = mol.expandedMonomersToAtoms()
        molfile = expanded.molfile()
        msg = (
            "Group pseudoatom OH should be expanded to explicit atoms "
            "in molfile."
        )
        self.assertNotIn(" OH ", molfile, msg)
        _ = expanded.molecularWeight()


class TestSaltsOfHapticComplex(TestIndigoBase):
    """The ionic drawing of a ferrocene with its rings bound to the iron by
    haptic bonds: the iron has no ordinary bond, as a lone ion has none."""

    RING_SIZE = 5
    FERROCENE_ATOMS = 2 * RING_SIZE + 1  # two rings and the iron
    FERROCENE_HAPTIC_BONDS = 2  # one from each ring to the iron

    def _bind_rings(
        self, smiles: str, ring_starts: Tuple[int, int], iron: int
    ) -> IndigoObject:
        m = self.indigo.loadMolecule(smiles)
        for first in ring_starts:
            ring = list(range(first, first + self.RING_SIZE))
            m.addHapticBond(m.addAttachmentGroup(ring), m.getAtom(iron))
        return m

    def _assert_whole_ferrocene(self, m: IndigoObject) -> None:
        self.assertEqual(m.countAtoms(), self.FERROCENE_ATOMS)
        self.assertEqual(m.countHapticBonds(), self.FERROCENE_HAPTIC_BONDS)

    def test_check_salt_finds_the_counter_ion_only(self) -> None:
        m = self._bind_rings("[cH-]1cccc1.[Fe+2].[cH-]1cccc1", (0, 6), 5)
        self.assertFalse(m.checkSalt(), "the iron is bonded to the rings")
        m.merge(self.indigo.loadMolecule("[Cl-]"))
        self.assertTrue(m.checkSalt(), "the chloride is bonded to nothing")

    def test_strip_salt_keeps_the_complex(self) -> None:
        m = self._bind_rings("[cH-]1cccc1.[Fe+2].[cH-]1cccc1", (0, 6), 5)
        self._assert_whole_ferrocene(m.stripSalt())

    def test_strip_salt_removes_the_ion_between_complex_atoms(self) -> None:
        m = self._bind_rings(
            "[cH-]1cccc1.[Cl-].[cH-]1cccc1.[Fe+2]", (0, 6), 11
        )
        self._assert_whole_ferrocene(m.stripSalt())
        self.assertEqual(
            m.countAtoms(), self.FERROCENE_ATOMS + 1, "a copy was stripped"
        )
        m.stripSalt(inplace=True)
        self._assert_whole_ferrocene(m)

    def test_strip_salt_keeps_a_complex_with_inorganic_ligands(self) -> None:
        # Zeise's salt. By its ordinary bonds alone the platinum with three
        # chlorides is a small inorganic ion; the haptic bond to the ethylene
        # makes it a part of the complex. The potassium is bonded to nothing.
        m = self.indigo.loadMolecule("[K+].Cl[Pt-](Cl)Cl.C=C")
        platinum, ethylene = m.getAtom(2), m.addAttachmentGroup([5, 6])
        m.addHapticBond(ethylene, platinum)
        self.assertTrue(m.checkSalt(), "the potassium is a counter ion")
        stripped = m.stripSalt()
        self.assertEqual(
            sorted(atom.symbol() for atom in stripped.iterateAtoms()),
            ["C", "C", "Cl", "Cl", "Cl", "Pt"],
        )
