import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
bingo = Bingo.createDatabaseFile(
    indigo, joiinPathPy("out/get_indigo_object_bug", __file__), "molecule"
)
for item in ("C1=CC=CC=C1", "C1=CN=CC=C1"):
    bingo.insert(indigo.loadMolecule(item))
result = bingo.searchSim(
    indigo.loadMolecule("C1=CC=CC=C1"), 0.3, 1.0, "tanimoto"
)
while result.next():
    print(
        result.getCurrentId(),
        result.getCurrentSimilarityValue(),
        result.getIndigoObject().smiles(),
    )
bingo.close()
