from __future__ import print_function

import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")


def test_mol_unfold(mol, expected_unfolded):
    print("\ntesting molfile:\n%s" % mol)
    molecule = indigo.loadMolecule(mol)
    molecule.unfoldHydrogens()
    unfolded = molecule.molfile()
    unfolded_list = sorted(unfolded.split("\n"))
    expected_unfolded_list = sorted(expected_unfolded.split("\n"))
    diff = "\n".join(
        difflib.context_diff(unfolded_list, expected_unfolded_list)
    )
    if diff:
        print("\nDiff between expected and after unfold molfile:\n%s" % diff)
    else:
        print("Unfolded mol equal to expected")
    molecule.foldHydrogens()
    print("\nAfter fold molfile:\n%s" % molecule.molfile())


multi_component_mol = """
  -INDIGO-01102422162D

  7  7  0  0  0  0  0  0  0  0999 V2000
    1.9394    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9406    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.4401    6.0909    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0972    2.5109    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0935    3.5060    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8603    0.9988    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0828    3.5060    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  1  1  0  0  0  0
  4  5  1  0  0  0  0
  5  7  1  0  0  0  0
  7  6  1  0  0  0  0
  6  4  1  0  0  0  0
M  END

"""

multi_component_mol_unfolded = """
  -INDIGO-01000000002D

 21 21  0  0  0  0  0  0  0  0999 V2000
    1.9394    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9406    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.4401    6.0909    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0972    2.5109    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0935    3.5060    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8603    0.9988    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0828    3.5060    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    5.5688    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.7654    4.2413    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.8801    5.5686    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.1146    4.2413    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.6743    6.7340    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.2060    6.7339    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    9.3856    1.5534    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   10.0862    2.6585    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    9.0923    4.5060    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   10.0935    3.5085    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    5.9099    0.0000    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    4.9168    1.3303    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    7.1974    3.9709    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    8.3222    4.4769    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  1  1  0  0  0  0
  4  5  1  0  0  0  0
  5  7  1  0  0  0  0
  7  6  1  0  0  0  0
  6  4  1  0  0  0  0
  1  8  1  0  0  0  0
  1  9  1  0  0  0  0
  2 10  1  0  0  0  0
  2 11  1  0  0  0  0
  3 12  1  0  0  0  0
  3 13  1  0  0  0  0
  4 14  1  0  0  0  0
  4 15  1  0  0  0  0
  5 16  1  0  0  0  0
  5 17  1  0  0  0  0
  6 18  1  0  0  0  0
  6 19  1  0  0  0  0
  7 20  1  0  0  0  0
  7 21  1  0  0  0  0
M  END
"""
test_mol_unfold(multi_component_mol, multi_component_mol_unfolded)

single_component_mol = """
  -INDIGO-01102422162D

  3  3  0  0  0  0  0  0  0  0999 V2000
    1.9394    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9406    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.4401    6.0909    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  1  1  0  0  0  0
M  END

"""

single_component_mol_unfolded = """
  -INDIGO-01000000002D

  9  9  0  0  0  0  0  0  0  0999 V2000
    1.9394    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9406    5.2260    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.4401    6.0909    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    5.5688    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.7654    4.2413    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.8801    5.5686    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.1146    4.2413    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.6743    6.7340    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.2060    6.7339    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  1  1  0  0  0  0
  1  4  1  0  0  0  0
  1  5  1  0  0  0  0
  2  6  1  0  0  0  0
  2  7  1  0  0  0  0
  3  8  1  0  0  0  0
  3  9  1  0  0  0  0
M  END
"""

test_mol_unfold(single_component_mol, single_component_mol_unfolded)

issue_1575_mol = """
  -INDIGO-01000000002D

  3  2  0  0  0  0  0  0  0  0999 V2000
   26.1692  -10.4274    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   27.0570  -10.9400    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   27.9449  -11.0681    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
M  END
"""

issue_1575_mol_unfolded = """
  -INDIGO-01000000002D

  5  4  0  0  0  0  0  0  0  0999 V2000
   26.1692  -10.4274    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   26.7297  -11.8849    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   27.0570  -10.9400    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   27.3843   -9.9951    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   27.9449  -11.0681    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  2  4  1  0  0  0  0
  2  5  1  0  0  0  0
M  END
"""

test_mol_unfold(issue_1575_mol, issue_1575_mol_unfolded)
