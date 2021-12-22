import sys

sys.path.append("../../common")
from env_indigo import *

if dir_exists(joinPathPy("out/append", __file__)):
    rmdir(joinPathPy("out/append", __file__))
makedirs(joinPathPy("out/append", __file__))

indigo = Indigo()
dbname = joinPathPy("out/append", __file__)
ids = []

smiles = ["CCCC1CCCCC1CPC", "CCC1CCNCC1CPC"]

print("Create database")
bingo = Bingo.createDatabaseFile(indigo, dbname, "molecule")
for i in range(10000):
    for sm in smiles:
        id = bingo.insert(indigo.loadMolecule(sm))
        ids.append((id, sm))
print("Optimize")
bingo.optimize()
bingo.close()

print("Append to the database")
bingo = Bingo.loadDatabaseFile(indigo, dbname)
smiles2 = ["C1CPCCC1", "C1CONCC1"]
lastid = max(id for id, sm in ids)
for i in range(10000):
    for sm in smiles2:
        lastid += 2  # Skip space on purpose
        bingo.insert(indigo.loadMolecule(sm), lastid)
        ids.append((lastid, sm))
bingo.close()

print("Validate")
bingo = Bingo.loadDatabaseFile(indigo, dbname, "read_only:true")
for id, sm in ids:
    obj = bingo.getRecordById(id)
    ref = indigo.loadMolecule(sm)
    if obj.canonicalSmiles() != ref.canonicalSmiles():
        print("Error:" + obj.smiles())

print("Do search")
for sm in smiles + smiles2:
    print(sm)
    q = indigo.loadQueryMolecule(sm)
    search = bingo.searchSub(q)
    found = []
    while search.next():
        id = search.getCurrentId()
        print(id)
        found.append(id)

    should_find = []
    for id, sm2 in ids:
        if sm2 == sm:
            should_find.append(id)

    # Compare sets
    if set(found) != set(should_find):
        print("Error!")

bingo.close()
