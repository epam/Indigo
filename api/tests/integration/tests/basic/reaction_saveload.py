import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testReactionSaveLoad (filename):
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
    
def testQueryReactionSaveLoad (filename):
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
    
testReactionSaveLoad(joinPath("reactions/disconnected_w_stereo.rxn"))
testReactionSaveLoad(joinPath("reactions/disconnected.rxn"))
testReactionSaveLoad(joinPath("reactions/catalysts3000.rxn"))
testReactionSaveLoad(joinPath("reactions/catalysts2000.rxn"))
testReactionSaveLoad(joinPath("reactions/amiderxn2.rxn"))
testReactionSaveLoad(joinPath("reactions/test_hi.rxn"))
testReactionSaveLoad(joinPath("reactions/pseudoatoms/psd-pol.rxn"))
#testQueryReactionSaveLoad("reactions/pseudoatoms/x-o.rxn")
    