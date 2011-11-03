import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testSmilesRGroups (filename):
  print
  print filename
  print
  mol = indigo.loadMoleculeFromFile(filename)
  try:
    print "  SMILES    : ",
    smi = mol.smiles()
    print smi
    print "  CanSMILES : ",
    cansmi = mol.canonicalSmiles()
    print cansmi 
    print "  Match     : ",
    cansmi2 = indigo.loadMolecule(smi).canonicalSmiles()
    print cansmi2 == cansmi
    print "  Match (2) : ",
    cansmi3 = indigo.loadMolecule(cansmi).canonicalSmiles()
    print cansmi3 == cansmi
  except IndigoException, e:
    print 'caught ' + getIndigoExceptionText(e)
  
  print
  for rgroup in mol.iterateRGroups():
    print '  Rgroup #' + str(rgroup.index())
    for frag in rgroup.iterateRGroupFragments():
      try:
        print "    SMILES    : ",
        smi = frag.smiles()
        print smi
        print "    CanSMILES : ",
        cansmi = frag.canonicalSmiles()
        print cansmi
        print "    Match     : ",
        cansmi2 = indigo.loadMolecule(smi).canonicalSmiles()
        print cansmi2 == cansmi
        print "    Match (2) : ",
        cansmi3 = indigo.loadMolecule(cansmi).canonicalSmiles()
        print cansmi3 == cansmi
      except IndigoException, e:
        print 'caught ' + getIndigoExceptionText(e)
      
testSmilesRGroups('molecules/r_occur.mol')
testSmilesRGroups('molecules/rgroup_a0.mol')
testSmilesRGroups('molecules/rgroup_a1.mol')
testSmilesRGroups('molecules/rgroup_all.mol')
testSmilesRGroups('molecules/recursive1.mol')
testSmilesRGroups('molecules/recursive2.mol')
