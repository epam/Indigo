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

print("*** KET to CDXML to KET ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "agents",
    "multi",
    "multi_overlap",
    "961-text_size",
    "generic",
    "2558-missed",
]

files.sort()

for filename in files:
    with open(os.path.join(root, filename + ".ket"), "r") as file:
        ket_str = file.read()
    try:
        ket = indigo.loadMolecule(ket_str)
    except IndigoException as e:
        try:
            ket = indigo.loadQueryMolecule(ket_str)
        except IndigoException as e:
            try:
                ket = indigo.loadReaction(ket_str)
            except IndigoException as e:
                try:
                    ket = indigo.loadQueryReaction(ket_str)
                except IndigoException as e:
                    print(getIndigoExceptionText(e))
                    raise SystemExit

    cdxml_text = ket.cdxml()
    compare_diff(ref_path, filename + ".cdxml", cdxml_text)

    try:
        ket = indigo.loadMolecule(cdxml_text)
    except IndigoException as e:
        ket = indigo.loadReaction(cdxml_text)

    ket_result = ket.json()
    compare_diff(ref_path, filename + ".ket", ket_result)

print("*** KET to CDXML ***")

root_m = joinPathPy("molecules/", __file__)
files = ["963-super", "macro/sa-mono", "images", "shapes", "3080-star-issue"]

files.sort()

for filename in files:
    with open(os.path.join(root_m, filename + ".ket"), "r") as file:
        ket_str = file.read()
    try:
        ket = indigo.loadMolecule(ket_str)
    except IndigoException as e:
        try:
            ket = indigo.loadQueryMolecule(ket_str)
        except IndigoException as eq:
            print(getIndigoExceptionText(e))
            raise SystemExit
    cdxml_text = ket.cdxml()
    indigo.loadMolecule(
        cdxml_text
    )  # just check if cdxml is valid and loaded without an exception
    compare_diff(ref_path, filename + ".cdxml", cdxml_text)

print("*** Reaction CDXML to KET ***")
indigo.setOption("ignore-stereochemistry-errors", True)

root_cdxml = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "2333-EnhancedStereochemistry",
]

for filename in files:
    with open(os.path.join(root_cdxml, filename + ".cdxml"), "r") as file:
        cdxml_str = file.read()
    try:
        reaction = indigo.loadReaction(cdxml_str)
        ket_result = reaction.json()
        compare_diff(ref_path, filename + ".ket", ket_result)
    except IndigoException as e:
        print(e)
