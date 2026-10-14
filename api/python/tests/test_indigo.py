import os

from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule("C1=CC=CC=C1")
        m.aromatize()
        self.assertEqual("c1ccccc1", m.smiles())

    def test_check_salt(self) -> None:
        self.assertIs(self.indigo.loadMolecule("CCO.[Na+]").checkSalt(), True)
        self.assertIs(self.indigo.loadMolecule("CCO").checkSalt(), False)

    def test_strip_salt(self) -> None:
        m = self.indigo.loadMolecule("CCO.[Na+]")
        self.assertEqual(m.stripSalt().smiles(), "CCO")
        self.assertEqual(m.smiles(), "CCO.[Na+]", "a copy was stripped")
        self.assertIs(m.stripSalt(inplace=True), m)
        self.assertEqual(m.smiles(), "CCO")

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
