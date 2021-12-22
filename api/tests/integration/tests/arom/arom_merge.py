import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

print("**** Merge aromatized and dearomatized structures")

indigo = Indigo()
ma = indigo.loadMolecule("c1ccccc1")
md = indigo.loadMolecule("c1ccccc1")
md.dearomatize()
ma.merge(md)
print(ma.smiles())
ma = indigo.loadMolecule("C1=CC=CC=C1")
ma.aromatize()
md = indigo.loadMolecule("C1=CC=CC=C1")
md.merge(ma)
print(md.smiles())

m = indigo.loadMolecule("c1ccccc1.c1ccccc1")
c = m.component(0)
cc = c.clone()
cc.dearomatize()
m.removeAtoms([0, 1, 2, 3, 4, 5])
m.merge(cc)
print(m.smiles())
