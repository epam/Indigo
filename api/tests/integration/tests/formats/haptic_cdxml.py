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
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)

print("*** Haptic bonds in CDX and CDXML ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

# Ferrocene as the CDX specification describes it: one MultiAttachment node per
# ring, each joined to the iron by an ordinary bond. The second file puts it on a
# page with two more fragments, a benzene and a half-sandwich manganese.
files = ["haptic-ferrocene", "haptic-several-molecules"]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".cdxml"))
    compare_diff(ref_path, filename + ".ket", mol.json())

    cdxml = mol.cdxml()
    compare_diff(ref_path, filename + ".cdxml", cdxml)

    cdx = mol.b64cdx()
    compare_diff(ref_path, filename + ".b64cdx", cdx)

    from_cdxml = indigo.loadMolecule(cdxml).json()
    compare_diff(ref_path, filename + "-from-cdxml.ket", from_cdxml)

    from_cdx = indigo.loadMolecule(cdx).json()
    compare_diff(ref_path, filename + "-from-cdx.ket", from_cdx)

# Friedel-Crafts acylation of ferrocene as Ketcher draws it: a haptic complex on
# each side of the arrow, each saved as a fragment of its own.
reactions = ["haptic-acylation"]

reactions.sort()
for filename in reactions:
    rxn = indigo.loadReactionFromFile(os.path.join(root, filename + ".ket"))

    cdxml = rxn.cdxml()
    compare_diff(ref_path, filename + ".cdxml", cdxml)

    cdx = rxn.b64cdx()
    compare_diff(ref_path, filename + ".b64cdx", cdx)

    from_cdxml = indigo.loadReaction(cdxml).json()
    compare_diff(ref_path, filename + "-from-cdxml.ket", from_cdxml)

    from_cdx = indigo.loadReaction(cdx).json()
    compare_diff(ref_path, filename + "-from-cdx.ket", from_cdx)
