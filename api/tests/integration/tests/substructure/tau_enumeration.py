import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
#indigo_inchi = IndigoInchi(indigo);

def testEnumTautomersForMolecule (molecule):
	iter = indigo.iterateTautomers(molecule, 'INCHI')
	lst = list()
	for mol in iter:
		prod = mol.clone()
		lst.append(prod.canonicalSmiles())
	lst.sort()
	print(" " + "\n ".join(map(lambda x, y: str(x) + ") " + y, range(1, len(lst) + 1), lst)) + '\n')
      
def testEnumTautomersForSDF(sdf_file):
	for idx, molecule in enumerate(indigo.iterateSDFile(sdf_file)):
		try:
			print("%d. %s" % (idx + 1, molecule.smiles()))
			molecule.dearomatize()
			testEnumTautomersForMolecule(molecule)
			molecule.aromatize()
			testEnumTautomersForMolecule(molecule)
		except IndigoException as e:
			print(getIndigoExceptionText(e))

print("This is the case when not all tautomers are found for the first time and the algorithm requires the second attempt:")
testEnumTautomersForMolecule (indigo.loadMolecule('OC1N=C2C(=NC(N)=NC(=O)2)NC(O)=1'));

print("Test tautomers1-small.sdf")
testEnumTautomersForSDF(joinPathPy('molecules/tautomers1-small.sdf', __file__))

print("Test tautomers2-small.sdf")
testEnumTautomersForSDF(joinPathPy('molecules/tautomers2-small.sdf', __file__))

print("Test tautomers1-large.sdf")
testEnumTautomersForSDF(joinPathPy('molecules/tautomers1-large.sdf.gz', __file__))

print("Test tautomers2-large.sdf")
testEnumTautomersForSDF(joinPathPy('molecules/tautomers2-large.sdf.gz', __file__))

