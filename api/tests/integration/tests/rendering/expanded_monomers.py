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

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
renderer = IndigoRenderer(indigo)

print("***  test expanded monomer  ***")

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPathPy("out/expanded.png", __file__))
print(checkImageSimilarity("expanded.png"))

mol2 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded2.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol2, joinPathPy("out/expanded2.png", __file__))
print(checkImageSimilarity("expanded2.png"))

mol = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/monomer_unused_atps.ket", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPathPy("out/monomer_unused_atps.svg", __file__))
print(checkImageSimilarity("monomer_unused_atps.svg"))

indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPathPy("out/monomer_unused_atps.png", __file__))
print(checkImageSimilarity("monomer_unused_atps.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
