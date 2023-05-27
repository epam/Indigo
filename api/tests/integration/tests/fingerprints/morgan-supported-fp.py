import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")
indigo.setOption("ignore-noncritical-query-features", "true")
indigo.setOption("ignore-bad-valence", "true")

mol = indigo.loadMolecule(
    "Cc1sc2c(C(=N[C@@H](CC(=O)OC(C)(C)C)c3nnc(C)n23)c4ccc(Cl)cc4)c1C"
)
print(mol.smiles())

for fp_type in [
    "SIM",
    "CHEM",
    "ECFP2",
    "ECFP4",
    "ECFP6",
    "ECFP8",
    "FCFP2",
    "FCFP4",
    "FCFP6",
    "FCFP8",
    "lmao6",
]:
    print(fp_type)

    try:
        indigo.setOption("similarity-type", fp_type)
    except IndigoException as e:
        print("setOption: %s" % getIndigoExceptionText(e))
        continue

    try:
        f = mol.fingerprint("sim")
        print(f.toString())
    except IndigoException as e:
        print("fingerprint: %s" % getIndigoExceptionText(e))
