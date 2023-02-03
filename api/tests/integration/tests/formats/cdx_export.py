import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

indigo = Indigo()

print("*** Mol to CDX ***")

root = joinPathPy("molecules/", __file__)
files = [
    "stereo_either-0020",
    "enhanced_stereo1",
    "enhanced_stereo2",
    "enhanced_stereo3",
]

ref_path = joinPathPy("ref/", __file__)
files.sort()

for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        resb64 = mol.b64cdx()
        # with open(os.path.join(ref_path, filename + ".cdxml"), 'w') as file:
        #    data = file.write(mol.cdxml())
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        resb64 = mol.b64cdx()
        # with open(os.path.join(ref_path, filename + ".cdxml"), 'w') as file:
        #   data = file.write(mol.cdxml())
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))

root_cdxml = joinPathPy("cdxml/", __file__)
# files = os.listdir(root_cdxml)
files = [
    "AlcoholOxidation_Rxn1",
    "AlcoholOxidation_Rxn1",
    "AlcoholOxidation_Rxn2",
    "Amidation_Rxn1",
    "Amidation_Rxn2",
    "BuchwaldHartwig_Rxn1",
    "BuchwaldHartwig_Rxn2",
    "CarbonylReduction_Rxn1",
    "CarbonylReduction_Rxn2",
    "Esterification_Rxn1",
    "Esterification_Rxn2",
    "Grignard_Rxn1",
    "Grignard_Rxn2",
    "N-Alkylation_Rxn1",
    "N-Alkylation_Rxn2",
    "N-Sulfonylation_Rxn1",
    "N-Sulfonylation_Rxn2",
    "O-Alkylation_Rxn1",
    "O-Alkylation_Rxn2",
    "Reductive Amination_Rxn1",
    "Reductive Amination_Rxn2",
    "SnAr_Rxn1",
    "SnAr_Rxn2",
    "Suzuki_Rxn1",
    "Suzuki_Rxn2",
]

files.sort()

for filename in files:
    try:
        rea = indigo.loadReactionFromFile(
            os.path.join(root_cdxml, filename + ".cdxml")
        )
        resb64 = rea.b64cdx()
        res = rea.cdxml()
        # with open(os.path.join(ref_path, filename + ".cdxml"), 'w') as file:
        #    data = file.write(res)
        # with open(os.path.join(ref_path, filename + ".b64cdx"), 'w') as file:
        #    data = file.write(resb64)
        # with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
        #    refb64 = file.read()
        # print(filename + (":success" if refb64 == resb64 else ":failed"))

        with open(os.path.join(ref_path, filename + ".cdxml"), "r") as file:
            refcdxml = file.read()
        diff = find_diff(res, refcdxml)
        if not diff:
            print(filename + ".cdxml:SUCCEED")
        else:
            print(filename + ".cdxml:FAILED")
            print(diff)

    except IndigoException as e:
        print(getIndigoExceptionText(e))
