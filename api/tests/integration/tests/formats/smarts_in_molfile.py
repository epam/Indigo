import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testSmartsMolfile (filename):
    print(relativePath(filename))
    try:
        indigo.loadMoleculeFromFile(filename)
    except IndigoException as e:
        print('caught ' + getIndigoExceptionText(e))
    indigo.setOption("ignore-noncritical-query-features", True)
    print(indigo.loadMoleculeFromFile(filename).canonicalSmiles())
    query = indigo.loadQueryMoleculeFromFile(filename)
    mol1 = indigo.loadMolecule("CNC=O")
    mol2 = indigo.loadMolecule("CN(c1ccccc1)C=O")
    mol3 = indigo.loadMolecule("CN(c1ccccc1)C(C)=O")
    match1 = indigo.substructureMatcher(mol1).match(query)
    if match1:
        print("matched")
    else:
        print("not matched")
    match2 = indigo.substructureMatcher(mol2).match(query)
    if match2:
        print("matched")
    else:
        print("not matched")
    match3 = indigo.substructureMatcher(mol3).match(query)
    if match3:
        print("matched")
    else:
        print("not matched")
      
testSmartsMolfile(joinPath('molecules/smarts-mrv.mol'))
