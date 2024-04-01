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
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to MOL ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

files = [
    "suplabel",
    "cysteine",
    "dcysteine",
    "thymine",
    "dthymine",
    "dala",
    "chem",
    "rna_mod",
    "conjugate",
    "dna_mod",
    "pepchem",
    "peptides",
    "fmoc",
    "anacyclamide",
    "accl_no_class",
    "conj_no_class",
    "sgroups_mul",
    "query_explicit_val",
    "mon_long_id",
    "acgt_1412",
    "issue_1476",
    "macro/hundred-monomers",
    "macro/conjugates/pep-chem-rna",
    "macro/conjugates/peptide_rna",
    "macro/dendromers/dendro-peptide",
    "macro/disulfide/disulfide_peptide_cycle",
    "macro/left_phosphate/left_phosphate",
    "macro/linear/linear_dna",
    "macro/linear/linear_rna",
    "macro/linear/linear_peptide",
    "macro/linear/linear_multi",
    "macro/modified/mod_dna",
    "macro/modified/mod_peptide",
    "macro/modified/mod_rna",
    "macro/modified/mod_rna_peptide",
    "macro/peptide-rna/peptide-rna",
    "macro/peptide-rna/peptide-rna-ac",
    "macro/polyphosphate/polyphosphate_rna",
    "macro/terminators/terms_peptide",
]

files.sort()
for filename in files:
    try:
        indigo.setOption("molfile-saving-mode", "auto")
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except IndigoException as e:
        indigo.setOption("molfile-saving-mode", "3000")
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
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
    #    file.write(ket)

    with open(os.path.join(ref_path, filename) + ".mol", "r") as file:
        ket_ref = file.read()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
