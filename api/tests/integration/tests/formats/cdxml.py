
import sys
sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

print("*** SMILES to CDXML ***")

if not os.path.exists(joinPath("out")):
    os.mkdir(joinPath("out"))

indigo.setOption("render-output-format", "cdxml")

arr2 = indigo.createArray()
qm = indigo.loadMolecule("[*+2].[*+2].CCCN([O-])C(=O)c1ccccc1C(=O)[O-].CCCN([O-])C(=O)c2ccccc2C(=O)[O-]")
arr2.arrayAdd(qm)
mol_r=indigo.loadMoleculeFromFile(joinPath("molecules/1434639506.sdf"))
arr2.arrayAdd(mol_r)
renderer.renderGridToFile(arr2, None, 2, joinPath("out/atom_pseudo2.cdxml"))
