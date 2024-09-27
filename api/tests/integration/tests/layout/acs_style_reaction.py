import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, joinPathPy, reactionLayoutDiff  # noqa


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("json-saving-pretty", "1")
indigo.setOption("json-use-native-precision", "1")

root = joinPathPy("reactions/", __file__)
ref = joinPathPy("ref/", __file__)

upd = False

print("*** ACS reaction layout testing ***")
print("\n*** test zero margin ***")
rxn = indigo.loadReactionFromFile(os.path.join(root, "acs_before_layout.ket"))
indigo.setOption("reaction-component-margin-size", "0.0")
rxn.layout()

# with open(os.path.join(ref, "acs_after_layout_zero_margin.ket"), "w") as file:
#     file.write(rxn.json())

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

# with open(os.path.join(ref, "acs_after_layout_default_margin.ket"), "w") as file:
#     file.write(rxn.json())

res = reactionLayoutDiff(
    indigo,
    rxn,
    "acs_after_layout_default_margin.ket",
    update=upd,
    update_format="ket",
)
print("  Result: {}".format(res))

print("\n*** 2389 wrong margin ***")
rxn = indigo.loadReaction("CN.CO>CC.CC>CF.CP")
rxn.layout()
filename = "acs_issue_2389.ket"
# with open(os.path.join(ref, filename), "w") as file:
#     file.write(rxn.json())
with open(os.path.join(ref, filename), "r") as file:
    ket_ref = file.read()
ket = rxn.json()
diff = find_diff(ket_ref, ket)
if not diff:
    print(filename + ".ket:SUCCEED")
else:
    print(filename + ".ket:FAILED")
    print(diff)
