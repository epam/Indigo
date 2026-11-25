import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)

print(
    "*** BondOrdering of a fragment node with several external connection points ***"
)

root = joinPathPy("molecules/cdxml_bond_ordering/", __file__)
ref_path = joinPathPy("ref/cdxml_bond_ordering/", __file__)

# The Nickname node is incident to bonds 8 and 14, and its BondOrdering lists both.
mol = indigo.loadMoleculeFromFile(
    os.path.join(root, "bond-ordering-valid.cdxml")
)
print("bond-ordering-valid.cdxml: " + mol.grossFormula())
compare_diff(ref_path, "bond-ordering-valid.ket", mol.json())

# Each of these leaves a bond of the node out of its BondOrdering: a duplicate, a longer
# duplicate list, a list that is too short, and the duplicate in binary CDX - the same
# molecule with its middle C-C as a two-point abbreviation, saved to CDX, with the second
# bond id of the abbreviation's BondOrdering replaced by the first.
malformed = [
    "bond-ordering-duplicated.cdxml",
    "bond-ordering-duplicated-long.cdxml",
    "bond-ordering-incomplete.cdxml",
    "bond-ordering-duplicated.cdx",
]
for filename in malformed:
    try:
        indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(filename + ": loaded")
    except IndigoException as e:
        print(filename + ": " + getIndigoExceptionText(e))
