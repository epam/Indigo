import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testLayout ():
  indigo.setOption("layout-max-iterations", 1)
  indigo.setOption("skip-3d-chirality", True)
  files = os.listdir(joinPath("../../data/chebi"))
  files.sort()
  for filename in files:
    mol = indigo.loadMoleculeFromFile(joinPath("../../data/chebi/", filename))
    print relativePath(joinPath("../../data/chebi/", filename))
    try:
      mol.layout()
    except IndigoException, e:
      print getIndigoExceptionText(e)
    print mol.smiles()
    newmol = indigo.loadMolecule(mol.smiles())
    try:
      newmol.layout()
    except IndigoException, e:
      print getIndigoExceptionText(e)
testLayout()
