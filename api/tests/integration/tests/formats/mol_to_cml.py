import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()

indigo.setOption("molfile-saving-skip-date", True)

print("*** Mol to CML ***")

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/disconnected-0106.mol", __file__)
)
print(mol.cml())

print(
    "*** CML load atoms with incorrect valency (without radical settings) ***"
)
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/incorrect_valency.mol", __file__)
)
print(mol.cml())

print("*** CML sgroup saving ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/sgroups.mol", __file__)
)
print(mol.cml())

print("*** CML rgroup saving ***")
mol = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/rgroups.mol", __file__)
)
print(mol.molfile())
print(mol.cml())
print(indigo.loadQueryMolecule(mol.cml()).molfile())

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/rgroup_all.mol", __file__)
)
print(mol.cml())

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/rgroup_all.mrv", __file__)
)
print(mol.cml())

print("*** CML query features saving ***")
mol = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/Query_for_CML.mol", __file__)
)
print(mol.molfile())
print(mol.cml())
print(indigo.loadQueryMolecule(mol.cml()).molfile())

mol = indigo.loadQueryMolecule("******** |$;AH_p;Q_e;QH_p;M_p;MH_p;X_p;XH_p$|")
print(mol.smiles())
print(mol.molfile())
print(indigo.loadQueryMolecule(mol.molfile()).molfile())
print(indigo.loadQueryMolecule(mol.molfile()).smiles())

print(mol.cml())
print(indigo.loadQueryMolecule(mol.cml()).molfile())
print(indigo.loadQueryMolecule(mol.cml()).smiles())


print("*** CML error at attempt to load query as molecule ***")
try:
    print(indigo.loadMolecule(mol.cml()).molfile())
except IndigoException as e:
    print(getIndigoExceptionText(e))

mol = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/Query_bonds.mol", __file__)
)
print(mol.molfile())
print(mol.cml())
print(indigo.loadQueryMolecule(mol.cml()).molfile())

print("*** CML error at attempt to load query as molecule ***")
try:
    print(indigo.loadMolecule(mol.cml()).molfile())
except IndigoException as e:
    print(getIndigoExceptionText(e))


print("*** CML error handling ***")
root = joinPathPy("molecules/cml", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(
            joinPathPy(os.path.join(root, filename), __file__)
        )
        print(mol.smiles())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
