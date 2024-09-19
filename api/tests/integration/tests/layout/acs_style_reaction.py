import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, joinPathPy, reactionLayoutDiff

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("json-saving-pretty", "1")
indigo.setOption("json-use-native-precision", "1")
indigo.setOption("json-saving-add-stereo-desc", "1")

root = joinPathPy("reactions/", __file__)
ref = joinPathPy("ref/", __file__)

upd = False

print("*** ACS reaction layout testing ***")
print("\n*** test zero margin ***")
rxn = indigo.loadReactionFromFile(os.path.join(root, "acs_before_layout.ket"))
indigo.setOption("reaction-component-margin-size", "0.0")
rxn.layout()
res = reactionLayoutDiff(
    indigo,
    rxn,
    "acs_after_layout_zero_margin.ket",
    update=upd,
    update_format="ket",
)
print("  Result: {}".format(res))

print("\n*** test default margin ***")
rxn = indigo.loadReactionFromFile(os.path.join(root, "acs_before_layout.ket"))
indigo.setOption("bond-length", "40.0")
indigo.setOption("reaction-component-margin-size", "20.0")
rxn.layout()
res = reactionLayoutDiff(
    indigo,
    rxn,
    "acs_after_layout_default_margin.ket",
    update=upd,
    update_format="ket",
)
print("  Result: {}".format(res))
