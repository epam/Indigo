import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *


indigo = Indigo()


def run(file, options):
    print("### (file: %s; options: \"%s\") ###" % (file, options))
    mol = indigo.loadMoleculeFromFile(joinPath(file))
    all = indigo.rgroupComposition(mol, "")
    mi = 1
    for m in all:
        print("===================================================")
        ri = 1
        fm = indigo.getFragmentedMolecule(m, options)
        print("molecule %02d: %s" % (mi, fm.smiles()))
        rs = fm.iterateRGroups()
        for r in rs:
            print("\trgroup %d" % ri)
            fi = 1
            fs = r.iterateRGroupFragments()
            for f in fs:
                print("\t\tfragment %d: %s" % (fi, f.smiles()))
                fi += 1
            ri += 1
        mi += 1


print("*** Testing SMILES ***")


run("molecules/composition1.mol", "composed")
run("molecules/composition1.mol", "source")
run("molecules/composition1.mol", "ordered")
run("molecules/composition2.mol", "composed")
run("molecules/composition2.mol", "source")
run("molecules/composition2.mol", "ordered")
run("molecules/composition3.mol", "composed")
run("molecules/composition3.mol", "source")
run("molecules/composition3.mol", "ordered")
