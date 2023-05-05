import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("ignore-stereochemistry-errors", "1")

m = indigo.loadMolecule("Nc1[nH]cnc-2ncnc1-2")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("c1ccc2ccc3ccccc3-c2c1")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("c1ccccc-1")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("c1ccc-2c(c1)-c1ccccc-21")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("F[Po@SP2](Br)(Cl)I")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("O[C@TH1H]([C@TH2H]1NCCCC1)C1=CC(O)=C(O)C=C1")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("OC(Cl)=[C@AL1]=C(C)F")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("s[As@TB2](F)(Cl)(Br)C=O")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("s[As@TB20](F)(Cl)(Br)C=O")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("S[Co@OH2](F)(Cl)(Br)(I)C=O")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule("S[Co@OH30](F)(Cl)(Br)(I)C=O")
print(m.smiles())
print(m.molfile())

m = indigo.loadMolecule(
    "C1c2cc3cc4ccc5cc6Cc7cc8ccc1c1c9c%10c%11c(c6c7c%10c81)c5c4c%11c3c29"
)
print(m.smiles())
print(m.molfile())


m = indigo.loadMolecule(
    "C12=C3C4=C(C=C5CC6=CC7=CC=C8C=C9C%10=C%11C8=C7C7=C6C5=C4C(=C7%11)C3=C%10C(=C9)C1)C=C2"
)
m.aromatize()
print(m.canonicalSmiles())

m = indigo.loadMolecule(
    "C12=C3C4=C(CC5=CC6=CC7=CC=C8C9=C7C7=C6C5=C4C4=C7C9=C5C(=C8)CC(=C1)C5=C43)C=C2"
)
m.aromatize()
print(m.canonicalSmiles())
