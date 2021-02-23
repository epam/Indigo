from tests import TestIndigoBase


class TestIndigo(TestIndigoBase):
    def test_version(self) -> None:
        self.assertTrue(self.indigo.version())

    def test_aromatize_smiles(self) -> None:
        m = self.indigo.loadMolecule('C1=CC=CC=C1')
        m.aromatize()
        self.assertEqual('c1ccccc1', m.smiles())
