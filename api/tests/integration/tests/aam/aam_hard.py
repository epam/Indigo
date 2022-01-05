import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("timeout", 1000)

s = "c1(P(N=[N+]=[N-])(c2ccccc2)=O)ccccc1.C(OC(=O)N([C@@H](C)[C@H](c1cc(C(F)(F)F)cc(C(F)(F)F)c1)O)Cc1cc(C(F)(F)F)ccc1-c1cc(C(C)C)ccc1OC)(C)(C)C.c1(P(c2ccccc2)c2ccccc2)ccccc1.O=C(OCC)/N=N/C(OCC)=O>C1OCCC1.CCOC(C)=O>C(OC(=O)N([C@@H](C)[C@H](N=[N+]=[N-])c1cc(C(F)(F)F)cc(C(F)(F)F)c1)Cc1cc(C(F)(F)F)ccc1-c1cc(C(C)C)ccc1OC)(C)(C)C"

r = indigo.loadReaction(s)
try:
    r.automap()
    print("ignore std timeout")
except IndigoException as e:
    print("timed out")

# should not raise an exception on molecules with small components
r = indigo.loadReaction(
    "CSC(=O)C.CC[O-].[K+].Cl.C(N(CC)CC)C.[Cl-].[NH4+].Cl.Cl.Cl>O1CCCC1.CCOC(=O)C>C(OC(N1CCC[C@H]1C(SC)C(C(OCC)=O)C)=O)(C)(C)C"
)
r.automap()
print("passed small")

rxn_smiles = "[F:1][C:2]([F:3])([C:4]([F:5])([F:6])[CH2:7][CH2:8][Si:9](Br)([CH2:10][CH2:11][C:12]([F:13])([F:14])[C:15]([F:16])([F:17])[C:18]([F:19])([F:20])[C:21]([F:22])([F:23])[C:24]([F:25])([F:26])[C:27]([F:28])([F:29])[F:30])[CH2:31][CH2:32][C:33]([F:34])([F:35])[C:36]([F:37])([F:38])[C:39]([F:40])([F:41])[C:42]([F:43])([F:44])[C:45]([F:46])([F:47])[C:48]([F:49])([F:50])[F:51])[C:52]([F:53])([F:54])[C:55]([F:56])([F:57])[C:58]([F:59])([F:60])[C:61]([F:62])([F:63])[F:64].[CH2:65]=[CH:66][CH2:67][OH:68]>CCN(CC)CC.C1CCOC1>[CH2:65]=[CH:66][CH2:67][O:68][Si:9]([CH2:10][CH2:11][C:12]([F:14])([F:13])[C:15]([F:17])([F:16])[C:18]([F:20])([F:19])[C:21]([F:23])([F:22])[C:24]([F:25])([F:26])[C:27]([F:30])([F:29])[F:28])([CH2:8][CH2:7][C:4]([F:5])([F:6])[C:2]([F:1])([F:3])[C:52]([F:53])([F:54])[C:55]([F:56])([F:57])[C:58]([F:59])([F:60])[C:61]([F:63])([F:62])[F:64])[CH2:31][CH2:32][C:33]([F:34])([F:35])[C:36]([F:37])([F:38])[C:39]([F:40])([F:41])[C:42]([F:43])([F:44])[C:45]([F:46])([F:47])[C:48]([F:50])([F:51])[F:49]"

indigo.resetOptions()
mapped_reaction = indigo.loadReaction(rxn_smiles)
indigo.setOption("aam-timeout", 1000)
# indigo.setOption("timeout", 1000)
try:
    mapped_reaction.automap(
        "discard ignore_charges ignore_radicals ignore_isotopes ignore_valence"
    )
except IndigoException as e:
    print("timed out")
# print(mapped_reaction.smiles())
# print("passed")
