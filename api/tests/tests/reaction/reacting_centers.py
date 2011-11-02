import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
print("*** Test 1 ***")
rxn = indigo.createReaction();
rxn.addProduct(indigo.loadMolecule("CCCC"));
rxn.addReactant(indigo.loadMolecule("CCCC"));
print(rxn.smiles())
print("reacting centers:")
for m in rxn.iterateMolecules():
   for b in m.iterateBonds():
      print(rxn.reactingCenter(b))
for m in rxn.iterateMolecules():
   for b in m.iterateBonds():
      rxn.setReactingCenter(b, Indigo.RC_CENTER | Indigo.RC_UNCHANGED)
print("modified centers:")
for m in rxn.iterateMolecules():
   for b in m.iterateBonds():
      print(rxn.reactingCenter(b))
print("*** Test AAM ***")
rxn2 = indigo.loadReaction("CC=O>>CCO")
print("reaction smiles " + rxn2.smiles())
for m in rxn2.iterateMolecules():
   for b in m.iterateBonds():
      rxn2.setReactingCenter(b, Indigo.RC_UNCHANGED | Indigo.RC_ORDER_CHANGED)
rxn2.automap("DISCARD")
print("aam reaction smiles for given RC " + rxn2.smiles())
