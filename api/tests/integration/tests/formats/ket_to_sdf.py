import os
import sys


def expect_monomer_library_load_error(root, filename, expected_error):
    try:
        indigo.loadMonomerLibraryFromFile(
            os.path.join(root, filename + ".ket")
        )
        print(filename + ".ket:FAILED")
        print("Expected error '%s'" % expected_error)
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if expected_error in text:
            print(filename + ".ket:SUCCEED")
        else:
            print(filename + ".ket:FAILED")
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
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("monomer-library-saving-mode", "sdf")

print("*** KET to SDF ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["acd2d_err3", "separate_monomers"]

files.sort()
for filename in files:
    try:
        ket = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        try:
            ket = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename + ".ket")
            )
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

    buffer = indigo.writeBuffer()
    sdfSaver = indigo.createSaver(buffer, "sdf")
    for frag in ket.iterateComponents():
        sdfSaver.append(frag.clone())
    sdfSaver.close()
    sdf = buffer.toString()
    compare_diff(ref_path, filename + ".sdf", sdf)

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["pathway1", "pathway2", "pathway3", "multi", "qreaction"]

files.sort()
for filename in files:
    try:
        ket = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        try:
            ket = indigo.loadQueryReactionFromFile(
                os.path.join(root, filename + ".ket")
            )
        except IndigoException as e:
            print("  %s" % (getIndigoExceptionText(e)))

    buffer = indigo.writeBuffer()
    sdfSaver = indigo.createSaver(buffer, "sdf")
    for mol in ket.iterateMolecules():
        sdfSaver.append(mol.clone())
    sdfSaver.close()
    sdf = buffer.toString()
    compare_diff(ref_path, filename + ".sdf", sdf)

print("*** KET-monomer library to SDF ***")
root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
files = [
    "lib_alanine",
    "lib_alanine_expanded",
    "lib_phos",
    "lib_rna_left_phosphate",
    "lib_rna_preset_g",
    "lib_rna_preset_same",
    "lib_default_type",
]

files.sort()
for filename in files:
    lib = indigo.loadMonomerLibraryFromFile(
        os.path.join(root, filename + ".ket")
    )
    sdf = lib.monomerLibrary()
    compare_diff(ref_path, filename + ".sdf", sdf)

print("*** Invalid KET-monomer library to SDF ***")
expect_monomer_library_load_error(
    root,
    "lib_rna_left_phosphate_disconnected",
    "Monomer template group A_l has disconnected templates.",
)
