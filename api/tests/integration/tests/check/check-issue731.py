import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

options = {
    "aromatize-skip-superatoms": "true",
    "gross-formula-add-isotopes": "true",
    "gross-formula-add-rsites": "true",
    "ignore-stereochemistry-errors": "true",
    "mass-skip-error-on-pseudoatoms": "true",
    "smart-layout": "true"
}
for option, value in options.items():
    indigo.setOption(option, value)

tests = [
    "3D_Structure",
    "Chirality",
    "Overlapping_Atoms",
    "Pseudoatom",
    "Radical",
    "Stereochemistry",
]

check_flags = "radicals;pseudoatoms;stereo;query;overlapping_atoms;overlapping_bonds;rgroups;chiral;chiral-flag;3d"
errors = ""
for test in tests:
    with open(
        joinPathPy("issue731/" + test + ".mol", __file__), "r"
    ) as file:
        molfile = file.read()
    print(
        "\nTEST: "
        + test
        + "\nResult: "
        + indigo.check(molfile, check_flags, "")
    )
