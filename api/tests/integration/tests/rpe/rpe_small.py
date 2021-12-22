import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo

indigo = Indigo()

indigo.setOption("rpe-multistep-reactions", "1")
indigo.setOption("rpe-mode", "grid")
indigo.setOption("rpe-self-reaction", "0")
indigo.setOption("rpe-max-depth", "3")
indigo.setOption("rpe-max-products-count", "4")

reaction = indigo.loadQueryReaction(
    "[*]C([#17,#35,#53])=O.O[*]>>[*]C(O[*])=O |$_R1;;;;;_R2;_R1;;;_R2;$|"
)
monomers = [
    [
        indigo.loadMolecule("CC(Cl)=O"),
        indigo.loadMolecule("C1(CCCCC1)C(Cl)=O"),
    ],
    [
        indigo.loadMolecule(
            "[C@@H]1(O)[C@@H](O)[C@H](O)[C@@H](O)[C@H](O)[C@H]1O"
        )
    ],
]

rpe_reactions = indigo.reactionProductEnumerate(reaction, monomers)

products_smiles = []
for rpe_reaction in rpe_reactions.iterateArray():
    rpe_product = rpe_reaction.iterateProducts().next()
    rpe_csmiles = rpe_product.canonicalSmiles()
    products_smiles.append(rpe_csmiles)
    products_smiles.sort()
    for prod_sm in products_smiles:
        print("  %s" % prod_sm)
