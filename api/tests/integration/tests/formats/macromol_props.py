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
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** HELM to KET ***")

root = joinPathPy("molecules/", __file__)
ref = joinPathPy("ref/", __file__)

macro_data = [
    "props_double_dna",
    "props_peptides",
    "props_peptides_micro",
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

for filename in sorted(macro_data):
    mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
    try:
        props = mol.macroProperties()
    except IndigoException as e:
        print("Test '%s' filed: %", (filename, getIndigoExceptionText(e)))
    # with open(os.path.join(ref, filename) + ".json", "w") as file:
    #     file.write(props)
    with open(os.path.join(ref, filename) + ".json", "r") as file:
        props_ref = file.read()
    diff = find_diff(props_ref, props)
    if not diff:
        print(filename + ".json: SUCCEED")
    else:
        print(filename + ".json: FAILED")
        print(diff)
