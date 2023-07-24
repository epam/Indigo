import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *
from rendering import *

if __name__ == "__main__":
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", True)
    rfile = open(joinPathPy("molecules/multistep.ket", __file__))
    reaction = rfile.read()
    r = indigo.loadReaction(reaction)
    print(r.smiles())
    print(r.rxnfile())
