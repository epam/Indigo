import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
smiles = [
    "Cl.C1CCCCC1",
    "Cl.C1=CC=CC=C1",
    "Cl.c1ccccc1",
    "Clc1ccccc1",
    "ClC1=CC=CC=C1",
    "ClC1CCC=CC1",
    "Cl.ClC1=CC=CC=C1",
]
qsmiles = ["([Cl]).([c])", "([Cl].[c])", "[Cl].[c]"]
for item in smiles:
    print(item)
    mol = indigo.loadMolecule(item)
    matcher = indigo.substructureMatcher(mol)
    for q in qsmiles:
        qmol = indigo.loadSmarts(q)
        cnt = matcher.countMatches(qmol)
        if cnt > 0:
            print("  %s: %d" % (q, cnt))
        qmol.optimize()
        cnt = matcher.countMatches(qmol)
        if cnt > 0:
            print("  %s (opt): %d" % (q, cnt))
