import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 4)

print("*** Sequence cycles layout ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "3291-selection",
]

files.sort()
for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )

    mol.layout()
    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(getRefFilepath(filename + ".ket"), "r") as file:
        ket_ref = file.read()

    ket = mol.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
