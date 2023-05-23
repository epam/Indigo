import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)


def readCdxAndPrintInfo(fname):
    for idx, m in enumerate(
        indigo.iterateCDXFile(joinPathPy(fname, __file__))
    ):
        print("*****")
        print(idx)
        print("Smiles:")
        print(m.smiles())
        print("Molfile:")
        print(m.molfile())


readCdxAndPrintInfo("molecules/CDX3_4molecules_prop.cdx")
