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
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

print("*** CDXML reaction step participants ***")

root = joinPathPy("reactions/step-participants/", __file__)
ref_path = joinPathPy("ref/step-participants/", __file__)

files = [
    "agent-caption-on-page",
    "agent-group-caption",
    "agent-text-graphic",
    "captions-on-page",
    "empty-participant",
    "nested-brackets",
    "step-participants",
]

loaded = {}
for filename in files:
    try:
        rxn = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".cdxml")
        )
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
        continue
    ket = rxn.json()
    loaded[filename] = json.loads(ket)
    compare_diff(ref_path, filename + ".ket", ket)


def text_nodes(doc):
    return sorted(
        json.dumps(node, sort_keys=True)
        for node in doc["root"]["nodes"]
        if node.get("type") == "text"
    )


print("*** A caption in a participant group is kept as on the page ***")
for grouped, on_page in [
    ("step-participants", "captions-on-page"),
    ("agent-group-caption", "agent-caption-on-page"),
]:
    same = (
        grouped in loaded
        and on_page in loaded
        and text_nodes(loaded[grouped]) == text_nodes(loaded[on_page])
    )
    print(
        "%s and %s: %s"
        % (grouped, on_page, "same text nodes" if same else "DIFFERENT")
    )
