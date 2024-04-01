import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

print("**** Dearomatize query molecules")

indigo = Indigo()

m = indigo.loadSmarts("c1ccccc1")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts("c1[ch0]cccc1")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts("c1[cX4]cccc1")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts("[#6]12:[#7;h1]:[#7]:[#6]:[#6]:1:[#6]:[#6]:[#6]:[#6]:2")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts("[#6]12:[#7;h0]:[#7]:[#6]:[#6]:1:[#6]:[#6]:[#6]:[#6]:2")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts("[#6]12:[#7]:[#7;h0]:[#6]:[#6]:1:[#6]:[#6]:[#6]:[#6]:2")
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Dearomatized: %s\n" % m.smarts())

m = indigo.loadSmarts(
    "[#6]12:[#7;h0]:[#7;h0]:[#6]:[#6]:1:[#6]:[#6]:[#6]:[#6]:2"
)
print("Aromatic: %s" % m.smarts())
m.dearomatize()
print("Cannot be dearomatized: %s\n" % m.smarts())
