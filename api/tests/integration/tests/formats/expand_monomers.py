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
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)

print("*** Expand monomers ***")

root = joinPathPy("molecules/", __file__)
ref = joinPathPy("ref/", __file__)

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

files = {
    "expand_monomers": range(0, 8),
}

for filename in sorted(files):
    mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
    try:
        mol.expandMonomers(list(files[filename]))
    except IndigoException as e:
        print("Test %s failed: %s" % (filename, getIndigoExceptionText(e)))
        continue
    except Exception as e:
        print("Test %s failed: %s" % (filename, e))
        continue

    # with open(os.path.join(ref, filename) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(os.path.join(ref, filename) + ".ket", "r") as file:
        ket_ref = file.read()
    ket = mol.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
