import json
import os
import struct
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoException, getIndigoExceptionText  # noqa

indigo = Indigo()

CDXML_TEMPLATE = """<?xml version="1.0" encoding="UTF-8"?>
<CDXML BondLength="14.4">
<fonttable><font id="3" charset="iso-8859-1" name="Arial"/>%s</fonttable>
<page id="1">
<fragment id="10"><n id="11" p="100 100"/><n id="12" p="112 107"/>
<n id="13" p="124 100"/><b id="14" B="11" E="12"/><b id="15" B="12" E="13"/>
</fragment>
%s
</page></CDXML>
"""

SYMBOL_FONT = '<font id="4" charset="Symbol" name="Symbol"/>'

# The three text objects of the issue: a caption whose "+" is a Symbol run,
# a Greek letter typed as a Latin one in the Symbol font, and the same
# caption as text 20 in one run.
TEXT_RUNS = (
    '<t id="20" p="100 130"><s font="3" size="10">(</s>'
    '<s font="4" size="10">+</s>'
    '<s font="3" size="10">)-1 9R,13S,14R</s></t>'
    '<t id="21" p="100 150"><s font="3" size="10">R = </s>'
    '<s font="4" size="10">b</s><s font="3" size="10">-OH</s></t>'
    '<t id="22" p="100 170"><s font="3" size="10">(+)-2 R</s></t>'
)


def plus_text(text_id, runs):
    return '<t id="%d" p="150 100">%s</t>' % (
        text_id,
        "".join(
            '<s font="%d" size="10">%s</s>' % (font, text)
            for font, text in runs
        ),
    )


def cdx_prop(tag, data):
    return struct.pack("<HH", tag, len(data)) + data


def cdx_object(tag, object_id, content):
    return struct.pack("<HI", tag, object_id) + content + b"\0\0"


def cdx_text(object_id, y, runs):
    styles = b""
    text = b""
    for font, chunk in runs:
        styles += struct.pack("<HHHHH", len(text), font, 0, 200, 3)
        text += chunk
    position = struct.pack("<ii", y * 65536, 100 * 65536)
    string = struct.pack("<H", len(runs)) + styles + text
    return cdx_object(
        0x8006,
        object_id,
        cdx_prop(0x0200, position) + cdx_prop(0x0700, string),
    )


CDX_FONTS = ((3, 1252, b"Arial"), (7, 0, b"Symbol"))


def cdx_document(texts, font_table=CDX_FONTS):
    fonts = struct.pack("<HH", 1, len(font_table))
    for font_id, charset, name in font_table:
        fonts += struct.pack("<HHH", font_id, charset, len(name)) + name
    page = cdx_object(0x8001, 1, b"".join(texts))
    document = cdx_object(0x8000, 0, cdx_prop(0x0100, fonts) + page)
    return b"VjCD0100" + struct.pack("<I", 0x01020304) + b"\0" * 10 + document


def text_content(node):
    content = json.loads(node["data"]["content"])
    return "\n".join(block["text"] for block in content["blocks"])


def describe(structure):
    nodes = json.loads(structure.json())["root"]["nodes"]
    texts = [text_content(n) for n in nodes if n.get("type") == "text"]
    pluses = sum(1 for n in nodes if n.get("type") == "plus")
    return "texts %s pluses %d" % (json.dumps(texts), pluses)


def run(name, load):
    try:
        print("%s: %s" % (name, describe(load())))
    except IndigoException as e:
        print("%s: %s" % (name, getIndigoExceptionText(e)))


issue_file = CDXML_TEMPLATE % (SYMBOL_FONT, TEXT_RUNS)
run("issue file, reaction", lambda: indigo.loadReaction(issue_file))
run("issue file, molecule", lambda: indigo.loadMolecule(issue_file))

one_run_plus = CDXML_TEMPLATE % (
    SYMBOL_FONT,
    TEXT_RUNS + plus_text(23, [(3, "+")]),
)
run("issue file + one-run plus", lambda: indigo.loadReaction(one_run_plus))

spaced_plus = CDXML_TEMPLATE % (
    SYMBOL_FONT,
    plus_text(23, [(3, " "), (4, "+"), (3, " ")]),
)
run("plus in runs with spaces", lambda: indigo.loadReaction(spaced_plus))

# A font that is the Symbol font only by its charset.
charset_only = CDXML_TEMPLATE % (
    '<font id="5" charset="x-mac-symbol" name="Greek"/>',
    plus_text(24, [(3, "T = 25 "), (5, "&#177;"), (3, " 1 K, "), (5, "D")]),
)
run("charset x-mac-symbol", lambda: indigo.loadReaction(charset_only))

# Binary CDX: the font table is a document property there.
cdx_file = cdx_document(
    [
        cdx_text(20, 130, [(3, b"("), (7, b"+"), (3, b")-1 9R,13S,14R")]),
        cdx_text(21, 150, [(3, b"R = "), (7, b"b"), (3, b"-OH")]),
        cdx_text(22, 170, [(3, b"("), (7, b"-"), (3, b")-1")]),
        cdx_text(
            23,
            190,
            [
                (3, b"A "),
                (7, b"\xae"),
                (3, b" B, 25"),
                (7, b"\xb0"),
                (3, b"C"),
            ],
        ),
        cdx_text(24, 210, [(3, b"+")]),
    ]
)
run("binary CDX", lambda: indigo.loadMoleculeFromBuffer(cdx_file))

# A malformed font table only loses the Symbol mapping; the file still loads.
greek_b = [cdx_text(21, 150, [(3, b"R = "), (7, b"b"), (3, b"-OH")])]
newline_name = cdx_document(
    greek_b, ((3, 1252, b"Ari\nxx yy zz"), (7, 0, b"Symbol"))
)
run(
    "binary CDX, line break in a font name",
    lambda: indigo.loadMoleculeFromBuffer(newline_name),
)
# Font 7 is Times; a font name that spells a table line must not change it.
table_line_name = cdx_document(
    greek_b, ((3, 1252, b"Ari\n7 0 Symbol"), (7, 0, b"Times"))
)
run(
    "binary CDX, font name spelling a table line",
    lambda: indigo.loadMoleculeFromBuffer(table_line_name),
)
fonttable_attribute = (CDXML_TEMPLATE % (SYMBOL_FONT, TEXT_RUNS)).replace(
    "<CDXML ", '<CDXML fonttable="ab cd ef" '
)
run(
    "CDXML fonttable attribute",
    lambda: indigo.loadReaction(fonttable_attribute),
)
