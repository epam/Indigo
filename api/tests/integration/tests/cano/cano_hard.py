import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("timeout", "6000")


def remove_prefix(text, prefix):
    return text[len(prefix) :] if text.startswith(prefix) else text


idx = 1
for m in indigo.iterateSDFile(joinPathPy("molecules/hard.sdf", __file__)):
    print("Item #%d" % (idx))
    print("  Try #1")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(remove_prefix(getIndigoExceptionText(e), "Molecule "))
    print("  Try #2")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(remove_prefix(getIndigoExceptionText(e), "Molecule "))
    idx += 1
