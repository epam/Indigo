from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=CC=C1')
        m.aromatize()
        self.assertEqual('c1ccccc1', m.smiles())

    def test_check_single_ion(self) -> None:
        m = self.indigo.loadMolecule('[Na+].C')
        self.assertTrue(m.checkSalt())

    def test_check_multiple_ions(self) -> None:
        m = self.indigo.loadMolecule('[Na+].[Cl-].C')
        self.assertTrue(m.checkSalt())

    def test_check_no_ions(self) -> None:
        m = self.indigo.loadMolecule('c1ccccc1')
        self.assertFalse(m.checkSalt())

    def test_check_bonded_metal_atom(self) -> None:
        m = self.indigo.loadMolecule('CC[Pb](CC)(CC)CC')
        self.assertFalse(m.checkSalt())

    def test_check_bonded_nitro_group(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=C(C=C1)[N+](=O)[O-]')
        self.assertFalse(m.checkSalt())
