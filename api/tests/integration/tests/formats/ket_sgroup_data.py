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


print("*** Values at the limits of the KET 2.0 keys ***")
filename = "data-keys-at-limits"
mol = indigo.loadMoleculeFromFile(path(filename))
compare_diff(ref_path, filename + "-saved.ket", mol.json())

print("*** KET 2.0 keys with values outside the contract ***")
for filename in (
    "rejected-field-name-not-string",
    "rejected-placement-not-boolean",
    "rejected-query-operator-unknown",
    "rejected-query-type-unknown",
    "rejected-tag-two-characters",
    "rejected-x-four-decimals-lost",
    "rejected-x-not-number",
    "rejected-y-beyond-f10-4",
):
    try:
        indigo.loadMoleculeFromFile(path(filename))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
