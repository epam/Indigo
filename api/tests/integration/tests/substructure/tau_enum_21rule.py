import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo_inchi = IndigoInchi(indigo)


def testEnumTautomersForMolecule (moleculeSmiles, tautomerSmiles):
	molecule = indigo.loadMolecule(moleculeSmiles)
	tautomer = indigo.loadMolecule(tautomerSmiles)
	iter = indigo.iterateTautomers(molecule, 'RSMARTS')
	lst = list()
	for mol in iter:
		prod = mol.clone()
		prod.aromatize()
		lst.append(prod.canonicalSmiles())
	tautomer.aromatize()
	tautomerCanonicalSmiles = tautomer.canonicalSmiles()
	print('Molecule: %s' % (moleculeSmiles))
	print('Tautomer: %s' % (tautomerCanonicalSmiles))
	if tautomerCanonicalSmiles in lst:
		return True
	return False
      
		
# Rule 1:
print(testEnumTautomersForMolecule('O=C1CCCCC1', 'OC1=CCCCC1'))
print(testEnumTautomersForMolecule('OC1=CCCCC1', 'O=C1CCCCC1'))

# Rule 2:
print(testEnumTautomersForMolecule('O=C1C=CCCC1', 'OC1=CC=CCC1'))
print(testEnumTautomersForMolecule('OC1=CC=CCC1', 'O=C1C=CCCC1'))

# Rule 3:
print(testEnumTautomersForMolecule('N=C1CCCCC1', 'NC1=CCCCC1'))
print(testEnumTautomersForMolecule('NC1=CCCCC1', 'N=C1CCCCC1'))

# Rule 4:
print(testEnumTautomersForMolecule('N1C(=CC)C=CC=C1', 'N1=C(CC)C=CC=C1'))
print(testEnumTautomersForMolecule('N1=C(CC)C=CC=C1', 'N1C(=CC)C=CC=C1'))

# Rule 5:
print(testEnumTautomersForMolecule('O=C1NC=CC=C1', 'OC1N=CC=CC=1'))
print(testEnumTautomersForMolecule('OC1N=CC=CC=1', 'O=C1NC=CC=C1'))

# Rule 6:
print(testEnumTautomersForMolecule('NC(=S)NC', 'N=C(S)NC'))
print(testEnumTautomersForMolecule('N=C(S)NC', 'NC(=S)NC'))

# Rule 7:
print(testEnumTautomersForMolecule('O=C1N=C(N)NC2=C(N=CN2)1', 'OC1N=C(N)NC2C(N=CN=2)=1'))
print(testEnumTautomersForMolecule('OC1N=C(N)NC2C(N=CN=2)=1', 'O=C1N=C(N)NC2=C(N=CN2)1'))

# Rule 8:
print(testEnumTautomersForMolecule('CC1=NNC2N(N=CN=2)1', 'CC1=NN=C2N(NC=N2)1'))
print(testEnumTautomersForMolecule('CC1=NN=C2N(NC=N2)1', 'CC1=NNC2N(N=CN=2)1'))

# Rule 9:
print(testEnumTautomersForMolecule('C1C=CC=C2C(NC(C3NC4=C(C=CC=C4)N=3)=N2)=1', 'C1C=CC=C2C(NC(=C3N=C4C(C=CC=C4)=N3)N2)=1'))
print(testEnumTautomersForMolecule('C1C=CC=C2C(NC(=C3N=C4C(C=CC=C4)=N3)N2)=1', 'C1C=CC=C2C(NC(C3NC4=C(C=CC=C4)N=3)=N2)=1'))

# Rule 10:
print(testEnumTautomersForMolecule('CNC1=CC=NC2N(N=CN=2)1', 'CN=C1C=CN=C2N(NC=N2)1'))
print(testEnumTautomersForMolecule('CN=C1C=CN=C2N(NC=N2)1', 'CNC1=CC=NC2N(N=CN=2)1'))

# Rule 11:
print(testEnumTautomersForMolecule('NC1C=CC(C=C2C=CC(=O)C=C2)=CC=1', 'N=C1C=CC(=CC2C=CC(O)=CC=2)C=C1'))
print(testEnumTautomersForMolecule('N=C1C=CC(=CC2C=CC(O)=CC=2)C=C1', 'NC1C=CC(C=C2C=CC(=O)C=C2)=CC=1'))

# Rule 12:
print(testEnumTautomersForMolecule('OC1OC=CC=1', 'O=C1OC=CC1'))
print(testEnumTautomersForMolecule('O=C1OC=CC1', 'OC1OC=CC=1'))

# Rule 13:
print(testEnumTautomersForMolecule('CC=C=O', 'CC#CO'))
print(testEnumTautomersForMolecule('CC#CO', 'CC=C=O'))

# Rule 14:
print(testEnumTautomersForMolecule('CC[N+]([O-])=O', 'CC=[N+]([O-])O'))
print(testEnumTautomersForMolecule('CC=[N+]([O-])O', 'CC[N+]([O-])=O'))

# Rule 15:
#print(testEnumTautomersForMolecule('CCN(=O)=O', 'CC=N(=O)O'))
#print(testEnumTautomersForMolecule('CC=N(=O)O', 'CCN(=O)=O'))

# Rule 16:
print(testEnumTautomersForMolecule('ON=C(C)C', 'O=NC(C)C'))
print(testEnumTautomersForMolecule('O=NC(C)C', 'ON=C(C)C'))

# Rule 17:
print(testEnumTautomersForMolecule('ON=C1C=CC(=O)C=C1', 'O=NC1=CC=C(O)C=C1'))
print(testEnumTautomersForMolecule('O=NC1=CC=C(O)C=C1', 'ON=C1C=CC(=O)C=C1'))

# Rule 18:
print(testEnumTautomersForMolecule('OC#N', 'O=C=N'))
print(testEnumTautomersForMolecule('O=C=N', 'OC#N'))

# Rule 19:
print(testEnumTautomersForMolecule('NC(N)=S(=O)=O', 'N=C(N)S(=O)O'))
print(testEnumTautomersForMolecule('N=C(N)S(=O)O', 'NC(N)=S(=O)=O'))

# Rule 20:
#print(testEnumTautomersForMolecule('C#N', '[C-]#[N+]'))
#print(testEnumTautomersForMolecule('[C-]#[N+]', 'C#N'))

# Rule 21:
print(testEnumTautomersForMolecule('OP(O)O', 'O=P(O)O'))
print(testEnumTautomersForMolecule('O=P(O)O', 'OP(O)O'))
