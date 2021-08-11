
import sys
sys.path.append('../../common')
from env_indigo import *
from rendering import *

indigo = Indigo()
renderer = IndigoRenderer(indigo)

print("*** SMILES to CDXML ***")

out_path = joinPathPy( "out", __file__ )

if not os.access( out_path, os.F_OK ):
    os.mkdir( out_path )

indigo.setOption("render-output-format", "cdxml")

arr2 = indigo.createArray()
qm = indigo.loadMolecule("[*+2].[*+2].CCCN([O-])C(=O)c1ccccc1C(=O)[O-].CCCN([O-])C(=O)c2ccccc2C(=O)[O-]")
arr2.arrayAdd(qm)
mol_r=indigo.loadMoleculeFromFile(joinPathPy("molecules/1434639506.sdf", __file__))
arr2.arrayAdd(mol_r)
renderer.renderGridToFile(arr2, None, 2, joinPathPy("out/atom_pseudo2.cdxml", __file__))
if isIronPython():
    renderer.Dispose()