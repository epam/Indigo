import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff, find_diff
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
    "gen-after-mul",
    "gen-before-mul",
    "mul-without-bonds",
    "no-crossing-bonds",
    "sru-around-mul",
    "sru-without-bonds",
]


def bond_order_free(molfile):
    # A KET round trip folds and expands a multiple group, which reorders
    # the bonds: a V2000 molfile is compared with its bond block sorted and
    # the crossing bonds given as atom pairs
    lines = molfile.splitlines()
    atom_count, bond_count = int(lines[3][0:3]), int(lines[3][3:6])
    first = 4 + atom_count
    block = lines[first : first + bond_count]
    pairs = [
        tuple(sorted(int(n) for n in line.split()[0:2])) for line in block
    ]
    lines[first : first + bond_count] = sorted(block)
    for k, line in enumerate(lines):
        if line.startswith("M  SBL"):
            fields = line.split()
            crossing = sorted(pairs[int(b) - 1] for b in fields[4:])
            lines[k] = "M  SBL %s %s" % (fields[2], crossing)
    return "\n".join(lines)


for filename in files:
    path = os.path.join(root, filename + ".ket")
    try:
        mol = indigo.loadMoleculeFromFile(path)
        reload = indigo.loadMolecule
    except IndigoException:
        mol = indigo.loadQueryMoleculeFromFile(path)
        reload = indigo.loadQueryMolecule
    for mode in ["2000", "3000"]:
        indigo.setOption("molfile-saving-mode", mode)
        compare_diff(ref_path, "%s-%s.mol" % (filename, mode), mol.molfile())

    indigo.setOption("molfile-saving-mode", "2000")
    diff = find_diff(
        bond_order_free(mol.molfile()),
        bond_order_free(reload(mol.json()).molfile()),
    )
    print(
        "%s KET round trip:%s"
        % (filename, "FAILED\n" + diff if diff else "SUCCEED")
    )
