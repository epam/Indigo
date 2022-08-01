import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereo_either-0020.mol", __file__)
)
print(mol.molfile())
