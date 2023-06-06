import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa
from rendering import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
renderer = IndigoRenderer(indigo)

mol1 = indigo.loadMoleculeFromFile(joinPathPy("molecules/contracted_fg_ss.ket", __file__))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol1, joinPathPy("out/contracted_fg_ss_ket.png", __file__))
print(checkImageSimilarity("contracted_fg_ss_ket.png"))

mol2 = indigo.loadMoleculeFromFile(joinPathPy("molecules/contracted_expanded_fg_ss.ket", __file__))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol2, joinPathPy("out/contracted_expanded_fg_ss_ket.png", __file__))
print(checkImageSimilarity("contracted_expanded_fg_ss_ket.png"))

mol3 = indigo.loadMoleculeFromFile(joinPathPy("molecules/contracted_expanded_fg_ss.mol", __file__))
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol3, joinPathPy("out/contracted_expanded_fg_ss_mol.png", __file__))
print(checkImageSimilarity("contracted_expanded_fg_ss_mol.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
