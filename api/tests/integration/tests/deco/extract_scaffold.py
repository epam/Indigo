import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()


def testCommon():
    m1 = indigo.loadMolecule(
        "COCC1=C(N=CC2=C1C1=C(OC3=CC=C(Cl)C=C3)C=CC=C1N2)C(=O)OC(C)C"
    )
    m2 = indigo.loadMolecule(
        "COCC1=CN=C(C(=O)OC(C)C)C2=C1C1=CC=C(OC3=CC=C(Cl)C=C3)C=C1N2"
    )
    indigo.setOption("deconvolution-aromatization", "false")
    print(
        "Should extract all possible exact MCS for given molecule set (non aromatic)"
    )
    deco = indigo.extractCommonScaffold([m1, m2], "EXACT")
    scaffold_smiles = []
    for scaffold in deco.allScaffolds().iterateArray():
        scaffold_smiles.append(
            indigo.loadMolecule(scaffold.smiles()).canonicalSmiles()
        )
    for smiles in sorted(scaffold_smiles):
        print("  %s" % smiles)


def testTimeout():
    print("should stop on timeout")
    indigo.setOption("timeout", 50)
    indigo.setOption("deconvolution-aromatization", "true")
    m1 = indigo.loadMolecule(
        "CC1CC2CCC3CC4=CC=C5CCCC6=C/C=C7/C8CCCC9CC%10CCCC%11CC%12=C(C(C%10%11)C89)C7=CC(C2=C3CC4=C56)=C1%12"
    )
    m2 = indigo.loadMolecule(
        "C1CC2CC3CCCC4C3C3C2C(C1)C1CCC2CC5CCC6CC7=CC=C8CCCC9=C%10C=C4C4=C%11C(C5=C6C(C7=C89)=C%10%11)=C2C1=C34"
    )
    try:
        deco = indigo.extractCommonScaffold([m1, m2], "EXACT")
    except:
        print("stopped")
    indigo.setOption("timeout", 0)


testCommon()
testTimeout()
