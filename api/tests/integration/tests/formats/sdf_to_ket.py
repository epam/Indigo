import os
import sys


def expect_monomer_library_load_error(root, filename, expected_error):
    try:
        indigo.loadMonomerLibraryFromFile(
            os.path.join(root, filename + ".sdf")
        )
        print(filename + ".sdf:FAILED")
        print("Expected error '%s'" % expected_error)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if expected_error in text:
            print(filename + ".sdf:SUCCEED")
        else:
            print(filename + ".sdf:FAILED")
            print("Expected error '%s' but got '%s'" % (expected_error, text))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** SDF to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["acd2d_err"]

files.sort()
for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".sdf")
        )
    except:
        try:
            mol = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename + ".sdf")
            )
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)

print("*** SDF-monomer library to KET ***")
files = [
    "lib_alanine",
    "lib_alanine_semicolon",
    "lib_alanine_expanded",
    "lib_phos",
    "lib_rna_left_phosphate",
    "lib_rna_preset_g",
    "lib_rna_preset_same",
    "lib_default_type",
    "lib_rna_preset_from_lib",
    "lib_same_struct",
    "lib_empty",
]

files.sort()
for filename in files:
    lib = indigo.loadMonomerLibraryFromFile(
        os.path.join(root, filename + ".sdf")
    )
    ket = lib.monomerLibrary()
    compare_diff(ref_path, filename + ".ket", ket)

print("*** Invalid SDF-monomer library to KET ***")
expect_monomer_library_load_error(
    root,
    "lib_rna_left_phosphate_disconnected",
    "Monomer template group A_left has disconnected templates.",
)
