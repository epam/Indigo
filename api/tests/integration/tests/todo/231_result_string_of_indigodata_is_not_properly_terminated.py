import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

# Issue #231 Result string of indigoData is not properly terminated


def main():
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", True)
    m = indigo.loadMolecule(
        "InChI=1S/C17H25N3O2/c18-9-14-2-1-3-20(14)15(21)10-19-16-5-12-4-13(6-16)8-17(22,7-12)11-16/h12-14,19,22H,1-8,10-11H2/t12?,13?,14-,16?,17?/m0/s1"
    )
    sgroup = m.addDataSGroup([0, 1, 2, 3], [], "color", "0.155, 0.55, 0.955")
    for id in range(m.countDataSGroups()):
        sgroup = m.getDataSGroup(id)
        name = sgroup.description()
        aString = sgroup.data()
        if "color" == name:
            print("#231 getDataSGroup::name:", name)
            print("#231 getDataSGroup::data:", aString)
            print("0.155, 0.55, 0.955" == aString)


if __name__ == "__main__":
    main()
