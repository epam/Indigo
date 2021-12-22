import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-noncritical-query-features", "true")

tests = [
    {"name": "issue269test_RGroupsWarning", "test": "rgroups"},
    {"name": "issue269test_ChiralityWarning", "test": "chiral"},
    {"name": "issue269test_StereochemistryWarning", "test": "stereo"},
    {"name": "issue269test_PseudoatomWarning", "test": "pseudoatoms"},
    {"name": "issue269test_RadicalWarning", "test": "radicals"},
    {"name": "issue269test_QueryWarning", "test": "query"},
    {"name": "issue269test_All", "test": "all"},
    {
        "name": "issue269test_Issue_293_All",
        "test": "radicals;pseudoatoms;stereo;query;overlap_atoms;overlap_bonds;rgroups;chiral;3d",
    },
]

errors = ""
for test in tests:
    with open(
        joinPathPy("molecules/" + test["name"] + ".mol", __file__), "r"
    ) as file:
        molfile = file.read()
    print(
        "\nTEST: "
        + test["name"]
        + "\nResult: "
        + indigo.check(molfile, test["test"], "")
    )
