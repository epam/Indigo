# coding=utf-8
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Load molfile with UTF-8 characters in Data S-group ********")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/sgroups_utf8.mol", __file__)
)
indigo.setOption("molfile-saving-mode", "2000")
res = m.molfile()
m = indigo.loadMolecule(res)

print(m.molfile())
print(res)
print(m.cml())
