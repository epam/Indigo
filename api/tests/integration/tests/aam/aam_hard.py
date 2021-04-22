import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("timeout", "2000")

s = "c1(P(N=[N+]=[N-])(c2ccccc2)=O)ccccc1.C(OC(=O)N([C@@H](C)[C@H](c1cc(C(F)(F)F)cc(C(F)(F)F)c1)O)Cc1cc(C(F)(F)F)ccc1-c1cc(C(C)C)ccc1OC)(C)(C)C.c1(P(c2ccccc2)c2ccccc2)ccccc1.O=C(OCC)/N=N/C(OCC)=O>C1OCCC1.CCOC(C)=O>C(OC(=O)N([C@@H](C)[C@H](N=[N+]=[N-])c1cc(C(F)(F)F)cc(C(F)(F)F)c1)Cc1cc(C(F)(F)F)ccc1-c1cc(C(C)C)ccc1OC)(C)(C)C"

r = indigo.loadReaction(s)
try:
   r.automap()
   print(r.smiles())
except IndigoException as e:
   print("timed out")

# should replace standard timeout by aam-timeout
indigo.setOption("aam-timeout", "4000")
r = indigo.loadReaction(s)
r.automap()
print("passed")

# should set handler back
indigo.setOption("aam-timeout", "0")
r = indigo.loadReaction(s)
try:
   r.automap()
   print(r.smiles())
except IndigoException as e:
   print("timed out")

# should not raise an exception on molecules with small components
r = indigo.loadReaction("CSC(=O)C.CC[O-].[K+].Cl.C(N(CC)CC)C.[Cl-].[NH4+].Cl.Cl.Cl>O1CCCC1.CCOC(=O)C>C(OC(N1CCC[C@H]1C(SC)C(C(OCC)=O)C)=O)(C)(C)C")
r.automap()
print("passed")
