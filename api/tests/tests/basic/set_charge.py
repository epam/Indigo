import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
mol = indigo.loadMolecule("N1C=CC=CC=1")
print('{0} {1} {2}'.format(mol.canonicalSmiles(), mol.checkBadValence(), mol.checkAmbiguousH()))
mol.getAtom(0).setCharge(1)
print('{0} {1} {2}'.format(mol.canonicalSmiles(), mol.checkBadValence(), mol.checkAmbiguousH()))
mol.getAtom(0).resetCharge()
print('{0} {1} {2}'.format(mol.canonicalSmiles(), mol.checkBadValence(), mol.checkAmbiguousH()))
