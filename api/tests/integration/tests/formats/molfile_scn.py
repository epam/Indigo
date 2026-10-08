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

print("*** M SCN on a defined Sgroup ***")
files = [
    "455-scn-sru-ht",
    # the last M SCN entry ends right after the connectivity, no space
    "455-scn-sup-no-trailing-space",
]
for filename in sorted(files):
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    compare_diff(ref_path, filename + ".mol", mol.molfile())

print("*** M SCN referring to an Sgroup that is not defined ***")
files = [
    "455-scn-no-sgroups",
    "455-scn-past-last-sgroup",
    "455-scn-index-0",
]
for filename in sorted(files):
    try:
        indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
