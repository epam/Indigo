import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

def testSmilesRGroups (filename):
    print('\n' + relativePath(filename) + '\n')
    mol = indigo.loadMoleculeFromFile(filename)
    try:
        sys.stdout.write("  SMILES    :  ")
        smi = mol.smiles()
        print(smi)
        sys.stdout.write("  CanSMILES :  ")
        cansmi = mol.canonicalSmiles()
        print(cansmi) 
        sys.stdout.write("  Match     :  ")
        cansmi2 = indigo.loadMolecule(smi).canonicalSmiles()
        print(cansmi2 == cansmi)
        sys.stdout.write("  Match (2) :  ")
        cansmi3 = indigo.loadMolecule(cansmi).canonicalSmiles()
        print(cansmi3 == cansmi)
    except IndigoException as e:
        print('caught ' + getIndigoExceptionText(e))
  
    sys.stdout.write('\n')
    for rgroup in mol.iterateRGroups():
        print('  Rgroup #' + str(rgroup.index()))
        for frag in rgroup.iterateRGroupFragments():
            try:
                sys.stdout.write("    SMILES    :  ")
                smi = frag.smiles()
                print(smi)
                sys.stdout.write("    CanSMILES :  ")
                cansmi = frag.canonicalSmiles()
                print(cansmi)
                sys.stdout.write("    Match     :  ")
                cansmi2 = indigo.loadMolecule(smi).canonicalSmiles()
                print(cansmi2 == cansmi)
                sys.stdout.write("    Match (2) :  ")
                cansmi3 = indigo.loadMolecule(cansmi).canonicalSmiles()
                print(cansmi3 == cansmi)
            except IndigoException as e:
                print('caught ' + getIndigoExceptionText(e))
      
testSmilesRGroups(joinPath('molecules/r_occur.mol'))
testSmilesRGroups(joinPath('molecules/rgroup_a0.mol'))
testSmilesRGroups(joinPath('molecules/rgroup_a1.mol'))
testSmilesRGroups(joinPath('molecules/rgroup_all.mol'))
testSmilesRGroups(joinPath('molecules/recursive1.mol'))
testSmilesRGroups(joinPath('molecules/recursive2.mol'))
