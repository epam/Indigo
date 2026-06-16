import os
import re
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** CDXML fragment connection preserves boronic acid valence ***")

reaction = indigo.loadReactionFromFile(
    joinPathPy("molecules/cdxml2/phenylboronic_acid_mat72091.cdxml", __file__)
)

found_boron = False
for mol in reaction.iterateMolecules():
    gross_formula = mol.grossFormula().replace(" ", "")
    if re.search(r"B(?![a-z])", gross_formula) is None:
        continue

    print(gross_formula)
    print("{0:.2f}".format(mol.molecularWeight()))
    found_boron = True
    break

if not found_boron:
    print("BORON_MOLECULE_NOT_FOUND")
