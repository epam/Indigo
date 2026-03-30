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

root_rea = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

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
filename = "stereo_not_center.ket"
reaction = indigo.loadQueryReactionFromFile(
    os.path.join(root_rea, "stereo_not_center.ket")
)
save_ket = reaction.json()
compare_diff(ref_path, filename, save_ket)
