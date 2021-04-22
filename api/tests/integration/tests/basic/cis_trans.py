import sys
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

# Test for INDSP-126
indigo = Indigo()
mol = indigo.loadMoleculeFromFile(joinPath("molecules", "cis_trans_either-indsp-126.mol"))
print(mol.smiles())
mol2 = indigo.loadMolecule(mol.molfile())
print(mol2.smiles())
mol.layout()
print(mol.smiles())
mol3 = indigo.loadMolecule(mol.molfile())
print(mol3.smiles())

print("*** Molfile V2000/V3000 ***")
indigo = Indigo()
mol = indigo.loadMoleculeFromFile(joinPath("molecules", "cis_trans_either-indsp-126.mol"))

indigo.setOption('molfile-saving-mode', '2000')
mol.saveMolfile(joinPath("out/cis_trans_either-indsp-126-2000.mol"))
mol2 = indigo.loadMolecule(mol.molfile())

indigo.setOption('molfile-saving-mode', '3000')
mol.saveMolfile(joinPath("out/cis_trans_either-indsp-126-3000.mol"))
mol3 = indigo.loadMolecule(mol.molfile())    
