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
indigo.setOption("serialize-preserve-ordering", True)


def testSerializeAttachmentPoints(filename):
    print(relativePath(filename))
    mol = indigo.loadMoleculeFromFile(filename)
    for rgp in mol.iterateRGroups():
        for frag in rgp.iterateRGroupFragments():
            print("  rgroup {0} frag {1}".format(rgp.index(), frag.index()))
            buf = frag.serialize()
            smiles_orig = frag.smiles()
            print("  fragment serialized to {0} bytes".format(len(buf)))
            print("    " + smiles_orig)
            frag2 = indigo.unserialize(buf)
            smiles_unserialize = frag2.smiles()
            buf2 = frag2.serialize()
            print("    " + smiles_unserialize)
            if buf != buf2:
                print("    SERIALIZED NOT EQUALS!!!")
            if smiles_orig != smiles_unserialize:
                print("    MISMATCH!!!")


testSerializeAttachmentPoints(joinPathPy("molecules/recursive1.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/recursive2.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/r_occur.mol", __file__))
testSerializeAttachmentPoints(joinPathPy("molecules/r_occur_2.mol", __file__))
testSerializeAttachmentPoints(
    joinPathPy("molecules/sub_mar_q01.mol", __file__)
)
