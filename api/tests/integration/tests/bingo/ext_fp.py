import sys
import array

sys.path.append('../../common')
from env_indigo import *

def searchSimExt(bingo, q, minSim, maxSim, ext_fp, metric=None):
    print("** searchSimExt({0}) metric='{1}' **".format(q.smiles(), metric))
    result = bingo.searchSimWithExtFP(q, minSim, maxSim,ext_fp,  metric)
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


print("*** Add external fingerprints ****")

indigo = Indigo()

bingo = Bingo.createDatabaseFile(indigo, joinPath('extfp'), 'molecule', '')
print(bingo.version())
m = indigo.loadMolecule('C1CCCCC1')
bingo.insert(m)
m = indigo.loadMolecule('C1CCNCC1')
bingo.insert(m)

m = indigo.loadMolecule('C1CCCCC1')

indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
indigo.setOption("fp-ext-enabled", False)

ext_fp = m.fingerprint("sim")
print(ext_fp.toString())

buffer = bytearray([0xFF, 0x00] * 32)

if isIronPython():
    from System import Array, Byte
    buf_arr = Array[Byte](buffer)
else:
    buf_arr = bytes(buffer)

ext_fp1 = indigo.loadFingerprintFromBuffer(buf_arr)
print(ext_fp1.toString())

m1 = indigo.loadMolecule('C1CNNCC1')
bingo.insertWithExtFP(m1, ext_fp1)

ext_fp2 = m1.fingerprint("sim")
print(ext_fp2.toString())
bingo.insertWithExtFP(m1, ext_fp2)

searchSimExt(bingo, m, 0.9, 1, ext_fp, 'tanimoto')
searchSimExt(bingo, m, 0.9, 1, ext_fp, 'tversky')
searchSimExt(bingo, m, 0.9, 1, ext_fp, 'tversky 0.1 0.9')
searchSimExt(bingo, m, 0.9, 1, ext_fp, 'tversky 0.9 0.1')
searchSimExt(bingo, m, 0.9, 1, ext_fp, 'euclid-sub')

searchSimExt(bingo, m1, 0.9, 1, ext_fp1, 'tanimoto')
searchSimExt(bingo, m1, 0.9, 1, ext_fp2, 'tanimoto')


print("*** Add external fingerprint with id ****")

m = indigo.loadMolecule('C1CNNCC1')
bingo.insertWithExtFP(m, ext_fp1, 100)

searchSimExt(bingo, m, 0.9, 1, ext_fp1, 'tanimoto')

