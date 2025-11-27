import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Bingo,
    Indigo,
    dir_exists,
    joinPathPy,
    makedirs,
    rmdir,
)


def testSearchSub(bingo, smile, options=""):
    print("** searchSub({0}, {1}) **".format(smile, options))
    qm = indigo.loadQueryMolecule(qsmile)
    # qm.aromatize()
    result = bingo.searchSub(qm, options)
    while result.next():
        id = result.getCurrentId()
        print("{0} {1}".format(id, bingo.getRecordById(id).smiles()))


indigo = Indigo()
indigo.setOption("aromaticity-model", "generic")

db_dir = joinPathPy("out/basic", __file__)
if dir_exists(db_dir):
    rmdir(db_dir)
makedirs(db_dir)

print("*** Creating temporary database ****")
bingo = Bingo.createDatabaseFile(indigo, db_dir, "molecule", "")

print(bingo.version())
bingo.optimize()
smiles = (
    "O=C1CNCC2=CC=CC=C12",
    "CC(C)=C1CCC(C)=CC1",
    "NC1=NC2=C(N=C(C=N2)C(O)=O)C(=O)N1",
    "NC1=NC2=C(N=CC(=O)N2)C(=O)N1",
    "CC(=C)NC1=CC=CC=C1C(N)=O",
    "O=C1N=CNC2=C1C=NN2",
    "P(=O)(O[H])(O[H])OC([H])([H])C1=C([H])N=C(C([H])([H])[H])N=C1N([H])[H]",
)
for smile in smiles:
    bingo.insert(indigo.loadMolecule(smile))

indigo.clearTautomerRules()
indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te")
indigo.setTautomerRule(2, "0C", "N,O,P,S")
indigo.setTautomerRule(3, "1C", "N,O")

query_smiles = (
    "OC1=CC=CNC1",
    "CC1CCC(=C)CC1",
    "NC1=NC2=NC=CN=C2C=N1",
    "CC1(C)NC(=O)C2=CC=CC=C2N1",
    "OCCCN",
    "CCC",
)

for qsmile in query_smiles:
    testSearchSub(bingo, qsmile)
    testSearchSub(bingo, qsmile, "TAU")
    testSearchSub(bingo, qsmile, "TAU R1")
    testSearchSub(bingo, qsmile, "TAU R2")
    testSearchSub(bingo, qsmile, "TAU R3")
    testSearchSub(bingo, qsmile, "TAU R*")
    testSearchSub(bingo, qsmile, "TAU R-C")

bingo.close()
