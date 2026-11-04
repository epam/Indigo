import json
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, IndigoException, getIndigoExceptionText  # noqa

indigo = Indigo()

CDXML_TEMPLATE = """<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE CDXML SYSTEM "http://www.cambridgesoft.com/xml/cdxml.dtd" >
<CDXML BondLength="14.4">
<fonttable><font id="3" charset="iso-8859-1" name="Arial"/></fonttable>
<page id="1">
%s
</page>
</CDXML>
"""


def propane(fragment_id, x):
    return (
        '<fragment id="{0}"><n id="{1}" p="{4} 100"/><n id="{2}" p="{5} 107"/>'
        '<n id="{3}" p="{6} 100"/><b id="{7}" B="{1}" E="{2}"/>'
        '<b id="{8}" B="{2}" E="{3}"/></fragment>'
    ).format(
        fragment_id,
        fragment_id + 1,
        fragment_id + 2,
        fragment_id + 3,
        x,
        x + 12,
        x + 24,
        fragment_id + 4,
        fragment_id + 5,
    )


def caption(text_id, x, text):
    return '<t id="%d" p="%d 125"><s font="3" size="10">%s</s></t>' % (
        text_id,
        x,
        text,
    )


def orbital_only(fragment_id, x):
    return (
        '<fragment id="%d"><graphic id="%d" BoundingBox="%d 95 %d 105" '
        'GraphicType="Orbital" OrbitalType="pFilled"/></fragment>'
        % (fragment_id, fragment_id + 1, x, x + 6)
    )


def arrow(arrow_id, tail, head):
    return (
        '<arrow id="%d" ArrowheadHead="Full" ArrowheadType="Solid" '
        'HeadSize="1000" Head3D="%d 100 0" Tail3D="%d 100 0"/>'
        % (arrow_id, head, tail)
    )


def step(step_id, reactants, products, arrows):
    return (
        '<step id="%d" ReactionStepReactants="%s" '
        'ReactionStepProducts="%s" ReactionStepArrows="%s"/>'
        % (step_id, reactants, products, arrows)
    )


def text_content(node):
    content = json.loads(node["data"]["content"])
    return "".join(block["text"] for block in content["blocks"])


def text_nodes(reaction):
    doc = json.loads(reaction.json())
    return sorted(
        json.dumps(n, sort_keys=True)
        for n in doc["root"]["nodes"]
        if n.get("type") == "text"
    )


def summary(reaction):
    doc = json.loads(reaction.json())
    nodes = doc["root"]["nodes"]
    return "reactants %d, products %d, arrows %d, texts %s" % (
        reaction.countReactants(),
        reaction.countProducts(),
        len([n for n in nodes if n.get("type") == "arrow"]),
        json.dumps(
            sorted(text_content(json.loads(n)) for n in text_nodes(reaction))
        ),
    )


# 80 holds 81; only the attachments of 81 have crossing bonds
NESTED_BRACKETS = (
    '<bracketedgroup id="80" BracketedObjectIDs="32" '
    'BracketUsage="MultipleGroup" RepeatCount="2">'
    '<bracketedgroup id="81" BracketedObjectIDs="32" BracketUsage="SRU">'
    '<bracketattachment id="82">'
    '<crossingbond id="83" BondID="34" InnerAtomID="32"/></bracketattachment>'
    '<bracketattachment id="84">'
    '<crossingbond id="85" BondID="35" InnerAtomID="32"/></bracketattachment>'
    "</bracketedgroup>"
    '<bracketattachment id="86"/><bracketattachment id="87"/>'
    "</bracketedgroup>"
)

cases = [
    (
        "reactant fragment with only an orbital graphic",
        propane(30, 60)
        + orbital_only(70, 90)
        + propane(40, 200)
        + arrow(50, 110, 180)
        + '<scheme id="60">'
        + step(61, "30 70", "40", "50")
        + "</scheme>",
    ),
    (
        "product fragment with only an orbital graphic",
        propane(30, 60)
        + propane(40, 200)
        + orbital_only(70, 230)
        + arrow(50, 100, 180)
        + '<scheme id="60">'
        + step(61, "30", "40 70", "50")
        + "</scheme>",
    ),
    (
        "intermediate fragment with only an orbital graphic",
        propane(30, 60)
        + orbital_only(70, 150)
        + propane(40, 260)
        + arrow(50, 100, 140)
        + arrow(51, 170, 240)
        + '<scheme id="60">'
        + step(61, "30", "70", "50")
        + step(62, "70", "40", "51")
        + "</scheme>",
    ),
    (
        "reactant and product groups with a caption",
        '<group id="20">'
        + propane(30, 60)
        + caption(39, 60, "7 [40]")
        + "</group>"
        + '<group id="21">'
        + propane(40, 200)
        + caption(49, 200, "(R)-8")
        + "</group>"
        + arrow(50, 100, 180)
        + '<scheme id="60">'
        + step(61, "20", "21", "50")
        + "</scheme>",
    ),
    (
        "bracketed group nested in a bracketed group",
        propane(30, 60)
        + propane(40, 200)
        + arrow(50, 100, 180)
        + NESTED_BRACKETS
        + '<scheme id="60">'
        + step(61, "30", "40", "50")
        + "</scheme>",
    ),
    (
        "caption on the page, not in a group",
        propane(30, 60)
        + caption(39, 60, "7 [40]")
        + propane(40, 200)
        + caption(49, 200, "(R)-8")
        + arrow(50, 100, 180)
        + '<scheme id="60">'
        + step(61, "30", "40", "50")
        + "</scheme>",
    ),
]

print("*** CDXML reaction step participants ***")
loaded = {}
for name, page in cases:
    try:
        loaded[name] = indigo.loadReaction(CDXML_TEMPLATE % page)
        result = summary(loaded[name])
    except IndigoException as e:
        result = "error: %s" % getIndigoExceptionText(e)
    print("%s: %s" % (name, result))

print("*** A caption in a participant group is kept as on the page ***")
grouped = loaded.get("reactant and product groups with a caption")
on_page = loaded.get("caption on the page, not in a group")
print(
    "same text nodes: %s"
    % (
        grouped is not None
        and on_page is not None
        and text_nodes(grouped) == text_nodes(on_page)
    )
)
