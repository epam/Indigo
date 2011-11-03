import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
patterns = []
for line in open("molecules/tpsa.tab").readlines()[1:]:
  parts = line.split("\t")
  patterns.append([float(parts[0]), indigo.loadSmarts(parts[1])])
def calcTPSA (smiles):
  mol = indigo.loadMolecule(smiles)
  matcher = indigo.substructureMatcher(mol)
  tpsa = 0
  for pattern in patterns:
    tpsa += pattern[0] * matcher.countMatches(pattern[1])
  print tpsa
calcTPSA("CN2C(=O)N(C)C(=O)C1=C2N=CN1C")
calcTPSA("OCSCCCCC1=CC2=C(C=CC=C2P)C(Cl)=C1")
calcTPSA("OCSCCCCc1cc(Cl)c2cccc(P)c2c1")
