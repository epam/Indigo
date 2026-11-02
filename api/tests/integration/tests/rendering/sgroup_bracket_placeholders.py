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
indigo.setOption("render-output-format", "png")
indigo.setOption("molfile-saving-skip-date", True)

molecules_path = joinPathPy("molecules/", __file__)


def render(name, mol):
    renderer.renderToBuffer(mol)
    brackets = [
        line for line in mol.molfile().split("\n") if line.startswith("M  SDI")
    ]
    print("%s: rendered, %d brackets saved" % (name, len(brackets)))


print(
    "*** A real bracket next to a zero-length one, before and after a MOL round trip ***"
)
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "gen-sup-zero-next-to-real.mol")
)
render("as loaded", mol)
render("reloaded", indigo.loadMolecule(mol.molfile()))

print("*** A group with a single bracket in the file ***")
text = (
    open(os.path.join(molecules_path, "gen-sup-zero-next-to-real.mol"), "rb")
    .read()
    .decode("utf-8")
)
text = "\n".join(
    line
    for line in text.split("\n")
    if "0.0000    0.0000    0.0000    0.0000" not in line
)
render("single brackets", indigo.loadMolecule(text))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
