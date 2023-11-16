import os
import re
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)


def reducePrecision(str_data):
    # Remove last digit because of float-point rounding differences between Windows and Linux
    return re.sub(r"(\d.\d\d\d\d\d)\d", lambda m: "%s0" % m.group(1), str_data)


def testSerializeBadRxns(filename):
    print(relativePath(filename))
    rxn = indigo.loadReactionFromFile(filename)
    print("loaded")
    print(rxn.rxnfile())
    buf = rxn.serialize()
    print("reaction serialized to {0} bytes".format(len(buf)))
    rxn2 = indigo.unserialize(buf)
    print(reducePrecision(rxn2.rxnfile()))

    query = indigo.loadReactionSmarts(">>[*]1~[*]~[*]~[*]~[*]~[*]~1")
    matcher = indigo.substructureMatcher(rxn)
    match = matcher.match(query)
    for qmol in query.iterateMolecules():
        for atom in qmol.iterateAtoms():
            match.mapAtom(atom).highlight()
        for bond in qmol.iterateBonds():
            match.mapBond(bond).highlight()
    buf = rxn.serialize()
    print("highlighted reaction serialized to {0} bytes".format(len(buf)))
    rxn2 = indigo.unserialize(buf)
    print(reducePrecision(rxn2.rxnfile()))


testSerializeBadRxns(joinPathPy("reactions/crazystereo.rxn", __file__))
testSerializeBadRxns(
    joinPathPy("reactions/crazystereo-badvalence.rxn", __file__)
)
