import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

print("*** Molfile V3000 coordinates keep their decimals ***")

root = joinPathPy("molecules/molfile_coordinates/", __file__)
ref_path = joinPathPy("ref/molfile_coordinates/", __file__)

# atoms beyond the columns of V2000: auto writes V3000, and V3000 keeps four decimals of a large coordinate
base = "large-coordinates"
mol = indigo.loadMoleculeFromFile(os.path.join(root, base + ".ket"))
for mode in ("auto", "3000"):
    indigo.setOption("molfile-saving-mode", mode)
    molfile = mol.molfile()
    compare_diff(ref_path, "%s-%s.mol" % (base, mode), molfile)
    compare_diff(
        ref_path,
        "%s-%s.mol" % (base, mode),
        indigo.loadMolecule(molfile).molfile(),
    )
indigo.setOption("molfile-saving-mode", "2000")
try:
    mol.molfile()
    print("%s as V2000: written" % base)
except IndigoException as e:
    print("%s as V2000: %s" % (base, getIndigoExceptionText(e)))
