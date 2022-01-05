import binascii
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)


def getAtomString(a, reaction, hasCoord):
    xyzStr = ""
    if hasCoord:
        x, y, z = [round(v, 1) for v in a.xyz()]
        xyzStr = " (%0.1f %0.1f %0.1f)" % (x, y, z)
    str = "%s%s" % (a.symbol(), xyzStr)

    if reaction:
        str += " m%d" % (reaction.atomMappingNumber(a))

    return str


def getBondString(b, reaction):
    str = "%d" % (b.bondOrder())

    if reaction:
        str += " rc%d" % (int(reaction.reactingCenter(b)))

    return str


def compareMolecules(m1, m2, r1, r2):
    print("Atoms:")
    for a1, a2 in zip(m1.iterateAtoms(), m2.iterateAtoms()):
        s1 = getAtomString(a1, r1, m1.hasCoord())
        s2 = getAtomString(a2, r2, m2.hasCoord())
        print("%d, %d: %s - %s" % (a1.index(), a2.index(), s1, s2))
        # if s1 != s2:
        #    sys.stderr.write("Error: %s != %s" % (s1, s2))

    print("Bonds:")
    for b1, b2 in zip(m1.iterateBonds(), m2.iterateBonds()):
        s1 = getBondString(b1, r1)
        s2 = getBondString(b2, r2)
        print("%d, %d: %s - %s" % (b1.index(), b2.index(), s1, s2))
        # if s1 != s2:
        #    sys.stderr.write("Error: %s != %s" % (s1, s2))


indigo.setOption("serialize-preserve-ordering", True)

print("Molecules")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ordering.mol", __file__)
)
buf = mol.serialize()
mol2 = indigo.unserialize(buf)
compareMolecules(mol, mol2, None, None)

print("Reactions")
rxn = indigo.loadReactionFromFile(
    joinPathPy("reactions/ordering.rxn", __file__)
)
buf = rxn.serialize()
rxn2 = indigo.unserialize(buf)
for mol in rxn.iterateMolecules():
    mol2 = rxn2.getMolecule(mol.index())
    compareMolecules(mol, mol2, rxn, rxn2)

print("After changes")
mol = indigo.loadMolecule("CNCNCNCN")
mol.getAtom(2).remove()
buf = mol.serialize()
mol2 = indigo.unserialize(buf)
compareMolecules(mol, mol2, None, None)
