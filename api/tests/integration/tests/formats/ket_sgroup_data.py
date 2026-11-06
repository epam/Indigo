import binascii
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff, isIronPython
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET data S-groups ***")

root = joinPathPy("molecules/ket_sgroup_data/", __file__)
ref_path = joinPathPy("ref/ket_sgroup_data/", __file__)


def path(name):
    return os.path.join(root, name + ".ket")


def cmf_text(name):
    with open(os.path.join(root, name + ".cmf")) as cmf_file:
        return cmf_file.read().strip()


def cmf_from_hex(text):
    buf = bytes(binascii.unhexlify(text))
    if isIronPython():
        from System import Array, Byte

        buf = Array[Byte](buf)
    return buf


def cmf_hex(buf):
    return binascii.hexlify(bytearray(buf)).decode("ascii")


print("*** KET 2.0 keys with values the contract allows ***")
for filename in ("data-keys-at-limits", "data-keys-beyond-molfile-limits"):
    mol = indigo.loadMoleculeFromFile(path(filename))
    compare_diff(ref_path, filename + "-saved.ket", mol.json())

print("*** Placement and display of KET 2.0, and the offset of a molfile ***")
filename = "data-placement-and-display"
mol = indigo.loadMoleculeFromFile(path(filename))
compare_diff(ref_path, filename + "-saved.ket", mol.json())
indigo.setOption("molfile-saving-mode", "2000")
molfile = mol.molfile()
compare_diff(ref_path, filename + "-2000.mol", molfile)
compare_diff(
    ref_path, filename + "-saved.ket", indigo.loadMolecule(molfile).json()
)

print("*** Contexts of data, saved, cloned and merged ***")
filename = "data-contexts"
mol = indigo.loadMoleculeFromFile(path(filename))
merged = indigo.createMolecule()
merged.merge(mol)
for copy in (mol, mol.clone(), merged):
    compare_diff(ref_path, filename + "-saved.ket", copy.json())

print("*** Contexts of data through CMF ***")
unserialized = indigo.unserialize(mol.serialize())
compare_diff(ref_path, filename + "-unserialized.ket", unserialized.json())
# Written before CMF had a record for the context: read as it was, and
# written again byte for byte
filename = "data-contexts-without-context"
written_before = cmf_text(filename)
mol = indigo.unserialize(cmf_from_hex(written_before))
compare_diff(ref_path, filename + "-unserialized.ket", mol.json())
print(
    "%s written again unchanged: %s"
    % (filename, cmf_hex(mol.serialize()) == written_before)
)
for filename in (
    "rejected-cmf-context-after-generic-sgroup",
    "rejected-cmf-context-before-any-sgroup",
):
    try:
        indigo.unserialize(cmf_from_hex(cmf_text(filename)))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))

print("*** KET 2.0 keys with values outside the contract ***")
for filename in (
    "rejected-context-lowercase",
    "rejected-context-not-string",
    "rejected-field-name-not-string",
    "rejected-placement-not-boolean",
    "rejected-tag-two-characters",
    "rejected-x-four-decimals-lost",
    "rejected-x-not-number",
):
    try:
        indigo.loadMoleculeFromFile(path(filename))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))

print("*** Values KET keeps and a molfile does not allow ***")
for filename in (
    "molfile-refuses-coordinate",
    "molfile-refuses-displayed-chars",
    "molfile-refuses-numeric-text",
    "molfile-refuses-query-operator",
    "molfile-refuses-query-type",
):
    mol = indigo.loadMoleculeFromFile(path(filename))
    for mode in ("2000", "3000"):
        indigo.setOption("molfile-saving-mode", mode)
        try:
            mol.molfile()
            print("%s, V%s: written" % (filename, mode))
        except IndigoException as e:
            print("%s, V%s: %s" % (filename, mode, getIndigoExceptionText(e)))

print(
    "*** KET 2.1 objects: written from 2.1 on, read before the keys of 2.0 ***"
)
VERSION_2_1 = "2.1.0"
saved_by_version = (
    ("1.0.0", "-saved-2.0.ket"),
    (VERSION_2_1, "-saved-2.1.ket"),
)
# Indigo's own 2.1 document; the objects next to 2.0 keys that differ;
# partial objects read as their defaults, an operator without QUERY and one
# outside the names kept as they are
for filename in (
    "objects-written-by-indigo",
    "objects-win-over-keys",
    "objects-partial-and-any-operator",
):
    mol = indigo.loadMoleculeFromFile(path(filename))
    for version, suffix in saved_by_version:
        indigo.setOption("ket-saving-version", version)
        compare_diff(ref_path, filename + suffix, mol.json())

indigo.setOption("ket-saving-version", VERSION_2_1)
filename = "objects-written-by-indigo"
saved = indigo.loadMoleculeFromFile(path(filename)).json()
compare_diff(
    ref_path, filename + "-saved-2.1.ket", indigo.loadMolecule(saved).json()
)
# every field but the context has a column in a molfile
indigo.setOption("molfile-saving-mode", "2000")
molfile = indigo.loadMolecule(saved).molfile()
compare_diff(ref_path, filename + "-2000.mol", molfile)
compare_diff(
    ref_path,
    filename + "-from-molfile-2.1.ket",
    indigo.loadMolecule(molfile).json(),
)
# a 2.0 document written as 2.1: the objects with every default filled in
filename = "data-placement-and-display"
compare_diff(
    ref_path,
    filename + "-saved-2.1.ket",
    indigo.loadMoleculeFromFile(path(filename)).json(),
)

# a kind of the value or a query type outside the names is refused, as the molfile writer refuses it
mol = indigo.loadMoleculeFromFile(path("data-placement-and-display"))
for data in mol.iterateDataSGroups():
    break
for setter, value, restore in (
    (data.setSGroupDataType, "X", "T"),
    (data.setSGroupQueryCode, " I", ""),
):
    setter(value)
    for version in ("1.0.0", VERSION_2_1):
        indigo.setOption("ket-saving-version", version)
        try:
            mol.json()
            print("%r as %s: written" % (value, version))
        except IndigoException as e:
            print("%r as %s: %s" % (value, version, getIndigoExceptionText(e)))
    setter(restore)
indigo.setOption("ket-saving-version", "1.0.0")

print("*** KET 2.1 objects with values outside the contract ***")
for filename in (
    "rejected-object-dasp-position-zero",
    "rejected-object-description-array",
    "rejected-object-display-lowercase",
    "rejected-object-display-units-string",
    "rejected-object-field-tag-empty",
    "rejected-object-field-value-type-unknown",
    "rejected-object-placement-boolean",
    "rejected-object-position-array",
    "rejected-object-position-x-four-decimals-lost",
    "rejected-object-query-operator-number",
    "rejected-object-query-type-molfile-code",
):
    try:
        indigo.loadMoleculeFromFile(path(filename))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
