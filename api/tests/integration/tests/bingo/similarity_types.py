import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *


def searchSim(bingo, q, minSim, maxSim, metric=None):
    print("\n **** \n")
    result = bingo.searchSim(q, minSim, maxSim, metric)
    while result.next():
        id = result.getCurrentId()
        rm = bingo.getRecordById(id)
        print(rm.smiles())
        print("%.10f" % result.getCurrentSimilarityValue())
        print(rm.fingerprint("sim").toString())
    result.close()


s1 = "CC(=O)Oc1ccccc1C(=O)O"
s2 = "C(c1c(O)cccc1)(Oc1c(C(=O)O)cccc1)=O"
s3 = "COC(=O)c1ccc(-c2cc(OC)c(O)c(C=O)c2)cc1"


print("*** Testing Similarity Types ***")

indigo = Indigo()
indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
indigo.setOption("fp-ext-enabled", False)
m1 = indigo.loadMolecule(s1)
m2 = indigo.loadMolecule(s2)
m3 = indigo.loadMolecule(s3)

if dir_exists(joinPathPy("out/similarity_types", __file__)):
    rmdir(joinPathPy("out/similarity_types", __file__))
makedirs(joinPathPy("out/similarity_types", __file__))

supported_similarity_types = [
    "sim",
    "chem",
    "ecfp2",
    "ecfp4",
    "ecfp6",
    "ecfp8",
]
for sim_type in supported_similarity_types:
    indigo.setOption("similarity-type", sim_type)

    bingo = Bingo.createDatabaseFile(
        indigo,
        joinPathPy(
            os.path.join("out", "similarity_types", sim_type), __file__
        ),
        "molecule",
        "",
    )
    bingo.insert(m1)
    bingo.insert(m2)
    bingo.insert(m3)

    searchSim(bingo, m1, 0, 1)

    bingo.close()
