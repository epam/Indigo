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

print("***   test svg output   ***")

print("***   test ket input   ***")
mol21 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol21, joinPathPy("out/contracted_fg_ss_ket.svg", __file__)
)
# print(checkImageSimilarity("contracted_fg_ss_ket.svg"))
# with open("out/contracted_fg_ss_ket.svg", "r") as file:
#    svg_str = file.read()
svg_str = renderer.renderToString(mol21)
print(svg_str)

mol22 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_expanded_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol22, joinPathPy("out/contracted_expanded_fg_ss_ket.svg", __file__)
)
# print(checkImageSimilarity("contracted_expanded_fg_ss_ket.svg"))
# with open("out/contracted_expanded_fg_ss_ket.svg", "r") as file:
#    svg_str = file.read()
svg_str = renderer.renderToString(mol22)
print(svg_str)

mol23 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded_fg_ss.ket", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol23, joinPathPy("out/expanded_fg_ss_ket.svg", __file__)
)
# print(checkImageSimilarity("expanded_fg_ss_ket.svg"))
# with open("out/expanded_fg_ss_ket.svg", "r") as file:
#     svg_str = file.read()
svg_str = renderer.renderToString(mol23)
print(svg_str)

print("***   test mol input   ***")

mol24 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol24, joinPathPy("out/contracted_fg_ss_mol.svg", __file__)
)
# print(checkImageSimilarity("contracted_fg_ss_mol.svg"))
# with open("out/contracted_fg_ss_mol.svg", "r") as file:
#    svg_str = file.read()
svg_str = renderer.renderToString(mol24)
print(svg_str)

mol25 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/contracted_expanded_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol25, joinPathPy("out/contracted_expanded_fg_ss_mol.svg", __file__)
)
# print(checkImageSimilarity("contracted_expanded_fg_ss_mol.svg"))
# with open("out/contracted_expanded_fg_ss_mol.svg", "r") as file:
#    svg_str = file.read()
svg_str = renderer.renderToString(mol25)
print(svg_str)

mol26 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/expanded_fg_ss.mol", __file__)
)
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(
    mol26, joinPathPy("out/expanded_fg_ss_mol.svg", __file__)
)
svg_str = renderer.renderToString(mol26)
# print(checkImageSimilarity("expanded_fg_ss_mol.svg"))
# with open("out/expanded_fg_ss_mol.svg", "r") as file:
#     svg_str = file.read()
print(svg_str)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
