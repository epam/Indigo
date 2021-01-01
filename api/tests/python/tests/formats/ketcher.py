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
    print(js)
    m2 = indigo.loadMolecule(js)
    print(indigo.exactMatch(m1, m2) is not None)
