import errno
import itertools
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

if not os.path.exists(joinPathPy("out/cdxml", __file__)):
    try:
        os.makedirs(joinPathPy("out/cdxml", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

arr = indigo.createArray()
for idx, m in itertools.islice(
    enumerate(
        indigo.iterateSmilesFile(
            joinPathPy(
                "../../../../../data/molecules/basic/pubchem_slice_5000.smi",
                __file__,
            )
        )
    ),
    None,
    10,
):
    print(m.smiles())
    # Set title
    m.setProperty(
        "title",
        "Molecule:%d\nMass: %f\nFormula: %s"
        % (idx, m.molecularWeight(), m.grossFormula()),
    )
    # Add to the array
    arr.arrayAdd(m)

# Set required options
indigo.setOption("render-grid-title-property", "title")
indigo.setOption("render-comment", "Comment:\nSet of molecules")

# Render
options_align = ["left", "right", "center", "center-left", "center-right"]
for alignment in options_align:
    indigo.setOption("render-grid-title-alignment", alignment)
    renderer.renderGridToFile(
        arr,
        None,
        3,
        joinPathPy("out/cdxml/cdxml-test-%s.cdxml" % alignment, __file__),
    )

options_length = [0, 10, 50, 100, 200]
for length in options_length:
    indigo.setOption("render-bond-length", length)
    renderer.renderGridToFile(
        arr,
        None,
        3,
        joinPathPy("out/cdxml/cdxml-test-len%d.cdxml" % (length), __file__),
    )

indigo.setOption("render-output-format", "cdxml")
buf = renderer.renderGridToBuffer(arr, None, 3)
print(len(buf) > 100)

print("issue 1686 extra plus")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
fname = "extra_plus_1686"
png_fname = fname + ".png"
cdxml_fname = joinPathPy("molecules/%s.cdxml" % fname, __file__)
mol = indigo.loadMoleculeFromFile(cdxml_fname)
renderer.renderToFile(mol, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2807 missing label")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
fname = "missing_label_2807"
png_fname = fname + ".png"
cdxml_fname = joinPathPy("molecules/%s.cdxml" % fname, __file__)
mol = indigo.loadMoleculeFromFile(cdxml_fname)
renderer.renderToFile(mol, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

print("issue 2778 multiple external connections")
indigo.resetOptions()
indigo.setOption("render-output-format", "png")
fname = "multiple_external_connections_2778"
png_fname = fname + ".png"
cdxml_fname = joinPathPy("molecules/%s.cdxml" % fname, __file__)
mol = indigo.loadMoleculeFromFile(cdxml_fname)
renderer.renderToFile(mol, joinPathPy("out/" + png_fname, __file__))
print(checkImageSimilarity(png_fname))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
