import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

i = Indigo()
r1 = i.loadReactionFromFile(joinPath('reactions/q_43.rxn'))
r2 = i.loadReactionFromFile(joinPath('reactions/q_43.rxn'))
print(i.exactMatch(r1, r2) is not None)
print(i.exactMatch(r1, r2, 'AAM') is not None)
print(i.exactMatch(r1, r2, 'STE MAS') is not None)
print(i.exactMatch(r1, r2, 'NONE') is not None)
