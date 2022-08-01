import os
import sys

sys.path.append("../../common")
from itertools import product

from env_indigo import *

indigo = Indigo()


def getProduct(reaction):
    for mol in reaction.iterateProducts():
        return mol
    return None


def loadSdf(sdf_path):
    sdfiterator = indigo.iterateSDFile(sdf_path)
    result = [m.clone() for m in sdfiterator]
    sdfiterator.dispose()
    return result


def buildRpeReactions(test_dir):
    reaction = indigo.loadQueryReactionFromFile(
        joinPathPy(os.path.join("tests", test_dir, "reaction.rxn"), __file__)
    )
    mons = []
    for i in range(reaction.countReactants()):
        reactant_mons = loadSdf(
            joinPathPy(
                os.path.join("tests", test_dir, "mons{0}.sdf".format(i + 1)),
                __file__,
            )
        )
        mons.append(reactant_mons)

    return indigo.reactionProductEnumerate(reaction, mons)


def testRpe():
    for test_dir in sorted(os.listdir(joinPathPy("tests", __file__))):
        print("Test %s" % test_dir)
        rpe_reactions = buildRpeReactions(test_dir)
        products_smiles = []
        for reaction in rpe_reactions.iterateArray():
            rpe_product = getProduct(reaction)
            rpe_csmiles = rpe_product.canonicalSmiles()
            products_smiles.append(rpe_csmiles)

        products_smiles.sort()
        for prod_sm in products_smiles:
            print("  %s" % prod_sm)


# make possible options combintation
opset = [
    product(["rpe-multistep-reactions"], ["0", "1"]),  # bug was caused by 1 \
    product(["rpe-mode"], ["grid", "one-tube"]),
    product(["rpe-self-reaction"], ["0", "1"]),
    product(["rpe-max-depth"], ["1", "3"]),
    product(["rpe-max-products-count"], ["4", "10"]),  # 10 -> 100 very long \
]
# example with bug for test #9
# opset = [ [ ("rpe-multistep-reactions", "1") ] ]
opt_combintations = product(*opset)
print("Testing reaction products enumberator with different options")
for opt_set in opt_combintations:
    print("\n*** Test set ***")
    for opt_tuple in opt_set:
        print(opt_tuple)
        indigo.setOption(*opt_tuple)
    testRpe()
