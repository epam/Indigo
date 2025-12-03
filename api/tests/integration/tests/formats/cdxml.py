import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

print("*** SMILES to CDXML ***")

out_path = joinPathPy("out", __file__)

if not os.access(out_path, os.F_OK):
    os.mkdir(out_path)

indigo.setOption("render-output-format", "cdxml")

arr2 = indigo.createArray()
qm = indigo.loadMolecule(
    "[*+2].[*+2].CCCN([O-])C(=O)c1ccccc1C(=O)[O-].CCCN([O-])C(=O)c2ccccc2C(=O)[O-]"
)
arr2.arrayAdd(qm)
mol_r = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/1434639506.sdf", __file__)
)
arr2.arrayAdd(mol_r)
renderer.renderGridToFile(
    arr2, None, 2, joinPathPy("out/atom_pseudo2.cdxml", __file__)
)

print("issue 3060 ch labels as pseudo atoms")
indigo.resetOptions()
fileList = ["iso_butane", "iso_butene", "ethyne"]
for testFile in fileList:
    mol = indigo.loadMoleculeFromFile(
        joinPathPy("molecules/3060_" + testFile + ".cdxml", __file__)
    )
    print(mol.molecularFormula())
    print("{:.3f}".format(mol.molecularWeight()))

print("issue 3360 cip labels not added when loading cdxml")
indigo.resetOptions()
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/3360_noCIPLabels.cdxml", __file__)
)
print("Before adding cip descriptors")
for atom in mol.iterateAtoms():
    output = (
        "atom: "
        + str(atom.index())
        + " cip: "
        + str(atom.stereocenterCIPDescriptor())
    )
    print(output)
mol.addCIPStereoDescriptors()
print("After adding cip descriptors")
for atom in mol.iterateAtoms():
    output = (
        "atom: "
        + str(atom.index())
        + " cip: "
        + str(atom.stereocenterCIPDescriptor())
    )
    print(output)

print("issue 3362 13C isotope not recognised")
indigo.resetOptions()
indigo.setOption("gross-formula-add-isotopes", True)
mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/3362_carbon_isotopes.cdxml", __file__)
)
print(mol.molecularFormula())
print("{:.3f}".format(mol.molecularWeight()))

if isIronPython():
    renderer.Dispose()
