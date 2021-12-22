import os
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()


def sssrInfo(mol):
    print("SSSR count = {0}".format(mol.countSSSR()))
    for submol in mol.iterateSSSR():
        sys.stdout.write("  ")
        for atom in submol.iterateAtoms():
            sys.stdout.write(str(atom.index()) + " ")
        sys.stdout.write("\n")


def testSSSR():
    indigo.setOption("skip-3d-chirality", True)
    chebi_dir = joinPathPy("../../../../../data/molecules/chebi", __file__)
    files = os.listdir(chebi_dir)
    files.sort()
    for filename in files:
        print(filename)
        mol = indigo.loadMoleculeFromFile(os.path.join(chebi_dir, filename))
        sssrInfo(mol)
        newmol = indigo.loadMolecule(mol.smiles())
        sssrInfo(newmol)


testSSSR()
