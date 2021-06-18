import os
import sys

sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

print("*** Similarity test ***")
aspirin = 'O=C(Oc1ccccc1C(=O)O)C'
mol = indigo.loadMolecule(aspirin)
fp = mol.fingerprint('sim')
print("sim=%0.3f" % indigo.similarity(fp, fp, 'tanimoto'))

print("*** Similarity fingerprints test ***")
indigo.setOption("fp-sim-qwords", 1)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
mol = indigo.loadMolecule("[Cl]c1ccccc1(N)OC(=P)C(CC)C1CCC1")
fp = mol.fingerprint("sim")
print(len(fp.toBuffer()))
print(fp.toString())
indigo.setOption("fp-ext-enabled", False)
fp = mol.fingerprint("sim")
print(len(fp.toBuffer()))
print(fp.toString())
indigo.setOption("fp-sim-qwords", 8)
fp = mol.fingerprint("sim")
print(len(fp.toBuffer()))
print(fp.toString())
indigo.setOption("fp-ext-enabled", True)
fp = mol.fingerprint("sim")
print(len(fp.toBuffer()))
print(fp.toString())

print("*** Similarity bits test ***")


def bitCnt(fp):
    ''' Simple and slow method to count number of bits '''
    return sum([bin(int(b)).count("1") for b in fp.toBuffer()])


mols = ['C', 'N', 'CN', 'CCCCCCN']
for mstr in mols:
    m = indigo.loadMolecule(mstr)
    q = indigo.loadQueryMolecule(mstr)
    qtypes = ['sim', 'sub', 'sub-tau', 'sub-res']
    ttypes = qtypes + ['full']
    print(mstr)
    for t in ttypes:
        print("  as target '%s': %d" % (t, bitCnt(m.fingerprint(t))))
    for t in qtypes:
        print("  as query '%s': %d" % (t, bitCnt(q.fingerprint(t))))

print("*** Undefined hydrogens ***")
m = indigo.loadMolecule("Clc1cc(C=NNc2nc(NCc3ccccc3)nc(c2)C)ccc1")
fp = m.fingerprint("full")
print(fp.toString())
