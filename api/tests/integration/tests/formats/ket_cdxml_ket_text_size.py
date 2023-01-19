import json
import os
import sys
import xml.etree.ElementTree as ET

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)


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


from env_indigo import *  # noqa

indigo = Indigo()

print("*** KET to CDXML to KET ***")

root = joinPathPy("molecules/", __file__)
files = [
    "text_size.ket",
]

files.sort()

for filename in files:
    print(filename)
    try:
        ket = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        ket.layout()
        cdxml_raw = ket.cdxml()
        cdxml = ET.XML(cdxml_raw)
        indent(cdxml)
        cdxml_text = ET.dump(cdxml)
        print(cdxml_text)
        ket = indigo.loadMolecule(cdxml_raw)
        ket.layout()
        print(json.dumps(json.loads(ket.json()), indent=2))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
