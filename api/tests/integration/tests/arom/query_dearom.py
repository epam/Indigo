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
print(f"Aromatic: {m.smarts()}")
m.dearomatize()
print(f"Dearomatized: {m.smarts()}\n")

m = indigo.loadSmarts("c1[ch0]cccc1")
print(f"Aromatic: {m.smarts()}")
m.dearomatize()
print(f"Dearomatized: {m.smarts()}\n")

m = indigo.loadSmarts("c1[cX4]cccc1")
print(f"Aromatic: {m.smarts()}")
m.dearomatize()
print(f"Dearomatized: {m.smarts()}\n")
