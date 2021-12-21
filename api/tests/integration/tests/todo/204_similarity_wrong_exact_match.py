import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *


def main():
    indigo = Indigo()
    indigo_inchi = IndigoInchi(indigo)
    m1 = indigo_inchi.loadMolecule(
        "InChI=1S/C17H25N3O2/c18-9-14-2-1-3-20(14)15(21)10-19-16-5-12-4-13(6-16)8-17(22,7-12)11-16/h12-14,19,22H,1-8,10-11H2/t12?,13?,14-,16?,17?/m0/s1"
    )
    m2 = indigo_inchi.loadMolecule(
        "InChI=1S/C18H24N4O2/c19-8-14-1-2-15(9-20)22(14)16(23)10-21-17-4-12-3-13(5-17)7-18(24,6-12)11-17/h12-15,21,24H,1-7,10-11H2/t12?,13?,14-,15+,17?,18?"
    )
    m1_fp = m1.fingerprint("sim").toString()
    m2_fp = m2.fingerprint("sim").toString()
    print(m1_fp != m2_fp)
    print(indigo.similarity(m1, m2) < 1.0)


if __name__ == "__main__":
    main()
