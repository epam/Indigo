import shutil
import tempfile

from indigo.bingo import Bingo

from tests import TestIndigoBase


class TestBingo(TestIndigoBase):
    def setUp(self) -> None:
        super().setUp()
        self.test_folder = tempfile.mkdtemp()

    def tearDown(self) -> None:
        shutil.rmtree(self.test_folder)

    def test_molecule_search_sub(self) -> None:
        bingo = Bingo.createDatabaseFile(self.indigo, self.test_folder, 'molecule', '')
        self.assertTrue(bingo)
        m1 = self.indigo.loadMolecule('C1CCCCC1')
        m2 = self.indigo.loadMolecule('C1CCCCC1')
        m3 = self.indigo.loadMolecule('C1CCNCC1')
        m4 = self.indigo.loadMolecule('N')
        m1_id = bingo.insert(m1)
        m2_id = bingo.insert(m2)
        m3_id = bingo.insert(m3)
        bingo.insert(m4)
        bingo.optimize()
        q = self.indigo.loadQueryMolecule('C')
        result = bingo.searchSub(q)
        ids = []
        while result.next():
            ids.append(result.getCurrentId())
        self.assertEqual(3, len(ids))
        self.assertEqual([m1_id, m2_id, m3_id], ids)
        self.assertTrue(self.indigo.exactMatch(m1, bingo.getRecordById(m1_id)))
        self.assertTrue(self.indigo.exactMatch(m2, bingo.getRecordById(m2_id)))
        self.assertTrue(self.indigo.exactMatch(m3, bingo.getRecordById(m3_id)))
