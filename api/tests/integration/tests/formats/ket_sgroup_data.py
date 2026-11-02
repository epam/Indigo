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
indigo.setOption("json-saving-pretty", True)

print("*** KET data S-groups ***")

root = joinPathPy("molecules/ket_sgroup_data/", __file__)
ref_path = joinPathPy("ref/ket_sgroup_data/", __file__)


def path(name):
    return os.path.join(root, name + ".ket")


print("*** KET 2.0 keys with values the contract allows ***")
for filename in ("data-keys-at-limits", "data-keys-beyond-molfile-limits"):
    mol = indigo.loadMoleculeFromFile(path(filename))
    compare_diff(ref_path, filename + "-saved.ket", mol.json())

print("*** Contexts of data, saved, cloned and merged ***")
filename = "data-contexts"
mol = indigo.loadMoleculeFromFile(path(filename))
merged = indigo.createMolecule()
merged.merge(mol)
for copy in (mol, mol.clone(), merged):
    compare_diff(ref_path, filename + "-saved.ket", copy.json())

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
