import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** MOL to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "1032-quadra",
    "1046-imp_hydrogen",
    "SgroupDifferent",
    "suplabel",
    "atropisomer",
    "non_atrop",
    "cysteine",
    "dcysteine",
    "thymine",
    "dthymine",
    "chem",
    "rna_mod",
    "conjugate",
    "dna_mod",
    "pepchem",
    "peptides",
    "fmoc",
    "anacyclamide",
    "acgt_1412",
    "apamine",
    "1465-lr_sugar",
    "removed_phosphate",
    "taspoglutide",
    "1972-case1",
    "1972-case2",
    "chem_rna_hydro",
    "tadfile",
    "2708-sgroup-data",
    "2704-stereocenters",
    "issue_2699_rlogic",
    "issue_2958_map_template",
    "sgroup_class",
    "flip_rotate",
    "flip_rotate_2000",
    "flip_rotate_rna",
    "3050-bad-cbonds",
    "3047-accldraw",
    "3094-chem-2000",
    "3094-chem-3000",
    "3227-copolymer",
    "3292-template-center",
    "3343-dir-expanded",
]

native_precision = [
    "2708-sgroup-data",
    "2704-stereocenters",
    "issue_2699_rlogic",
    "issue_2958_map_template",
]

with_lib = [
    "issue_2958_map_template",
    "flip_rotate",
    "flip_rotate_2000",
    "flip_rotate_rna",
    "taspoglutide",
    "apamine",
    "anacyclamide",
    "3094-chem-2000",
    "3094-chem-3000",
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref_path, "monomer_library_ket.ket")
)

files.sort()
for filename in files:
    fname = os.path.join(root, filename + ".mol")
    if filename in with_lib:
        mol = indigo.loadMoleculeWithLibFromFile(fname, lib)
    else:
        mol = indigo.loadMoleculeFromFile(fname)

    if filename in native_precision:
        indigo.setOption("json-use-native-precision", True)
    else:
        indigo.setOption("json-use-native-precision", False)
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)
