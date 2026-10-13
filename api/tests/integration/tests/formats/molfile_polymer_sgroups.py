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
indigo.setOption("molfile-saving-skip-date", True)

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    # two numbered components; the first holds a head-to-tail block
    # copolymer of two repeating units
    "3953-formulation",
    # the same kinds of S-groups in V3000, type and subtype spelled out,
    # the copolymer and one repeating unit without CONNECT
    "3953-formulation-words-v3000",
]
for filename in files:
    for mode in ("2000", "3000"):
        print("*** %s saved as V%s ***" % (filename, mode))
        indigo.setOption("molfile-saving-mode", mode)
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        ref_name = "%s-v%s.mol" % (filename, mode)
        compare_diff(ref_path, ref_name, mol.molfile())
        clone = mol.clone()
        compare_diff(ref_path, ref_name, clone.molfile())
