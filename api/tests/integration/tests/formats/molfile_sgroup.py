import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff, find_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** Molfile S-groups ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/sgroup/", __file__)

all_modes = ["auto", "2000", "3000"]
explicit_modes = ["2000", "3000"]
files = [
    ("multiline-sgroups-ketcher-457-empty.mol", all_modes),
    ("multiline-sgroups-ketcher-457-single.mol", all_modes),
    ("multiline-sgroups-ketcher-457-v2000.mol", all_modes),
    ("multiline-sgroups-ketcher-457-v3000.mol", all_modes),
    ("sgroup/data-displayed-chars-out-of-range.ket", explicit_modes),
    ("sgroup/v2000-data-single-empty-line.mol", explicit_modes),
    ("sgroup/v2000-sdd-character-count.mol", explicit_modes),
    ("sgroup/v2000-sdd-character-count-forms.mol", explicit_modes),
    ("sgroup/v2000-sdt-on-gen-a.mol", explicit_modes),
    ("sgroup/v2000-sdt-on-gen-empty.mol", explicit_modes),
    ("sgroup/v2000-sdt-on-gen-g.mol", explicit_modes),
    ("sgroup/v2000-sgroup-index-one-column-short.mol", explicit_modes),
    ("sgroup/v2000-sgroup-lines-ending-after-index.mol", explicit_modes),
    ("sgroup/v3000-data-line-of-69-characters.mol", explicit_modes),
    ("sgroup/v3000-data-lines-around-69-characters.mol", explicit_modes),
    ("sgroup/v3000-default-forms.mol", explicit_modes),
    ("sgroup/v3000-default-line.mol", explicit_modes),
    ("sgroup/v3000-fielddata-forms.mol", explicit_modes),
]
broken_files = [
    "sgroup/v2000-sgroup-index-out-of-range.mol",
    "sgroup/v2000-sgroup-index-zero.mol",
    "sgroup/v2000-sgroup-without-sty.mol",
    "sgroup/v3000-default-without-value.mol",
]


def strip_sed(molfile):
    return "\n".join(
        line.rstrip() if line.startswith("M  SED") else line
        for line in molfile.splitlines()
    )


for filename, modes in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    base = os.path.splitext(os.path.basename(filename))[0]
    for mode in modes:
        indigo.setOption("molfile-saving-mode", mode)
        molfile = mol.molfile()
        name = "%s-%s.mol" % (base, mode)
        compare_diff(ref_path, name, molfile)

        resaved = indigo.loadMolecule(molfile).molfile()
        diff = find_diff(strip_sed(molfile), strip_sed(resaved))
        print(
            "%s reload:%s" % (name, "FAILED\n" + diff if diff else "SUCCEED")
        )

for filename in broken_files:
    try:
        indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print("%s: no error" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
