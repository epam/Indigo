import itertools
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Bingo,
    BingoException,
    Indigo,
    dataPath,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
db_name = joinPathPy("out/db_molecule", __file__)
bingo = Bingo.createDatabaseFile(indigo, db_name, "molecule")

index = 0
mols = {}
with_coords = 0
for mol in indigo.iterateSDFile(
    dataPath("molecules/basic/rand_queries_small.sdf")
):
    try:
        bingo.insert(mol, index)
        mols[index] = mol
        if mol.hasCoord():
            with_coords += 1
    except BingoException as e:
        print(
            "Structure {0} excluded: {1}".format(
                index, getIndigoExceptionText(e)
            )
        )
    index += 1

print(
    "Finished indexing {0} structures. {1} has coords".format(
        index, with_coords
    )
)
bingo.close()

bingo = Bingo.loadDatabaseFile(indigo, db_name)
enumerate = bingo.enumerateId()
count = 0
while enumerate.next():
    id = enumerate.getCurrentId()
    mol = bingo.getRecordById(id)
    if mol.hasCoord():
        count += 1
    if mols[id].hasCoord() != mol.hasCoord():
        print(
            "molecule with",
            id,
            "different",
            mols[id].hasCoord(),
            mol.hasCoord(),
        )

print("Loaded", count, "molecules with coordinates from bingo db")
