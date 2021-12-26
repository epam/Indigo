import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

print("*** Molecule ***")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/pubchem-1.mol", __file__)
)
print(m.smiles())


def printProperies(m):
    print("Properties:")
    for prop in m.iterateProperties():
        print("%s: %s" % (prop.name(), prop.rawData()))

    print("Properties via get:")
    for prop in m.iterateProperties():
        print("%s: %s" % (prop.name(), m.getProperty(prop.name())))


printProperies(m)

print("******** Add properties **********")
m.setProperty("PUBCHEM_IUPAC_NAME", "noname")
m.setProperty("p1", "any text")
m.setProperty(
    "p2", "any text line 1\nany text line 2\nany text line 3\nany text line 4"
)

printProperies(m)

print("******** Remove properties **********")
m.removeProperty("PUBCHEM_IUPAC_NAME")
m.removeProperty("p1")

printProperies(m)

print("******** Clear properties **********")
m.clearProperties()

printProperies(m)

print("******** Add properties #2 **********")
m.setProperty("PUBCHEM_IUPAC_NAME", "noname")
m.setProperty("p1", "any text")
m.setProperty(
    "p2", "any text line 1\nany text line 2\nany text line 3\nany text line 4"
)

printProperies(m)

print("******** Parse structures with empty SDF properties **********")
for m in indigo.iterateSDFile(
    joinPathPy("molecules/properties-empty.sdf", __file__)
):
    print("*** Structure %s ***" % m.name())
    printProperies(m)
