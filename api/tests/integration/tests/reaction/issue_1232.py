import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *

indigo = Indigo()

# Chack that multi-line reaction loaded and converted without errors
r1 = indigo.loadReactionFromFile(
    joinPathPy("reactions/issue_1232.ket", __file__)
)
rxn = r1.rxnfile()
print("Issue 1232: OK")
