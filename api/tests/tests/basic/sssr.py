import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def sssrInfo (mol):
    print("SSSR count = {0}".format(mol.countSSSR()))
    for submol in mol.iterateSSSR():
        sys.stdout.write('  ')
        for atom in submol.iterateAtoms():
            sys.stdout.write(str(atom.index()) + ' ') 
        sys.stdout.write('\n')
    
def testSSSR ():
    indigo.setOption("skip-3d-chirality", True)
    files = os.listdir(joinPath("../../data/chebi"))
    files.sort()
    for filename in files:
        print(filename)
        mol = indigo.loadMoleculeFromFile(joinPath("../../data/chebi/", filename))
        sssrInfo(mol)
        newmol = indigo.loadMolecule(mol.smiles())
        sssrInfo(newmol)
testSSSR()
