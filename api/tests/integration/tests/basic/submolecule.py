from __future__ import print_function

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoException

if __name__ == "__main__":
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", "1")
    m = indigo.loadMolecule("C1=CC=CC=C1.C1=CC=CC=C1")
    sm = m.getSubmolecule([0, 1, 2, 3, 4, 5])

    print(m.molfile())
    print(sm.molfile())

    for item in (
        "aromatize",
        "dearomatize",
        "molfile",
        "smiles",
        "cml",
        "layout",
        "clean2d",
        "grossFormula",
        "molecularWeight",
        "mostAbundantMass",
        "monoisotopicMass",
        "massComposition",
        "hasZCoord",
        "checkBadValence",
        "checkAmbiguousH",
    ):  # , 'grossFormula'):
        try:
            print(item + ": ", end="")
            getattr(sm, item)()
            print("ok")
        except IndigoException as e:
            print("Exception: {0}".format(e.value))
