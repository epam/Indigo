import math
import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")
indigo.setOption("ignore-noncritical-query-features", "true")
indigo.setOption("ignore-bad-valence", "true")
indigo.setOption("similarity-type", "ECFP6")

mol = indigo.loadMolecule("Cc1sc2c(C(=N[C@@H](CC(=O)OC(C)(C)C)c3nnc(C)n23)c4ccc(Cl)cc4)c1C")
fp = mol.fingerprint("sim")

for i, m in enumerate(indigo.iterateSmilesFile(joinPathPy("molecules/b2000.smi", __file__))):
    f = m.fingerprint("sim")

    print(i + 1)
    print(m.smiles())
    print(round(indigo.similarity(fp, f), 6))
