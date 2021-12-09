SALTS = [
    "[NH4+]", "[Li+]", "[Na+]", "[K+]", "[Mg+2]", "[Ca+2]", "[Al+3]",
    "[Fe+2]", "[Fe+3]", "[Co+2]", "[Ni+2]", "[Cu+2]", "[Zn+2]", "[Ag+]",
    "[F-]", "[Cl-]", "[Br-]", "[I-]", "[S-2]", "[C-]#N",
    "N(=O)[O-]", "[N+](=O)([O-])[O-]", "[O-]P(=O)([O-])[O-]",
    "[O-][As](=O)([O-])[O-]", "[O-]S(=O)[O-]", "[O-]S(=O)(=O)[O-]",
    "C(=O)([O-])[O-]", "[O-][Si](=O)[O-]", "[O-][Cr](=O)(=O)[O-] ",
    "[O-][Cr](=O)(=O)O[Cr](=O)(=O)[O-]", "[O-][Mn](=O)(=O)=O", "[O-]Cl",
    "[[O-]Cl=O]", "[O-]Cl(=O)=O", "[O-]Cl(=O)(=O)=O",
    "C(=O)[O-]", "CC(=O)[O-]", "CCC(=O)[O-]", "CCCC(=O)[O-]", "CCCCC(=O)[O-]",
    "CCCCCCCCCCCCCCCC(=O)[O-]", "CCCCCCCCCCCCCCCCCC(=O)[O-]",
    "C(C(=O)[O-])Cl", "C(C(=O)[O-])(Cl)Cl", "C(=O)(C(Cl)(Cl)Cl)[O-]",
    "C=CC(=O)[O-]", "CC(=C)C(=O)[O-]", "CCCCCCCC/C=C\CCCCCCCC(=O)[O-]",
    "CCCCC/C=C\C/C=C\CCCCCCCC(=O)[O-]", "CC/C=C\C/C=C\C/C=C\CCCCCCCC(=O)[O-]",
    "C/C=C/C(=O)[O-]", "C1=CC=C(C=C1)C(=O)[O-]", "C1=CC=C(C=C1)CC(=O)[O-]",
    "C1=CC=C(C=C1)CC(=O)[O-]", "C(=O)(C(=O)[O-])[O-]",
    "C(CCC(=O)[O-])CC(=O)[O-]", "C(CCCC(=O)[O-])CCCC(=O)[O-]",
    "C(C(=O)[O-])C(=O)[O-]", "C1=CC(=CC=C1C(=O)[O-])C(=O)[O-]",
    "CC(C(=O)[O-])O", "C(C(C(=O)[O-])O)(C(=O)[O-])O",
    "C(C(=O)[O-])C(CC(=O)[O-])(C(=O)[O-])O", "C(CC(=O)[O-])C(=O)[O-]",
    "C([C@H]([C@H]([C@@H]([C@H](C(=O)[O-])O)O)O)O)O",
    "C1=CC=C(C(=C1)C(=O)O)[O-]", "CC(=O)OC1=CC=CC=C1C(=O)[O-]"
]

BASIC_METALS = [
    "[Li]", "[Na]", "[K]", "[Mg]", "[Ca]", "[Al]", "[Fe]", "[Co]", "[Ni]",
    "[Cu]", "[Zn]", "[Ag]"
]

INORGANIC_CATIONS = [
    "[NH4+]", "[Li+]", "[Na+]", "[K+]", "[Mg+2]", "[Ca+2]", "[Al+3]",
    "[Fe+2]", "[Fe+3]", "[Co+2]", "[Ni+2]", "[Cu+2]", "[Zn+2]", "[Ag+]"
]

HYDRACIDS_ANIONS = [
    "[F-]", "[Cl-]", "[Br-]", "[I-]", "[S-2]", "[C-]#N"
]

INORGANIC_OXYACIDS_ANIONS = [
    "N(=O)[O-]", "[N+](=O)([O-])[O-]", "[O-]P(=O)([O-])[O-]",
    "[O-][As](=O)([O-])[O-]", "[O-]S(=O)[O-]", "[O-]S(=O)(=O)[O-]",
    "C(=O)([O-])[O-]", "[O-][Si](=O)[O-]", "[O-][Cr](=O)(=O)[O-] ",
    "[O-][Cr](=O)(=O)O[Cr](=O)(=O)[O-]", "[O-][Mn](=O)(=O)=O", "[O-]Cl",
    "[[O-]Cl=O]", "[O-]Cl(=O)=O", "[O-]Cl(=O)(=O)=O"
]

ORGANIC_OXYACIDS_ANIONS = [
    # acyclic monocarboxylic saturated acids and it`s halogen derivatives
    "C(=O)[O-]", "CC(=O)[O-]", "CCC(=O)[O-]", "CCCC(=O)[O-]", "CCCCC(=O)[O-]",
    "CCCCCCCCCCCCCCCC(=O)[O-]", "CCCCCCCCCCCCCCCCCC(=O)[O-]",
    "C(C(=O)[O-])Cl", "C(C(=O)[O-])(Cl)Cl", "C(=O)(C(Cl)(Cl)Cl)[O-]",
    # acyclic monocarboxylic unsaturated acids, cyclic acids
    # and it`s halogen derivatives
    "C=CC(=O)[O-]", "CC(=C)C(=O)[O-]", "CCCCCCCC/C=C\CCCCCCCC(=O)[O-]",
    "CCCCC/C=C\C/C=C\CCCCCCCC(=O)[O-]", "CC/C=C\C/C=C\C/C=C\CCCCCCCC(=O)[O-]",
    "C/C=C/C(=O)[O-]", "C1=CC=C(C=C1)C(=O)[O-]", "C1=CC=C(C=C1)CC(=O)[O-]",
    "C1=CC=C(C=C1)CC(=O)[O-]",
    # polycarboxylic acids and it`s halogen derivatives
    "C(=O)(C(=O)[O-])[O-]", "C(CCC(=O)[O-])CC(=O)[O-]",
    "C(CCCC(=O)[O-])CCCC(=O)[O-]", "C(C(=O)[O-])C(=O)[O-]",
    "C1=CC(=CC=C1C(=O)[O-])C(=O)[O-]",
    # acids with additional functional group
    "CC(C(=O)[O-])O", "C(C(C(=O)[O-])O)(C(=O)[O-])O",
    "C(C(=O)[O-])C(CC(=O)[O-])(C(=O)[O-])O", "C(CC(=O)[O-])C(=O)[O-]",
    "C([C@H]([C@H]([C@@H]([C@H](C(=O)[O-])O)O)O)O)O",
    "C1=CC=C(C(=C1)C(=O)O)[O-]", "CC(=O)OC1=CC=CC=C1C(=O)[O-]"
]
