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
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "images",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))

    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
        ket_ref = file.read()
    ket = mol.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)


def check_res(filename, format, ket_ref, ket):
    diff = find_diff(ket_ref, ket)
    if not diff:
        print("{}.ket {}: SUCCEED".format(filename, format))
    else:
        print("{}.ket {}: FAILED".format(filename, format))
        print(diff)


indigo.setOption("json-use-native-precision", True)
files = [
    "monomer_shape",
    "ambiguous_monomer",
]
formats = {
    "doc": [indigo.loadKetDocument],
    "mol": [indigo.loadMolecule, indigo.loadQueryMolecule],
}
for filename in sorted(files):
    for format in sorted(formats.keys()):
        file_path = os.path.join(ref_path, filename)
        with open("{}_{}.ket".format(file_path, format), "r") as file:
            ket_ref = file.read()
        for loader in formats[format]:
            mol = loader(ket_ref)
            # with open("{}_{}.ket".format(file_path, format), "w") as file:
            #     file.write(mol.json())
            ket = mol.json()
            check_res(filename, format + " " + loader.__name__, ket_ref, ket)
