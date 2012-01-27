import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("*** Reaction layout testing ***")
reaction = indigo.createReaction();
reaction.addProduct(indigo.loadMolecule("CN(S(=O)(=O)C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)CC1OC1"));
reaction.addReactant(indigo.loadMolecule("ClCC1CO1"));
reaction.layout();
print(reaction.rxnfile())
