import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("timeout", "6000")

idx = 1
for m in indigo.iterateSDFile(joinPath("molecules/hard.sdf")):
    print("Item #%d" % (idx))
    print("  Try #1")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    print("  Try #2")
    try:
        print(m.canonicalSmiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    idx += 1
