import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, IndigoException, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET S-groups ***")

root = joinPathPy("molecules/sgroup/", __file__)
ref_path = joinPathPy("ref/sgroup/", __file__)

files = [
    "bracketed-without-bonds",
    "explicit-bonds",
    "no-crossing-bonds",
    "sru-without-bonds",
]

for filename in files:
    path = os.path.join(root, filename + ".ket")
    try:
        mol = indigo.loadMoleculeFromFile(path)
    except IndigoException:
        mol = indigo.loadQueryMoleculeFromFile(path)
    for mode in ["2000", "3000"]:
        indigo.setOption("molfile-saving-mode", mode)
        compare_diff(ref_path, "%s-%s.mol" % (filename, mode), mol.molfile())
