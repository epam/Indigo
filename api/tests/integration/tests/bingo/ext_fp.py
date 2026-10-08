import array
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa


def searchSimExt(bingo, q, minSim, maxSim, ext_fp, metric=None):
    print("** searchSimExt({0}) metric='{1}' **".format(q.smiles(), metric))
    result = bingo.searchSimWithExtFP(q, minSim, maxSim, ext_fp, metric)
    print(
        "{0} {1} {2}".format(
            result.estimateRemainingResultsCount(),
            result.estimateRemainingResultsCountError(),
            result.estimateRemainingTime(),
        )
    )
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

bingo = Bingo.createDatabaseFile(
    indigo, joinPathPy("out/extfp", __file__), "molecule", ""
)
print(bingo.version())
m = indigo.loadMolecule("C1CCCCC1")
bingo.insert(m)
m = indigo.loadMolecule("C1CCNCC1")
bingo.insert(m)

m = indigo.loadMolecule("C1CCCCC1")

# External-fingerprint bookkeeping (see GitHub issue #408 for the FAQ this
# answers): "external" here means a fingerprint computed outside of Indigo
# (e.g. by RDKit -- MACCS keys, Morgan/ECFP, Avalon, ...) and handed to Bingo
# as a raw byte buffer via loadFingerprintFromBuffer, instead of one Bingo
# computes itself via m.fingerprint(...).
#
# fp-sim-qwords sets how many 8-byte "qwords" Bingo allocates to the
# similarity-fingerprint component of each record; it is the ONLY one of the
# four fp-*-qwords options that matters for similarity search. The buffer
# size (in bytes) passed to loadFingerprintFromBuffer must equal exactly
# fp-sim-qwords * 8 -- Bingo compares that byte count against the database's
# configured fingerprint size and raises
#   BingoException: external fingerprint is incompatible with current database
# on any mismatch (see BaseSimilarityMatcher::setQueryDataWithExtFP in
# bingo_matcher.cpp). It is unrelated to the length of whatever fingerprint
# algorithm you used upstream (e.g. RDKit MACCS keys are 167 bits -- that
# does not need to divide evenly into fp-sim-qwords*8; just zero-pad).
#
# fp-ord-qwords/fp-any-qwords/fp-tau-qwords configure Bingo's own
# substructure/exact-match fingerprint components, which we are not
# supplying externally here, so they're zeroed out.
#
# fp-ext-enabled is an unrelated, confusingly-named option: it toggles
# whether Indigo computes its own *extended* fingerprint component
# internally. It has nothing to do with "external" fingerprints in the
# searchSimWithExtFP/insertWithExtFP sense used throughout this file.
#
# These options must be set identically on every Indigo instance that
# creates, inserts into, or searches this database, since the sizes are
# fixed into the database's on-disk layout the first time it's created.
indigo.setOption("fp-sim-qwords", 8)
indigo.setOption("fp-ord-qwords", 0)
indigo.setOption("fp-tau-qwords", 0)
indigo.setOption("fp-any-qwords", 0)
indigo.setOption("fp-ext-enabled", False)

# For comparison: this is what a *native* Bingo fingerprint looks like --
# computed by Bingo itself rather than supplied externally. It happens to
# also satisfy the fp-sim-qwords=8 (64-byte) size we just configured, so it
# can be fed into the same ext-fp APIs below for a same-size sanity check.
ext_fp = m.fingerprint("sim")
print(ext_fp.toString())

# A real external fingerprint instead would come from an outside library,
# e.g.:
#   from rdkit import Chem
#   from rdkit.Chem import MACCSkeys
#   bits = MACCSkeys.GenMACCSKeys(Chem.MolFromSmiles("C1CCCCC1")).ToBitString()
#   padded = bits.ljust(64 * 8, "0")  # pad 167-bit MACCS up to the 64-byte
#                                      # (fp-sim-qwords=8) buffer configured above
#   buffer = bytes(int(padded[i : i + 8], 2) for i in range(0, len(padded), 8))
#   ext_fp_maccs = indigo.loadFingerprintFromBuffer(buffer)
# This test uses a fixed dummy pattern instead so it doesn't depend on RDKit.
buffer = bytearray([0xFF, 0x00] * 32)

if isIronPython():
    from System import Array, Byte

    buf_arr = Array[Byte](buffer)
else:
    buf_arr = bytes(buffer)

ext_fp1 = indigo.loadFingerprintFromBuffer(buf_arr)
print(ext_fp1.toString())

m1 = indigo.loadMolecule("C1CNNCC1")
bingo.insertWithExtFP(m1, ext_fp1)

ext_fp2 = m1.fingerprint("sim")
print(ext_fp2.toString())
bingo.insertWithExtFP(m1, ext_fp2)

searchSimExt(bingo, m, 0.9, 1, ext_fp, "tanimoto")
searchSimExt(bingo, m, 0.9, 1, ext_fp, "tversky")
searchSimExt(bingo, m, 0.9, 1, ext_fp, "tversky 0.1 0.9")
searchSimExt(bingo, m, 0.9, 1, ext_fp, "tversky 0.9 0.1")
searchSimExt(bingo, m, 0.9, 1, ext_fp, "euclid-sub")

searchSimExt(bingo, m1, 0.9, 1, ext_fp1, "tanimoto")
searchSimExt(bingo, m1, 0.9, 1, ext_fp2, "tanimoto")


print("*** Add external fingerprint with id ****")

m = indigo.loadMolecule("C1CNNCC1")
bingo.insertWithExtFP(m, ext_fp1, 100)

searchSimExt(bingo, m, 0.9, 1, ext_fp1, "tanimoto")
