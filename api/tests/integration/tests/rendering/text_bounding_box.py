import base64
import errno
import io
import json
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa: E402
    Indigo,
    IndigoRenderer,
    isIronPython,
    joinPathPy,
)

# Regression test for #3770: a text node's "boundingBox" is computed by
# Ketcher for the font the end user picked there, sized just large enough
# for that font's own metrics. Indigo does not wrap or clip text to this
# box (it is only used to align text within it), so the rendered text
# stays inside the box only when Indigo draws it with the same font
# Ketcher used to size the box. Before custom-fonts support, Indigo always
# substituted its own default font, so text sized for one font's metrics
# could overflow a box that fit that font perfectly.
#
# The two cases below render the same fixture, once with the matching
# custom font (fits, as Ketcher's user would see) and once without any
# custom font, i.e. Indigo's pre-#3770 default-font fallback (overflows).
FONT_NAME = "Almendra-Regular.ttf"


def encode_font(path):
    with open(path, "rb") as font_file:
        return base64.b64encode(font_file.read()).decode("ascii")


def load_document():
    path = joinPathPy("molecules/text_bounding_box.ket", __file__)
    with io.open(path, "r", encoding="utf-8") as ket_file:
        return json.load(ket_file)


def render_to_file(indigo, renderer, document, fonts, path):
    indigo.setOption("render-output-format", "png")
    indigo.setOption("render-background-color", "255, 255, 255")
    indigo.setOption("render-coloring", "false")
    indigo.setOption("render-fonts", fonts)
    molecule = indigo.loadMolecule(json.dumps(document))
    try:
        renderer.renderToFile(molecule, path)
    finally:
        if isIronPython():
            molecule.Dispose()


def test_text_fits_bounding_box_with_matching_font(indigo, renderer):
    from rendering import checkImageSimilarity

    document = load_document()
    font_dir = joinPathPy("fonts", __file__)
    font_data = encode_font(os.path.join(font_dir, FONT_NAME))
    font = json.dumps(
        [{"name": "Almendra", "data": font_data}],
        separators=(",", ":"),
    )

    out_dir = joinPathPy("out/text_bounding_box", __file__)
    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as error:
            if error.errno != errno.EEXIST:
                raise

    cases = (
        ("matched_font.png", font),
        ("mismatched_font.png", "[]"),
    )
    for image_name, fonts in cases:
        render_to_file(
            indigo,
            renderer,
            document,
            fonts,
            os.path.join(out_dir, image_name),
        )

    for image_name, _ in cases:
        filename = "text_bounding_box/%s" % image_name
        actual = checkImageSimilarity(filename)
        expected = "%s rendering status: OK" % filename
        if actual != expected:
            raise AssertionError(
                "%s PNG reference: expected %r, got %r"
                % (filename, expected, actual)
            )


indigo = Indigo()
renderer = IndigoRenderer(indigo)

test_text_fits_bounding_box_with_matching_font(indigo, renderer)

print("Text bounding box tests: OK")

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
