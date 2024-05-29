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
ref = joinPathPy("ref/", __file__)

indigo.loadMoleculeFromFile(os.path.join(ref, "monomer_library.ket"))

# same ref ket files used to check idt-to-ket and to check ket-to-idt
idt_data = {
    "idt_single_nucleoside": "A",
    "idt_bases": "ATCGUI",
    "idt_prefix_suffix": "mA*rT*+C*G*+UrImA",
    "idt_modifications": "/52MOErA/*/i2MOErA//32MOErA/",
    "idt_52moera_with_3phos": "/52MOErA//3Phos/",
    "idt_singe_32moera_nucleoside": "/32MOErA/",
    "idt_std_phosphates": "/5Phos/ATG/3Phos/",
    "idt_mod_phosphates": "/5Phos//i2MOErC//3Phos/",
    "idt_mixed": "/5Phos/+A*/i2MOErA/*rG/3Phos/",
    "idt_many_molecules": "ACTG\n/52MOErA/*AU/3Phos/\nAC/i2MOErC//3Phos/\nTACG",
}

for filename in sorted(idt_data.keys()):
    mol = indigo.loadMoleculeFromFile(os.path.join(ref, filename + ".ket"))
    idt = mol.idt()
    idt_ref = idt_data[filename]
    if idt_ref == idt:
        print(filename + ".ket:SUCCEED")
    else:
        print(
            "%s.idt FAILED : expected '%s', got '%s'"
            % (filename, idt_ref, idt)
        )

idt_errors = {
    "ket-to-idt-r1r1connection": "Cannot save molecule in IDT format - sugar MOE connected to monomer MOE with class SUGAR (only base or phosphate expected).",
    "ket-to-idt-peptide": "Cannot save molecule in IDT format - AA monomer DPhe4C cannot be first.",
    "ket-to-idt-two-bases": "Cannot save molecule in IDT format - sugar R with two base connected A and C.",
    "ket-to-idt-invalid-posphates": "Cannot save molecule in IDT format - sugar R with too much phosphates connected P and P.",
    "ket-to-idt-invalid-last-phosphate": "Cannot save molecule in IDT format - phosphate sP cannot be last monomer in sequence.",
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
