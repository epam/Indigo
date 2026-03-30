import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** MOL to SEQ ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "rna_mod",
    "conjugate",
    "dna_mod",
    "pepchem",
    "peptides",
    "fmoc",
    "anacyclamide",
    "acgt_1412",
    "apamine",
]

expected_exceptions = {
    "rna_mod": "Sequence saver: '5fU' nucleotide has no natural analog and cannot be saved into a sequence.",
    "dna_mod": "Sequence saver: 'cdaC' nucleotide has no natural analog and cannot be saved into a sequence.",
}

# empty library - internal used for now
lib = indigo.loadMonomerLibrary('{"root":{}}')

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    try:
        seq = mol.sequence(lib)
        compare_diff(ref_path, filename + ".seq", seq)
    except IndigoException as e:
        text_exception = getIndigoExceptionText(e)
        if (
            filename in expected_exceptions
            and expected_exceptions[filename] == text_exception
        ):
            print(filename + ".seq:SUCCEED")
        else:
            print(filename + ".seq:FAILED")
            print(text_exception)
