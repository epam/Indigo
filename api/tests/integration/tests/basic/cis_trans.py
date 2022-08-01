import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

# Test for INDSP-126
indigo = Indigo()
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/cis_trans_either-indsp-126.mol", __file__)
)
print(mol.smiles())
mol2 = indigo.loadMolecule(mol.molfile())
print(mol2.smiles())
mol.layout()
print(mol.smiles())
mol3 = indigo.loadMolecule(mol.molfile())
print(mol3.smiles())

print("*** Molfile V2000/V3000 ***")
indigo = Indigo()
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/cis_trans_either-indsp-126.mol", __file__)
)

indigo.setOption("molfile-saving-mode", "2000")
mol.saveMolfile(
    joinPathPy("out/cis_trans_either-indsp-126-2000.mol", __file__)
)
mol2 = indigo.loadMolecule(mol.molfile())

indigo.setOption("molfile-saving-mode", "3000")
mol.saveMolfile(
    joinPathPy("out/cis_trans_either-indsp-126-3000.mol", __file__)
)
mol3 = indigo.loadMolecule(mol.molfile())
