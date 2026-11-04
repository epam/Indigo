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
    # a data line of 345 characters: V2000 holds 200, so auto writes V3000
    ("multiline-sgroups-ketcher-457-v3000.mol", ["auto", "3000"]),
    ("sgroup/v2000-data-query-operators.mol", explicit_modes),
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
    "sgroup/data-displayed-chars-negative.ket",
    "sgroup/v2000-sdd-character-count-negative.mol",
    "sgroup/v2000-sdd-character-count-negative-compact.mol",
    "sgroup/v2000-sdd-coordinate-not-a-number.mol",
    "sgroup/v2000-sdd-coordinate-not-kept.mol",
    "sgroup/v2000-sdd-dasp-position-unknown.mol",
    "sgroup/v2000-sdt-field-type-unknown.mol",
    "sgroup/v2000-sdt-query-operator-unknown.mol",
    "sgroup/v2000-sdt-query-type-shifted.mol",
    "sgroup/v2000-sed-numeric-not-a-number.mol",
    "sgroup/v2000-sgroup-index-out-of-range.mol",
    "sgroup/v2000-sgroup-index-zero.mol",
    "sgroup/v2000-sgroup-without-sty.mol",
    "sgroup/v3000-default-without-value.mol",
    "sgroup/v3000-fielddisp-character-count-negative.mol",
    "sgroup/v3000-querytype-unknown.mol",
    "sgroup/v3000-queryop-unknown.mol",
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


def saved_as(mol, mode):
    indigo.setOption("molfile-saving-mode", mode)
    return mol.molfile()


def version_written(molfile):
    return molfile.splitlines()[3][-5:]


print(
    "*** What V2000 cannot hold: auto writes V3000, a requested V2000 fails ***"
)
beyond_v2000 = [
    "sgroup/v2000-beyond-field-name.ket",
    "sgroup/v2000-beyond-units.ket",
    "sgroup/v2000-beyond-data-line.ket",
    "sgroup/v2000-beyond-atom-coordinate.ket",
    "sgroup/v3000-beyond-bracket-coordinate.mol",
]
for filename in beyond_v2000:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    base = os.path.splitext(os.path.basename(filename))[0]
    compare_diff(ref_path, base + "-auto.mol", saved_as(mol, "auto"))
for filename in ["multiline-sgroups-ketcher-457-v3000.mol"] + beyond_v2000:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    base = os.path.splitext(os.path.basename(filename))[0]
    try:
        saved_as(mol, "2000")
        print("%s as V2000: written" % base)
    except IndigoException as e:
        print("%s as V2000: %s" % (base, getIndigoExceptionText(e)))

print("*** At the limits of V2000: auto stays V2000, and it reads back ***")
for filename in [
    "sgroup/v2000-at-limits.ket",
    "sgroup/v3000-bracket-at-limits.mol",
]:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    base = os.path.splitext(os.path.basename(filename))[0]
    molfile = saved_as(mol, "auto")
    compare_diff(ref_path, base + "-auto.mol", molfile)
    compare_diff(
        ref_path,
        base + "-auto.mol",
        saved_as(indigo.loadMolecule(molfile), "auto"),
    )

print("*** More S-groups than V2000 numbers ***")
for count in (999, 1000):
    mol = indigo.loadMolecule("C")
    for i in range(count):
        mol.addDataSGroup([0], [], "F", str(i))
    molfile = saved_as(mol, "auto")
    print(
        "%d data S-groups in auto mode: %s, %d read back"
        % (
            count,
            version_written(molfile),
            indigo.loadMolecule(molfile).countDataSGroups(),
        )
    )
    try:
        saved_as(mol, "2000")
        print("%d data S-groups as V2000: written" % count)
    except IndigoException as e:
        print(
            "%d data S-groups as V2000: %s"
            % (count, getIndigoExceptionText(e))
        )

print("*** A data line longer than 200 characters is cut when read ***")
base = "v2000-sed-line-of-210-characters"
mol = indigo.loadMoleculeFromFile(
    os.path.join(root, "sgroup/" + base + ".mol")
)
for data in mol.iterateDataSGroups():
    print("%s: %d characters read" % (base, len(data.data())))
compare_diff(ref_path, base + "-2000.mol", saved_as(mol, "2000"))
