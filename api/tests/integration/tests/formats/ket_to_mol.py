﻿import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoException, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to MOL ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

files = [
    ("suplabel", "auto"),
    ("cysteine", "auto"),
    ("dcysteine", "auto"),
    ("thymine", "auto"),
    ("dthymine", "auto"),
    ("dala", "auto"),
    ("chem", "auto"),
    ("rna_mod", "auto"),
    ("conjugate", "auto"),
    ("dna_mod", "auto"),
    ("pepchem", "auto"),
    ("peptides", "auto"),
    ("fmoc", "auto"),
    ("anacyclamide", "auto"),
    ("accl_no_class", "auto"),
    ("conj_no_class", "auto"),
    ("sgroups_mul", "auto"),
    ("query_explicit_val", "auto"),
    ("mon_long_id", "auto"),
    ("acgt_1412", "auto"),
    ("issue_1476", "auto"),
    ("taspoglutide", "auto"),
    ("macro/hundred-monomers", "auto"),
    ("macro/conjugates/pep-chem-rna", "auto"),
    ("macro/conjugates/peptide_rna", "auto"),
    ("macro/dendromers/dendro-peptide", "auto"),
    ("macro/disulfide/disulfide_peptide_cycle", "auto"),
    ("macro/left_phosphate/left_phosphate", "auto"),
    ("macro/linear/linear_dna", "auto"),
    ("macro/linear/linear_rna", "auto"),
    ("macro/linear/linear_peptide", "auto"),
    ("macro/linear/linear_multi", "auto"),
    ("macro/modified/mod_dna", "auto"),
    ("macro/modified/mod_peptide", "auto"),
    ("macro/modified/mod_rna", "auto"),
    ("macro/modified/mod_rna_peptide", "auto"),
    ("macro/peptide-rna/peptide-rna", "auto"),
    ("macro/peptide-rna/peptide-rna-ac", "auto"),
    ("macro/polyphosphate/polyphosphate_rna", "auto"),
    ("macro/terminators/terms_peptide", "auto"),
    ("macro/sa-mono", "auto"),
    ("macro/R2R3R4", "auto"),
    ("5amd", "2000"),
    ("chem_rna_hydro", "3000"),
    ("issue_2702", "auto"),
    ("issue_2699_rlogic", "3000"),
    ("hydro_atp_ch", "3000"),
    ("sgroup_class", "3000"),
    ("flip_rotate", "3000"),
    ("flip_rotate_2000", "2000"),
    ("flip_rotate_rna", "3000"),
]

files.sort(key=lambda x: x[0])
for test_tuple in files:
    filename = test_tuple[0]
    try:
        indigo.setOption("molfile-saving-mode", "auto")
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except IndigoException:
        indigo.setOption("molfile-saving-mode", "3000")
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )

    if len(test_tuple) > 1:
        indigo.setOption("molfile-saving-mode", test_tuple[1])

    # with open(os.path.join(ref_path, filename) + ".mol", "w") as file:
    #     file.write(mol.molfile())

    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        ket_ref = file.read()
    ket = mol.molfile()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)

files = ["ket-reaction-arrow", "empty_apid"]

files.sort()
for filename in files:
    rc = indigo.loadReactionFromFile(os.path.join(root_rea, filename + ".ket"))
    ket = rc.rxnfile()
    # with open(os.path.join(ref_path, filename) + ".mol", "w") as file:
    #   file.write(ket)

    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        ket_ref = file.read()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
