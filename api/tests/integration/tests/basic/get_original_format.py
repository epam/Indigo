import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def test_mol(molecule, mol, expected):
    original_format = mol.getOriginalFormat()
    if original_format == expected:
        print('Molecule "%s" : got "%s" - OK' % (molecule, expected))
    else:
        print(
            'Molecule "%s" failed: expected original format "%s", got "%s"'
            % (molecule, expected, original_format)
        )


print("******  Test get original format: smiles  ***")
molecule = "N[CH](C)C(=O)O"
mol = indigo.loadMolecule(molecule)
test_mol(molecule, mol, "chemical/x-daylight-smiles")

print("******  Test get original format: smarts  ***")
molecule = "([#6]1-[#6]-[#6]-1.[#6])"
mol = indigo.loadSmarts(molecule)
test_mol(molecule, mol, "chemical/x-daylight-smarts")
