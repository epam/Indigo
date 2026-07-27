import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

EXPECTED_HELM = "PEPTIDE1{C.E.F.A.D}$PEPTIDE1,PEPTIDE1,5:R2-1:R1$$$V2.0"
# Expected HELM string for the molblock string above

max_iterations = 200
success_count = 0

root = joinPathPy("molecules/", __file__)
lib = os.path.join(root, "3297-library.ket")
monomer_library = indigo.loadMonomerLibraryFromFile(lib)
monomer_template = indigo.loadMoleculeFromFile(lib)


def main():
    # Call the conversion from molblock to HELM multiple times.
    for i in range(1, max_iterations + 1):
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, "3297-molecule.mol")
        )
        mol.transformCTABtoSCSR(monomer_template)
        helm_string = mol.helm(monomer_library)
        if helm_string != EXPECTED_HELM:
            print(
                "FAILURE on iteration %s after %s successes"
                % (i, success_count)
            )
            print("Expected: %s" % EXPECTED_HELM)
            print("Got:      %s" % helm_string)
            return

        if i % 100 == 0:
            print("Iteration %s: OK" % i)

    print("Completed %s iterations without failure." % max_iterations)


if __name__ == "__main__":
    main()
