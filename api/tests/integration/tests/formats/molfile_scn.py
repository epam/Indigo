import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def molfile(atoms, bonds, properties):
    lines = ["", "  -INDIGO-", ""]
    lines.append("%3d%3d" % (len(atoms), len(bonds)) + "  0" * 8 + "999 V2000")
    for i, label in enumerate(atoms):
        lines.append(
            "%10.4f%10.4f%10.4f %-3s" % (1.5 * i, 0.0, 0.0, label)
            + " 0  0  0  0  0  0  0  0  0  0  0  0"
        )
    for begin, end in bonds:
        lines.append("%3d%3d  1  0  0  0  0" % (begin, end))
    lines.extend(properties)
    lines.append("M  END")
    return "\n".join(lines) + "\n"


def load(name, text):
    try:
        mol = indigo.loadMolecule(text)
        # print the lines, not the list: Jython would show them as u'...'
        scn = [line for line in mol.molfile().splitlines() if "SCN" in line]
        saved = ", ".join('"%s"' % line for line in scn) or "none"
        print(
            "%s: OK, %d atoms, M SCN saved: %s"
            % (name, mol.countAtoms(), saved)
        )
    except IndigoException as e:
        print("%s: %s" % (name, getIndigoExceptionText(e)))


print("*** M SCN on a defined repeating unit ***")
load(
    "SRU with head-to-tail connectivity",
    molfile(
        ["C", "C"],
        [(1, 2)],
        ["M  STY  1   1 SRU", "M  SAL   1  2   1   2", "M  SCN  1   1 HT "],
    ),
)

print("*** M SCN on a superatom, last entry without the trailing space ***")
load(
    "SUP, line ends right after the connectivity",
    molfile(
        ["C", "C"],
        [(1, 2)],
        [
            "M  STY  1   1 SUP",
            "M  SAL   1  1   2",
            "M  SMT   1 Me",
            "M  SCN  1   1 HT",
        ],
    ),
)

print("*** M SCN referring to an Sgroup that is not defined ***")
load(
    "no Sgroups at all",
    molfile(["C"], [], ["M  SCN  1   1 HT "]),
)
load(
    "index past the last Sgroup",
    molfile(
        ["C", "C"],
        [(1, 2)],
        ["M  STY  1   1 SRU", "M  SAL   1  2   1   2", "M  SCN  1   2 HT "],
    ),
)
load(
    "index 0",
    molfile(
        ["C", "C"],
        [(1, 2)],
        ["M  STY  1   1 SRU", "M  SAL   1  2   1   2", "M  SCN  1   0 HT "],
    ),
)
