import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def testReactionSaveLoad(filename):
    print("Loading " + relativePath(filename))
    rxn = indigo.loadReactionFromFile(filename)
    print(rxn.smiles())
    rxn2 = indigo.loadReaction(rxn.smiles())
    print(rxn2.smiles())
    if rxn.countMolecules() != rxn2.countMolecules():
        print("ERROR: rxn != rxn2")
    rxn3 = indigo.loadReaction(rxn.rxnfile())
    print(rxn3.smiles())
    if not indigo.exactMatch(rxn, rxn3):
        print("ERROR: rxn != rxn3")


def testQueryReactionSaveLoad(filename):
    print("Loading " + filename)
    rxn = indigo.loadQueryReactionFromFile(filename)
    print(rxn.smiles())
    rxn2 = indigo.loadQueryReaction(rxn.smiles())
    print(rxn2.smiles())
    if rxn.countMolecules() != rxn2.countMolecules():
        print("ERROR: rxn != rxn2")
    rxn3 = indigo.loadQueryReaction(rxn.rxnfile())
    print(rxn3.smiles())
    if rxn.countMolecules() != rxn3.countMolecules():
        print("ERROR: rxn != rxn3")


# Its stereo marks are on N atoms with a bond outside the ring, which are not
# chiral centers (issue #3853 req. 2.2), so the loader rejects it
try:
    testReactionSaveLoad(
        joinPathPy("reactions/disconnected_w_stereo.rxn", __file__)
    )
except IndigoException as e:
    print(getIndigoExceptionText(e))
testReactionSaveLoad(joinPathPy("reactions/disconnected.rxn", __file__))
testReactionSaveLoad(joinPathPy("reactions/catalysts3000.rxn", __file__))
testReactionSaveLoad(joinPathPy("reactions/catalysts2000.rxn", __file__))
testReactionSaveLoad(joinPathPy("reactions/amiderxn2.rxn", __file__))
testReactionSaveLoad(joinPathPy("reactions/test_hi.rxn", __file__))
testReactionSaveLoad(joinPathPy("reactions/pseudoatoms/psd-pol.rxn", __file__))
# testQueryReactionSaveLoad("reactions/pseudoatoms/x-o.rxn")
