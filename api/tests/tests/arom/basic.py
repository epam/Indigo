import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
print("****** Arom/Dearom ********")
m = indigo.loadMolecule("[As]1C=N[AsH]S=1C")
origin_smiles = m.smiles()
print(origin_smiles)
m.aromatize()
print(m.smiles())
m.dearomatize()
restored_smiles = m.smiles()
print(restored_smiles)

if origin_smiles != restored_smiles:
   sys.stderr.write("%s != %s" % (origin_smiles, restored_smiles))
