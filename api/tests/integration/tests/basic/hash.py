import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()

hashes = {}
for molecule in indigo.iterateSmilesFile(
    dataPath("molecules/basic/sample_100000.smi")
):
    hash_ = molecule.hash()
    if hash_ in hashes:
        if not indigo.exactMatch(
            molecule, indigo.loadMolecule(hashes[hash_]), "NONE"
        ):
            print(
                "Hash for molecule {} collides with molecule {}".format(
                    molecule.rawData(), hashes[hash_]
                )
            )
    else:
        hashes[hash_] = molecule.rawData()
