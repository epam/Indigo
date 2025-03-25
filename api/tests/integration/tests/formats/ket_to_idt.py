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

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(ref, "monomer_library.ket")
)

# same ref ket files used to check idt-to-ket and to check ket-to-idt
idt_data = {
    "idt_single_nucleoside": "A",
    "idt_bases": "ATCGUI",
    "idt_prefix_suffix": "mA*rT*+C*G*+UrImA",
    "idt_modifications": "/52MOErA/*/i2MOErA//32MOErA/",
    "idt_52moera_with_3phos": "/52MOErA//3Phos/",
    "idt_std_phosphates": "/5Phos/ATG/3Phos/",
    "idt_mod_phosphates": "/5Phos//i2MOErC//3Phos/",
    "idt_mixed": "/5Phos/+A*/i2MOErA/*rG/3Phos/",
    "idt_many_molecules": "ACTG\n/52MOErA/*AU/3Phos/\nAC/i2MOErC//3Phos/\nTACG",
    "idt_unresolved": "/unr1//unr2/",
    "idt_unresolved_many": "/unr0/A/unr1/C/unr2/ACTG/unr3/G/unr4/",
    "idt_52moera": "/52MOErA/",
    "idt_i2moera": "/52MOErA/",
    "idt_32moera": "/32MOErA/",
    "idt_52moera_sp": "/52MOErA/*",
    "idt_i2moera_sp": "/52MOErA/*",
    "idt_3phos": "/5Phos/",
    "idt_5phos": "/5Phos/",
    "idt_i2moera_t": "/52MOErA/T",
    "idt_t_i2moera": "T/i2MOErA/",
    "idt_52moera_32moera": "/52MOErA//32MOErA/",
    "idt_i2moera_32moera": "/52MOErA//32MOErA/",
    "idt_52moera_i2moera": "/52MOErA//i2MOErA/",
    "idt_52moera_sp_32moera": "/52MOErA/*/32MOErA/",
    "idt_i2moera_sp_32moera": "/52MOErA/*/32MOErA/",
    "idt_52moera_sp_i2moera_sp": "/52MOErA/*/i2MOErA/*",
    "idt_unsplit": "/5UNSPLIT//iUNSPLIT//3UNSPLIT/",
    "idt_more_than_80_chars": "/52MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//i2MOErA//3Phos/",
    "idt_mixed_std": "ARAS",
    "idt_mixed_custom": "(N1:10203050)(N1)N",
    "idt_rna_dna_mixed_custom": "r(R1:50003000)(R1)",
    "idt_mixed_ketcher": "KrK(K1:00003070)r(K2:00003070)",
    "idt_issue_2257": "/3ThioMC3-D/",
}

for filename in sorted(idt_data.keys()):
    mol = indigo.loadKetDocumentFromFile(os.path.join(ref, filename + ".ket"))
    try:
        idt = mol.idt(lib)
        idt_ref = idt_data[filename]
        if idt_ref == idt:
            print(filename + ".ket:SUCCEED")
        else:
            print(
                "%s.idt FAILED : expected '%s', got '%s'"
                % (filename, idt_ref, idt)
            )
    except IndigoException as e:
        text = getIndigoExceptionText(e)
        print(filename + ".ket:FAILED - %s" % text)

idt_errors = {
    "ket-to-idt-r1r1connection": "Sequence saver: Cannot save in IDT format - nonstandard connection found.",
    "ket-to-idt-peptide": "Sequence saver: Cannot save molecule in IDT format - expected sugar but found AminoAcid monomer DPhe4C.",
    "ket-to-idt-two-bases": "Sequence saver: Cannot save in IDT format - nonstandard connection found.",
    "ket-to-idt-invalid-posphates": "Sequence saver: Cannot save in IDT format - nonstandard connection found.",
    "ket-to-idt-invalid-last-phosphate": "Sequence saver: Cannot save molecule in IDT format - phosphate R cannot be last monomer in sequence.",
    "ket-to-idt-invalid-nucleotide": "IDT alias for group sugar:m2e2r base:z8c3G phosphate:mepo2 not found.",
    "ket-to-idt-invalid-sugar-phosphate": "IDT alias for group sugar:m2e2r phosphate:mepo2 not found.",
    "ket-to-idt-invalid-sugar": "Cannot save molecule in IDT format - sugar whithout base.",
    "ket-to-idt-invalid-sugar-base": "IDT alias for group sugar:m2e2r base:z8c3G not found.",
    "ket-to-idt-alternatives-base": "Cannot save IDT - only mixture supported but found alternatives.",
}
for filename in sorted(idt_errors.keys()):
    error = idt_errors[filename]
    try:
        doc = indigo.loadKetDocumentFromFile(
            os.path.join(root, filename + ".ket")
        )
        idt = doc.idt(lib)
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
