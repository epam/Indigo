from threading import Thread
import sys

sys.path.append('../../common')
from env_indigo import *


class testThread(Thread):
    def __init__(self, id, smiles):
        Thread.__init__(self)
        self.id = id
        self.smiles = smiles

    def run(self):
        indigo_1 = Indigo()
        indigo_2 = Indigo()
        indigo_1.setOption("molfile-saving-skip-date", "1")
        indigo_2.setOption("molfile-saving-skip-date", "0")

        m1 = indigo_1.loadMolecule(self.smiles)
        m2 = indigo_2.loadMolecule(self.smiles)
        m1.layout()
        m2.layout()

        c1 = str(m1.molfile()).splitlines()
        c2 = str(m2.molfile()).splitlines()

        header_1 = '\n'.join(c1[:3])
        body_1 = '\n'.join(c1[3:])
        header_2 = '\n'.join(c2[:3])
        body_2 = '\n'.join(c2[3:])
        # Check that two instances of Indigo even in one thread have different options - headers should differ
        assert header_1 != header_2
        # Bodies should be equal
        assert body_1 == body_2


threads = []
indigo = Indigo()
for i, smi in enumerate(
        indigo.iterateSmilesFile(joinPathPy("../../../../../data/molecules/basic/pubchem_slice_5000.smi", __file__))):
    current = testThread(i, smi.rawData())
    threads.append(current)
    current.start()
for thread in threads:
    thread.join()
