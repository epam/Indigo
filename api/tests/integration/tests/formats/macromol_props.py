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
    "props_rna_with_mol",
    "props_nucleoside_peptide",
    "props_connected_via_micro",
    "props_connected_via_chem",
    "props_only_micro",
    "props_mol_connected_to_mol",
    "props_double_dna_gc",
    "props_bases_no_sugar",
    "props_double_dna_p",
    "props_double_dna_unsplit",
    "props_amino_full_mol_selected",
    "props_amino_mol_selected",
    "props_amino_one_selected",
    "props_amino_selected_mol",
    "props_amino_selected_mol_part",
    "props_dna_base_selected",
    "props_dna_nucleotide_selected",
    "props_dna_phosphate_selected",
    "props_double_dna_single",
]

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

upc = 0.14
nac = 0.2
for filename in sorted(macro_data):
    mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
    try:
        props = mol.macroProperties(upc, nac)
    except IndigoException as e:
        print("Test '%s' failed: %", (filename, getIndigoExceptionText(e)))
        continue
    except Exception as e:
        print("Test '%s' failed: %", (filename, e))
        continue
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

filename = "props_double_dna_gc"
mol = indigo.loadKetDocumentFromFile(os.path.join(root, filename + ".ket"))
with open(os.path.join(ref, filename) + "_zero.json", "r") as file:
    props_ref = file.read()
# test invalid UPC - 0 or too big value
for invalid_upc in (0, 1e41):
    props = mol.macroProperties(invalid_upc, nac)
    diff = find_diff(props_ref, props)
    if not diff:
        print("UPC=%s: SUCCEED" % invalid_upc)
    else:
        print("UPC=%s: FAILED" % invalid_upc)
        print(diff)
# test invalid NAC - 0 or too big value
for invalid_nac in (0, 1e41):
    props = mol.macroProperties(upc, invalid_nac)
    diff = find_diff(props_ref, props)
    if not diff:
        print("NAC=%s: SUCCEED" % invalid_nac)
    else:
        print("NAC=%s: FAILED" % invalid_nac)
        print(diff)
