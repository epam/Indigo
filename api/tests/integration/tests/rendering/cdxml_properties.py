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
font_size = 12
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
    m.setProperty("title", "Molecule %d" % (idx))
    m.setProperty(
        "propertyNames",
        '<s font="3" size="%d" color="10">Mass:\n</s><s font="3" size="%d" >Formula :</s>'
        % (font_size, font_size),
    )
    m.setProperty(
        "propertyValues",
        '<s font="3" size="%d" color="10">  %f\n</s><s font="3" size="%d" >  %s</s>'
        % (font_size, m.molecularWeight(), font_size, m.grossFormula()),
    )
    # Add to the array
    arr.arrayAdd(m)

# Set required options
# indigo.setOption("render-grid-title-property", "title")
# indigo.setOption("render-comment", "Comment:\nSet of molecules")

indigo.setOption("render-output-format", "cdxml")

indigo.setOption("render-grid-title-alignment", "center")
indigo.setOption("render-cdxml-title-font", "3")
indigo.setOption("render-cdxml-title-face", "2")
indigo.setOption("render-grid-title-font-size", "20")
indigo.setOption("render-cdxml-properties-size", font_size)

indigo.setOption("render-grid-title-property", "title")
indigo.setOption("render-cdxml-properties-enabled", True)

indigo.setOption("render-cdxml-properties-name-property", "propertyNames")
indigo.setOption("render-cdxml-properties-value-property", "propertyValues")

renderer.renderGridToFile(
    arr, None, 2, joinPathPy("out/cdxml/cdxml-test-properties.cdxml", __file__)
)

fonts = ["Symbol", "Arial", "Algerian"]
for f in fonts:
    indigo.setOption("render-cdxml-title-font", f)
    renderer.renderGridToFile(
        arr,
        None,
        3,
        joinPathPy("out/cdxml/cdxml-test-font%s.cdxml" % (f), __file__),
    )

indigo.setOption(
    "render-cdxml-properties-fonttable",
    '<font id="3" charset="iso-8859-1" name="Arial"/>\n'
    + '<font id="4" charset="iso-8859-1" name="Times New Roman"/>\n'
    + '<font id="5" charset="iso-8859-1" name="Algerian"/>',
)
indigo.setOption(
    "render-cdxml-properties-colortable", '<color id="10" r="1" g="0" b="0"/>'
)
indigo.setOption("render-cdxml-properties-key-alignment", "right")

renderer.renderGridToFile(
    arr,
    None,
    2,
    joinPathPy("out/cdxml/cdxml-test-properties-color.cdxml", __file__),
)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
