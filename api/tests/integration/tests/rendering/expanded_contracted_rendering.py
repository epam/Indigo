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

print("***   test png output   ***")

print("***   test ket input   ***")

mol11 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol11, joinPathPy("out/contracted_fg_ss_ket.png", __file__)
)
print(checkImageSimilarity("contracted_fg_ss_ket.png"))

mol12 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_expanded_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol12, joinPathPy("out/contracted_expanded_fg_ss_ket.png", __file__)
)
print(checkImageSimilarity("contracted_expanded_fg_ss_ket.png"))

mol13 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol13, joinPathPy("out/expanded_fg_ss_ket.png", __file__)
)
print(checkImageSimilarity("expanded_fg_ss_ket.png"))

print("***   test mol input   ***")

mol14 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol14, joinPathPy("out/contracted_fg_ss_mol.png", __file__)
)
print("***   contracted_fg_ss_mol.png done  ***")
print(checkImageSimilarity("contracted_fg_ss_mol.png"))

mol15 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_expanded_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol15, joinPathPy("out/contracted_expanded_fg_ss_mol.png", __file__)
)
print(checkImageSimilarity("contracted_expanded_fg_ss_mol.png"))

mol16 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "png")
renderer.renderToFile(
    mol16, joinPathPy("out/expanded_fg_ss_mol.png", __file__)
)
print(checkImageSimilarity("expanded_fg_ss_mol.png"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
