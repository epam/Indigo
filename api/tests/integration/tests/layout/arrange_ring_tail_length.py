import math
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

indigo = Indigo()

print("*** Arrange ring tail length ***")

library = indigo.loadMonomerLibraryFromFile(
    dataPath("molecules/basic/monomer_library.ket")
)
helm = (
    "PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C}$"
    "PEPTIDE1,PEPTIDE1,5:R3-8:R3$$$V2.0"
)
ket_doc = indigo.loadHelm(helm, library)
mol = indigo.loadMolecule(ket_doc.json())

for atom in mol.iterateAtoms():
    if atom.index() >= 4:
        atom._lib().indigoSelect(atom.id)

mol.layout()

expected = 1.5
tolerance = 0.05
tail_pairs = [(7, 8), (8, 9), (9, 10), (10, 11), (11, 12)]

ok = True
for start_idx, end_idx in tail_pairs:
    start_atom = mol.getAtom(start_idx)
    end_atom = mol.getAtom(end_idx)
    sx, sy, _ = start_atom.xyz()
    ex, ey, _ = end_atom.xyz()
    dist = math.sqrt((sx - ex) ** 2 + (sy - ey) ** 2)
    if abs(dist - expected) > tolerance:
        ok = False
        break

if ok:
    print("arrange_ring_tail_length:SUCCEED")
else:
    print("arrange_ring_tail_length:FAILED")
