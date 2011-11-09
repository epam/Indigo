import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def handleMolecule (mol):
  print '%8.3f %8.3f %8.3f %s' % (mol.molecularWeight(), mol.monoisotopicMass(), mol.mostAbundantMass(), mol.grossFormula())
for item in indigo.iterateSDFile(joinPath('molecules/mass.sdf')):
  handleMolecule(item)
handleMolecule(indigo.loadMolecule("CS=C"))
handleMolecule(indigo.loadMolecule("C[S]=C"))
