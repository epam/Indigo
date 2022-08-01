import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "true")
indigo.setOption("molfile-saving-mode", "3000")

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ferrocene-variant1.mol", __file__)
)
print(m.canonicalSmiles())
print(m.molfile())

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ferrocene-variant5.mol", __file__)
)
print(m.canonicalSmiles())
print(m.molfile())


m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ferrocene-variant6.mol", __file__)
)
print(m.canonicalSmiles())
print(m.molfile())


m = indigo.loadMoleculeFromFile(joinPathPy("molecules/h_bond.mol", __file__))
print(m.canonicalSmiles())
print(m.molfile())
