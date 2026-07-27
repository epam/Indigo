import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
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
    "expand_monomers",
    "expand_monomers_no_selection",
}

for filename in sorted(files):
    mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
    try:
        mol.expandMonomers()
    except IndigoException as e:
        print("Test %s failed: %s" % (filename, getIndigoExceptionText(e)))
        continue
    except Exception as e:
        print("Test %s failed: %s" % (filename, e))
        continue
    ket = mol.json()
    compare_diff(ref, filename + ".ket", ket)
