import os;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)
def testSerializeAttachmentPoints (filename):
  print filename
  mol = indigo.loadMoleculeFromFile(filename)
  for rgp in mol.iterateRGroups():
    for frag in rgp.iterateRGroupFragments():
      print "  rgroup", rgp.index(), "frag", frag.index()
      buf = frag.serialize()
      print '  fragment serialized to', len(buf), 'bytes'
      print "    " + frag.canonicalSmiles()
      frag2 = indigo.unserialize(buf)
      print "    " + frag2.canonicalSmiles()
      if frag.canonicalSmiles() != frag2.canonicalSmiles():
        print "    MISMATCH!!!"
testSerializeAttachmentPoints("molecules/recursive1.mol")
testSerializeAttachmentPoints("molecules/recursive2.mol")
testSerializeAttachmentPoints("molecules/r_occur.mol")
testSerializeAttachmentPoints("molecules/r_occur_2.mol")
testSerializeAttachmentPoints("molecules/sub_mar_q01.mol")
