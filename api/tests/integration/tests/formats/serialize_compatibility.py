# This test is to check compatibility with previous serilaization format CMF, CM2

import sys
import binascii
import array
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()


# Serializion

def serializeToFileAndPrint(obj, is_rxn):
    name = "out/serialize"
    if is_rxn:
        suffix = "_r"
    else:
        suffix = "_m"

    f = open(joinPathPy("out/serialize" + suffix + ".cmf", __file__), "w")
    f.write(indigo.version() + "\n")
    buf = obj.serialize()

    newbuf = bytearray(buf)
    
    hex_buf = binascii.hexlify(newbuf)
    obj2 = indigo.deserialize(buf)

    if is_rxn:
        obj2.saveRxnfile(joinPathPy("out/serialize" + suffix + "2.rxn", __file__))
    else:
        obj2.saveMolfile(joinPathPy("out/serialize" + suffix + "2.mol", __file__))

    print(hex_buf.decode('ascii') if sys.version_info > (3, 0) else hex_buf)
    f.write(hex_buf.decode('ascii'))
    f.close()

    print(obj.smiles())


print("*** Molecule ***")
m = indigo.loadMoleculeFromFile(joinPathPy("serialize/m.mol", __file__))
serializeToFileAndPrint(m, False)

print("*** Reaction ***")
r = indigo.loadReactionFromFile(joinPathPy("serialize/r.rxn", __file__))
serializeToFileAndPrint(r, True)


# Unserializion
# Read all molecules from the serialized file
def testUnserialize(fname):
    f = open(fname, "r")
    while True:
        version = f.readline().strip()
        print("** " + version)
        if len(version) == 0:
            break
        buf_hex = f.readline().strip()
        print(buf_hex)
        buf = bytes(binascii.unhexlify(buf_hex))
        if isIronPython():
            from System import Array, Byte
            buf = Array[Byte]([Byte( ord(symbol) ) for symbol in buf])
        try:
            obj = indigo.unserialize(buf)
            print(obj.smiles())
        except IndigoException as ex:
            print(getIndigoExceptionText(ex))


print("*** Molecule unserialize ***")
testUnserialize(joinPathPy("serialize/m.cmf", __file__))

print("*** Reaction unserialize ***")
testUnserialize(joinPathPy("serialize/r.cmf", __file__))
