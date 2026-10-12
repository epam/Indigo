import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff, find_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** Molfile S-groups ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/sgroup/", __file__)

files = [
    "multiline-sgroups-ketcher-457-empty",
    "multiline-sgroups-ketcher-457-single",
    "multiline-sgroups-ketcher-457-v2000",
    "multiline-sgroups-ketcher-457-v3000",
]


def strip_sed(molfile):
    return "\n".join(
        line.rstrip() if line.startswith("M  SED") else line
        for line in molfile.splitlines()
    )


for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    for mode in ["auto", "2000", "3000"]:
        indigo.setOption("molfile-saving-mode", mode)
        molfile = mol.molfile()
        name = "%s-%s.mol" % (filename, mode)
        compare_diff(ref_path, name, molfile)

        resaved = indigo.loadMolecule(molfile).molfile()
        diff = find_diff(strip_sed(molfile), strip_sed(resaved))
        print(
            "%s reload:%s" % (name, "FAILED\n" + diff if diff else "SUCCEED")
        )
