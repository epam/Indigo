import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
mol = indigo.loadMolecule("[CH][C@H]1C(C)C(N)C(C)C(*)[C@H]1C |o2:10,w:2.2,6.6,wU:1.0,10.11,$;;;;;;;;;_AP1;;$,^4:0,(-.88,-4.34,;-.88,-2.8,;-2.21,-2.04,;-3.55,-2.8,;-2.21,-.5,;-3.38,2.86,;-.88,.28,;-.88,1.82,;.45,-.5,;1.79,.28,;.45,-2.04,;1.79,-2.8,)|")
print(mol.molfile())
indigo.setOption("molfile-saving-mode", "2000")
print(mol.molfile())
