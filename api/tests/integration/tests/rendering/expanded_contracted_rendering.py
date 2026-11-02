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
indigo.setOption("render-fonts", "[]")

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

print("***   issue 3990: S-group without a label   ***")
# An S-group without a label is drawn as if it had the default one: none for a
# superatom, "n" for a repeating unit. The picture check cannot see a caption
# as small as "(null)" or "n", so the SVG of a structure is also compared with
# the SVG of the same structure after setSGroupName().
# The bundled font draws the same pixels everywhere: one reference per image.
NO_LABEL_DIR = "sgroup_no_label/"
COLLAPSED_MOL = "superatom_no_label.mol"
EXPANDED_MOL = "superatom_no_label_expanded.mol"
COLLAPSED_KET = "superatom_no_label.ket"
SRU_MOL = "sru_no_label.mol"
indigo.resetOptions()
indigo.setOption("render-svg-unique-id", False)


def noLabelPath(file_name):
    return joinPathPy("molecules/" + NO_LABEL_DIR + file_name, __file__)


def renderSvg(structure):
    indigo.setOption("render-output-format", "svg")
    return "".join(chr(b & 0xFF) for b in renderer.renderToBuffer(structure))


def printDrawnAsLabel(case, structure, label=""):
    svg_without_label = renderSvg(structure)
    for sgroup in structure.iterateSGroups():
        sgroup.setSGroupName(label)
    if renderSvg(structure) == svg_without_label:
        print("%s: drawn as label '%s'" % (case, label))
    else:
        print("%s: differs from label '%s'" % (case, label))


for mol_name in (COLLAPSED_MOL, EXPANDED_MOL):
    png_name = os.path.splitext(mol_name)[0] + ".png"
    indigo.setOption("render-output-format", "png")
    renderer.renderToFile(
        indigo.loadMoleculeFromFile(noLabelPath(mol_name)),
        joinPathPy("out/" + png_name, __file__),
    )
    print(checkImageSimilarity(png_name, NO_LABEL_DIR + png_name, shared=True))
    printDrawnAsLabel(
        mol_name, indigo.loadMoleculeFromFile(noLabelPath(mol_name))
    )
printDrawnAsLabel(
    COLLAPSED_MOL + ", query",
    indigo.loadQueryMoleculeFromFile(noLabelPath(COLLAPSED_MOL)),
)
printDrawnAsLabel(
    COLLAPSED_KET, indigo.loadMoleculeFromFile(noLabelPath(COLLAPSED_KET))
)
printDrawnAsLabel(
    SRU_MOL, indigo.loadMoleculeFromFile(noLabelPath(SRU_MOL)), "n"
)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
