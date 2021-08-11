import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("timeout", "6000")

def remove_prefix(text, prefix):
    return text[len(prefix):] if text.startswith(prefix) else text

idx = 1
for m in indigo.iterateSDFile(joinPath("molecules/hard.sdf")):
    print("Item #%d" % (idx))
    print("  Try #1")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print( remove_prefix( getIndigoExceptionText(e), "Molecule ") )
    print("  Try #2")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print( remove_prefix( getIndigoExceptionText(e), "Molecule " ) )
    idx += 1
