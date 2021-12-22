import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/aniline_pol_psd.mol", __file__)
)
print("aniline_pol_psd.mol")
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/aniline_pol_psd_alias.mol", __file__)
)
print("aniline_pol_psd_alias.mol")
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())
