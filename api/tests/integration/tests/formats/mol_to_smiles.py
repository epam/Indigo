import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/13rsites.mol", __file__)
    ).smiles()
)
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/1e-0.mol", __file__)
    ).smiles()
)

indigo.setOption("ignore-stereochemistry-errors", True)
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/atropisomer.mol", __file__)
    ).smiles()
)

print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/macro/sa-mono.mol", __file__)
    ).smiles()
)
