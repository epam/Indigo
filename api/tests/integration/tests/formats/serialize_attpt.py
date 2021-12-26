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


def testSerializeAttachmentPoints(filename):
    print(relativePath(filename))
    mol = indigo.loadMoleculeFromFile(filename)
    for rgp in mol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            print("  rgroup {0} frag {1}".format(rgp.index(), frag.index()))
            buf = frag.serialize()
            print("  fragment serialized to {0} bytes".format(len(buf)))
            print("    " + frag.canonicalSmiles())
            frag2 = indigo.unserialize(buf)
            print("    " + frag2.canonicalSmiles())
            if frag.canonicalSmiles() != frag2.canonicalSmiles():
                print("    MISMATCH!!!")


testSerializeAttachmentPoints(joinPathPy("molecules/recursive1.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/recursive2.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/r_occur.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/r_occur_2.mol", __file__))
testSerializeAttachmentPoints(
    joinPathPy("molecules/sub_mar_q01.mol", __file__)
)
