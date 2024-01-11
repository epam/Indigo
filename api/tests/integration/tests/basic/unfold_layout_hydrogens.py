from __future__ import print_function

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


def test_mol_unfold(mol):
    print("\ntesting molfile:\n%s" % mol)
    molecule = indigo.loadMolecule(mol)
    molecule.unfoldHydrogens()
    print("\nAfter unfold molfile:\n%s" % molecule.molfile())
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

test_mol_unfold(multi_component_mol)

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

test_mol_unfold(single_component_mol)
