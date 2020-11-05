import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

def checkHasMatchMol (indigo, m, q):
    m.checkBadValence()
    assert(indigo.exactMatch(m, q, '') != None)

    m2 = indigo.unserialize(m.serialize())
    assert(indigo.exactMatch(m2, q, '') != None)

    q2 = indigo.unserialize(q.serialize())
    assert(indigo.exactMatch(m, q2, '') != None)
    assert(indigo.exactMatch(m2, q2, '') != None)

def checkHasMatch (indigo, targetName, queryName):
    print(targetName + " " + queryName)
    q = indigo.loadMoleculeFromFile(joinPath(queryName))
    m = indigo.loadMoleculeFromFile(joinPath(targetName))
    checkHasMatchMol(indigo, m, q)

print("***** Structure should match itself *****")
indigo = Indigo()
checkHasMatch(indigo, "molecules/Calcitonin_Salmon.mol", "molecules/Calcitonin_Salmon.mol")
indigo.setOption("stereochemistry-bidirectional-mode", True)
checkHasMatch(indigo, "molecules/Calcitonin_Salmon.mol", "molecules/Calcitonin_Salmon.mol")
