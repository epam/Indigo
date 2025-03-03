import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoRenderer, isIronPython, joinPathPy  # noqa
from rendering import checkImageSimilarity  # noqa

indigo = Indigo()
renderer = IndigoRenderer(indigo)
out_dir = "out/chemaxon_smiles/"

if not os.path.exists(joinPathPy(out_dir, __file__)):
    try:
        os.makedirs(joinPathPy(out_dir, __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


print("issue 425 smiles with attachment points")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")

smiles = {
    "O": "[O-]%91.[*:1]%91 |$;_AP1$|",
    "S": "S%91C(C1C=CC=CC=1)C1C=CC=CC=1.[*:1]%91 |$;;;;;;;;;;;;;;_AP1$|",
    "N": "[NH3+]%91.[*:1]%91 |$;_AP1$|",
}

for key, value in smiles.items():
    png_fname = "smiles_attachment_points_452_" + key + ".png"
    mol = indigo.loadMolecule(value)
    renderer.renderToFile(mol, joinPathPy(out_dir + png_fname, __file__))
    print(checkImageSimilarity("chemaxon_smiles/" + png_fname))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
