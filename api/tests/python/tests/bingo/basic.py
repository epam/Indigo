import sys

sys.path.append('../../common')
from env_indigo import *


def searchSub(bingo, q, options=''):
    print("** searchSub({0}) **".format(q.smiles()))
    result = bingo.searchSub(q, options)
    while result.next():
        print(result.getCurrentId())

def searchExact(bingo, q, options=''):
    print("** searchExact({0}) **".format(q.smiles()))
    result = bingo.searchExact(q, options)
    while result.next():
        print(result.getCurrentId())


def searchSim(bingo, q, minSim, maxSim, metric=None):
    print("** searchSim({0}) **".format(q.smiles()))
    result = bingo.searchSim(q, minSim, maxSim, metric)
    print("{0} {1} {2}".format(result.estimateRemainingResultsCount(), result.estimateRemainingResultsCountError(), result.estimateRemainingTime()))
    rm = result.getIndigoObject()
    while result.next():
        print(result.getCurrentId())
        print(result.getCurrentSimilarityValue())
        try:
            print(rm.smiles())
        except BingoException as e:
            print("BingoException: {0}".format(getIndigoExceptionText(e)))
    result.close()

indigo = Indigo()

if dir_exists(joinPath("out", 'basic')):
    rmdir(joinPath("out", 'basic'))
makedirs(joinPath("out", 'basic'))

print("*** Creating temporary database ****")
bingo = Bingo.createDatabaseFile(indigo, joinPath('out', 'basic'), 'molecule', '')
print(bingo.version())
m = indigo.loadMolecule('C1CCCCC1')
bingo.insert(m)
m = indigo.loadMolecule('C1CCCCC1')
bingo.insert(m)
m = indigo.loadMolecule('C1CCNCC1')
bingo.insert(m)
insertedIndex = bingo.insert(m, 100)
print("Inserted index: {0}".format(insertedIndex))
bingo.optimize()
print("Index optimized")
qm = indigo.loadQueryMolecule('C')
searchSub(bingo, qm)
bingo.delete(insertedIndex)
searchSub(bingo, qm)

print("*** Deleting record with incorrect index ***")
try:
    bingo.delete(31459)
except BingoException as e:
    print("BingoException: {0}".format(getIndigoExceptionText(e)))

bingo.close()

print("*** Loading existing database ****")
bingo = Bingo.loadDatabaseFile(indigo, joinPath('out', 'basic'), '')
m = indigo.loadMolecule('C1CCCCC1')
searchSim(bingo, m, 0.9, 1, 'tanimoto')
searchSim(bingo, m, 0.9, 1, 'tversky')
searchSim(bingo, m, 0.9, 1, 'tversky 0.1 0.9')
searchSim(bingo, m, 0.9, 1, 'tversky 0.9 0.1')
searchSim(bingo, m, 0.9, 1, 'euclid-sub')


try:
    print("*** Loading non-existent database ***")
    bingo = Bingo.loadDatabaseFile(indigo, joinPath('db', 'idonotexist'), 'molecule')
except BingoException as e:
    print("BingoException: {0}".format(getIndigoExceptionText(e)))

print("*** Using closed database ***")
bingo.close()
try:
    m = indigo.loadMolecule('C')
    bingo.insert(m)
except BingoException as e:
    print("BingoException: {0}".format(getIndigoExceptionText(e)))
except SystemError as e:
    print("SystemError: {0}".format(e))


indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ord-qwords", 25)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 15)
indigo.setOption("fp-ext-enabled", True)


print("*** Simple exact search ****")
bingo = Bingo.createDatabaseFile(indigo, joinPath('out', 'basic'), 'molecule')
mol1 = indigo.loadMolecule("ICCCCOC(=O)C1=CC([N+]([O-])=O)=C([N+]([O-])=O)C=C1")
mol2 = indigo.loadMolecule("CCCC")
bingo.insert(mol1)
bingo.insert(mol2)
searchExact(bingo, mol1)
searchExact(bingo, mol2)
bingo.close()

print("*** Simple enumerate id ****")
bingo = Bingo.createDatabaseFile(indigo, joinPath('out', 'basic'), 'molecule')
mol = indigo.loadMolecule("CCCC")
bingo.insert(mol)
bingo.insert(mol)
bingo.insert(mol)
count=0

result = bingo.enumerateId()

while result.next():
    count+=1

print("number of molecules: {0}".format(count))
bingo.close()