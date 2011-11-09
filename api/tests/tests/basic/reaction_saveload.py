import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testReactionSaveLoad (filename):
  print "Loading", relativePath(filename)
  rxn = indigo.loadReactionFromFile(filename)
  print rxn.smiles()
  rxn2 = indigo.loadReaction(rxn.smiles())
  print rxn2.smiles()
  if rxn.countMolecules() != rxn2.countMolecules():
    print "ERROR: rxn != rxn2"
  rxn3 = indigo.loadReaction(rxn.rxnfile())
  print rxn3.smiles()
  if not indigo.exactMatch(rxn, rxn3):
    print "ERROR: rxn != rxn3"
def testQueryReactionSaveLoad (filename):
  print "Loading", filename
  rxn = indigo.loadQueryReactionFromFile(filename)
  print rxn.smiles()
  rxn2 = indigo.loadQueryReaction(rxn.smiles())
  print rxn2.smiles()
  if rxn.countMolecules() != rxn2.countMolecules():
    print "ERROR: rxn != rxn2"
  rxn3 = indigo.loadQueryReaction(rxn.rxnfile())
  print rxn3.smiles()
  if rxn.countMolecules() != rxn3.countMolecules():
    print "ERROR: rxn != rxn3"
testReactionSaveLoad(joinPath("../../../../rxnfiles/disconnected_w_stereo.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/disconnected.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/catalysts3000.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/catalysts2000.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/amiderxn2.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/test_hi.rxn"))
testReactionSaveLoad(joinPath("../../../../rxnfiles/pseudoatoms/psd-pol.rxn"))
#testQueryReactionSaveLoad("../../../../rxnfiles/pseudoatoms/x-o.rxn")
