import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo_inchi = IndigoInchi(indigo)

print("*** Basic *** ")
m = indigo_inchi.loadMolecule("InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2")
print(m.canonicalSmiles())
print(indigo_inchi.getInchi(m))
print(indigo_inchi.getWarning())


