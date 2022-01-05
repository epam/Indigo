import os
import re
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)


def reducePrecision(str_data):
    # Remove last digit because of float-point rounding differences between Windows and Linux
    return re.sub("(\d.\d\d\d\d\d)\d", lambda m: "%s0" % m.group(1), str_data)


def testSerializeBadMols(filename):
    print(relativePath(filename))
    mol = indigo.loadMoleculeFromFile(filename)
    buf = mol.serialize()
    print("molecule serialized to {0} bytes".format(len(buf)))
    mol2 = indigo.unserialize(buf)
    print(reducePrecision(mol2.molfile()))

    query = indigo.loadSmarts("[*]1~[*]~[*]~[*]~[*]~[*]~1")
    matcher = indigo.substructureMatcher(mol)
    for match in matcher.iterateMatches(query):
        for atom in query.iterateAtoms():
            match.mapAtom(atom).highlight()
        for bond in query.iterateBonds():
            match.mapBond(bond).highlight()
    buf = mol.serialize()
    print("highlighted molecule serialized to {0} bytes".format(len(buf)))
    mol2 = indigo.unserialize(buf)
    print(reducePrecision(mol2.molfile()))


testSerializeBadMols(joinPathPy("molecules/crazystereo.mol", __file__))
testSerializeBadMols(
    joinPathPy("molecules/crazystereo-badvalence.mol", __file__)
)
