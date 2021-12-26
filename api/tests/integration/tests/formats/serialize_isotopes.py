import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)


def testSerializeIsotopes():
    mol = indigo.loadMolecule(
        "[H][12C]1=[8C]([2H])[10C]([3H])=C([2H])[14C]([3H])=[13C]1[1H]"
    )
    mol2 = indigo.unserialize(mol.serialize())
    print(mol2.smiles())
    if indigo.exactMatch(mol, mol2) is None:
        print("NOT MATCHED!")


def testSerializeIsotopes2():
    mol = indigo.loadMolecule("C")
    for n in range(1, 300):
        mol.getAtom(0).setIsotope(n)
        try:
            mol2 = indigo.unserialize(mol.serialize())
            if indigo.exactMatch(mol, mol2) is None:
                print("NOT MATCHED! " + n)
        except IndigoException as e:
            print("caught " + getIndigoExceptionText(e))
            break


testSerializeIsotopes()
testSerializeIsotopes2()
