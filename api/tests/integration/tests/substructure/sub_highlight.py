import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def testSubstructureHL():
    query = indigo.loadQueryMolecule("C=CC[H,O]")
    target = indigo.loadMolecule("[H]C([H])=C([H])C")
    matcher = indigo.substructureMatcher(target)
    match = matcher.match(query)
    for item in query.iterateAtoms():
        atom = match.mapAtom(item)
        if atom is None:
            continue
        atom.highlight()

        for nei in atom.iterateNeighbors():
            if (
                not nei.isPseudoatom()
                and not nei.isRSite()
                and nei.atomicNumber() == 1
            ):
                nei.highlight()
                nei.bond().highlight()
    for bond in query.iterateBonds():
        bond = match.mapBond(bond)
        if bond is None:
            continue
        bond.highlight()
    print(target.smiles())
    target.unhighlight()
    print(target.smiles())


testSubstructureHL()
