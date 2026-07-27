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
indigo.setOption("json-saving-pretty", True)

ref_path = joinPathPy("ref/", __file__)


def check_ket_file(name):
    mol = indigo.loadQueryMoleculeFromFile(os.path.join(ref_path, name))
    ket = mol.json()
    compare_diff(ref_path, name, ket)


print("*** KET with query components ***")
check_ket_file("ket_with_query_components.ket")

print("*** KET with query properties ***")
check_ket_file("ket_with_query_properties.ket")

print("*** KET with custom query ***")
check_ket_file("ket_with_custom_query.ket")

print("*** KET with implicit H count ***")
check_ket_file("ket_with_implicit_h_count.ket")

print("*** KET with lists ***")
check_ket_file("ket_with_lists.ket")

indigo.setOption("json-use-native-precision", "1")
print("*** Issue 1567 ***")
check_ket_file("ket_with_custom_query_issue1567.ket")

print("*** KET with chirality only ***")
check_ket_file("ket_with_chirality_only.ket")
