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
indigo.setOption("json-saving-pretty", True)

print("*** SDF-monomer library properties ***")

root = joinPathPy("molecules/monomer_library/", __file__)
ref_path = joinPathPy("ref/monomer_library/", __file__)

files = [
    "3666-preset",
    "3671-modification-types",
    "3672-idt-aliases",
    "3695-monomer-template-type",
]
broken_files = [
    "3666-preset-empty-group-name",
    "3666-preset-without-group-class",
    "3666-preset-without-group-name",
    "3671-empty-modification-types",
    "3672-idt-alias-empty-ep3",
    "3672-idt-alias-without-value",
    "3672-idt-aliases-empty-values",
    "3695-empty-type",
    "3695-unknown-type",
]

for filename in files:
    lib = indigo.loadMonomerLibraryFromFile(
        os.path.join(root, filename + ".sdf")
    )
    compare_diff(ref_path, filename + ".ket", lib.monomerLibrary())

for filename in broken_files:
    try:
        indigo.loadMonomerLibraryFromFile(
            os.path.join(root, filename + ".sdf")
        )
        print("%s: no error" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
