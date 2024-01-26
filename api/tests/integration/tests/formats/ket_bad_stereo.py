import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

root_rea = joinPathPy("reactions/", __file__)

try:
    reaction = indigo.loadReactionFromFile(
        os.path.join(root_rea, "bad_stereo.ket")
    )
except IndigoException as e:
    print(getIndigoExceptionText(e))

indigo.setOption("ignore-stereochemistry-errors", "1")
reaction = indigo.loadReactionFromFile(
    os.path.join(root_rea, "bad_stereo.ket")
)
print(reaction.smiles())


print("Test load/save 'reaction not center' bond:")
indigo.setOption("json-saving-pretty", "1")
filename = os.path.join(root_rea, "stereo_not_center.ket")
reaction = indigo.loadQueryReactionFromFile(filename)
with open(filename) as f:
    ref_ket = f.read()
save_ket = reaction.json()
if ref_ket == save_ket:
    print("SUCCESS")
else:
    print("FAILED: expected\n%s\ngenerated\n%s" % (ref_ket, save_ket))
