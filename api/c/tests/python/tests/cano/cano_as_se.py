import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
smiles = [
"C1=CC=[As]C=C1",
"C1=CC=C2C(=C1)C=CC=C2[As]3C4=CC=CC=C4C5=CC=CC=C53",
"C[Se+]1C=CC=C1",
"[B-](F)(F)(F)F.CCC1=CC(=[Se+]C2=CC=CC=C21)C(C)(C)C",
"C1=CC2=C3C(=C1)[Se]C4=CC=CC5=C4C(=CC=C5)[Se](=O)C3=CC=C2",
"CN(C)CC1=C(C2=CC=CC=C2[Se]1(Cl)Cl)Cl",
"CC1=C(C=[Se](C(=C1)C#N)C)C",
"CC1=NC2=CC=CC=C2[Se]1"]
for item in smiles:
    print(item)
    mol = indigo.loadMolecule(item)
    mol.aromatize()
    smi = mol.canonicalSmiles()
    print('  ' + smi)
    
    mol_reloaded_arom = indigo.loadMolecule(smi)
    smi_reloaded = mol_reloaded_arom.canonicalSmiles()
    print('   ' + smi_reloaded)
    if smi != smi_reloaded:
        print('  MISMATCH: ' + smi + '  and  ' + smi_reloaded)
      
    mol.dearomatize()
    mol_reloaded_dearom = indigo.loadMolecule(mol.canonicalSmiles())
    mol_reloaded_dearom.aromatize()
    smi_reloaded_dearom = mol_reloaded_dearom.canonicalSmiles()
    print('     ' + smi_reloaded_dearom)
    if smi != smi_reloaded_dearom:
        print('  MISMATCH DEAROM: ' + smi + '  and  ' + smi_reloaded_dearom)
