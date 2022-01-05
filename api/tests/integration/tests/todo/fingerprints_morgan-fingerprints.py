import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")
indigo.setOption("ignore-noncritical-query-features", "true")
indigo.setOption("ignore-bad-valence", "true")

indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ext-enabled", False)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)


for i, molecule in enumerate(
    indigo.iterateSmilesFile(joinPathPy("molecules/b2000.smi", __file__))
):
    print(i + 1)
    print(molecule.smiles())

    for fp_type in ["ECFP2", "ECFP4", "ECFP6", "ECFP8"]:
        indigo.setOption("similarity-type", fp_type)
        fingerprint = molecule.fingerprint("sim")

        print(fingerprint.toString())
