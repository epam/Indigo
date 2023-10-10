import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def test_mol(mol, expected):
    original_format = mol.getOriginalFormat()
    if original_format == expected:
        print('"%s" : got "%s" - OK' % (mol, expected))
    else:
        print(
            '"%s" failed: expected original format "%s", got "%s"'
            % (mol, expected, original_format)
        )


print("******  Test get original format: smiles  ***")
mol = indigo.loadMolecule("N[CH](C)C(=O)O")
test_mol(mol, "chemical/x-daylight-smiles")

print("******  Test get original format: smarts  ***")
mol = indigo.loadSmarts("([#6]1-[#6]-[#6]-1.[#6])")
test_mol(mol, "chemical/x-daylight-smarts")
