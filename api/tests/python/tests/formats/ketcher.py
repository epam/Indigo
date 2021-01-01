import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")


structs = ("C", "C1=CC=CC=C1", "Nc1[nH]cnc-2ncnc1-2")
for struct in structs:
    m1 = indigo.loadMolecule(struct)
    m1.layout()
    js = m1.json()
    m2 = indigo.loadMolecule(js)
    print(indigo.exactMatch(m1, m2) is not None)

paths = ("molecules/1.ket", "molecules/sgroups.mol", "molecules/all_features_mol.mol")
for path in paths:
    m1 = indigo.loadMoleculeFromFile(joinPath(path))
    js = m1.json()
    m2 = indigo.loadMolecule(js)
    assert m1.countAtoms() == m2.countAtoms()
    for i in range(m1.countAtoms()):
        print(i, m1.getAtom(i).symbol(), m2.getAtom(i).symbol())
        assert m1.getAtom(i).symbol() == m2.getAtom(i).symbol()
    assert m1.countBonds() == m2.countBonds()
    for i in range(m2.countBonds()):
        print(i, m1.getBond(i).bondOrder(), m2.getBond(i).bondOrder())
        assert m1.getBond(i).bondOrder() == m2.getBond(i).bondOrder()
        print(i, m1.getBond(i).bondStereo(), m2.getBond(i).bondStereo())
        assert m1.getBond(i).bondStereo() == m2.getBond(i).bondStereo()
    print(indigo.exactMatch(m1, m2) is not None)
