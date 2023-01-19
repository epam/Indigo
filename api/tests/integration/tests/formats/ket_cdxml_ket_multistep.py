import json
import os
import sys
import xml.etree.ElementTree as ET

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** KET to CDXML to KET ***")

root = joinPathPy("reactions/", __file__)
files = [
    "multi.ket",
]

files.sort()

for filename in files:
    print(filename)
    try:
        ket = indigo.loadReactionFromFile(os.path.join(root, filename))
        ket.layout()
        cdxml_raw = ket.cdxml()
        cdxml = ET.XML(cdxml_raw)
        ET.indent(cdxml)
        cdxml_text = ET.dump(cdxml)
        print(cdxml_text)
        ket = indigo.loadReaction(cdxml_raw)
        ket.layout()
        print(json.dumps(json.loads(ket.json()), indent=2))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
