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

CASES = {
    # issue 3834
    "issue_3834": "PEPTIDE1{[-Et]}|RNA1{[Rsp]}|RNA2{[hn]}$PEPTIDE1,RNA1,1:R1-1:R1|RNA2,RNA1,1:R1-1:R2$$$V2.0",
    # chiral C with R1 and implicit H
    "chiral_ap_r1": "PEPTIDE1{A}|CHEM1{[C[C@H]([*:1])CO]}$PEPTIDE1,CHEM1,1:R1-1:R1$$$V2.0",
    # both R1 and R2 on the same chiral C
    "chiral_ap_r1r2": "PEPTIDE1{A}|CHEM1{[[*:2]C[C@H]([*:1])CO]}$PEPTIDE1,CHEM1,1:R1-1:R1$$$V2.0",
    # two attachment to one chiral center
    "chiral_ap_both": (
        "PEPTIDE1{A}|PEPTIDE2{G}|CHEM1{[[*:2]C[C@H]([*:1])CO]}"
        "$PEPTIDE1,CHEM1,1:R1-1:R1|PEPTIDE2,CHEM1,1:R1-1:R2$$$V2.0"
    ),
    # chiral C in a ring with an attachment point
    "chiral_ring_ap": "PEPTIDE1{A}|CHEM1{[O[C@H]1CC[C@@H]([*:1])CC1]}$PEPTIDE1,CHEM1,1:R1-1:R1$$$V2.0",
    # amino acid with R1 directly on the alpha carbon
    "aa_alpha_ap": "PEPTIDE1{A}|CHEM1{[[*:1][C@@H](C)C(O)=O]}$PEPTIDE1,CHEM1,1:R1-1:R1$$$V2.0",
}


def main():
    print("*** Issue 3834 templates with stereocenter at attachment point ***")

    indigo = Indigo()
    indigo.setOption("json-saving-pretty", True)
    indigo.setOption("ignore-stereochemistry-errors", True)

    ref_path = joinPathPy("ref/", __file__)
    lib_path = os.path.join(ref_path, "monomer_library.ket")
    lib = indigo.loadMonomerLibraryFromFile(lib_path)

    for name in sorted(CASES):
        try:
            expanded = indigo.loadHelm(
                CASES[name], lib
            ).expandedMonomersToAtoms()
            centers = []
            for stereo_atom in expanded.iterateStereocenters():
                idx = stereo_atom.index()
                atom = expanded.getAtom(idx)
                centers.append(
                    "%d%s(deg=%d) pyramid=%s"
                    % (
                        idx,
                        atom.symbol(),
                        atom.degree(),
                        ",".join(
                            [str(i) for i in stereo_atom.stereocenterPyramid()]
                        ),
                    )
                )
            smiles = expanded.canonicalSmiles()
            print(
                "%-16s atoms=%-4d centers=%-2d %s"
                % (name, expanded.countAtoms(), len(centers), smiles)
            )
            print("%-16s   %s" % ("", " ".join(centers)))
        except IndigoException as exc:
            text = getIndigoExceptionText(exc)
            print("%-16s EXCEPTION: %s" % (name, text))
            continue


if __name__ == "__main__":
    main()
