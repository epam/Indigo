import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, IndigoRenderer, joinPathPy

indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "svg")
indigo.setOption("render-svg-unique-id", False)

molecules_path = joinPathPy("molecules/implicit_hydrogens/", __file__)
ref_path = joinPathPy("ref/", __file__)


def compare_svg(name):
    mol = indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, name + ".mol")
    )
    svg = "".join(chr(b & 0xFF) for b in renderer.renderToBuffer(mol))
    compare_diff(ref_path, "implicit_hydrogens/" + name + ".svg", svg)


print("*** The examples of the ticket ***")
compare_svg("ticket-pyrrole")
compare_svg("ticket-hexamethyl-sulfur")
indigo.setOption("render-label-mode", "all")
compare_svg("ticket-hexane")
indigo.setOption("render-label-mode", "terminal-hetero")

print("*** A terminal atom has its hydrogens opposite its bond ***")
compare_svg("terminal-atoms")
compare_svg("atoms-without-bonds")

print("*** An internal atom has them in the first free quadrant ***")
compare_svg("internal-atoms")
compare_svg("piperazine")
compare_svg("three-and-four-bonds")

print("*** A radical takes the top quadrant ***")
compare_svg("radicals")

print("*** A charge stands clear of hydrogens above and below ***")
compare_svg("charge-beside-stacked-hydrogens")
