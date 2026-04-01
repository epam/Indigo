import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to KET ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

files = [
    "images",
    "3087-star-process",
]

files.sort()
for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)

indigo.setOption("json-use-native-precision", True)
files = [
    "monomer_shape",
    "ambiguous_monomer",
    "expanded_monomer",
    "helm_alias",
    "modification_types",
    "monomer_transform",
    "helm_annotation",
]
savers = {
    "doc": [indigo.loadKetDocument],
    "mol": [indigo.loadMolecule, indigo.loadQueryMolecule],
}
for filename in sorted(files):
    for format in sorted(savers.keys()):
        _filename = "{}_{}.ket".format(filename, format)
        file_path = os.path.join(ref_path, _filename)
        with open(file_path, "r") as file:
            ket_ref = file.read()
        for loader in savers[format]:
            mol = loader(ket_ref)
            ket = mol.json()
            diff = compare_diff(ref_path, _filename, ket, stdout=False)
            if not diff:
                print("{} {}:SUCCEED".format(_filename, loader.__name__))
            else:
                print("{} {}:FAILED".format(_filename, loader.__name__))
                print(diff)

filename = "2707_subst_count"
file_path = os.path.join(ref_path, filename)
mol = indigo.loadQueryMoleculeFromFile("{}.ket".format(file_path))
savers = {"ket": mol.json, "mol": mol.molfile}
for format in sorted(savers.keys()):
    data = savers[format]()
    _filename = filename + ".{}".format(format)
    compare_diff(ref_path, _filename, data)

files = ["multi_merge4", "3069-reaction"]

files.sort()
for filename in files:
    try:
        rea = indigo.loadReactionFromFile(
            os.path.join(root_rea, filename + ".ket")
        )
    except:
        try:
            rea = indigo.loadQueryReactionFromFile(
                os.path.join(root_rea, filename + ".ket")
            )
        except:
            print("bad reaction data")
    ket = rea.json()
    compare_diff(ref_path, filename + ".ket", ket)

# reaction data
files = [
    "mixed_reaction",
    "two_pathways",
    "multi_reaction",
    "multi1",
    "special_condition",
]

indigo.setOption("json-saving-add-reaction-data", True)

files.sort()
for filename in files:
    rea = indigo.loadReactionFromFile(
        os.path.join(root_rea, filename + ".ket")
    )
    ket = rea.json()
    compare_diff(ref_path, filename + ".ket", ket)
