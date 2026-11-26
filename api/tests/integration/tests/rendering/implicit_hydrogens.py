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
compare_svg("near-vertical-bonds")

print("*** An atom without bonds has them where its formula is written ***")
compare_svg("atoms-without-bonds")

print("*** An internal atom has them in the first free quadrant ***")
compare_svg("internal-atoms")
compare_svg("bonds-on-diagonals")
compare_svg("piperazine")
compare_svg("three-and-four-bonds")

print("*** Hydrogens above the symbol shorten the lines that reach them ***")
compare_svg("hydrogens-beside-bonds")

print("*** A radical takes the top quadrant ***")
compare_svg("radicals")
compare_svg("radical-kinds")

print("*** A radical stands where no bond passes ***")
compare_svg("radicals-with-bond-up")
compare_svg("radical-places")

print("*** A charge stands to the right of a hydrogen count above it ***")
compare_svg("charge-beside-stacked-hydrogens")
