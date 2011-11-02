import os;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def loadSMILES (smiles):
  try:
    mol = indigo.loadMolecule(smiles)
    print smiles, ': OK'
  except IndigoException, e:
    print smiles, getIndigoExceptionText(e)
loadSMILES('O[C@H](N)C |&1:0|')
loadSMILES('O[C@H](N)C |o1:0|')
loadSMILES('O[C@H](N)C |a:3|')
