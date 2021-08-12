import sys
sys.path.append('../../common')
from env_indigo import *

def checkHasMatchMol (indigo, m, q):
    q.aromatize()
    m.checkBadValence()
    m.checkBadValence()
    matcher = indigo.substructureMatcher(m)
    assert(matcher.match(q) != None)

    m2 = indigo.unserialize(m.serialize())
    matcher2 = indigo.substructureMatcher(m2)
    assert(matcher2.match(q) != None)

    q.optimize()
    assert(matcher.match(q) != None)
    assert(matcher2.match(q) != None)

def checkHasMatch (indigo, targetName, queryName):
    print(targetName + " " + queryName)
    q = indigo.loadQueryMoleculeFromFile(joinPathPy(queryName, __file__))
    m = indigo.loadMoleculeFromFile(joinPathPy(targetName, __file__))
    checkHasMatchMol(indigo, m, q)


print("***** Substructure with either and bidirectional mode *****")
indigo = Indigo()
indigo.setOption("stereochemistry-bidirectional-mode", True)

checkHasMatch(indigo, "molecules/either1.mol", "molecules/either1.mol")
checkHasMatch(indigo, "molecules/either1.mol", "molecules/either2.mol")
checkHasMatch(indigo, "molecules/either1.mol", "molecules/either1_query.mol")

print("***** Substructure with explicit hydrogens *****")
indigo = Indigo()
q = indigo.loadQueryMolecule("c1c(N([H])[H])cccc1")
m = indigo.loadMolecule("Nc1ccc(cc1)-c1nnn(Cc2ccc(cc2)C#N)n1")
checkHasMatchMol(indigo, m, q)

print("***** Structure should match itself *****")
indigo = Indigo()
checkHasMatch(indigo, "molecules/Calcitonin_Salmon.mol", "molecules/Calcitonin_Salmon.mol")
indigo.setOption("stereochemistry-bidirectional-mode", True)
checkHasMatch(indigo, "molecules/Calcitonin_Salmon.mol", "molecules/Calcitonin_Salmon.mol")

print("***** Check specific SMILES/SMARTS behaviour *****")
indigo = Indigo()
queries_sm = [
    "[#6,#7,#8]c1nc2ccccc2n1",
    "C1=CC=CC2=C1N=C(N2)[*;#6,#7,#8]",
    "c1cccc2c1nc(n2)C",
    "C1=CC=CC2=C1N=C(N2)C",
    "[$([#6,#7,#8]c1nc2ccccc2n1)]"
]

targets_sm = [
    "Cn1c(CN)nc2ccccc12",
    "c1ccc(cc1)-c1nc2ccccc2[nH]1"    
]

queires = []
for q_sm in queries_sm:
    try:
        q = indigo.loadQueryMolecule(q_sm)
        q.aromatize()
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        q = None
    queires.append(q)

def checkMatch (matcher, q):
    if q is None:
        return "none"
    if matcher.match(q):
        return "match"
    return "not match"

for t_sm in targets_sm:
    print(t_sm)
    t = indigo.loadMolecule(t_sm)
    matcher = indigo.substructureMatcher(t)
    for idx, q in enumerate(queires):
        print("  %d - %s" % (idx, checkMatch(matcher, q)))

