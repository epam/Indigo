import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

idx = 1
for idx, m in enumerate(indigo.iterateSDFile(joinPath("molecules/symmetry.sdf"))):
    print("Item #%d" % (idx))
    classes = m.symmetryClasses()
    for a in m.iterateAtoms():
        print("  %d: %d" % (a.index(), classes[a.index()]))
