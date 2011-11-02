import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def sssrInfo (mol):
  print "SSSR count =", mol.countSSSR()
  for submol in mol.iterateSSSR():
    print '  ',
    for atom in submol.iterateAtoms():
      print atom.index(), 
    print
def testSSSR ():
  indigo.setOption("skip-3d-chirality", True)
  files = os.listdir("../../data/chebi")
  files.sort()
  for filename in files:
    print filename
    mol = indigo.loadMoleculeFromFile("../../data/chebi/" + filename)
    sssrInfo(mol)
    newmol = indigo.loadMolecule(mol.smiles())
    sssrInfo(newmol)
testSSSR()
