import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "true")
indigo.setOption("molfile-saving-mode", "3000")

m = indigo.loadMoleculeFromFile(joinPath("molecules/ferrocene-variant1.mol"))
print(m.canonicalSmiles())
print(m.molfile());

m = indigo.loadMoleculeFromFile(joinPath("molecules/ferrocene-variant5.mol"))
print(m.canonicalSmiles())
print(m.molfile());


m = indigo.loadMoleculeFromFile(joinPath("molecules/ferrocene-variant6.mol"))
print(m.canonicalSmiles())
print(m.molfile());


m = indigo.loadMoleculeFromFile(joinPath("molecules/h_bond.mol"))
print(m.canonicalSmiles())
print(m.molfile());