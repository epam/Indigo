import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
patterns = []
for line in open(joinPathPy("molecules/tpsa.tab", __file__)).readlines()[1:]:
  parts = line.split("\t")
  patterns.append([float(parts[0]), indigo.loadSmarts(parts[1])])
def calcTPSA (smiles):
  mol = indigo.loadMolecule(smiles)
  matcher = indigo.substructureMatcher(mol)
  tpsa = 0
  for pattern in patterns:
    tpsa += pattern[0] * matcher.countMatches(pattern[1])
  print('{0:0.2f}'.format(tpsa))
calcTPSA("CN2C(=O)N(C)C(=O)C1=C2N=CN1C")
calcTPSA("OCSCCCCC1=CC2=C(C=CC=C2P)C(Cl)=C1")
calcTPSA("OCSCCCCc1cc(Cl)c2cccc(P)c2c1")
