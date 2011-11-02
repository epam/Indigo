import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
m1 = indigo.loadMolecule("COCC1=C(N=CC2=C1C1=C(OC3=CC=C(Cl)C=C3)C=CC=C1N2)C(=O)OC(C)C")
m2 = indigo.loadMolecule("COCC1=CN=C(C(=O)OC(C)C)C2=C1C1=CC=C(OC3=CC=C(Cl)C=C3)C=C1N2")
indigo.setOption("deconvolution-aromatization", "false")
print("Should extract all possible exact MCS for given molecule set (non aromatic)")
deco = indigo.extractCommonScaffold([m1, m2], "EXACT")
scaffold_smiles = []
for scaffold in deco.allScaffolds().iterateArray():
   scaffold_smiles.append(indigo.loadMolecule(scaffold.smiles()).canonicalSmiles())
for smiles in sorted(scaffold_smiles):
   print("  %s" % (smiles))
   
