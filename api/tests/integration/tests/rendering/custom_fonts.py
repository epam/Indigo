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
    IndigoException,
    IndigoRenderer,
    getIndigoExceptionText,
    isIronPython,
    joinPathPy,
)

FONT_SETS = (
    (
        "Almendra static",
        "almendra-static.png",
        (
            "Almendra-Regular.ttf",
            "Almendra-Bold.ttf",
            "Almendra-Italic.ttf",
            "Almendra-BoldItalic.ttf",
        ),
    ),
    (
        "Roboto variable",
        "roboto-variable.png",
        (
            "Roboto-Regular-Variable.ttf",
            "Roboto-Italic-Variable.ttf",
        ),
    ),
    (
        "Urbanist variable ital+wght",
        "urbanist-variable.png",
        ("Urbanist-Variable.ttf",),
    ),
    (
        "Roboto Flex variable slnt+wght",
        "roboto-flex-variable.png",
        ("Roboto-Flex-Variable.ttf",),
    ),
    (
        "Almendra Regular only",
        "almendra-regular-only.png",
        ("Almendra-Regular.ttf",),
    ),
)


def assert_equal(actual, expected, message):
    if actual != expected:
        raise AssertionError(
            "%s: expected %r, got %r" % (message, expected, actual)
        )


def assert_json_equal(actual, expected, message):
    # Compares parsed JSON rather than raw strings: under Jython, dict key
    # order from json.dumps() is not guaranteed to match the source literal,
    # while Indigo's fonts option always round-trips through a fixed
    # "name" then "data" key order. Reporting lengths rather than the full
    # (base64-encoded font) content keeps a failure message readable.
    if json.loads(actual) != json.loads(expected):
        raise AssertionError(
            "%s: expected %d chars, got %d chars"
            % (message, len(expected), len(actual))
        )


def assert_option_error(indigo, value, expected):
    try:
        indigo.setOption("render-fonts", value)
    except IndigoException as error:
        message = getIndigoExceptionText(error)
        if expected not in message:
            raise AssertionError(
                "expected %r in error message %r" % (expected, message)
            )
    else:
        raise AssertionError("fonts option accepted invalid value %r" % value)


def test_render_fonts_option(indigo):
    assert_equal(
        indigo.getOptionType("render-fonts"), "str", "fonts option type"
    )
    assert_equal(
        indigo.getOption("render-fonts"), "[]", "default fonts option"
    )

    font_dir = joinPathPy("fonts", __file__)
    first_font = encode_font(os.path.join(font_dir, "Almendra-Bold.ttf"))
    second_font = encode_font(os.path.join(font_dir, "Almendra-Italic.ttf"))

    first_value = json.dumps(
        [{"name": "first", "data": first_font}], separators=(",", ":")
    )
    indigo.setOption("render-fonts", first_value)
    assert_json_equal(
        indigo.getOption("render-fonts"), first_value, "single font"
    )

    multiple_value = json.dumps(
        [
            {"name": "first", "data": first_font},
            {"name": "second", "data": second_font},
        ],
        separators=(",", ":"),
    )
    indigo.setOption("render-fonts", multiple_value)
    assert_json_equal(
        indigo.getOption("render-fonts"), multiple_value, "multiple fonts"
    )

    # Decodes fine, but matches no supported TrueType/OpenType signature.
    unsupported_format_data = base64.b64encode(b"\x00\x00\x00\x00").decode(
        "ascii"
    )
    # WOFF is not one of the signatures indigoRenderSetFonts recognizes.
    woff_format_data = encode_font(
        os.path.join(font_dir, "Almendra-Regular.woff")
    )
    # One character past the maximum allowed Base64 length for a single
    # font, so the size check rejects it before attempting to decode.
    oversized_data = "A" * (((16 * 1024 * 1024 + 2) // 3) * 4 + 1)

    invalid_values = (
        # Only the offset is ours; the rest of the message is rapidjson's
        # own wording, which is not our contract to assert on.
        ("[", "Invalid fonts JSON at offset 1:"),
        ("{}", "Invalid fonts JSON: expected an array"),
        ('["font"]', "Invalid font at index 0: expected an object"),
        ("[{}]", "Invalid font at index 0: 'name' must be a string"),
        (
            '[{"name":1,"data":""}]',
            "Invalid font at index 0: 'name' must be a string",
        ),
        (
            '[{"name":"broken"}]',
            "Invalid font at index 0: 'data' must be a Base64 string",
        ),
        (
            '[{"name":"broken","data":1}]',
            "Invalid font at index 0: 'data' must be a Base64 string",
        ),
        (
            '[{"name":"broken","data":""}]',
            "Invalid font at index 0: unsupported font format",
        ),
        (
            '[{"name":"broken","data":"%%%"}]',
            "Invalid Base64 data for font 'broken' at index 0:",
        ),
        (
            '[{"name":"broken","data":"%s"}]' % unsupported_format_data,
            "Invalid font at index 0: unsupported font format",
        ),
        (
            '[{"name":"broken","data":"%s"}]' % woff_format_data,
            "Invalid font at index 0: unsupported font format",
        ),
        (
            '[{"name":"broken","data":"%s"}]' % oversized_data,
            "Invalid font at index 0: data exceeds",
        ),
        (
            "[{},{},{},{},{},{},{},{},{}]",
            "Invalid fonts JSON: too many fonts (maximum 8)",
        ),
    )
    for value, expected in invalid_values:
        assert_option_error(indigo, value, expected)
        assert_json_equal(
            indigo.getOption("render-fonts"),
            multiple_value,
            "invalid value must not replace fonts",
        )

    indigo.resetOptions()
    assert_equal(indigo.getOption("render-fonts"), "[]", "reset fonts option")


def encode_font(path):
    with open(path, "rb") as font_file:
        encoded = base64.b64encode(font_file.read())
    if not isinstance(encoded, str):
        encoded = encoded.decode("ascii")
    return encoded


def load_font_sets():
    font_dir = joinPathPy("fonts", __file__)
    result = []
    for set_name, image_name, filenames in FONT_SETS:
        fonts = []
        for filename in filenames:
            fonts.append(
                {
                    "name": filename,
                    "data": encode_font(os.path.join(font_dir, filename)),
                }
            )
        result.append(
            (
                set_name,
                image_name,
                json.dumps(fonts, separators=(",", ":")),
            )
        )
    return result


def load_document(name):
    path = joinPathPy("molecules/%s" % name, __file__)
    with io.open(path, "r", encoding="utf-8") as ket_file:
        return json.load(ket_file)


def render(indigo, renderer, document, output_format, fonts=None):
    indigo.setOption("render-output-format", output_format)
    if fonts is not None:
        indigo.setOption("render-fonts", fonts)
    molecule = indigo.loadMolecule(json.dumps(document))
    try:
        return list(renderer.renderToBuffer(molecule))
    finally:
        if isIronPython():
            molecule.Dispose()


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


def assert_default_differs_from_explicit_empty_fonts(
    indigo, renderer, document
):
    # render-fonts has never been requested right after resetOptions():
    # that must fall back to the bundled Noto Sans. Explicitly clearing
    # the font list to "[]" is a deliberate opt-out of Noto Sans, so it
    # must fall back to cairo's own default font instead - the two must
    # render differently.
    indigo.resetOptions()
    never_set = render(indigo, renderer, document, "svg")
    explicit_empty = render(indigo, renderer, document, "svg", "[]")
    if never_set == explicit_empty:
        raise AssertionError(
            "never setting render-fonts and explicitly clearing it to "
            "'[]' must render differently (Noto Sans vs cairo default)"
        )


def assert_custom_svg_rendering(indigo, renderer, document, font_sets):
    default = render(indigo, renderer, document, "svg", "[]")
    for set_name, _, fonts in font_sets:
        custom = render(indigo, renderer, document, "svg", fonts)
        if custom == default:
            raise AssertionError("%s did not affect SVG rendering" % set_name)


def assert_custom_png_rendering(indigo, renderer, document, font_sets):
    from rendering import checkImageSimilarity

    out_dir = joinPathPy("out/custom_fonts", __file__)
    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as error:
            if error.errno != errno.EEXIST:
                raise

    for _, image_name, fonts in font_sets:
        render_to_file(
            indigo,
            renderer,
            document,
            fonts,
            os.path.join(out_dir, image_name),
        )

    for set_name, image_name, _ in font_sets:
        filename = "custom_fonts/%s" % image_name
        actual = checkImageSimilarity(filename)
        expected = "%s rendering status: OK" % filename
        assert_equal(actual, expected, "%s PNG reference" % set_name)


def assert_first_matching_font_wins(indigo, renderer):
    # Plain ASCII: both fonts fully cover it, so a fallback to Noto Sans
    # for a glyph one font lacks cannot confound the comparison.
    document = load_document("custom_fonts_ascii.ket")

    font_dir = joinPathPy("fonts", __file__)
    almendra = encode_font(os.path.join(font_dir, "Almendra-Regular.ttf"))
    roboto = encode_font(os.path.join(font_dir, "Roboto-Regular-Variable.ttf"))

    almendra_only = json.dumps(
        [{"name": "almendra", "data": almendra}], separators=(",", ":")
    )
    roboto_only = json.dumps(
        [{"name": "roboto", "data": roboto}], separators=(",", ":")
    )
    almendra_first = json.dumps(
        [
            {"name": "almendra", "data": almendra},
            {"name": "roboto", "data": roboto},
        ],
        separators=(",", ":"),
    )

    with_almendra = render(indigo, renderer, document, "svg", almendra_only)
    with_roboto = render(indigo, renderer, document, "svg", roboto_only)
    with_both = render(indigo, renderer, document, "svg", almendra_first)

    if with_both != with_almendra:
        raise AssertionError(
            "listing a second font with the same style changed rendering: "
            "the first matching font must win"
        )
    if with_both == with_roboto:
        raise AssertionError(
            "rendering matched the second font: the first one seems unused"
        )


def assert_otf_font_accepted(indigo, renderer):
    # Almendra-Regular.otf is the same font as Almendra-Regular.ttf,
    # re-encoded as 'OTTO'-tagged CFF, exercising the OpenType/CFF
    # parsing path that the TrueType fixtures never touch.
    document = load_document("custom_fonts_ascii.ket")
    font_dir = joinPathPy("fonts", __file__)
    otf_font = encode_font(os.path.join(font_dir, "Almendra-Regular.otf"))
    fonts = json.dumps(
        [{"name": "Almendra-Regular-OTF", "data": otf_font}],
        separators=(",", ":"),
    )
    default = render(indigo, renderer, document, "svg", "[]")
    custom = render(indigo, renderer, document, "svg", fonts)
    if custom == default:
        raise AssertionError(
            "OTF font (CFF-flavored OpenType) did not affect rendering"
        )


def assert_invalid_font_rejected(indigo, renderer, document):
    # Valid TrueType signature so the font passes the format check, but the
    # data that follows is not a real font, so FreeType fails to parse it.
    broken_data = base64.b64encode(b"\x00\x01\x00\x00" + b"\x00" * 16).decode(
        "ascii"
    )
    invalid_font = json.dumps(
        [{"name": "broken-font", "data": broken_data}],
        separators=(",", ":"),
    )
    try:
        render(indigo, renderer, document, "svg", invalid_font)
    except IndigoException as error:
        message = getIndigoExceptionText(error)
        expected = "Error loading font 'broken-font'"
        if expected not in message:
            raise AssertionError(
                "expected %r in error message %r" % (expected, message)
            )
    else:
        raise AssertionError("renderer accepted an invalid font file")


def test_custom_font_rendering(indigo, renderer):
    document = load_document("custom_fonts_utf.ket")
    font_sets = load_font_sets()
    assert_default_differs_from_explicit_empty_fonts(
        indigo, renderer, document
    )
    assert_custom_svg_rendering(indigo, renderer, document, font_sets)
    assert_custom_png_rendering(indigo, renderer, document, font_sets)
    assert_first_matching_font_wins(indigo, renderer)
    assert_otf_font_accepted(indigo, renderer)
    assert_invalid_font_rejected(indigo, renderer, document)


indigo = Indigo()
renderer = IndigoRenderer(indigo)

test_render_fonts_option(indigo)
test_custom_font_rendering(indigo, renderer)

print("Custom fonts option tests: OK")

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
