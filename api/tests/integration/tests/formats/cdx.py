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
        # 		print("Rawdata:")
        # 		print(m.rawData())
        print("Properties:")
        for prop in m.iterateProperties():
            print("%s: %s" % (prop.name(), prop.rawData()))


print("**** Read CDX from file ****")
readCdxAndPrintInfo("molecules/test-multi.cdx")

readCdxAndPrintInfo("molecules/CDX3_4molecules_prop.cdx")


print("**** Read CDX with wrong empty objects ****")
readCdxAndPrintInfo("molecules/test_title_0.cdx")
readCdxAndPrintInfo("molecules/test_title_1.cdx")
readCdxAndPrintInfo("molecules/test_title_2.cdx")
readCdxAndPrintInfo("molecules/test_title_3.cdx")
readCdxAndPrintInfo("molecules/test_title_4.cdx")
readCdxAndPrintInfo("molecules/test_title_5.cdx")
readCdxAndPrintInfo("molecules/test_title_6.cdx")
