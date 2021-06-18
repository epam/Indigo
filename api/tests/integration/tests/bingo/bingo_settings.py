import sys
import array

sys.path.append('../../common')
from env_indigo import *

print("*** Test if Indigo and Bingo use the same settings for fingerprints ***")
indigo = Indigo()

indigo.setOption("fp-sim-qwords", 1)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
indigo.setOption("fp-ext-enabled", False)

print("indigo fp_params: sim %d, ord %d, tau %d, any %d, ext %s" % (
    indigo.getOptionInt("fp-sim-qwords"),
    indigo.getOptionInt("fp-ord-qwords"),
    indigo.getOptionInt("fp-tau-qwords"),
    indigo.getOptionInt("fp-any-qwords"),
    indigo.getOptionBool("fp-ext-enabled")))

bingo = Bingo.createDatabaseFile(indigo, joinPath('tempdb'), 'molecule', '')

print("*** Insert molecules ***")

buffer = [0xFF, 0x00] * 4

buf_arr = array.array('b')
buf_arr.frombytes(bytearray(buffer))

if isIronPython():
    from System import Array, Byte
    buf_arr = Array[Byte](buffer)

ext_fp1 = indigo.loadFingerprintFromBuffer(buf_arr)

m1 = indigo.loadMolecule('C1CNNCC1')
bingo.insertWithExtFP(m1, ext_fp1)

m2 = indigo.loadMolecule('C1CNNCC1')
bingo.insert(m2)

q = indigo.loadMolecule('C1CNNC(CCCCC)C1')

print("*** Compare similarities ***")

result = bingo.searchSim(q, 0, 1, 'tanimoto')
while result.next():
    id = result.getCurrentId()
    rm = bingo.getRecordById(id)
    bingo_sim = result.getCurrentSimilarityValue()
    print("%2d: bingo sim %f; smiles %s" % (id, bingo_sim, rm.smiles()))
result.close()

bingo.close()
