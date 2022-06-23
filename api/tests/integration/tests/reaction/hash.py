import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()

hashes = {}
with open(dataPath("reactions/basic/40000.rsmi")) as f:
    for smiles in f:
        reaction = indigo.loadReaction(smiles)
        hash_ = reaction.hash()
        if hash_ in hashes:
            if not indigo.exactMatch(
                reaction, indigo.loadReaction(hashes[hash_]), "NONE"
            ):
                print(
                    "Hash for reaction {} collides with reactions {}".format(
                        smiles, hashes[hash_]
                    )
                )
        else:
            hashes[hash_] = smiles
