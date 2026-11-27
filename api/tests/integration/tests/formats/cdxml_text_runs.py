import json
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff  # noqa
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    IndigoRenderer,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "svg")

root = joinPathPy("molecules/text-runs/", __file__)
ref_path = joinPathPy("ref/text-runs/", __file__)


def render_error(structure):
    try:
        renderer.renderToBuffer(structure)
    except IndigoException as e:
        return getIndigoExceptionText(e)
    return None


print("*** CDX and CDXML text runs to KET ***")
# A file with a reaction plus is loaded as a reaction, the others as molecules.
files = [
    ("caption-plus-runs.cdxml", indigo.loadReactionFromFile),
    ("font-table-truncated.cdx", indigo.loadMoleculeFromFile),
    ("latin1-bold-degree-signs.cdxml", indigo.loadMoleculeFromFile),
    ("plus-with-spaces-one-run.cdxml", indigo.loadReactionFromFile),
    ("plus-with-spaces-runs.cdx", indigo.loadReactionFromFile),
    ("symbol-by-charset.cdxml", indigo.loadMoleculeFromFile),
    ("symbol-bytes-valid-as-utf8.cdx", indigo.loadMoleculeFromFile),
    ("symbol-codes-above-7f.cdxml", indigo.loadMoleculeFromFile),
    ("symbol-label.cdxml", indigo.loadMoleculeFromFile),
    ("symbol-letter-before-bold.cdx", indigo.loadMoleculeFromFile),
    ("symbol-letter-before-bold.cdxml", indigo.loadMoleculeFromFile),
    ("symbol-letter-subscript-superscript.cdxml", indigo.loadMoleculeFromFile),
    ("symbol-minus-in-caption.cdxml", indigo.loadMoleculeFromFile),
]
for filename, load in files:
    try:
        structure = load(os.path.join(root, filename))
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
        continue
    compare_diff(ref_path, filename + ".ket", structure.json())
    error = render_error(structure)
    if error:
        print("%s: render: %s" % (filename, error))

print("*** KET styles after non-ASCII text to KET 2.0 and CDXML ***")
filename = "styles-after-non-ascii.ket"
structure = indigo.loadMoleculeFromFile(os.path.join(root, filename))
indigo.setOption("ket-saving-version", "2.0.0")
compare_diff(ref_path, filename + ".v2.ket", structure.json())
indigo.setOption("ket-saving-version", "1.0.0")
compare_diff(ref_path, filename + ".cdxml", structure.cdxml())
error = render_error(structure)
if error:
    print("%s: render: %s" % (filename, error))

print("*** ChemDraw file: Symbol font run in a text and an S-group label ***")
structure = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/test-multi.cdx", __file__)
)
doc = json.loads(structure.json())
for node in doc["root"]["nodes"]:
    if node.get("type") == "text":
        for block in json.loads(node["data"]["content"])["blocks"]:
            if block["text"] and ord(block["text"][0]) == 0x3B2:  # beta
                print(
                    "text %s styles %s"
                    % (
                        json.dumps(block["text"]),
                        json.dumps(block["inlineStyleRanges"], sort_keys=True),
                    )
                )
names = []
for key in sorted(doc):
    if key.startswith("mol"):
        names += [s.get("name", "") for s in doc[key].get("sgroups", [])]
print("S-group names %s" % json.dumps(sorted(set(names))))
error = render_error(structure)
if error:
    print("test-multi.cdx: render: %s" % error)
