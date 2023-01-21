import json
import os
import sys
import xml.etree.ElementTree as ET


def find_diff(a, b):
    if a == b:
        return ""
    return "not equal"


def indent(elem, level=0):
    i = "\n" + level * "  "
    j = "\n" + (level - 1) * "  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for subelem in elem:
            indent(subelem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = j
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = j
    return elem


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** KET to CDXML to KET ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "agents",
    "multi",
    "multi_overlap",
    "961-text_size",
]

files.sort()

for filename in files:
    try:
        with open(os.path.join(root, filename + ".ket"), "r") as file:
            ket_str = file.read()
        ket = indigo.loadReaction(ket_str)
        ket.layout()
        cdxml_raw = ket.cdxml()
        cdxml = ET.XML(cdxml_raw)
        indent(cdxml)
        cdxml_formatted = ET.tostring(
            cdxml, encoding="utf-8" if sys.version_info[0] < 3 else "unicode"
        )
        ket = indigo.loadReaction(cdxml_raw)
        ket_result = json.dumps(json.loads(ket.json()), indent=2)
        with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
            ket_ref = file.read()
        with open(os.path.join(ref_path, filename) + ".cdxml", "r") as file:
            cdxml_ref = file.read()
        diff = find_diff(cdxml_ref, cdxml_formatted)
        if not diff:
            print(filename + ".cdxml:SUCCEED")
        else:
            print(filename + ".cdxml:FAILED")
            print("difference:" + diff)
        diff = find_diff(ket_ref, ket_result)
        if not diff:
            print(filename + ".ket:SUCCEED")
        else:
            print(filename + ".ket:FAILED")
            print("difference:" + diff)
    except IndigoException as e:
        print(getIndigoExceptionText(e))
