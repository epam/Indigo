import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

print("*** Molecule ***")
m = indigo.loadMolecule("CCCCCC")
print(m.dbgInternalType())
aiter = m.iterateAtoms()
print(aiter.dbgInternalType())
for a in aiter:
    print("  " + a.dbgInternalType())

print("*** Reaction ***")
r = indigo.loadReaction("CCCCCC>>CCCCCC")
print(r.dbgInternalType())
mit = r.iterateMolecules()
print(mit.dbgInternalType())
for m in mit:
    print("  " + m.dbgInternalType())
    aiter = m.iterateAtoms()
    print("  " + aiter.dbgInternalType())
    for a in aiter:
        print("    " + a.dbgInternalType())
