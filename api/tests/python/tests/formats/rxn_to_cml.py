
import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

print("*** Rxn to CML ***")

rxn = indigo.loadReactionFromFile(joinPath('molecules/reaction_for_cml.rxn'))
print(rxn.cml())

