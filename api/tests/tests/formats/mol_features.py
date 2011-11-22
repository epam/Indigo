from __future__ import print_function
from __future__ import print_function
import sys
sys.path.append('../../common')
from env_indigo import *

def runIndigoTest():
    indigo = None
    def runIndigoTest():
        indigo = None
        indigo = Indigo()
        indigo.setOption("molfile-saving-skip-date", True)
        if not os.path.exists(joinPath("out")):
           os.makedirs(joinPath("out"))
        saver = indigo.createFileSaver(joinPath("out/mol_features.sdf"), "sdf")
        mol = indigo.loadMoleculeFromFile(joinPath('molecules/sgroups_2.mol'))
        print(mol.molfile())
        saver.append(mol)
        mol = indigo.loadMoleculeFromFile(joinPath('molecules/all_features_mol.mol'))
        print(mol.molfile())
        saver.append(mol)
        return indigo
    return indigo