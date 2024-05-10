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

print("*** KET to IDT ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

indigo.loadMoleculeFromFile(os.path.join(ref_path, "monomer_library.ket"))

files = [
    "1654-dna-to-idt",
    "ket-to-idt-a",
    "ket-to-idt-2moeract",
    "ket-to-idt-52moera",
    "ket-to-idt-32moera",
    "ket-to-idt-5phos3moera",
]

for filename in sorted(files):
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    # with open(os.path.join(ref_path, filename) + ".idt", "w") as file:
    #     file.write(mol.idt())
    with open(os.path.join(ref_path, filename) + ".idt", "r") as file:
        idt_ref = file.read()
    idt = mol.idt()
    diff = find_diff(idt_ref, idt)
    if not diff:
        print(filename + ".idt:SUCCEED")
    else:
        print(filename + ".idt:FAILED")
        print(diff)

idt_errors = {
    "ket-to-idt-r1r1connection": "Canot save molecule in IDT format - sugar MOE connected to monomer MOE with class SUGAR (only base or phosphate expected).",
    "ket-to-idt-peptide": "Canot save molecule in IDT format - AA monomer DPhe4C cannot be first.",
    "ket-to-idt-two-bases": "Canot save molecule in IDT format - sugar R with two base connected A and C.",
    "ket-to-idt-invalid-posphates": "Canot save molecule in IDT format - sugar R with too much phosphates connected P and P.",
    "ket-to-idt-invalid-last-phosphate": "Canot save molecule in IDT format - phosphate sP cannot be last monomer in sequence.",
    "ket-to-idt-invalid-nucleotide": "IDT alias for group sugar:m2e2r base:z8c3G phosphate:mepo2 not found.",
    "ket-to-idt-invalid-sugar-phosphate": "IDT alias for group sugar:m2e2r phosphate:mepo2 not found.",
    "ket-to-idt-invalid-sugar": "IDT alias for sugar:m2e2r not found.",
    "ket-to-idt-invalid-sugar-base": "IDT alias for group sugar:m2e2r base:z8c3G not found.",
}
for filename in sorted(idt_errors.keys()):
    error = idt_errors[filename]
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
        idt = mol.idt()
        print(
            "Test %s failed: exception expected but got next idt - '%s'."
            % (filename, idt)
        )
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        if error in text:
            print("Test %s: got expected error '%s'" % (filename, error))
        else:
            print(
                "Test %s: expected error '%s' but got '%s'"
                % (filename, error, text)
            )
