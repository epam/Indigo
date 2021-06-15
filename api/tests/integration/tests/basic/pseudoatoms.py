import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

m = indigo.loadMoleculeFromFile(joinPath("molecules", "aniline_pol_psd.mol"))
print('aniline_pol_psd.mol')
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())
m = indigo.loadMoleculeFromFile(joinPath("molecules", "aniline_pol_psd_alias.mol"))
print('aniline_pol_psd_alias.mol')
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())