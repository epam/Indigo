import os;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testMultipleSave (smifile, iterfunc, issmi):
  print "TESTING", smifile
  sdfout = indigo.writeFile("structures.sdf")
  cmlout = indigo.writeFile("structures.cml")
  rdfout = indigo.writeFile("structures.rdf")
  smiout = indigo.writeFile("structures.smi")
  rdfout.rdfHeader()
  cmlout.cmlHeader()
  for item in iterfunc(smifile):
    exc = False
    try:
      item.countAtoms()
    except IndigoException, e:
      print item.index(), ':', getIndigoExceptionText(e)
      if issmi:
        print item.rawData()
      exc = True
    if not exc:
      #item.clearCisTrans()
      for bond in item.iterateBonds():
        if bond.topology() == Indigo.RING and bond.bondOrder() == 2:
          bond.resetStereo()
      try:
        item.markEitherCisTrans()
      except IndigoException, e:
        print item.index(), '(while markEitherCisTrans) :', getIndigoExceptionText(e)
        if issmi:
          print item.rawData()
        continue
      
      if issmi:
        item.setName("structure-" + str(item.index()) + " " + item.rawData())
      else:
        item.setName("structure-" + str(item.index()))
      item.setProperty("NUMBER", str(item.index()))
      cmlout.cmlAppend(item)
      smiout.smilesAppend(item)
      item.layout()
      indigo.setOption("molfile-saving-mode", "2000")
      sdfout.sdfAppend(item)
      indigo.setOption("molfile-saving-mode", "3000")
      rdfout.rdfAppend(item)
  cmlout.cmlFooter()
  
  sdfout.close()
  cmlout.close()
  rdfout.close()
  smiout.close()
  
  cmliter = indigo.iterateCMLFile("structures.cml")
  sdfiter = indigo.iterateSDFile("structures.sdf")
  rdfiter = indigo.iterateRDFile("structures.rdf")
  smiiter = indigo.iterateSmilesFile("structures.smi")
  
  while sdfiter.hasNext():
    cml = cmliter.next()
    sdf = sdfiter.next()
    rdf = rdfiter.next()
    smi = smiiter.next()
 
    print sdf.index(), sdf.name()
    sdf.resetSymmetricCisTrans()
    rdf.resetSymmetricCisTrans()
    try:
      cs1 = sdf.canonicalSmiles()
      cs2 = rdf.canonicalSmiles()
      cs3 = smi.canonicalSmiles()
      cs4 = cml.canonicalSmiles()
    except IndigoException, e:
      print getIndigoExceptionText(e)
      continue
    print cs1
    print cs2
    print cs3
    print cs4
    if (cs2 != cs1):
      print "MISMATCH"
    if (cs3 != cs1):
      print "MISMATCH"
    if (cs4 != cs1):
      print "MISMATCH"
testMultipleSave("molecules/helma.smi", indigo.iterateSmilesFile, True)
testMultipleSave("molecules/chemical-structures.smi", indigo.iterateSmilesFile, True)
testMultipleSave("molecules/pubchem_7m_err.sdf", indigo.iterateSDFile, False)
testMultipleSave("molecules/acd2d_err.sdf", indigo.iterateSDFile, False)
