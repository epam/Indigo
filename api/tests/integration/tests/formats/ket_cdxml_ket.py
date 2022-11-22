import os
import sys

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
    "agents.ket",
]

files.sort()

for filename in files:
    print(filename)
    try:
        print("*** Try as Reaction ***")
        ket = indigo.loadReactionFromFile(os.path.join(root, filename))
        ket.layout()
        cdxml_text = ket.cdxml()
        print(cdxml_text)
        ket = indigo.loadReaction(cdxml_text)
        ket.layout()
        print(ket.json())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as QueryReaction ***")
